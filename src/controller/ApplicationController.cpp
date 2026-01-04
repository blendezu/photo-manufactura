#include "ApplicationController.h"

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QWidget>

#include "ICommand.h"
#include "PresetManager.h"

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
void ApplicationController::newDocument() {
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
    setState("currentFilter", QString());
    setState("currentPreset", QString());
    emit fileClosed();
}

void ApplicationController::openFile() {
    // Load last used directory
    QSettings settings("PhotoManufactura", "UI");
    QString lastDir = settings
                          .value("lastImageDirectory",
                                 QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
                          .toString();

    QString fileName = QFileDialog::getOpenFileName(m_mainWindow, tr("Open Image"), lastDir,
                                                    tr(FileFormats::OpenFilter));

    if (!fileName.isEmpty()) {
        // Save zoom for current file before switching
        QString currentFile = m_documentManager->currentFilePath();
        if (!currentFile.isEmpty()) {
            m_appState->saveZoomForFile(currentFile);
        }

        // Save the directory for next time
        QFileInfo fileInfo(fileName);
        settings.setValue("lastImageDirectory", fileInfo.absolutePath());

        if (m_documentManager->openDocument(fileName)) {
            setState("currentFile", fileName);
            setState("isModified", false);

            // Restore zoom for this file
            m_appState->restoreZoomForFile(fileName);

            // Emit signal with the loaded image
            if (m_documentManager->currentDocument() &&
                m_documentManager->currentDocument()->hasImage()) {
                QImage image = m_documentManager->currentDocument()->originalImage();
                emit imageLoaded(image, fileName);
            }
            emit fileOpened(fileName);
        } else {
            emit errorOccurred(tr("Failed to open file: %1").arg(fileName));
        }
    }
}

void ApplicationController::saveFile() {
    qDebug() << "saveFile() called";

    if (!m_documentManager->hasDocument()) {
        qDebug() << "No document to save";
        emit errorOccurred(tr("No document to save"));
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
            emit errorOccurred(tr("Failed to save file: %1").arg(currentFile));
        }
    }
}

