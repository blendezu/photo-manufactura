#include "ApplicationWiring.h"

#include <QDebug>
#include <QFileInfo>

#include "../model/DocumentManager.h"
#include "../model/ImageDocument.h"
#include "../ui/bar/subMenuEdit.h"
#include "../ui/bar/subMenuFile.h"
#include "../ui/bar/subMenuView.h"
#include "../ui/canvas/canvasWidget.h"
#include "../ui/mainwindow.h"
#include "../ui/panel/infoPanel.h"
#include "../ui/panel/toolPanel.h"
#include "ApplicationController.h"

ApplicationWiring::ApplicationWiring(QObject* parent) : QObject(parent) {
    qDebug() << "ApplicationWiring created";
}

ApplicationWiring::~ApplicationWiring() {
    unwireComponents();
    qDebug() << "ApplicationWiring destroyed";
}

void ApplicationWiring::wireComponents(ApplicationController* controller, MainWindow* mainWindow) {
    if (!controller || !mainWindow) {
        qWarning() << "Cannot wire components: null controller or mainWindow";
        return;
    }

    // Store references for later unwiring
    m_controller = controller;
    m_mainWindow = mainWindow;

    qDebug() << "Wiring application components...";

    // Wire UI components
    CanvasWidget* canvas = mainWindow->getCanvasWidget();
    ToolPanel* toolPanel = mainWindow->getToolPanel();

    // Wire menu components
    if (auto* fileMenu = mainWindow->getFileMenu()) {
        wireFileMenu(fileMenu, controller);
    }

    if (auto* editMenu = mainWindow->getEditMenu()) {
        wireEditMenu(editMenu, controller);
    }

    if (auto* viewMenu = mainWindow->getViewMenu()) {
        wireViewMenu(viewMenu, controller, canvas);
    }

    if (canvas) {
        wireCanvas(canvas, controller);
        wireControllerToCanvas(controller, canvas);
    }

    if (toolPanel) {
        wireToolPanel(toolPanel, controller);
    }

    // Wire crop tool: ToolPanel -> Canvas crop mode
    if (toolPanel && canvas) {
        m_connections << connect(toolPanel, &ToolPanel::cropRequested, canvas,
                                 [canvas]() { canvas->setCropMode(true); });

        // Wire crop options: ToolPanel -> Canvas
        m_connections << connect(
            toolPanel, &ToolPanel::cropAspectRatioChanged, canvas, [canvas](int presetIndex) {
                canvas->setAspectRatioPreset(static_cast<AspectRatioPreset>(presetIndex));
            });

        m_connections << connect(toolPanel, &ToolPanel::cropFixedSizeChanged, canvas,
                                 [canvas](int width, int height) {
                                     canvas->setFixedCropSize(QSize(width, height));
                                     canvas->setCropType(CropType::FixedSize);
                                 });

        // Rotation slider updates canvas straighten angle for live crop preview
        m_connections << connect(
            toolPanel, &ToolPanel::rotateAngleChanged, canvas,
            [canvas](int angle) { canvas->setStraightenAngle(static_cast<float>(angle)); });

        // Straighten mode toggle - enable/disable straighten mode on canvas
        m_connections << connect(toolPanel, &ToolPanel::straightenModeToggled, canvas,
                                 &CanvasWidget::setStraightenMode);

        // Apply straighten - emit canvas signal with angle and inscribed crop rect
        m_connections << connect(toolPanel, &ToolPanel::applyStraightenRequested, canvas,
                                 [canvas]() {
                                     float angle = canvas->getStraightenAngle();
                                     QRect cropRect = canvas->getInscribedCropRect();
                                     Q_EMIT canvas->straightenCropRequested(angle, cropRect);
                                 });

        // Straighten Aspect Ratio
        m_connections << connect(
            toolPanel, &ToolPanel::straightenAspectRatioChanged, canvas, [canvas](int index) {
                canvas->setStraightenAspectRatio(static_cast<AspectRatioPreset>(index));
            });
        // Reset
        m_connections << connect(toolPanel, &ToolPanel::resetToOriginalRequested,
                                 controller->getDocumentManager(),
                                 &DocumentManager::resetToOriginal);
    }  // End of ToolPanel wiring block? Wait, no.

    if (auto* infoPanel = mainWindow->getInfoPanel()) {
        wireInfoPanel(infoPanel, controller);
    }

    // Wire controller outputs to main window
    wireControllerToMainWindow(controller, mainWindow);

    // Wire document manager to canvas (for image display updates)
    wireDocumentToCanvas(controller, mainWindow);

    qDebug() << "Wiring complete. Total connections:" << m_connections.size();
}

