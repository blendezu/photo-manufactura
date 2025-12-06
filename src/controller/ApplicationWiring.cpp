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

    // Wire menu components
    if (auto* fileMenu = mainWindow->getFileMenu()) {
        wireFileMenu(fileMenu, controller);
    }

    if (auto* editMenu = mainWindow->getEditMenu()) {
        wireEditMenu(editMenu, controller);
    }

    if (auto* viewMenu = mainWindow->getViewMenu()) {
        wireViewMenu(viewMenu, controller);
    }

    // Wire UI components
    if (auto* canvas = mainWindow->getCanvasWidget()) {
        wireCanvas(canvas, controller);
        wireControllerToCanvas(controller, canvas);
    }

    if (auto* toolPanel = mainWindow->getToolPanel()) {
        wireToolPanel(toolPanel, controller);
    }

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
                             &ApplicationController::closeFile);  // TODO: Add newDocument slot

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

    // TODO: Add these connections when SubMenuEdit signals are implemented
    // m_connections << connect(editMenu, &SubMenuEdit::undoRequested,
    //                          controller, &ApplicationController::undo);
    // m_connections << connect(editMenu, &SubMenuEdit::redoRequested,
    //                          controller, &ApplicationController::redo);
    // m_connections << connect(editMenu, &SubMenuEdit::copyRequested,
    //                          controller, &ApplicationController::copy);
    // m_connections << connect(editMenu, &SubMenuEdit::pasteRequested,
    //                          controller, &ApplicationController::paste);
    // m_connections << connect(editMenu, &SubMenuEdit::cutRequested,
    //                          controller, &ApplicationController::cut);

    Q_UNUSED(editMenu);
    Q_UNUSED(controller);
}

void ApplicationWiring::wireViewMenu(SubMenuView* viewMenu, ApplicationController* controller) {
    qDebug() << "Wiring View Menu...";

    // TODO: Add these connections when SubMenuView signals are implemented
    // m_connections << connect(viewMenu, &SubMenuView::zoomInRequested,
    //                          controller, &ApplicationController::zoomIn);
    // m_connections << connect(viewMenu, &SubMenuView::zoomOutRequested,
    //                          controller, &ApplicationController::zoomOut);
    // m_connections << connect(viewMenu, &SubMenuView::fitToWindowRequested,
    //                          controller, &ApplicationController::fitToWindow);
    // m_connections << connect(viewMenu, &SubMenuView::toggleHistogramRequested,
    //                          controller, &ApplicationController::toggleHistogram);

    Q_UNUSED(viewMenu);
    Q_UNUSED(controller);
}

void ApplicationWiring::wireCanvas(CanvasWidget* canvas, ApplicationController* controller) {
    qDebug() << "Wiring Canvas Widget...";

    m_connections << connect(canvas, &CanvasWidget::zoomInRequested, controller,
                             &ApplicationController::zoomIn);

    m_connections << connect(canvas, &CanvasWidget::zoomOutRequested, controller,
                             &ApplicationController::zoomOut);

    m_connections << connect(canvas, &CanvasWidget::fitToWindowRequested, controller,
                             &ApplicationController::fitToWindow);
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

    // TODO: Add filter selection connections when available
    // m_connections << connect(toolPanel, &ToolPanel::filterSelected,
    //                          controller, &ApplicationController::applyFilter);
}

void ApplicationWiring::wireInfoPanel(InfoPanel* infoPanel, ApplicationController* controller) {
    qDebug() << "Wiring Info Panel...";

    // TODO: Add info panel connections if needed
    // Info panel is typically display-only, but may have interactive elements

    Q_UNUSED(infoPanel);
    Q_UNUSED(controller);
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
                canvas->setImage(processedImage);

                // Request fit-to-window through controller
                controller->fitToWindow();

                // Update info panel with image metadata
                if (infoPanel) {
                    QFileInfo fileInfo(filePath);
                    infoPanel->updateImageInfo(fileInfo.fileName(), processedImage.width(),
                                               processedImage.height(),
                                               fileInfo.suffix().toUpper());
                }

                // Update window title
                QFileInfo fileInfo(filePath);
                mainWindow->setWindowTitle(
                    QString("Photo Manufactura - %1").arg(fileInfo.fileName()));

                qDebug() << "Document opened and canvas updated:" << filePath;
            }
        });

    // When processed image changes (after adjustments), update canvas
    if (docManager->currentDocument()) {
        m_connections << connect(docManager->currentDocument(),
                                 &ImageDocument::processedImageChanged,
                                 [canvas](const QImage& image) { canvas->setImage(image); });
    }

    // Also connect for future documents - reconnect processedImageChanged signal
    m_connections << connect(
        docManager, &DocumentManager::documentOpened, [this, docManager, canvas](const QString&) {
            // Reconnect processed image signal for new document
            if (docManager->currentDocument()) {
                m_connections << connect(
                    docManager->currentDocument(), &ImageDocument::processedImageChanged,
                    [canvas](const QImage& image) { canvas->setImage(image); });
            }
        });
}
