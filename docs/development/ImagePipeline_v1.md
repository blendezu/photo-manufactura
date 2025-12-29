# ImagePipeline - Technical Documentation

## 📋 OVERVIEW
The ImagePipeline is the core system for non-destructive image editing. It manages a history of editing operations and applies them in real-time to the original image.

## 🏗️ ARCHITECTURE

### Core Components:
- `originalImage`: Unchanged source image (cv::Mat)
- `operations`: List of all editing operations (Vector of ImageOperation)
- `undoneOperations`: Undo history (Vector of ImageOperation) 
- `cachedResult`: Cached result (cv::Mat)
- `cacheValid`: Cache validity flag (bool)
- `liveOperation`: Temporary operation for real-time preview (ImageOperation)

## 🔧 FUNCTIONS

### Image Management:
- `setImage()`: Loads image into pipeline (makes a copy)
- `getOriginalImage()`: Returns original image
- `hasImage()`: Checks if image is loaded

### Operations Management:
- `addOperation()`: Adds operation (clears undo history)
- `insertOperation()`: Inserts operation at position
- `removeOperation()`: Removes operation at position
- `clearOperations()`: Clears all operations
- `getOperation()`: Returns operation at index

### Processing:
- `process()`: Applies all operations (with caching)
- `processUpTo()`: Processes up to a specific operation

### Real-time Preview:
- `setLiveOperation()`: Sets temporary live operation
- `clearLiveOperation()`: Removes live operation

### Undo/Redo:
- `undo()`: Undoes last operation
- `redo()`: Restores last operation
- `canUndo()`/`canRedo()`: Checks if action is possible

### Cache Management:
- `invalidateCache()`: Marks cache as invalid
- `isCacheValid()`: Checks cache validity

## 🔄 WORKFLOWS

### Normal Editing Flow:
1. `setImage()` - Load image
2. `addOperation()` - Add editing steps
3. `process()` - Calculate and display result

### Real-time Slider Flow:
1. `setLiveOperation()` - Set temporary operation
2. `process()` - Show immediate preview
3. On release: `addOperation()` + `clearLiveOperation()`

### Undo/Redo Flow:
1. `undo()` - Move operation to undo history
2. `redo()` - Move operation back to active list

## 🎯 KEY FEATURES

### Non-destructive Editing:
- Original image always remains unchanged
- All edits are reproducible
- Full editing history available

### Performance Optimizations:
- Cache system avoids repeated calculations
- Live operations for smooth real-time preview
- Exception safety on operation errors

### Color Space Handling:
- Pipeline does not manage color space
- Each operation decides for itself (BGR/HSL)
- Geometric operations: Direct on BGR
- Color operations: Automatic HSL conversion

## 💡 GUI USAGE

### For HSL Sliders (Brightness, Contrast, Saturation):
- During movement: `setLiveOperation()` for real-time preview
- On release: `addOperation()` for permanent change

### For Geometric Operations (Crop, Rotate, Flip):
- Direct `addOperation()` - no live operation needed

### For Undo/Redo:
- Call `undo()`/`redo()`
- Update UI state with `canUndo()`/`canRedo()`

## 📝 Example: Brightness Slider

```cpp
// Connection in constructor
connect(ui->brightnessSlider, &QSlider::valueChanged, 
        this, &MainWindow::onBrightnessChanged);
connect(ui->brightnessSlider, &QSlider::sliderReleased,
        this, &MainWindow::onBrightnessReleased);

void MainWindow::onBrightnessChanged(int value) {
    // Live operation for real-time preview
    if (!liveBrightness) {
        liveBrightness = std::make_shared<BrightnessAdjust>();
    }
    liveBrightness->setBrightness(value);
    
    // Set combined live operation
    auto combinedOp = createCombinedHSLOperation();
    pipeline.setLiveOperation(combinedOp);
    
    updatePreview(); // Immediate preview
}

void MainWindow::onBrightnessReleased() {
    // Auto-Apply: Final operation to pipeline
    int finalValue = ui->brightnessSlider->value();
    auto finalOp = std::make_shared<BrightnessAdjust>(finalValue);
    pipeline.addOperation(finalOp);
    pipeline.clearLiveOperation();
    
    // Enable undo
    ui->undoButton->setEnabled(true);
    updatePreview();
}
```

### 🎯 What This Means for the User:

**While slider is moving:**
- 🔄 Immediate preview → Image changes live
- ⏳ Temporary change → Not saved yet
- ↶ No undo yet → Can simply move slider back

**After releasing:**
- ✅ Change saved → In pipeline history
- ↶ Undo available → Can be undone
- 💾 In project → Will be saved with project

## ⚠️ ERROR HANDLING

- Empty images are caught
- Missing operations are skipped
- On processing errors: Fallback to previous state
- Exception safety in `process()` methods

## 🚀 PERFORMANCE TIPS

- Use cache optimally (for unchanged operations)
- Use live operations for smooth slider movements
- Frame rate limiting for frequent updates
- Use `processUpTo()` for history display instead of `process()`

## 📈 STATE MANAGEMENT

Pipeline state after:
- **New image**: operations empty, cache invalid
- **Operation added**: operations grows, cache invalid
- **Live operation**: cache invalid, temporary preview
- **Undo**: Operation moves from operations → undoneOperations
- **Redo**: Operation moves from undoneOperations → operations

This architecture enables professional image editing with full history and real-time preview.