void ApplicationController::saveAsFile() {
    if (!m_documentManager->hasDocument()) {
        emit errorOccurred(tr("No document to save"));
        return;
    }

    // Load last used directory
    QSettings settings("PhotoManufactura", "UI");
    QString lastDir = settings
                          .value("lastImageDirectory",
                                 QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
                          .toString();

    QString fileName = QFileDialog::getSaveFileName(m_mainWindow, tr("Save Image"), lastDir,
                                                    tr(FileFormats::SaveFilter));

    if (!fileName.isEmpty()) {
        // Save the directory for next time
        QFileInfo fileInfo(fileName);
        settings.setValue("lastImageDirectory", fileInfo.absolutePath());

        if (m_documentManager->saveDocumentAs(fileName)) {
            setState("currentFile", fileName);
            setState("isModified", false);
            emit fileSaved(fileName);
        } else {
            emit errorOccurred(tr("Failed to save file: %1").arg(fileName));
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

    // Save zoom before closing
    QString currentFile = m_documentManager->currentFilePath();
    if (!currentFile.isEmpty()) {
        m_appState->saveZoomForFile(currentFile);
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

// Private helper for filter operations - eliminates code duplication
void ApplicationController::applyFilterAndNotify(const QString& filterName, bool isRemoveFilter) {
    if (!m_documentManager->hasDocument()) {
        return;
    }

    if (isRemoveFilter) {
        m_documentManager->removeFilter();
    } else {
        m_documentManager->applyFilter(filterName);
        setState("isModified", true);
    }

    setState("currentFilter", filterName);

    if (m_documentManager->currentDocument()) {
        QImage image = m_documentManager->currentDocument()->processedImage();
        emit imageLoaded(image, m_documentManager->currentFilePath());
    }
}

// Filter/Effect operations
void ApplicationController::applyFilterOriginal() {
    applyFilterAndNotify("Original", true);
}

void ApplicationController::applyFilterGrayscale() {
    applyFilterAndNotify("Grayscale");
}

void ApplicationController::applyFilterVintage() {
    applyFilterAndNotify("Vintage");
}

void ApplicationController::applyAutoEnhance() {
    applyFilterAndNotify("AutoEnhance");
}

void ApplicationController::applyStyleTransfer(StyleTransferType styleType) {
    if (!m_documentManager->hasDocument()) {
        return;
    }

    QString styleName;
    switch (styleType) {
        case StyleTransferType::Mosaic:
            styleName = "StyleTransfer_Mosaic";
            break;
        case StyleTransferType::Candy:
            styleName = "StyleTransfer_Candy";
            break;
        case StyleTransferType::RainPrincess:
            styleName = "StyleTransfer_RainPrincess";
            break;
        case StyleTransferType::Udnie:
            styleName = "StyleTransfer_Udnie";
            break;
        case StyleTransferType::Pointillism:
            styleName = "StyleTransfer_Pointillism";
            break;
        default:
            styleName = "StyleTransfer_Mosaic";
            break;
    }

    applyFilterAndNotify(styleName);
}

// Preset operations
void ApplicationController::applyPreset(const QString& presetName) {
    if (!m_documentManager->hasDocument()) {
        qDebug() << "No document open to apply preset";
        return;
    }

    PresetManager presetManager;
    AdjustmentSettings* settings = m_documentManager->adjustments();

    if (presetManager.loadPreset(presetName, settings)) {
        qDebug() << "Preset applied:" << presetName;
        // Settings have been updated, now apply to image
        m_documentManager->applyAdjustments();
        setState("currentPreset", presetName);
        setState("isModified", true);
        
        // Notify UI to update sliders with preset values
        emit adjustmentsChanged(
            settings->brightness(),
            settings->contrast(),
            settings->saturation(),
            settings->exposure(),
            settings->highlights(),
            settings->shadows(),
            settings->whites(),
            settings->blacks(),
            settings->temperature(),
            settings->tint()
        );
    } else {
        qDebug() << "Failed to load preset:" << presetName;
    }
}

void ApplicationController::saveCurrentAsPreset(const QString& presetName) {
    if (!m_documentManager->hasDocument()) {
        qDebug() << "No document open to save preset";
        return;
    }

    PresetManager presetManager;
    AdjustmentSettings* settings = m_documentManager->adjustments();

    if (presetManager.savePreset(presetName, settings)) {
        qDebug() << "Preset saved:" << presetName;
    } else {
        qDebug() << "Failed to save preset:" << presetName;
    }
}

void ApplicationController::applyCorrections() {
    if (!m_documentManager->hasDocument()) {
        emit errorOccurred(tr("No image open to apply corrections"));
        return;
    }

    // Apply all current adjustments permanently to the image
    if (m_documentManager->applyAdjustmentsPermanently()) {
        qDebug() << "Corrections applied permanently";
        setState("isModified", true);
        emit imageProcessed();
    } else {
        emit errorOccurred(tr("Failed to apply corrections"));
    }
}

void ApplicationController::showSavePresetDialog() {
    if (!m_documentManager->hasDocument()) {
        emit errorOccurred(tr("No image open to save preset"));
        return;
    }

    bool ok;
    QString presetName = QInputDialog::getText(
        m_mainWindow, tr("Save Preset"),
        tr("Enter preset name:"), QLineEdit::Normal,
        tr("My Preset"), &ok);

    if (ok && !presetName.trimmed().isEmpty()) {
        saveCurrentAsPreset(presetName.trimmed());
        
        // Notify UI to refresh presets list
        PresetManager presetManager;
        emit presetsChanged(presetManager.userPresets());
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

void ApplicationController::toggleTheme() {
    QString currentTheme = m_appState->theme();
    QString newTheme = (currentTheme == "dark") ? "light" : "dark";
    setTheme(newTheme);
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

void ApplicationController::setGpuMode(bool enabled) {
    m_documentManager->setGpuMode(enabled);
    qDebug() << "GPU mode set to:" << enabled;
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

AdjustmentSettings* ApplicationController::getAdjustments() const {
    return m_documentManager ? m_documentManager->adjustments() : nullptr;
}