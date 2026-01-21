# GUI Integration Guide: Image Processing Pipeline

This guide explains how to connect GUI sliders and buttons to the Image Processing Engine.

**Target Audience:** GUI Developers  
**Goal:** Bind UI elements (Sliders, Checkboxes) to image operations (Brightness, Contrast, etc.).

---

## 🚨 Coming from Pipeline v1 / v2? (Read this!)

**Key Difference:** Stop calling `pipeline.addOperation(...)`!

*   **Old Way (v1/v2):** You manually added operations to a stack (`pipeline.add(new ExposureOp(1.0))`).
*   **New Way (v3):** You **only update the State** (`state.exposure = 1.0`).

The `ImageController` manages the pipeline internally. It automatically decides clear/rebuild/cache logic. **Do not touch the pipeline directly** unless you are switching CPU/GPU mode.

---

## 1. How it Works

The communication between the GUI and the Engine is handled by the **`ImageController`** and the **`ImageState`**.

*   **`ImageState`**: A simple structure (Source of Truth) that holds **all** parameter values (Brightness, Contrast, etc.).
*   **`ImageController`**: The engine manager. You give it the new State, and it gives you back the processed Image.

You do **NOT** need to call specific functions like `apply()` manually. You simply update the state, and the controller handles the rest (including GPU acceleration).

---

## 2. Integration Workflow

### Step 1: Setup
In your main Window or Widget class, ensure you have an instance of the Controller and the State.

```cpp
#include "image_processing/controller/image_controller.h"

class MainWindow {
private:
    ImageController m_controller;
    ImageState m_state;  // Holds current values (brightness=0, contrast=1.0, etc.)

public:
    void loadImage(std::string path) {
        cv::Mat img = cv::imread(path);
        m_controller.setImage(img);
        updateView();
    }
};
```

### Step 2: Handle Slider Changes
When a user moves a slider, follow this 3-step pattern:

1.  **Update State:** Write the new value into `m_state`.
2.  **Push to Controller:** Call `m_controller.update(m_state)`.
3.  **Process & Display:** Call `m_controller.process()` and show the result.

#### Example: Brightness Slider (-100 to 100)

```cpp
void MainWindow::onBrightnessSliderChanged(int newValue) {
    // 1. Update the State
    // 'newValue' is directly from the slider (e.g. -100 to +100)
    m_state.brightness = static_cast<float>(newValue);

    // 2. Notify Controller
    m_controller.update(m_state);

    // 3. Process Image (Blocking call, usually fast on GPU)
    cv::Mat resultInfo = m_controller.process();

    // 4. Update UI
    if (!resultInfo.empty()) {
        displayImage(resultInfo);
    }
}
```

---

## 3. Available Parameters

Here is the list of parameters you can control in `ImageState`.

| Category | Field Name | Type | Range | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Light** | `exposure` | float | -5.0 to +5.0 | Exposure Compensation |
| | `contrast` | float | -100 to +100 | Contrast adjustment |
| | `brightness` | float | -100 to +100 | Simple brightness |
| | `highlight` | float | -100 to +100 | Recover highlights |
| | `shadow` | float | -100 to +100 | Recover shadows |
| | `white` | float | -100 to +100 | Adjust white point |
| | `black` | float | -100 to +100 | Adjust black point |
| **Color** | `saturation` | float | -100 to +100 | Color intensity |
| | `vibrance` | float | -100 to +100 | Smart saturation |
| | `temp` | float | -100 to +100 | White Balance (Temperature) |
| | `tint` | float | -100 to +100 | White Balance (Tint) Green/Magenta |
| | `tintMagenta`| float | -100 to +100 | Specific Magenta correction |
| **Detail** | `sharpen` | float | 0.0 to 100.0 | Sharpening amount |
| | `clarity` | float | 0.0 to 100.0 | Local contrast (Structure) |
| | `denoise` | float | 0.0 to 100.0 | Noise Reduction (Warning: Slow!) |
| **Geometry** | `rotation` | float | -180.0 to 180.0 | Rotation in degrees |
| | `flip` | int | -1 | `-1`=None, `0`=Vertical, `1`=Horizontal |
| | `cropRect` | cv::Rect | Empty | ROI (x, y, w, h) - **First in CPU Pipeline!** |
| | `resizeWidth` | int | 0 | Target Width (0=Keep/Ratio) |
| | `resizeHeight` | int | 0 | Target Height (0=Keep/Ratio)|
| | `resizeRatio` | float | 0.0 | Aspect Ratio (Requires Height > 0) |

---

## 4. Tips for High Performance

*   **Live Preview:** The `process()` call is optimized. It uses GPU acceleration (Halide) where possible.
*   **AOT Mode:** The engine automatically uses the pre-compiled AOT pipeline if available. You don't need to configure anything.

---

## 5. Pipeline Architecture

