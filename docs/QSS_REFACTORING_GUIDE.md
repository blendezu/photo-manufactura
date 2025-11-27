# Refactoring Qt Stylesheets - Using External QSS Files

## Overview

This guide shows how we refactored from hardcoded stylesheets to external `.qss` files for better maintainability and organization.

## Benefits of Using QSS Files

✅ **Separation of Concerns**: Styles separate from code  
✅ **Easy to Modify**: Edit styles without recompiling  
✅ **Better Organization**: One file per theme  
✅ **Hot Reloading**: Can reload styles at runtime  
✅ **Version Control Friendly**: Easier to track style changes  
✅ **Team Collaboration**: Designers can edit QSS directly  

---

## Project Structure

```
src/ui/
├── resources/
│   ├── resources.qrc          # Qt Resource file
│   └── styles/
│       ├── dark_theme.qss     # Dark theme styles
│       └── light_theme.qss    # Light theme styles
├── widgets/
│   ├── styleSheet.h/cpp       # Style loader utility
│   └── themeManager.h/cpp     # Theme management singleton
└── mainwindow.cpp             # Uses ThemeManager
```

---

## 1. Creating QSS Files

### `dark_theme.qss`
```css
/* Main Window */
QMainWindow {
    background-color: #1e1e1e;
}

/* Buttons */
QPushButton {
    background-color: #0078d4;
    color: white;
    border: none;
    padding: 6px 12px;
    border-radius: 3px;
}

QPushButton:hover {
    background-color: #1084d8;
}
```

### `light_theme.qss`
```css
/* Main Window */
QMainWindow {
    background-color: #f5f5f5;
}

/* Buttons */
QPushButton {
    background-color: #0078d4;
    color: white;
}
```

---

## 2. Qt Resource File (`.qrc`)

### `resources/resources.qrc`
```xml
<RCC>
    <qresource prefix="/styles">
        <file>styles/dark_theme.qss</file>
        <file>styles/light_theme.qss</file>
    </qresource>
</RCC>
```

**Key Points:**
- `prefix="/styles"` creates a virtual directory
- Files accessed via `:/styles/styles/dark_theme.qss`
- Embedded in executable at compile time

---

## 3. StyleSheet Loader Class

### `styleSheet.h`
```cpp
class StyleSheet {
public:
    enum class Theme { Dark, Light };
    
    static QString loadTheme(Theme theme);
    static QString loadFromFile(const QString& filePath);
    static QString getDarkTheme();
    static QString getLightTheme();
    
private:
    static QString loadQssFile(const QString& resourcePath);
};
```

### `styleSheet.cpp`
```cpp
QString StyleSheet::loadQssFile(const QString& resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Failed to open:" << resourcePath;
        return QString();
    }
    
    QTextStream stream(&file);
    QString styleSheet = stream.readAll();
    file.close();
    
    return styleSheet;
}

QString StyleSheet::getDarkTheme() {
    return loadQssFile(":/styles/styles/dark_theme.qss");
}
```

---

## 4. Theme Manager (Singleton)

### `themeManager.h`
```cpp
class ThemeManager : public QObject {
    Q_OBJECT
public:
    enum class Theme { Dark, Light, Auto };
    
    static ThemeManager& instance();
    void applyTheme(Theme theme);
    Theme currentTheme() const;
    
signals:
    void themeChanged(Theme theme);
    
private:
    ThemeManager();
    Theme m_currentTheme;
};
```

### Usage
```cpp
// Apply theme globally
ThemeManager::instance().applyTheme(ThemeManager::Theme::Dark);

// Listen for theme changes
connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
        this, &MyWidget::onThemeChanged);
```

---

## 5. CMakeLists.txt Configuration

```cmake
target_sources(ui PRIVATE
    # ... other files ...
    
    widgets/styleSheet.cpp
    widgets/styleSheet.h
    widgets/themeManager.cpp
    widgets/themeManager.h
    
    # Qt Resources - IMPORTANT!
    resources/resources.qrc
)
```

**Note**: Adding `.qrc` file automatically invokes `rcc` (Resource Compiler)

---