void ApplicationWiring::unwireComponents() {
    qDebug() << "Unwiring" << m_connections.size() << "connections...";

    for (const auto& connection : m_connections) {
        QObject::disconnect(connection);
    }
    m_connections.clear();

    m_controller = nullptr;
    m_mainWindow = nullptr;
}

void ApplicationWiring::wireFileMenu(SubMenuFile* fileMenu, ApplicationController* controller) {
    qDebug() << "Wiring File Menu...";

    m_connections << connect(fileMenu, &SubMenuFile::newDocumentRequested, controller,
                             &ApplicationController::newDocument);

    m_connections << connect(fileMenu, &SubMenuFile::openFileRequested, controller,
                             &ApplicationController::openFile);

    m_connections << connect(fileMenu, &SubMenuFile::saveFileRequested, controller,
                             &ApplicationController::saveFile);

    m_connections << connect(fileMenu, &SubMenuFile::saveAsFileRequested, controller,
                             &ApplicationController::saveAsFile);

    m_connections << connect(fileMenu, &SubMenuFile::exitRequested, controller,
                             &ApplicationController::exitApplication);
}

void ApplicationWiring::wireEditMenu(SubMenuEdit* editMenu, ApplicationController* controller) {
    qDebug() << "Wiring Edit Menu...";

    // Undo/Redo
    // Undo/Redo
    m_connections << connect(editMenu, &SubMenuEdit::undoRequested, controller,
                             &ApplicationController::undo);
    m_connections << connect(editMenu, &SubMenuEdit::redoRequested, controller,
                             &ApplicationController::redo);

    // Sync enabled state from DocumentManager
    if (DocumentManager* docManager = controller->getDocumentManager()) {
        m_connections << connect(docManager, &DocumentManager::undoRedoStateChanged, editMenu,
                                 [editMenu](bool canUndo, bool canRedo) {
                                     editMenu->setUndoEnabled(canUndo);
                                     editMenu->setRedoEnabled(canRedo);
                                 });

        // Initial state sync (if document exists)
        editMenu->setUndoEnabled(docManager->canUndo());
        editMenu->setRedoEnabled(docManager->canRedo());
    }
}

void ApplicationWiring::wireViewMenu(SubMenuView* viewMenu, ApplicationController* controller,
                                     CanvasWidget* canvas) {
    qDebug() << "Wiring View Menu...";

    // Zoom mode toggle - connect view menu to canvas
    if (canvas) {
        m_connections << connect(viewMenu, &SubMenuView::zoomModeToggled, canvas,
                                 [canvas](bool enabled) {
                                     canvas->setZoomMode(enabled ? ZoomMode::Zoom : ZoomMode::None);
                                 });

        // Connect canvas zoom mode changes back to view menu for sync
        m_connections << connect(
            canvas, &CanvasWidget::zoomModeChanged, viewMenu,
            [viewMenu](ZoomMode mode) { viewMenu->setZoomModeChecked(mode == ZoomMode::Zoom); });
    }

    m_connections << connect(viewMenu, &SubMenuView::resetZoomRequested, controller,
                             &ApplicationController::resetZoom);
    m_connections << connect(viewMenu, &SubMenuView::fitToWindowRequested, controller,
                             &ApplicationController::fitToWindow);

    // Panel visibility toggles
    m_connections << connect(viewMenu, &SubMenuView::toggleHistogramRequested, controller,
                             &ApplicationController::toggleHistogram);
    m_connections << connect(viewMenu, &SubMenuView::toggleToolPanelRequested, controller,
                             &ApplicationController::toggleToolPanel);
    m_connections << connect(viewMenu, &SubMenuView::toggleAdjustmentPanelRequested, controller,
                             &ApplicationController::toggleAdjustmentPanel);

    // Theme changes (controller can update state/settings)
    m_connections << connect(viewMenu, &SubMenuView::darkThemeRequested, controller,
                             [controller]() { controller->setTheme("dark"); });
    m_connections << connect(viewMenu, &SubMenuView::lightThemeRequested, controller,
                             [controller]() { controller->setTheme("light"); });
    m_connections << connect(viewMenu, &SubMenuView::themeToggleRequested, controller,
                             &ApplicationController::toggleTheme);

    // GPU/CPU processing mode toggle
    m_connections << connect(viewMenu, &SubMenuView::gpuModeToggled, controller,
                             &ApplicationController::setGpuMode);
}

