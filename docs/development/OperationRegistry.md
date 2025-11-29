# OperationRegistry - Filter Management System

## 📋 OVERVIEW
The `OperationRegistry` is a factory system for dynamic management of image filters. It serves as the central registration point for all filter operations.

## 🎯 WHY OPERATIONREGISTRY?

### Problem without Registry:
- Every new filter requires manual UI changes
- Filters must be hard-coded in GUI code
- No unified filter management
- Difficult to extend for plugins

### Solution with Registry:
- Filters are automatically displayed in the UI
- New filters require no GUI changes
- Centralized management of all filters
- Easy extensibility

## 🏗️ ARCHITECTURE

### Core Concept: Factory Pattern
The Registry manages a list of factory functions that can create filter instances.

### Category System:
Filters are organized into categories:
- `COLOR_EFFECTS`: Color effects
- `VINTAGE`: Vintage/Looks  
- `BLACK_WHITE`: Black & white filters
- `DETAIL`: Detail enhancement
- `CREATIVE`: Creative effects

### Singleton Pattern:
The Registry is implemented as a singleton - there is only one instance in the entire program.

## 💡 BENEFITS

**For Developers:**
- Unified interface: All filters have the same API
- Easy extension: New filters without GUI changes
- Plugins possible: External filters can be registered
- Consistent categorization: Filters are logically organized

**For Users:**
- Complete filter list: No hidden filters
- Organized interface: Filters grouped by category
- Quick access: One click to apply
- Descriptions: Tooltips explain each filter

## 🔧 INTEGRATION WITH PIPELINE

The OperationRegistry creates filters that are directly compatible with the ImagePipeline:

1. Filter is created from Registry
2. Filter is added to Pipeline
3. Pipeline applies filter to image
4. Result is displayed in GUI

## 🚀 FUTURE EXTENSIONS
- Filter presets: Preset configurations for filters
- User filters: Save custom filters
- Filter strength: Adjustable intensity
- Filter stacks: Combine multiple filters

## 📝 GUI USAGE

### 1. Creating a Filter Menu

```cpp
// Dynamically fill filter menu from Registry
void setupFilterMenu() {
    auto& registry = OperationRegistry::getInstance();
    
    // Create submenu for each category
    for (auto category : categories) {
        auto filters = registry.getFiltersByCategory(category);
        QMenu* categoryMenu = filterMenu->addMenu(categoryName);
        
        // Add each filter as menu entry
        for (const auto& filterName : filters) {
            QAction* action = new QAction(filterName);
            connect(action, &QAction::triggered, [filterName]() {
                applyFilter(filterName);
            });
            categoryMenu->addAction(action);
        }
    }
}
```

### 2. Applying a Filter

```cpp
void applyFilter(const std::string& filterName) {
    auto& registry = OperationRegistry::getInstance();
    auto filter = registry.createFilter(filterName);
    
    if (filter) {
        // Add filter to pipeline
        pipeline.addOperation(filter);
        updatePreview();
    }
}
```

### 3. Filter Panel with Preview

```cpp
// Create filter buttons with preview
void createFilterPanel() {
    auto& registry = OperationRegistry::getInstance();
    auto filters = registry.getAvailableFilters();
    
    for (const auto& filterName : filters) {
        // Create button for each filter
        QPushButton* btn = new QPushButton(filterName);
        
        // Show tooltip with description
        auto info = registry.getFilterInfo(filterName);
        btn->setToolTip(info.description);
        
        connect(btn, &QPushButton::clicked, [filterName]() {
            applyFilter(filterName);
        });
    }
}
```

## 🔧 REGISTERING NEW FILTERS

```cpp
// In your filter's source file
static bool registered = []() {
    OperationRegistry::getInstance().registerFilter(
        "MyFilter",
        FilterCategory::CREATIVE,
        "Applies a custom effect",
        []() { return std::make_shared<MyFilter>(); }
    );
    return true;
}();
```

This architecture enables a plugin-like system where new filters can be added with minimal code changes.
