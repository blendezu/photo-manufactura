# Model Layer - MVC Architecture

## 📋 OVERVIEW

The Model layer (`src/model/`) contains the application's data and business state. It follows the MVC pattern where Models are independent of Views and communicate via Qt signals.

## 🏗️ ARCHITECTURE

```
┌─────────────────────────────────────────────────────────────┐
│                      Model Layer                            │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────────┐     ┌──────────────────────┐          │
│  │ DocumentManager │────▶│    ImageDocument     │          │
│  │   (Facade)      │     │  - filePath          │          │
│  │                 │     │  - originalImage     │          │
│  │                 │     │  - processedImage    │          │
│  │                 │     │  - isModified        │          │
│  │                 │     └──────────────────────┘          │
│  │                 │                                        │
│  │                 │     ┌──────────────────────┐          │
│  │                 │────▶│ AdjustmentSettings   │          │
│  │                 │     │  - exposure          │          │
│  └─────────────────┘     │  - contrast          │          │
│                          │  - brightness        │          │
│                          │  - highlights        │          │
│  ┌─────────────────┐     │  - shadows           │          │
│  │    AppState     │     │  - temperature       │          │
│  │  - theme        │     │  - saturation        │          │
│  │  - zoomLevel    │     └──────────────────────┘          │
│  │  - panelVisible │                                        │
│  └─────────────────┘                                        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## 📁 FILES

| File | Purpose |
|------|---------|
| `ImageDocument.h/cpp` | Single image document state |
| `AdjustmentSettings.h/cpp` | All adjustment slider values |
| `AppState.h/cpp` | Application UI state (theme, zoom) |
| `DocumentManager.h/cpp` | Facade for document operations |
| `CMakeLists.txt` | Build configuration |

## 🔧 CLASSES

### ImageDocument

Represents a single image being edited.

```cpp
class ImageDocument : public QObject {
    Q_OBJECT

public:
    // File properties
    QString filePath() const;
    QString fileName() const;
    bool isModified() const;
    
    // Image data
    QImage originalImage() const;
    QImage processedImage() const;
    
public slots:
    void setFilePath(const QString& path);
    void setOriginalImage(const QImage& image);
    void setProcessedImage(const QImage& image);
    void setModified(bool modified);
    void clear();

signals:
    void filePathChanged(const QString& path);
    void originalImageChanged(const QImage& image);
    void processedImageChanged(const QImage& image);
    void modifiedChanged(bool modified);
};
```

### AdjustmentSettings

Holds all image adjustment values with Qt properties.

```cpp
class AdjustmentSettings : public QObject {
    Q_OBJECT
    
    // All adjustments as Q_PROPERTY for QML compatibility
    Q_PROPERTY(int exposure READ exposure WRITE setExposure NOTIFY exposureChanged)
    Q_PROPERTY(int contrast READ contrast WRITE setContrast NOTIFY contrastChanged)
    // ... etc

public:
    // Getters
    int exposure() const;
    int contrast() const;
    int brightness() const;
    int highlights() const;
    int shadows() const;
    int whites() const;
    int blacks() const;
    int temperature() const;
    int tint() const;
    int saturation() const;
    
    bool hasAdjustments() const;

public slots:
    void setExposure(int value);
    void setContrast(int value);
    // ... etc
    void resetAll();

signals:
    void exposureChanged(int value);
    void contrastChanged(int value);
    // ... etc
    void anySettingChanged();  // Emitted for any change
    void settingsReset();
};
```

### AppState

Application-wide state like theme and panel visibility.

```cpp
class AppState : public QObject {
    Q_OBJECT

public:
    QString theme() const;           // "dark" or "light"
    double zoomLevel() const;        // 0.1 to 10.0
    bool histogramVisible() const;
    bool toolPanelVisible() const;
    bool adjustmentPanelVisible() const;

public slots:
    void setTheme(const QString& theme);
    void setZoomLevel(double level);
    void zoomIn();
    void zoomOut();
    void zoomToFit();
    void toggleHistogram();
    void toggleToolPanel();

signals:
    void themeChanged(const QString& theme);
    void zoomLevelChanged(double level);
    void histogramVisibleChanged(bool visible);
};
```

### DocumentManager

Facade that coordinates all model operations.

```cpp
class DocumentManager : public QObject {
    Q_OBJECT

public:
    // Access to sub-models
    ImageDocument* currentDocument() const;
    AdjustmentSettings* adjustments() const;
    
    // Document state
    bool hasDocument() const;
    bool hasUnsavedChanges() const;
    QString currentFilePath() const;

public slots:
    // Document lifecycle
    bool openDocument(const QString& filePath);
    bool saveDocument();
    bool saveDocumentAs(const QString& filePath);
    void closeDocument();
    void newDocument(int width = 1920, int height = 1080);
    
    // Processing
    void applyAdjustments();

signals:
    void documentOpened(const QString& filePath);
    void documentSaved(const QString& filePath);
    void documentClosed();
    void documentStateChanged();
    void errorOccurred(const QString& error);
};
```

## 🔄 SIGNAL FLOW

```
UI Slider Changed
       │
       ▼
┌──────────────────┐
│ AdjustmentSettings│
│ setExposure(val) │
└────────┬─────────┘
         │ emit exposureChanged()
         │ emit anySettingChanged()
         ▼
┌──────────────────┐
│ DocumentManager  │
│ applyAdjustments()│
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ ImageDocument    │
│ setProcessedImage│
└────────┬─────────┘
         │ emit processedImageChanged()
         ▼
┌──────────────────┐
│ CanvasWidget     │
│ updateImage()    │
└──────────────────┘
```

## 💡 USAGE IN CONTROLLER

```cpp
// ApplicationController constructor
ApplicationController::ApplicationController(QObject* parent)
    : QObject(parent),
      m_documentManager(std::make_unique<DocumentManager>(this)),
      m_appState(std::make_unique<AppState>(this)) 
{
    // Connect model signals to controller slots
    connect(m_documentManager.get(), &DocumentManager::errorOccurred,
            this, &ApplicationController::errorOccurred);
    
    connect(m_documentManager.get(), &DocumentManager::documentOpened,
            this, &ApplicationController::fileOpened);
}

// Adjust brightness via controller
void ApplicationController::adjustBrightness(int value) {
    if (auto* adjustments = m_documentManager->adjustments()) {
        adjustments->setBrightness(value);
        // Model automatically triggers processing and updates
    }
}
```

## 🎯 DESIGN PRINCIPLES

1. **Immutable Original**: `originalImage` never changes after loading
2. **Observable State**: All state changes emit signals
3. **Single Source of Truth**: One `DocumentManager` owns all state
4. **Decoupled from UI**: Models know nothing about Views
5. **Qt Properties**: Enable QML binding if needed later

## 📝 EXTENDING THE MODEL

To add a new adjustment:

1. Add property to `AdjustmentSettings`:
```cpp
Q_PROPERTY(int vibrance READ vibrance WRITE setVibrance NOTIFY vibranceChanged)
```

2. Add getter/setter/signal:
```cpp
int vibrance() const { return m_vibrance; }
void setVibrance(int value);
signals:
    void vibranceChanged(int value);
```

3. Update `hasAdjustments()`:
```cpp
bool hasAdjustments() const {
    return m_exposure != 0 || m_vibrance != 0 || /* ... */;
}
```

4. Update `resetAll()`:
```cpp
void resetAll() {
    m_vibrance = 0;
    // ...
}
```

5. Add to `DocumentManager::applyAdjustments()` to process with ImagePipeline.
