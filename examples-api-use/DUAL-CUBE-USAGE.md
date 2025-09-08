# Dual Cube Matrix Template Usage Guide

This template provides an easy-to-use interface for controlling a 2-display LED matrix setup configured as two virtual 128×128 cubes.

## Hardware Setup

### Physical Configuration
- **2 LED displays**: Each 64×128 pixels
- **Chained horizontally**:
  ```
  [Display 1] [Display 2]
  ```
- **Logical canvas**: 256×64 pixels (two displays wide)
- **Virtual cubes**: Two 128×128 cubes mapped to this canvas
  - Left Cube: Left half (0-127, scaled vertically)
  - Right Cube: Right half (128-255, scaled vertically)

### Command Line Parameters
Always use these parameters for your 2-display chained setup:
```bash
--led-rows=64 --led-cols=128 --led-chain=2
```

## Building the Template

1. **Install dependencies** (if not already installed):
   ```bash
   sudo apt-get update
   sudo apt-get install libgraphicsmagick++-dev libwebp-dev -y
   ```

2. **Build the template**:
   ```bash
   cd examples-api-use
   make dual-cube-template
   ```

## Basic Usage

### 1. Test Pattern Mode (No Images)
Shows colored quadrants to verify coordinate mapping:
```bash
sudo ./dual-cube-template --led-rows=64 --led-cols=128 --led-chain=2 --led-slowdown-gpio=4
```

**Expected result:** Each cube should display 4 colored quadrants (scaled vertically):
- Bottom-left: Red, Bottom-right: Green  
- Top-left: Blue, Top-right: White

Note: Due to vertical scaling (128→64), the pattern will appear compressed vertically.

### 2. Single Image
Display image on left cube only:
```bash
sudo ./dual-cube-template --led-rows=64 --led-cols=128 --led-chain=2 --led-slowdown-gpio=4 image.jpg
```

### 3. Two Images
Display different images on both cubes:
```bash
sudo ./dual-cube-template --led-rows=64 --led-cols=128 --led-chain=2 --led-slowdown-gpio=4 left_image.png right_image.gif
```

### 4. Animated GIFs
The template automatically handles animated GIFs:
```bash
sudo ./dual-cube-template --led-rows=64 --led-cols=128 --led-chain=2 --led-slowdown-gpio=4 animation1.gif animation2.gif
```

### 5. Image Rotation
Rotate images to fix orientation or reduce vertical compression:
```bash
# Rotate images by 90 degrees clockwise
sudo ./dual-cube-template --led-rows=64 --led-cols=128 --led-chain=2 --led-rotate=90 --led-slowdown-gpio=4 portrait1.jpg portrait2.png

# Other rotation options: 0, 90, 180, 270 degrees
sudo ./dual-cube-template --led-rotate=180 --led-rows=64 --led-cols=128 --led-chain=2 --led-slowdown-gpio=4 upside_down.jpg
```

## Supported Image Formats
- **PNG** - Static images with transparency support
- **JPG/JPEG** - Static images
- **GIF** - Static and animated images

## Virtual Coordinate System

Each cube uses a virtual coordinate system where:
- **(0,0)** = Bottom-left corner
- **(127,127)** = Top-right corner
- **Width**: 0-127 (128 pixels) → maps to physical width 1:1
- **Height**: 0-127 (128 pixels) → **scaled to physical height 64 pixels**

**Important:** Virtual Y coordinates are automatically scaled by factor of 2 to fit the 64-pixel physical height. This means images will appear vertically compressed but maintain their full horizontal resolution.

**Tip:** Use `--led-rotate=90` to rotate portrait-oriented images, which can help compensate for the vertical compression by using the full display height more effectively.

## Programming Interface

### Key Functions for Custom Development

#### Basic Pixel Operations
```cpp
// Set a pixel in virtual coordinates
set_cube_pixel(canvas, LEFT_CUBE, vx, vy, r, g, b);

// Clear entire cube
clear_cube(canvas, LEFT_CUBE);

// Fill cube with solid color
fill_cube(canvas, LEFT_CUBE, 255, 0, 0);  // Red
```