## 6. Using in Your Application

### Option 1: Direct StyleSheet Loading
```cpp
#include "widgets/styleSheet.h"

// In your widget/window
setStyleSheet(StyleSheet::getDarkTheme());
```

### Option 2: Theme Manager (Recommended)
```cpp
#include "widgets/themeManager.h"

// In main() or app initialization
ThemeManager::instance().applyTheme(ThemeManager::Theme::Dark);

// In View menu
void SubMenuView::onDarkThemeTriggered() {
    ThemeManager::instance().applyTheme(ThemeManager::Theme::Dark);
}

void SubMenuView::onLightThemeTriggered() {
    ThemeManager::instance().applyTheme(ThemeManager::Theme::Light);
}
```

---

## 7. Loading from External Files (Optional)

If you want to load QSS from filesystem (not embedded resources):

```cpp
// Load from external file
QString customTheme = StyleSheet::loadFromFile("/path/to/custom.qss");
qApp->setStyleSheet(customTheme);
```

**Use Cases:**
- User-created themes
- Development/testing
- Dynamic theme updates without rebuild

---

## 8. Hot Reloading During Development

```cpp
class MainWindow : public QMainWindow {
public:
    void reloadStyleSheet() {
        // Load from file instead of resources during dev
        QString style = StyleSheet::loadFromFile(
            QCoreApplication::applicationDirPath() + "/dark_theme.qss"
        );
        qApp->setStyleSheet(style);
    }
};

// Add keyboard shortcut for reload
QShortcut* reloadShortcut = new QShortcut(QKeySequence("Ctrl+R"), this);
connect(reloadShortcut, &QShortcut::activated, this, &MainWindow::reloadStyleSheet);
```

---

## 9. Best Practices

### ✅ DO:
- Use consistent naming (`dark_theme.qss`, not `darkTheme.qss`)
- Comment your QSS files
- Group related selectors together
- Use variables (via preprocessor or string replacement)
- Test themes on all platforms

### ❌ DON'T:
- Mix hardcoded styles with QSS files
- Use absolute paths in QSS
- Forget to add new QSS files to `.qrc`
- Override QSS styles with inline `setStyleSheet()`

---

## 10. Advanced: Variables in QSS

QSS doesn't support variables natively, but you can add them:

```cpp
QString StyleSheet::loadThemeWithColors(
    Theme theme, 
    const QColor& accentColor) 
{
    QString qss = loadTheme(theme);
    
    // Replace placeholder with actual color
    qss.replace("%ACCENT%", accentColor.name());
    
    return qss;
}
```

In your QSS:
```css
QPushButton {
    background-color: %ACCENT%;
}
```

---

## 11. Theme Persistence

Save/load user's theme preference:

```cpp
// In ThemeManager
void ThemeManager::saveThemeToSettings() {
    QSettings settings("PhotoManufactura", "UI");
    settings.setValue("theme", static_cast<int>(m_currentTheme));
}

void ThemeManager::loadThemeFromSettings() {
    QSettings settings("PhotoManufactura", "UI");
    int themeValue = settings.value("theme", 0).toInt();
    m_currentTheme = static_cast<Theme>(themeValue);
    applyTheme(m_currentTheme);
}
```

---

## Summary

**Before Refactoring:**
```cpp
QString StyleSheet::getDarkTheme() {
    return R"(
        QMainWindow { background: #1e1e1e; }
        /* 200+ lines of CSS... */
    )";
}
```

**After Refactoring:**
```cpp
QString StyleSheet::getDarkTheme() {
    return loadQssFile(":/styles/styles/dark_theme.qss");
}
```

**Result:**
- Cleaner code
- Easier maintenance
- Better organization
- Runtime theme switching
- Persistent user preferences

---

## Quick Reference

```cpp
// Apply theme
ThemeManager::instance().applyTheme(ThemeManager::Theme::Dark);

// Get current theme
Theme current = ThemeManager::instance().currentTheme();

// Load custom QSS
QString custom = StyleSheet::loadFromFile("/path/to/theme.qss");

// Listen for changes
connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
        this, &MyWidget::onThemeChanged);
```