1.  **Stage 1: AOT (GPU Fused)**
    *   *Includes:* Exposure, Color (Sat, Vib, Tint), Details (Sharpen, Clarity), Light (Highlight, Shadow, etc.).
    *   *Mechanism:* Halide kernel on Metal GPU.

2.  **Stage 2: Hybrid / CPU Post-Processing**
    *   *Mechanism:* Self Implementation or OpenCV on CPU (Applied sequentially).
    *   **Denoise:** (Bilateral Filter) - *Heavy operation!* Only applied if `state.denoise > 0`. (>1s)
    *   **Geometry:** Crop -> Resize -> Flip -> Rotate.
    *   **Note:** Rotate is the most expensive geometry operation.

---

## 6. System Control (CPU vs. GPU)

You can switch the engine between **CPU (Sequential)** and **GPU (Fused AOT)** modes at runtime, for example via a Checkbox in the settings.

```cpp
// Switch to CPU
m_controller.getPipeline().setFusionMode(false);

// Switch to GPU
m_controller.getPipeline().setFusionMode(true);
```

**Note:** The GPU mode is significantly faster for complex operation chains.

---

## 6. Full Integration Example

Here is a complete, copy-pasteable example of a MainWindow integration:

```cpp
#include <iostream>
#include "image_processing/controller/image_controller.h"

class MainWindow {
private:
    // 1. The Controller owns the engine
    ImageController m_controller;

    // 2. The State holds the current values of all sliders
    ImageState m_state;

public:
    MainWindow() {
        // Defaults are auto-set by ImageState constructor
    }

    void onFileLoaded(std::string path) {
        cv::Mat img = cv::imread(path);
        // Load into Controller (Resets cache)
        m_controller.setImage(img);
        refreshImage();
    }

    // SLIDER: Exposure (-5.0 to +5.0)
    void onExposureSliderChanged(float value) {
        m_state.exposure = value; // Update State
        refreshImage();           // Trigger Process
    }

    // SLIDER: Contrast (-100 to +100)
    void onContrastSliderChanged(float value) {
        m_state.contrast = value;
        refreshImage();
    }

    // SLIDER: Rotate (0 to 360)
    void onRotationSliderChanged(float degrees) {
        m_state.rotation = degrees;
        refreshImage();
    }

    // CHECKBOX: GPU Acceleration
    void onGpuToggled(bool enabled) {
        // Access pipeline directly ONLY for system config
        m_controller.getPipeline().setFusionMode(enabled);
        refreshImage();
    }

private:
    void refreshImage() {
        // 1. Send new state to controller
        m_controller.update(m_state);

        // 2. Execute pipeline (Blocking)
        cv::Mat result = m_controller.process();

        // 3. Display result
        if (!result.empty()) {
             // displayOnScreen(result);
             std::cout << "Image processed: " << result.cols << "x" << result.rows << std::endl;
        }
    }
};
```

---

## 7. Undo / Redo

**Important:** The Engine is stateless. It does **not** store history.
**Responsibility:** The GUI must manage the Undo/Redo stack.

### How to implement Undo:
1.  Create a `std::stack<ImageState> m_undoStack;` in your MainWindow.
2.  **Before** modifying `m_state` (e.g., on Slider Press), push the current copy to the stack.
3.  **On Undo:** Pop the old state, overwrite `m_state`, and call `m_controller.update(m_state)` + `process()`.

```cpp
void MainWindow::undo() {
    if (m_undoStack.empty()) return;
    
    // Restore old state
    m_state = m_undoStack.top();
    m_undoStack.pop();
    
    // Update Engine
    m_controller.update(m_state);
    refreshImage();
}
```

---

## 8. Preview Strategy (High Performance)

**Problem:** Processing large images (e.g. 24MP+) takes too long for smooth slider movement (laggy UI).
**Solution:** `ImageController` has a **Built-in Preview Mode**.

### Automatic Preview Generation
When you call `controller.setImage(img)`, the controller automatically:
1.  Stores the full-resolution image.
2.  Creates a cached **Preview Image** (downscaled to max 1920px).
3.  Enters **Preview Mode** immediately (`m_isPreviewMode = true`).

### Usge pattern

1.  **Slider Dragging (Preview):**
    By default, `controller.process()` works on the small preview image. This is extremely fast (< 50ms).
    *   **Action:** Slider Move
    *   **Code:** Just call `update(state)` + `process()`.

2.  **Exports / Zoom / Final View (Full Res):**
    When you need the full resolution result (e.g. when the user releases the mouse button, or zooms in 100%, or clicks "Save"), you switch modes.

    ```cpp
    // 1. Switch to Full Resolution
    controller.setPreviewMode(false);
    
    // 2. Process (Slower, but full detail)
    cv::Mat fullRes = controller.process();
    
    // 3. (Optional) Switch back to Preview for next slider move
    controller.setPreviewMode(true);
    ```

**Workflow Summary:**
- **On Slider Move:** Update & Process (Uses Preview by default).
- **On Mouse Release / Save:** `setPreviewMode(false)` -> Process -> `setPreviewMode(true)`.
