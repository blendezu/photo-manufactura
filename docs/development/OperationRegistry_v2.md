# OperationRegistry V2: Dynamic Effects Integration Guide

**Status**: Active (v2)
**Goal**: Decouple the GUI from specific filter classes and allow simplified "plug-and-play" addition of new effects into the hybrid image processing pipeline (AOT + CPU).

---

## 1. Architecture Overview (V2 Update)

In Version 2, the `ImageController` and `ImageState` have been refactored to support a **dynamic list of active effects**. This replaces the hardcoded boolean flags (e.g., `isVintage`) from v1.

### The Components:
*   **`OperationRegistry`**: A Singleton catalog that knows all available effects (Name, Category, Factory, Icon).
*   **`ImageState`**: Contains a `std::vector<std::string> activeEffects` list instead of specific effect flags.
*   **`ImageController`**: Automatically iterates through `activeEffects`, looks them up in the registry, and applies them to the pipeline (sequentially via CPU).

---

## 2. Integration with GUI

The GUI needs to perform two main tasks: **listing availability** and **toggling state**.

### Step A: Populating the Menu / Panel
This remains similar to v1. You ask the registry what exists to build your UI.

```cpp
#include "../image_processing/core/operation_registry.h"

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
        std::string catName = OperationRegistry::categoryToString(cat);
        createMenuSection(catName);

        // Get list of filters
        auto filterNames = registry.getFiltersByCategory(cat);

        for (const auto& name : filterNames) {
            auto info = registry.getFilterInfo(name);
            // Create a Toggle Button (Checkable)
            createToggleButton(info.name, info.iconName, info.description);
        }
    }
}
```

### Step B: Toggling an Effect (Update State)
**Critical Change from v1:** You do **not** create the operation object yourself anymore. Instead, you update the `ImageState` string list.

```cpp
// Assuming you have access to your controller instance
void onFilterToggled(const std::string& filterName, bool isChecked) {
    // 1. Get current state (copy)
    ImageState state = controller.getState(); // You might need a getter for state if not publicly stored
   
    // 2. Modify activeEffects list
    auto& effects = state.activeEffects;
    
    if (isChecked) {
        // Add if not present
        if (std::find(effects.begin(), effects.end(), filterName) == effects.end()) {
            effects.push_back(filterName);
        }
    } else {
        // Remove if present
        effects.erase(std::remove(effects.begin(), effects.end(), filterName), effects.end());
    }

    // 3. Push update to Controller
    controller.update(state);
    
    // 4. Trigger Repaint
    updateView(controller.process());
}
```

---

## 3. How to Add a New Filter (Backend Developer)

To add a new effect (e.g., "Cyberpunk Look"), you **do not** need to touch `ImageController` or `ImageState`.

1.  **Create your class** (e.g., `CyberpunkEffect`) inheriting from `ImageOperation`.
2.  **Open** `src/image_processing/core/operation_registry.cpp`.
3.  **Register it**:

```cpp
void OperationRegistry::registerDefaultFilters() {
    // ...
    registerFilter(
        "Cyberpunk",                                   // Unqiue ID/Name
        []() { return std::make_shared<CyberpunkEffect>(); }, 
        Category::GENERAL,
        "Neon lights and high contrast",               // Description
        "cyberpunk_icon"                               // Icon resource
    );
}
```

**That's it!** The Controller will now automatically pick it up if the GUI adds "Cyberpunk" to the `activeEffects` list.

---

## 4. Technical Constraints

*   **Execution Order**: Effects in `activeEffects` are applied sequentially **after** the main GPU pipeline (Exposure, Color, WB) but **before** final Resize/Rotate.
*   **Order in List**: The order in `activeEffects` matters! If you enlist `["Vintage 1", "Gray Image"]`, the image becomes Vintage first, then Grayscale. If you reverse it, you apply Vintage warmth to a black & white image (getting a sepia-like result). The GUI should likely append new effects to the end or manage order explicitly.
