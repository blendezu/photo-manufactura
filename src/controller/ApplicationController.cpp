#include "ApplicationController.h"

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QWidget>

#include "ICommand.h"

ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent),
      m_mainWindow(nullptr),
      m_documentManager(std::make_unique<DocumentManager>(this)),
      m_appState(std::make_unique<AppState>(this)) {
    qDebug() << "ApplicationController created";
    connectModelSignals();
}

ApplicationController::~ApplicationController() {
    qDebug() << "ApplicationController destroyed";
}

void ApplicationController::initialize() {
    qDebug() << "Initializing ApplicationController";

    setupCommands();
    initializeServices();

    // Initialize default state from model
    m_applicationState["currentFile"] = QString();
    m_applicationState["isModified"] = false;
    m_applicationState["zoomLevel"] = static_cast<int>(m_appState->zoomLevel() * 100);
    m_applicationState["theme"] = m_appState->theme();
}

void ApplicationController::setMainWindow(QWidget* mainWindow) {
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
    // Load last used directory
    QSettings settings("PhotoManufactura", "UI");
    QString lastDir = settings
                          .value("lastImageDirectory",
                                 QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
                          .toString();

    QString fileName = QFileDialog::getOpenFileName(
        m_mainWindow, tr("Open Image"), lastDir,
        tr("Image Files (*.png *.jpg *.jpeg *.bmp *.tiff *.raw *.cr2 *.nef *.arw)"));

    if (!fileName.isEmpty()) {
        // Save the directory for next time
        QFileInfo fileInfo(fileName);
        settings.setValue("lastImageDirectory", fileInfo.absolutePath());

        if (m_documentManager->openDocument(fileName)) {
            setState("currentFile", fileName);
            setState("isModified", false);

            // Emit signal with the loaded image
            if (m_documentManager->currentDocument() &&
                m_documentManager->currentDocument()->hasImage()) {
                QImage image = m_documentManager->currentDocument()->originalImage();
                emit imageLoaded(image, fileName);
            }
            emit fileOpened(fileName);
        }
    }
}

void ApplicationController::saveFile() {
    qDebug() << "saveFile() called";

    if (!m_documentManager->hasDocument()) {
        qDebug() << "No document to save";
        emit errorOccurred("No document to save");
        return;
    }

    QString currentFile = m_documentManager->currentFilePath();
    qDebug() << "Current file path:" << currentFile;

    if (currentFile.isEmpty()) {
        qDebug() << "No file path, calling saveAsFile()";
        saveAsFile();
    } else {
        qDebug() << "Attempting to save to:" << currentFile;
        if (m_documentManager->saveDocument()) {
            setState("isModified", false);
            emit fileSaved(currentFile);
            qDebug() << "File saved successfully";
        } else {
            qDebug() << "Save failed";
        }
    }
}

void ApplicationController::saveAsFile() {
    // Load last used directory
    QSettings settings("PhotoManufactura", "UI");
    QString lastDir = settings
                          .value("lastImageDirectory",
                                 QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
                          .toString();

    QString fileName =
        QFileDialog::getSaveFileName(m_mainWindow, tr("Save Image"), lastDir,
                                     tr("Image Files (*.png *.jpg *.jpeg *.bmp *.tiff)"));

    if (!fileName.isEmpty()) {
        // Save the directory for next time
        QFileInfo fileInfo(fileName);
        settings.setValue("lastImageDirectory", fileInfo.absolutePath());

        if (m_documentManager->saveDocumentAs(fileName)) {
            setState("currentFile", fileName);
            setState("isModified", false);
            emit fileSaved(fileName);
        }
    }
}

void ApplicationController::closeFile() {
    if (m_documentManager->hasUnsavedChanges()) {
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

    m_documentManager->closeDocument();
    setState("currentFile", QString());
    setState("isModified", false);
    emit fileClosed();
}

void ApplicationController::exitApplication() {
    if (m_documentManager->hasUnsavedChanges()) {
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
    m_documentManager->undo();
}

void ApplicationController::redo() {
    m_documentManager->redo();
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
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setBrightness(value);
        setState("isModified", true);
    }
}

void ApplicationController::adjustContrast(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setContrast(value);
        setState("isModified", true);
    }
}

void ApplicationController::adjustExposure(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setExposure(value);
        setState("isModified", true);
    }
}

void ApplicationController::adjustHighlights(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setHighlights(value);
        setState("isModified", true);
    }
}

void ApplicationController::adjustShadows(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setShadows(value);
        setState("isModified", true);
    }
}

void ApplicationController::adjustWhites(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setWhites(value);
        setState("isModified", true);
    }
}

void ApplicationController::adjustBlacks(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setBlacks(value);
        setState("isModified", true);
    }
}