#### Image Operations
```cpp
// Load image for a cube
ImageVector images = load_image_for_cube("image.png");

// Initialize cube with image
init_cube(LEFT_CUBE, "image.png");

// Copy image to cube
copy_image_to_cube(image, canvas, LEFT_CUBE);
```

#### Drawing Functions
```cpp
// Draw rectangle
draw_rect_on_cube(canvas, RIGHT_CUBE, 10, 10, 50, 50, 0, 255, 0);  // Green rectangle

// Draw text (requires Font object)
draw_text_on_cube(canvas, LEFT_CUBE, font, 10, 10, Color(255,255,255), "Hello");
```

#### Coordinate Transformation
```cpp
// Convert virtual to physical coordinates
int px, py;
if (virtual_to_physical(LEFT_CUBE, vx, vy, &px, &py)) {
    // Use px, py for direct matrix operations
}
```

## Customization

### Physical Layout and Coordinate Mapping

The template is configured for the following physical layout:
```
[Display 1] [Display 2]  ← Single row, chained horizontally (256×64 canvas)
```

**Virtual cube mapping:**
- **Left Cube**: Left half of canvas (physical x: 0-127, y: 0-63)
- **Right Cube**: Right half of canvas (physical x: 128-255, y: 0-63)

**How coordinates map:**
- Virtual (0,0) = Bottom-left corner of cube
- Virtual (127,127) = Top-right corner of cube
- **Vertical scaling**: Virtual Y coordinates (0-127) are scaled to physical Y (0-63)

For LEFT_CUBE:
- Virtual X 0-127 → Physical X 0-127
- Virtual Y 0-127 → Physical Y 0-63 (scaled by /2)

For RIGHT_CUBE:
- Virtual X 0-127 → Physical X 128-255
- Virtual Y 0-127 → Physical Y 0-63 (scaled by /2)

### Adjusting Physical Mapping
If your displays are wired differently, modify the `virtual_to_physical()` function in `dual-cube-template.cc` and adjust the coordinate calculations in the switch statement.

### Adding Custom Effects
You can easily extend the template:

1. **Add new drawing functions**
2. **Implement custom animations**
3. **Add image filters or effects**
4. **Create interactive features**

## Troubleshooting

### Common Issues

1. **Image not displaying correctly**:
   - Check your wiring matches the coordinate mapping
   - Verify image format is supported
   - Ensure image file exists and is readable

2. **Wrong cube receiving image**:
   - Adjust the `virtual_to_physical()` function
   - Check your physical display arrangement

3. **Compilation errors**:
   - Ensure GraphicsMagick is installed
   - Check that all dependencies are met

4. **Performance issues**:
   - Reduce image size if needed
   - Adjust animation frame rates
   - Consider using simpler images for better performance

5. **Images appear squished or wrong orientation**:
   - Use `--led-rotate=90` to rotate landscape images to portrait
   - Try `--led-rotate=180` for upside-down images
   - Remember that Y coordinates are scaled (128→64) causing vertical compression

### Testing Your Setup

1. **Test with demo mode** first to verify basic functionality:
   ```bash
   sudo ./dual-cube-template --led-rows=64 --led-cols=128 --led-chain=2 --led-slowdown-gpio=4
   ```
2. **Try simple static images** before animated ones
3. **Test one cube at a time** to isolate issues
4. **Note the vertical compression** - images will appear 50% compressed vertically
5. **Test rotation feature**:
   ```bash
   # Compare normal vs rotated display
   sudo ./dual-cube-template --led-rows=64 --led-cols=128 --led-chain=2 --led-slowdown-gpio=4 image.jpg
   sudo ./dual-cube-template --led-rotate=90 --led-rows=64 --led-cols=128 --led-chain=2 --led-slowdown-gpio=4 image.jpg
   ```

## Example Applications

- **Digital art displays**
- **Information dashboards**
- **Interactive installations**
- **Ambient lighting systems**
- **Gaming displays**
- **Educational demonstrations**

The template provides a solid foundation that you can build upon for your specific needs!