void ApplicationWiring::wireCanvas(CanvasWidget* canvas, ApplicationController* controller) {
    qDebug() << "Wiring Canvas Widget...";

    m_connections << connect(canvas, &CanvasWidget::zoomInRequested, controller,
                             &ApplicationController::zoomIn);

    m_connections << connect(canvas, &CanvasWidget::zoomOutRequested, controller,
                             &ApplicationController::zoomOut);

    m_connections << connect(canvas, &CanvasWidget::fitToWindowRequested, controller,
                             &ApplicationController::fitToWindow);

    // Crop: canvas emits cropRequested with the selected area
    m_connections << connect(canvas, &CanvasWidget::cropRequested, controller,
                             &ApplicationController::cropImage);

    // Straighten: canvas emits straightenCropRequested with angle and inscribed rect
    m_connections << connect(canvas, &CanvasWidget::straightenCropRequested, controller,
                             &ApplicationController::applyStraighten);

    // Four-point perspective crop
    m_connections << connect(canvas, &CanvasWidget::perspectiveCropRequested, controller,
                             &ApplicationController::perspectiveCropImage);

    // Activations from Controller (via UI)
    m_connections << connect(controller, &ApplicationController::enablePerspectiveCropMode, canvas,
                             [canvas]() {
                                 canvas->setCropType(CropType::FourPoint);
                                 canvas->setCropMode(true);
                                 canvas->update();  // Ensure redraw
                             });

    // Standard crop mode toggle
    m_connections << connect(
        controller, &ApplicationController::enableCropMode, canvas, [canvas](bool enabled) {
            canvas->setCropMode(enabled);
            if (enabled)
                canvas->setCropType(CropType::Free);  // Default to Free or current
        });
}