void ApplicationController::adjustTemperature(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setTemperature(value);
        setState("isModified", true);
    }
}

void ApplicationController::adjustTint(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setTint(value);
        setState("isModified", true);
    }
}

void ApplicationController::adjustSaturation(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setSaturation(value);
        setState("isModified", true);
    }
}

void ApplicationController::rotateImage(int degrees) {
    if (m_documentManager->hasDocument()) {
        m_documentManager->rotateImage(degrees);
        setState("isModified", true);

        // Update canvas with transformed image
        if (m_documentManager->currentDocument()) {
            QImage image = m_documentManager->currentDocument()->processedImage();
            emit imageLoaded(image, m_documentManager->currentFilePath());
        }
    }
}

void ApplicationController::flipImage(int direction) {
    if (m_documentManager->hasDocument()) {
        m_documentManager->flipImage(direction);
        setState("isModified", true);

        // Update canvas with transformed image
        if (m_documentManager->currentDocument()) {
            QImage image = m_documentManager->currentDocument()->processedImage();
            emit imageLoaded(image, m_documentManager->currentFilePath());
        }
    }
}

void ApplicationController::cropImage(const QRect& cropArea) {
    if (m_documentManager->hasDocument()) {
        m_documentManager->cropImage(cropArea);
        setState("isModified", true);

        // Update canvas with transformed image
        if (m_documentManager->currentDocument()) {
            QImage image = m_documentManager->currentDocument()->processedImage();
            emit imageLoaded(image, m_documentManager->currentFilePath());
        }
    }
}

void ApplicationController::resetAdjustments() {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->resetAll();
    }
}

// View operations
void ApplicationController::zoomIn() {
    m_appState->zoomIn();
    setState("zoomLevel", static_cast<int>(m_appState->zoomLevel() * 100));
}

void ApplicationController::zoomOut() {
    m_appState->zoomOut();
    setState("zoomLevel", static_cast<int>(m_appState->zoomLevel() * 100));
}

void ApplicationController::resetZoom() {
    m_appState->zoomToActual();
    setState("zoomLevel", 100);
}

void ApplicationController::fitToWindow() {
    m_appState->zoomToFit();
    setState("zoomLevel", static_cast<int>(m_appState->zoomLevel() * 100));
}

// Theme operations
void ApplicationController::setTheme(const QString& themeName) {
    m_appState->setTheme(themeName);
    setState("theme", themeName);
    emit themeChanged(themeName);
}

void ApplicationController::toggleHistogram() {
    m_appState->toggleHistogram();
}

void ApplicationController::toggleToolPanel() {
    m_appState->toggleToolPanel();
}

void ApplicationController::toggleAdjustmentPanel() {
    m_appState->toggleAdjustmentPanel();
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

    // UI signal connections are now handled in main.cpp to avoid circular dependencies
    // between controller and ui libraries.
    // This method is kept for potential future use when controller needs to
    // set up internal state based on the main window.

    qDebug() << "MainWindow set, UI signal connections will be done externally";
}

void ApplicationController::connectModelSignals() {
    // Connect DocumentManager signals
    connect(m_documentManager.get(), &DocumentManager::errorOccurred, this,
            &ApplicationController::errorOccurred);

    connect(m_documentManager.get(), &DocumentManager::documentOpened, this,
            &ApplicationController::fileOpened);

    connect(m_documentManager.get(), &DocumentManager::documentSaved, this,
            &ApplicationController::fileSaved);

    connect(m_documentManager.get(), &DocumentManager::documentClosed, this,
            &ApplicationController::fileClosed);

    // Connect AppState signals
    connect(m_appState.get(), &AppState::zoomLevelChanged, this,
            &ApplicationController::zoomChanged);

    connect(m_appState.get(), &AppState::themeChanged, this, &ApplicationController::themeChanged);
}

void ApplicationController::initializeServices() {
    // TODO: Initialize business logic services when implemented
    // m_imageProcessingService = std::make_unique<ImageProcessingService>();
    // m_rawProcessingService = std::make_unique<RawProcessingService>();
}

void ApplicationController::onImageProcessingComplete() {
    setState("isModified", true);
    emit imageProcessed();
}

void ApplicationController::onFileOperationComplete() {
    // Handle file operation completion
}

// Service access methods - to be implemented when services exist
// ImageProcessingService* ApplicationController::getImageProcessingService() const {
//     return m_imageProcessingService.get();
// }

// RawProcessingService* ApplicationController::getRawProcessingService() const {
//     return m_rawProcessingService.get();
// }

AdjustmentSettings* ApplicationController::getAdjustments() const {
    return m_documentManager ? m_documentManager->adjustments() : nullptr;
}