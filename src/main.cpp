#include <QApplication>
#include <QDebug>
#include <QFileInfo>

#include "controller/ApplicationController.h"
#include "model/DocumentManager.h"
#include "model/ImageDocument.h"
#include "ui/bar/subMenuFile.h"
#include "ui/canvas/canvasWidget.h"
#include "ui/mainwindow.h"
#include "ui/panel/infoPanel.h"
#include "ui/panel/toolPanel.h"

/**
 * @brief Connect UI signals to Controller slots
 *
 * This function wires up the UI components to the application controller,
 * establishing the communication between the View and Controller layers.
 */
void connectUIToController(MainWindow& mainWindow, ApplicationController& controller) {
    // Get UI components
    ToolPanel* toolPanel = mainWindow.getToolPanel();
    CanvasWidget* canvas = mainWindow.getCanvasWidget();
    SubMenuFile* fileMenu = mainWindow.getFileMenu();
    InfoPanel* infoPanel = mainWindow.getInfoPanel();
    DocumentManager* docManager = controller.getDocumentManager();

    // === Connect ToolPanel slider signals to Controller adjustment slots ===
    if (toolPanel) {
        QObject::connect(toolPanel, &ToolPanel::brightnessChanged, &controller,
                         &ApplicationController::adjustBrightness);
        QObject::connect(toolPanel, &ToolPanel::contrastChanged, &controller,
                         &ApplicationController::adjustContrast);
        QObject::connect(toolPanel, &ToolPanel::exposureChanged, &controller,
                         &ApplicationController::adjustExposure);
        QObject::connect(toolPanel, &ToolPanel::highlightsChanged, &controller,
                         &ApplicationController::adjustHighlights);
        QObject::connect(toolPanel, &ToolPanel::shadowsChanged, &controller,
                         &ApplicationController::adjustShadows);
        QObject::connect(toolPanel, &ToolPanel::whitesChanged, &controller,
                         &ApplicationController::adjustWhites);
        QObject::connect(toolPanel, &ToolPanel::blacksChanged, &controller,
                         &ApplicationController::adjustBlacks);
        QObject::connect(toolPanel, &ToolPanel::temperatureChanged, &controller,
                         &ApplicationController::adjustTemperature);
        QObject::connect(toolPanel, &ToolPanel::tintChanged, &controller,
                         &ApplicationController::adjustTint);
        QObject::connect(toolPanel, &ToolPanel::saturationChanged, &controller,
                         &ApplicationController::adjustSaturation);
        qDebug() << "ToolPanel signals connected to controller";
    }

    // === Connect File menu signals ===
    // Disconnect existing connections from SubMenuFile to MainWindow (if any)
    if (fileMenu) {
        QObject::disconnect(fileMenu, &SubMenuFile::imageLoaded, nullptr, nullptr);
        QObject::disconnect(fileMenu, &SubMenuFile::imageSaveRequested, nullptr, nullptr);
        qDebug() << "File menu default connections disconnected";
    }

    // === Connect DocumentManager signals to update Canvas ===
    if (docManager && canvas) {
        // When document is opened, display the image on canvas
        QObject::connect(docManager, &DocumentManager::documentOpened,
                         [canvas, docManager, infoPanel, &mainWindow](const QString& filePath) {
                             if (docManager->hasDocument()) {
                                 QImage processedImage =
                                     docManager->currentDocument()->processedImage();
                                 canvas->setImage(processedImage);
                                 canvas->zoomToFit();

                                 // Update info panel with image metadata
                                 if (infoPanel) {
                                     QFileInfo fileInfo(filePath);
                                     infoPanel->updateImageInfo(
                                         fileInfo.fileName(), processedImage.width(),
                                         processedImage.height(), fileInfo.suffix().toUpper());
                                 }

                                 // Update window title
                                 QFileInfo fileInfo(filePath);
                                 mainWindow.setWindowTitle(
                                     QString("Photo Manufactura - %1").arg(fileInfo.fileName()));

                                 qDebug() << "Document opened and canvas updated:" << filePath;
                             }
                         });

        // When processed image changes (after adjustments), update canvas
        if (docManager->currentDocument()) {
            QObject::connect(docManager->currentDocument(), &ImageDocument::processedImageChanged,
                             [canvas](const QImage& image) { canvas->setImage(image); });
        }

        // Also connect for future documents
        QObject::connect(
            docManager, &DocumentManager::documentOpened, [docManager, canvas](const QString&) {
                // Reconnect processed image signal for new document
                if (docManager->currentDocument()) {
                    QObject::connect(docManager->currentDocument(),
                                     &ImageDocument::processedImageChanged,
                                     [canvas](const QImage& image) { canvas->setImage(image); });
                }
            });

        qDebug() << "DocumentManager signals connected to canvas";
    }

    qDebug() << "All UI-Controller connections established";
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    qDebug() << "Starting Photo Manufactura Application";

    // Initialize Application Controller
    ApplicationController controller;
    controller.initialize();

    // Create and set up Main Window
    MainWindow mainWindow;
    controller.setMainWindow(&mainWindow);

    // Connect UI signals to Controller (Step 1 & 2 integration)
    connectUIToController(mainWindow, controller);

    mainWindow.show();

    return app.exec();
}