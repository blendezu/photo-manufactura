# OperationRegistry: Effects Integration Guide

**Purpose**: This registry manages "One-Click Effects" (Presets/Filters) like *Vintage*, *Black & White*, etc.
**Goal**: Decouple the GUI from specific filter classes. You ask for "Vintage", the registry gives you the object.

---

## 1. How it works

The registry is a **Singleton** that organizes effects into categories (right now only 3 categories):
*   `MONOCHROME` (Black & White, Sepia...)
*   `VINTAGE` (Retro looks, Grain...)
*   `GENERAL` (Auto-Enhance, etc.)

---

## 2. Integration with GUI

### Step A: Populating the Menu / Panel
Instead of hardcoding buttons ("Add Vintage Filter"), you ask the registry what is available. This allows us to add new filters in the backend without changing a single line of GUI code.

```cpp
#include "core/operation_registry.h"

void populateFilterMenu() {
    auto& registry = OperationRegistry::getInstance();
    
    // 1. Define Categories to show
    std::vector<OperationRegistry::Category> categories = {
        OperationRegistry::Category::VINTAGE,
        OperationRegistry::Category::MONOCHROME,
        OperationRegistry::Category::GENERAL
    };

    // 2. Iterate and Create UI Elements
    for (auto cat : categories) {
        // Get friendly name (e.g., "Vintage Effects")
        std::string catName = OperationRegistry::categoryToString(cat);
        createMenuSection(catName);

        // Get list of filters in this category
        auto filterNames = registry.getFiltersByCategory(cat);

        for (const auto& name : filterNames) {
            // Get details (Icon, Description)
            auto info = registry.getFilterInfo(name);
            
            // Create Button/Action
            createButton(info.name, info.iconName, info.description);
        }
    }
}
```

### Step B: Applying an Effect
When the user clicks a button, use the name to create and add the effect to the pipeline.

```cpp
void onFilterClicked(const std::string& filterName) {
    // 1. Get Factory Instance
    auto& registry = OperationRegistry::getInstance();
    
    // 2. Create the actual Operation Object
    std::shared_ptr<ImageOperation> newEffect = registry.createFilter(filterName);
    
    if (newEffect) {
        // 3. Add to Pipeline (See ImagePipeline_v2.md)
        // Note: Effects are usually added to the stack, not used as Live Operations
        pipeline.addOperation(newEffect);
        
        // 4. Update View
        updateView(pipeline.process());
    }
}
```

---

## 3. API Reference

### `getInstance()`
Returns the global registry instance.

### `getFiltersByCategory(Category cat)`
Returns a `std::vector<std::string>` of filter names (IDs).

### `getFilterInfo(std::string name)`
Returns a struct with metadata for UI display:
*   `name`: Display name
*   `description`: Tooltip text
*   `iconName`: Resource ID for the icon

### `createFilter(std::string name)`
Returns a `std::shared_ptr<ImageOperation>`. Returns `nullptr` if the name is not found.

### Categories
*   `OperationRegistry::Category::MONOCHROME`
*   `OperationRegistry::Category::VINTAGE`
*   `OperationRegistry::Category::GENERAL`

---

## 4. How to Add a New Filter (Backend)

The "Magic" works because the GUI only reads what is registered. To add a new filter to the App, you only need to touch `OperationRegistry.cpp`.

**Example:**
You created a new cool filter class `HoaDao`.

1.  **Open** `src/image_processing/core/operation_registry.cpp`
2.  **Include header**:
    ```cpp
    #include "../operations/effects/hoa_dao.h"
    ```
3.  **Register in `registerDefaultFilters()`**:

    ```cpp
    void OperationRegistry::registerDefaultFilters() {
        // ... existing filters ...

        // ONE LINE to add it to the GUI:
        registerFilter(
            "HoaDao",                                      // 1. Name (ID)
            []() { return std::make_shared<HoaDao>(); }, // 2. Factory Lambda
            Category::GENERAL,                                // 3. Category (Where it appears)
            "Futuristic neon colors",                         // 4. Trace/Tooltip
            "neon_icon"                                       // 5. Icon Name
        );
    }
    ```

**Result:**
*   The GUI automatically creates a button "HoaDao" in the "General" menu.
*   When clicked, it creates your `HoaDao` class and adds it to the pipeline.
*   **Zero GUI code changes required.**