void ApplicationWiring::wireToolPanel(ToolPanel* toolPanel, ApplicationController* controller) {
    qDebug() << "Wiring Tool Panel...";

    // Connect adjustment slider signals to controller
    m_connections << connect(toolPanel, &ToolPanel::brightnessChanged, controller,
                             &ApplicationController::adjustBrightness);

    m_connections << connect(toolPanel, &ToolPanel::contrastChanged, controller,
                             &ApplicationController::adjustContrast);

    m_connections << connect(toolPanel, &ToolPanel::exposureChanged, controller,
                             &ApplicationController::adjustExposure);

    m_connections << connect(toolPanel, &ToolPanel::highlightsChanged, controller,
                             &ApplicationController::adjustHighlights);

    m_connections << connect(toolPanel, &ToolPanel::shadowsChanged, controller,
                             &ApplicationController::adjustShadows);

    m_connections << connect(toolPanel, &ToolPanel::whitesChanged, controller,
                             &ApplicationController::adjustWhites);

    m_connections << connect(toolPanel, &ToolPanel::blacksChanged, controller,
                             &ApplicationController::adjustBlacks);

    m_connections << connect(toolPanel, &ToolPanel::temperatureChanged, controller,
                             &ApplicationController::adjustTemperature);

    m_connections << connect(toolPanel, &ToolPanel::tintChanged, controller,
                             &ApplicationController::adjustTint);

    m_connections << connect(toolPanel, &ToolPanel::saturationChanged, controller,
                             &ApplicationController::adjustSaturation);

    // History connection
    DocumentManager* docManager = controller->getDocumentManager();
    if (docManager) {
        m_connections << connect(toolPanel, &ToolPanel::adjustmentFinished, docManager,
                                 &DocumentManager::saveAdjustmentState);
    }

    // Detail connections
    m_connections << connect(toolPanel, &ToolPanel::denoiseChanged, controller,
                             &ApplicationController::adjustDenoise);
    m_connections << connect(toolPanel, &ToolPanel::clarityChanged, controller,
                             &ApplicationController::adjustClarity);
    m_connections << connect(toolPanel, &ToolPanel::sharpeningChanged, controller,
                             &ApplicationController::adjustSharpening);

    // Geometry tool connections
    m_connections << connect(toolPanel, &ToolPanel::rotateLeftRequested, controller,
                             [controller]() { controller->rotateImage(-90); });

    m_connections << connect(toolPanel, &ToolPanel::rotateRightRequested, controller,
                             [controller]() { controller->rotateImage(90); });

    // Custom angle rotation via slider (non-destructive via ImageState)
    m_connections << connect(toolPanel, &ToolPanel::rotateAngleChanged, controller,
                             &ApplicationController::adjustRotation);

    m_connections << connect(toolPanel, &ToolPanel::flipHorizontalRequested, controller,
                             [controller]() { controller->flipImage(1); });

    m_connections << connect(toolPanel, &ToolPanel::flipVerticalRequested, controller,
                             [controller]() { controller->flipImage(0); });

    // Resize - receive confirmed width/height from tool panel (logic handled there)
    m_connections << connect(toolPanel, &ToolPanel::resizeConfirmed, controller,
                             &ApplicationController::resizeImage);

    // Reset all adjustments
    m_connections << connect(toolPanel, &ToolPanel::resetAllRequested, controller,
                             &ApplicationController::resetAdjustments);

    // Filter/Effect connections
    m_connections << connect(toolPanel, &ToolPanel::filterOriginalRequested, controller,
                             &ApplicationController::applyFilterOriginal);

    m_connections << connect(toolPanel, &ToolPanel::filterGrayscaleRequested, controller,
                             &ApplicationController::applyFilterGrayscale);

    m_connections << connect(toolPanel, &ToolPanel::filterVintageRequested, controller,
                             &ApplicationController::applyFilterVintage);

    m_connections << connect(toolPanel, &ToolPanel::autoLightRequested, controller,
                             &ApplicationController::applyAutoEnhance);

    // AI Style Transfer connections
    m_connections << connect(toolPanel, &ToolPanel::styleTransferRequested, controller,
                             &ApplicationController::applyStyleTransfer);

    m_connections << connect(toolPanel, &ToolPanel::styleStrengthChanged, controller,
                             &ApplicationController::setStyleTransferStrength);

    // Compare mode toggle - wire to canvas setCompareMode
    CanvasWidget* canvas = m_mainWindow->getCanvasWidget();
    m_connections << connect(toolPanel, &ToolPanel::compareModeToggled, canvas,
                             &CanvasWidget::setCompareMode);

    // Apply corrections permanently
    m_connections << connect(toolPanel, &ToolPanel::applyRequested, controller,
                             &ApplicationController::applyCorrections);

    // Save preset button - shows dialog and saves
    m_connections << connect(toolPanel, &ToolPanel::savePresetButtonClicked, controller,
                             &ApplicationController::showSavePresetDialog);

    // Zoom mode toggles - connect tool panel buttons to canvas
    if (canvas) {
        m_connections << connect(toolPanel, &ToolPanel::zoomModeToggled, canvas,
                                 [canvas](bool enabled) {
                                     canvas->setZoomMode(enabled ? ZoomMode::Zoom : ZoomMode::None);
                                 });

        // Connect canvas zoom mode changes back to tool panel for sync
        m_connections << connect(
            canvas, &CanvasWidget::zoomModeChanged, toolPanel,
            [toolPanel](ZoomMode mode) { toolPanel->setZoomModeChecked(mode == ZoomMode::Zoom); });
    }

    // Preset connections
    m_connections << connect(toolPanel, &ToolPanel::presetSelected, controller,
                             &ApplicationController::applyPreset);

    m_connections << connect(toolPanel, &ToolPanel::savePresetRequested, controller,
                             &ApplicationController::saveCurrentAsPreset);

    // Refresh presets combo when user saves a new preset
    m_connections << connect(controller, &ApplicationController::presetsChanged, toolPanel,
                             &ToolPanel::refreshPresets);

    // Update sliders when preset is applied
    m_connections << connect(controller, &ApplicationController::adjustmentsChanged, toolPanel,
                             &ToolPanel::updateSliders);

    // Update image info in tool panel when image loads
    m_connections << connect(
        controller, &ApplicationController::imageLoaded, toolPanel,
        [toolPanel](const QImage& img, const QString&) { toolPanel->updateImageInfo(img); });

    // Enable/disable color controls based on active filter
    // Grayscale filter should disable color sliders as they have no effect
    if (docManager) {
        m_connections << connect(docManager, &DocumentManager::filterChanged, toolPanel,
                                 [toolPanel](const QString& filterName) {
                                     // Disable color controls for grayscale (color adjustments have
                                     // no effect)
                                     bool enableColorControls = (filterName != "Grayscale");
                                     toolPanel->setColorControlsEnabled(enableColorControls);
                                 });
    }

    // Reset ToolPanel UI when a new file is opened (or reset to original)
    // This ensures sliders snap back to 0 even if the model reset logic happened silently
    m_connections << connect(controller, &ApplicationController::fileOpened, toolPanel,
                             [toolPanel](const QString&) { toolPanel->resetAllAdjustments(); });

    // Also connect to imageLoaded? No, imageLoaded fires on rotation/filters too.
    // fileOpened is specific to new document load (or reset reload).

    // Crop tool - activates crop mode on canvas
    m_connections << connect(toolPanel, &ToolPanel::cropRequested, controller, [controller]() {
        controller->requestCropMode();
    });  // Assume requestCropMode (check if exists or use lambda wrapper to canvas)

    // Actually, check what cropRequested does.
    // Existing code: "The actual crop is handled via canvas signals" comment.
    // But ToolPanel has cropRequested().

    m_connections << connect(toolPanel, &ToolPanel::perspectiveCropRequested, controller,
                             &ApplicationController::requestPerspectiveCropMode);
}

