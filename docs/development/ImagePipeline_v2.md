# ImagePipeline V2: Architecture & Integration Guide

**Version:** 2.0 (Hybrid CPU/GPU)
**Status:** Active Development

This document provides a comprehensive overview of the V2 pipeline architecture, explaining the hybrid execution model, the Halide fusion engine, and the smart caching system. It also includes an integration guide for GUI developers.

---

## Part 1: Architecture Deep Dive

The `ImagePipeline` is the central "brain" of the application. It manages the image state, the history of operations, and decides *how* to process the image (CPU vs GPU) for maximum performance.

### 1. The Three Layers
1.  **Operation Stack**: An ordered list of `ImageOperation` objects (e.g., Brightness, Contrast, Denoise).
2.  **Smart Supervisor**: Logic that decides *what* needs to be calculated. It checks caches and handles "Live Preview" vs "Full Processing".
3.  **Dual Execution Engines**:
    *   **Sequential Engine (CPU)**: Traditional processing. Executes filter 1 -> filter 2 -> filter 3 using standard OpenCV/C++ implementations.
    *   **Fused Engine (GPU/Halide)**: The high-performance engine. It "fuses" multiple operations into a single GPU kernel to minimize memory bandwidth.

### 2. The Smart Supervisor (Process Flow)

When `process()` is called, the pipeline follows this decision tree:

1.  **Cache Check**:
    *   *Question*: "Is the result of the main stack (filters 1...N) already cached?"
    *   *If Yes*: Return the cached image immediately. Zero computation.

2.  **Live Operation Handling (The "Slider" Case)**:
    *   *Scenario*: User is dragging a slider. This is a "temporary" operation not yet added to the stack.
    *   *Action*: Take the **Cached Image** (result of filters 1...N) -> Apply **Live Operation** -> Return Result.
    *   *Benefit*: Only the *current* adjustment is calculated. All previous filters are skipped.

3.  **Full Reprocessing (Cache Miss)**:
    *   *Scenario*: User added a new permanently filter or changed the order.
    *   *Action*: The cache is invalidated. The Supervisor activates one of the Execution Engines to process the full stack (Original Image -> 1 -> 2 -> ... -> N).

---

### 3. The Fused Engine (Halide / GPU)

This is the core innovation of V2. Instead of processing images step-by-step (which requires writing the full image to memory after *every* filter), we use **Halide** to compile a pipeline on the fly.

#### How it works:
1.  **Chain Accumulation**: As we iterate through the operations, we don't execute them. We "collect" them into a `HalideChain`.
2.  **JIT Compilation**: When we trigger execution, Halide compiles a custom GPU kernel that represents *specifically* that sequence of operations.
3.  **Single Pass Execution**: The kernel reads the input pixel *once*, applies all math (Brightness + Contrast + WhiteBalance...), and writes the output pixel *once*. This is drastically faster than CPU execution.

#### The "Flush" Concept (Hybrid Fallback)
Not all operations support Halide (e.g., complex AI models or legacy OpenCV filters). The engine handles this automatically:

*   **Step 1**: Collect Halide-supported ops (e.g., Op 1, Op 2).
*   **Step 2**: Encounter Non-Halide Op (Op 3).
*   **Step 3**: **FLUSH**.
    *   Compile & Run chain (1+2) on GPU. Retrieve intermediate result.
*   **Step 4**: Run Op 3 on CPU.
*   **Step 5**: Start new Halide chain for Op 4, Op 5...

This allows us to mix and match CPU and GPU filters seamlessly.

#### JIT Caching (Performance Critical)
Compiling a Halide pipeline takes time (~10-50ms). We don't want to do this every frame.
The `m_pipelineCache` stores the compiled pipeline. It only recompiles if:
*   The **sequence** of operations changes.
*   The **image format** changes (e.g., 8-bit to 16-bit). *Note: This is critical because compiled code has hardcoded memory strides.*

---

## Part 2: Integration Guide (User Interface)

This section describes how to connect the GUI to the pipeline.

### 1. Basic Lifecycle

```cpp
#include "core/image_pipeline.h"

// 1. Initialization
ImagePipeline pipeline;
pipeline.setImg(cv::imread("photo.jpg"));

// 2. Enable GPU Mode (The User should be able to toggle this)
pipeline.setFusionMode(true);
```

### 2. Live Preview Workflow (Sliders)
**Crucial**: Do NOT add operations to the stack while dragging a slider. Use `setLiveOperation`.

```cpp
// On Slider Drag (Value changing frequently)
void onSliderChanged(float value) {
    // 1. Create a temporary op
    auto tempOp = std::make_shared<AdjustBrightness>(value);
    
    // 2. Set as Live Op (This is fast, uses cache as base)
    pipeline.setLiveOperation(tempOp);
    
    // 3. Update Display
    updateView(pipeline.process()); 
}

// On Slider Release (User finished interaction)
void onSliderReleased(float finalValue) {
    // 1. Clear Live Op
    pipeline.clearLiveOperations();
    
    // 2. Commit to Stack (This updates the Base Cache)
    // If editing existing:
    pipeline.modifyOperation(currentIndex, std::make_shared<AdjustBrightness>(finalValue));
    // If new:
    // pipeline.addOperation(...);
    
    // 3. Final Update
    updateView(pipeline.process());
}
```

### 3. Managing the Stack

```cpp
// Add new
pipeline.addOperation(std::make_shared<AdjustContrast>(1.2f));

// Remove
pipeline.removeOperation(index);

// Reorder (Not native yet, use remove + insert)
auto op = pipeline.getOperation(oldIndex);
pipeline.removeOperation(oldIndex);
pipeline.insertOperation(newIndex, op);
```

### 4. Undo / Redo
The pipeline checks if history exists. Just bind these to buttons.

```cpp
void onUndo() {
    if (pipeline.canUndo()) {
        pipeline.undo();
        updateView(pipeline.process());
    }
}
```

### 5. Error Handling
*   If an operation fails (e.g., GPU memory error), the pipeline prints to `std::cerr` and attempts to return the last valid state or the original image.
*   If `process()` returns an empty matrix, check if an image was loaded (`hasImg()`).
