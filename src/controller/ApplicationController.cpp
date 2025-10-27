#include "ApplicationController.h"

#include "ICommand.h"

// UI includes
#include "../ui/mainwindow.h"
#include "../ui/widgets/canvasWidget.h"

// Service includes (when implemented)
// #include "../image_processing/ImageProcessingService.h"
// #include "../raw_processing/RawProcessingService.h"
// #include "../document/DocumentManager.h"

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent), m_mainWindow(nullptr), m_canvas(nullptr) {
    qDebug() << "ApplicationController created";
}

ApplicationController::~ApplicationController() {
    qDebug() << "ApplicationController destroyed";
}

void ApplicationController::initialize() {
    qDebug() << "Initializing ApplicationController";

    setupCommands();
    initializeServices();

    // Initialize state
    m_applicationState["currentFile"] = QString();
    m_applicationState["isModified"] = false;
    m_applicationState["zoomLevel"] = 100;
}

void ApplicationController::setMainWindow(MainWindow* mainWindow) {
    if (m_mainWindow) {
        // Disconnect previous connections
        // TODO: Disconnect signals
    }

    m_mainWindow = mainWindow;

    if (m_mainWindow) {
        connectUISignals();
    }
}

void ApplicationController::executeCommand(const QString& commandName,
                                           const QVariantMap& parameters) {
    auto it = m_commands.find(commandName);
    if (it != m_commands.end()) {
        qDebug() << "Executing command:" << commandName;
        bool success = it->second->execute(parameters);
        if (!success) {
            emit errorOccurred(QString("Failed to execute command: %1").arg(commandName));
        }
    } else {
        qDebug() << "Unknown command:" << commandName;
        emit errorOccurred(QString("Unknown command: %1").arg(commandName));
    }
}

void ApplicationController::registerCommand(const QString& name,
                                            std::unique_ptr<ICommand> command) {
    m_commands[name] = std::move(command);
    qDebug() << "Registered command:" << name;
}

void ApplicationController::setState(const QString& key, const QVariant& value) {
    QVariant oldValue = m_applicationState.value(key);
    if (oldValue != value) {
        m_applicationState[key] = value;
        emit stateChanged(key, value);
        qDebug() << "State changed:" << key << "=" << value;
    }
}

QVariant ApplicationController::getState(const QString& key) const {
    return m_applicationState.value(key);
}

// File operations
void ApplicationController::openFile() {
    QString fileName = QFileDialog::getOpenFileName(
        m_mainWindow, tr("Open Image"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        tr("Image Files (*.png *.jpg *.jpeg *.bmp *.tiff *.raw *.cr2 *.nef *.arw)"));

    if (!fileName.isEmpty()) {
        executeCommand("OpenFile", {{"filePath", fileName}});
    }
}

void ApplicationController::saveFile() {
    QString currentFile = getState("currentFile").toString();
    if (currentFile.isEmpty()) {
        saveAsFile();
    } else {
        executeCommand("SaveFile", {{"filePath", currentFile}});
    }
}

void ApplicationController::saveAsFile() {
    QString fileName = QFileDialog::getSaveFileName(
        m_mainWindow, tr("Save Image"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        tr("Image Files (*.png *.jpg *.jpeg *.bmp *.tiff)"));

    if (!fileName.isEmpty()) {
        executeCommand("SaveAsFile", {{"filePath", fileName}});
    }
}

void ApplicationController::closeFile() {
    executeCommand("CloseFile");
}

void ApplicationController::exitApplication() {
    // Check for unsaved changes
    if (getState("isModified").toBool()) {
        int ret = QMessageBox::warning(
            m_mainWindow, tr("Unsaved Changes"),
            tr("The document has been modified. Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

        if (ret == QMessageBox::Save) {
            saveFile();
        } else if (ret == QMessageBox::Cancel) {
            return;
        }
    }

    QApplication::quit();
}

// Edit operations
void ApplicationController::undo() {
    executeCommand("Undo");
}

void ApplicationController::redo() {
    executeCommand("Redo");
}

void ApplicationController::copy() {
    executeCommand("Copy");
}

void ApplicationController::paste() {
    executeCommand("Paste");
}

void ApplicationController::cut() {
    executeCommand("Cut");
}

// Image operations
void ApplicationController::applyFilter(const QString& filterName) {
    executeCommand("ApplyFilter", {{"filterName", filterName}});
}

void ApplicationController::adjustBrightness(int value) {
    executeCommand("AdjustBrightness", {{"value", value}});
}

void ApplicationController::adjustContrast(int value) {
    executeCommand("AdjustContrast", {{"value", value}});
}

void ApplicationController::rotateImage(int degrees) {
    executeCommand("RotateImage", {{"degrees", degrees}});
}

void ApplicationController::cropImage(const QRect& cropArea) {
    executeCommand("CropImage", {{"cropArea", cropArea}});
}

// View operations
void ApplicationController::zoomIn() {
    int currentZoom = getState("zoomLevel").toInt();
    int newZoom = qMin(currentZoom + 25, 500);  // Max 500%
    setState("zoomLevel", newZoom);

    if (m_canvas) {
        // m_canvas->setZoom(newZoom / 100.0);
    }
}

void ApplicationController::zoomOut() {
    int currentZoom = getState("zoomLevel").toInt();
    int newZoom = qMax(currentZoom - 25, 25);  // Min 25%
    setState("zoomLevel", newZoom);

    if (m_canvas) {
        // m_canvas->setZoom(newZoom / 100.0);
    }
}

void ApplicationController::resetZoom() {
    setState("zoomLevel", 100);

    if (m_canvas) {
        // m_canvas->setZoom(1.0);
    }
}

void ApplicationController::fitToWindow() {
    if (m_canvas) {
        // m_canvas->fitToWindow();
    }
}

// Private methods
void ApplicationController::setupCommands() {
    // TODO: Register all commands
    // registerCommand("OpenFile", std::make_unique<OpenFileCommand>(this));
    // registerCommand("SaveFile", std::make_unique<SaveFileCommand>(this));
    // etc.
}

void ApplicationController::connectUISignals() {
    if (!m_mainWindow)
        return;

    // TODO: Connect UI signals to controller slots
    // This will be done when integrating with UI components
}

void ApplicationController::initializeServices() {
    // TODO: Initialize business logic services
    // m_imageProcessingService = std::make_unique<ImageProcessingService>();
    // m_rawProcessingService = std::make_unique<RawProcessingService>();
    // m_documentManager = std::make_unique<DocumentManager>();
}

void ApplicationController::onImageProcessingComplete() {
    setState("isModified", true);
    emit imageProcessed();
}

void ApplicationController::onFileOperationComplete() {
    // Handle file operation completion
}

// Service access methods (to be implemented when services exist)
/*
ImageProcessingService* ApplicationController::getImageProcessingService() const {
    return m_imageProcessingService.get();
}

RawProcessingService* ApplicationController::getRawProcessingService() const {
    return m_rawProcessingService.get();
}

DocumentManager* ApplicationController::getDocumentManager() const {
    return m_documentManager.get();
}
*/