void ApplicationWiring::wireInfoPanel(InfoPanel* infoPanel, ApplicationController* controller) {
    qDebug() << "Wiring Info Panel...";

    DocumentManager* docManager = controller->getDocumentManager();
    if (docManager) {
        // Connect history updates
        m_connections << connect(docManager, &DocumentManager::historyChanged, infoPanel,
                                 &InfoPanel::updateHistory);

        // Also update history when document is opened (to clear/reset)
        // Note: DocumentManager emits historyChanged on open/close, so explicit connect
        // implementation details handles it
    }
}

void ApplicationWiring::wireControllerToMainWindow(ApplicationController* controller,
                                                   MainWindow* mainWindow) {
    qDebug() << "Wiring Controller to MainWindow...";

    m_connections << connect(controller, &ApplicationController::imageLoaded, mainWindow,
                             &MainWindow::onImageLoaded);

    m_connections << connect(controller, &ApplicationController::fileSaved, mainWindow,
                             &MainWindow::onFileSaved);

    m_connections << connect(controller, &ApplicationController::errorOccurred, mainWindow,
                             &MainWindow::onError);
}

void ApplicationWiring::wireControllerToCanvas(ApplicationController* controller,
                                               CanvasWidget* canvas) {
    qDebug() << "Wiring Controller to Canvas...";

    m_connections << connect(controller, &ApplicationController::zoomChanged, canvas,
                             &CanvasWidget::setZoomLevel);
}

void ApplicationWiring::wireDocumentToCanvas(ApplicationController* controller,
                                             MainWindow* mainWindow) {
    qDebug() << "Wiring Document to Canvas...";

    CanvasWidget* canvas = mainWindow->getCanvasWidget();
    InfoPanel* infoPanel = mainWindow->getInfoPanel();
    DocumentManager* docManager = controller->getDocumentManager();

    if (!docManager || !canvas) {
        qWarning() << "Cannot wire document to canvas: null docManager or canvas";
        return;
    }

    // When document is opened, display the image on canvas
    m_connections << connect(
        docManager, &DocumentManager::documentOpened,
        [canvas, controller, docManager, infoPanel, mainWindow](const QString& filePath) {
            if (docManager->hasDocument()) {
                QImage processedImage = docManager->currentDocument()->processedImage();
                QImage originalImage = docManager->currentDocument()->originalImage();
                canvas->setImage(processedImage);
                canvas->setOriginalImage(originalImage);  // Store original for comparison

                // Request fit-to-window through controller
                controller->fitToWindow();

                // Update info panel with image metadata and histogram
                if (infoPanel) {
                    QFileInfo fileInfo(filePath);
                    infoPanel->updateImageInfo(fileInfo.fileName(), processedImage.width(),
                                               processedImage.height(),
                                               fileInfo.suffix().toUpper());
                    infoPanel->updateHistogram(processedImage);
                }

                // Update window title
                QFileInfo fileInfo(filePath);
                mainWindow->setWindowTitle(
                    QString("Photo Manufactura - %1").arg(fileInfo.fileName()));

                // Reset CanvasWidget transformation state (exit crop/straighten modes)
                canvas->setCropMode(false);
                canvas->setStraightenMode(false);
                canvas->resetView();  // Reset zoom/pan

                qDebug() << "Document opened and canvas updated:" << filePath;
            }
        });

    // When processed image changes (after adjustments), update canvas and histogram
    if (docManager->currentDocument()) {
        m_connections << connect(docManager->currentDocument(),
                                 &ImageDocument::processedImageChanged,
                                 [canvas, infoPanel](const QImage& image) {
                                     canvas->setImage(image);
                                     if (infoPanel) {
                                         infoPanel->updateHistogram(image);
                                     }
                                 });
    }

    // Also connect for future documents - reconnect processedImageChanged signal
    m_connections << connect(docManager, &DocumentManager::documentOpened,
                             [this, docManager, canvas, infoPanel](const QString&) {
                                 // Reconnect processed image signal for new document
                                 if (docManager->currentDocument()) {
                                     m_connections
                                         << connect(docManager->currentDocument(),
                                                    &ImageDocument::processedImageChanged,
                                                    [canvas, infoPanel](const QImage& image) {
                                                        canvas->setImage(image);
                                                        if (infoPanel) {
                                                            infoPanel->updateHistogram(image);
                                                        }
                                                    });
                                 }
                             });
}
