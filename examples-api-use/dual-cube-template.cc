/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *
 * Dual Cube Matrix Display Template
 *
 * Hardware Setup:
 * - 4 displays (64x128 each) in 2x2 grid configuration
 * - Display 1 + Display 2 = Left Cube (128x128 virtual)
 * - Display 3 + Display 4 = Right Cube (128x128 virtual)
 * - Command line: --led-chain=2 --led-parallel=2
 *
 * Virtual coordinate system: (0,0) at bottom-left of each cube
 */

#include "led-matrix.h"
#include "graphics.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <exception>
#include <vector>

// Include GraphicsMagick for image loading
#include <Magick++.h>
#include <magick/image.h>

using namespace rgb_matrix;
using ImageVector = std::vector<Magick::Image>;

// Cube definitions
#define CUBE_WIDTH 128
#define CUBE_HEIGHT 128
#define NUM_CUBES 2

// Cube identifiers
enum CubeID
{
    LEFT_CUBE = 0,
    RIGHT_CUBE = 1
};

// Structure to hold cube state
typedef struct
{
    ImageVector images;
    int current_frame;
    int animation_delay;
    bool is_animated;
    bool enabled;
} CubeState;

// Global variables
RGBMatrix *matrix = NULL;
CubeState cubes[NUM_CUBES];
volatile bool interrupt_received = false;
int image_rotation = 0; // Image rotation angle (0, 90, 180, 270)

// Signal handler for clean shutdown
void signal_handler(int signum)
{
    interrupt_received = true;
    if (matrix)
    {
        matrix->Clear();
        delete matrix;
    }
    exit(signum);
}

/**
 * Virtual to Physical Coordinate Transformation
 *
 * Virtual coordinates: (0,0) at bottom-left of cube, (127,127) at top-right
 * Physical coordinates depend on actual wiring configuration
 *
 * @param cube_id: LEFT_CUBE or RIGHT_CUBE
 * @param vx, vy: Virtual coordinates (0-127)
 * @param px, py: Output physical coordinates
 * @return: true if coordinates are valid
 */
bool virtual_to_physical(CubeID cube_id, int vx, int vy, int *px, int *py)
{
    // Validate virtual coordinates
    if (vx < 0 || vx >= CUBE_WIDTH || vy < 0 || vy >= CUBE_HEIGHT)
    {
        return false;
    }

    // Convert from bottom-left origin to top-left origin
    int flipped_vy = CUBE_HEIGHT - 1 - vy;

    // Map to physical coordinates based on cube and wiring
    // Physical layout with --led-chain=2:
    // [Display1] [Display2]  <- Single row, chained horizontally (256×64 canvas)
    //
    // Desired cube mapping:
    // LEFT_CUBE = Left half of canvas (0-127, 0-63)
    // RIGHT_CUBE = Right half of canvas (128-255, 0-63)
    //
    // Virtual cube coordinates (128×128) are scaled to fit physical canvas (256×64)

    switch (cube_id)
    {
    case LEFT_CUBE:
        // Left cube maps to left half of the 256×64 canvas
        *px = vx;             // Virtual x (0-127) maps to physical x (0-127)
        *py = flipped_vy / 2; // Scale virtual y (0-127) to physical y (0-63)
        break;

    case RIGHT_CUBE:
        // Right cube maps to right half of the 256×64 canvas
        *px = vx + CUBE_WIDTH; // Virtual x (0-127) maps to physical x (128-255)
        *py = flipped_vy / 2;  // Scale virtual y (0-127) to physical y (0-63)
        break;

    default:
        return false;
    }

    return true;
}

/**
 * Set a pixel in virtual cube coordinates
 */
void set_cube_pixel(Canvas *canvas, CubeID cube_id, int vx, int vy,
                    uint8_t r, uint8_t g, uint8_t b)
{
    int px, py;
    if (virtual_to_physical(cube_id, vx, vy, &px, &py))
    {
        canvas->SetPixel(px, py, r, g, b);
    }
}

/**
 * Clear a specific cube
 */
void clear_cube(Canvas *canvas, CubeID cube_id)
{
    for (int vy = 0; vy < CUBE_HEIGHT; vy++)
    {
        for (int vx = 0; vx < CUBE_WIDTH; vx++)
        {
            set_cube_pixel(canvas, cube_id, vx, vy, 0, 0, 0);
        }
    }
}

/**
 * Load and scale image for a cube
 */
ImageVector load_image_for_cube(const char *filename)
{
    ImageVector result;

    if (!filename)
        return result;

    ImageVector frames;
    try
    {
        readImages(&frames, filename);
    }
    catch (std::exception &e)
    {
        if (e.what())
        {
            fprintf(stderr, "Error loading image %s: %s\n", filename, e.what());
        }
        return result;
    }

    if (frames.empty())
    {
        fprintf(stderr, "No image found in %s\n", filename);
        return result;
    }

    // Handle animated images
    if (frames.size() > 1)
    {
        Magick::coalesceImages(&result, frames.begin(), frames.end());
    }
    else
    {
        result.push_back(frames[0]);
    }

    // Apply rotation and scale all frames to cube size
    for (Magick::Image &image : result)
    {
        // Apply rotation if specified
        if (image_rotation != 0)
        {
            image.rotate(image_rotation);
        }

        // Scale to cube size
        image.scale(Magick::Geometry(CUBE_WIDTH, CUBE_HEIGHT));
    }

    return result;
}

/**
 * Copy image to cube using virtual coordinates
 */
void copy_image_to_cube(const Magick::Image &image, Canvas *canvas, CubeID cube_id)
{
    // Copy image pixels to cube
    for (size_t y = 0; y < image.rows() && y < CUBE_HEIGHT; ++y)
    {
        for (size_t x = 0; x < image.columns() && x < CUBE_WIDTH; ++x)
        {
            const Magick::Color &c = image.pixelColor(x, y);

            // Skip transparent pixels
            if (c.alphaQuantum() < 256)
            {
                // Convert ImageMagick coordinates to virtual coordinates
                // ImageMagick: (0,0) at top-left
                // Virtual: (0,0) at bottom-left
                int vx = x;
                int vy = CUBE_HEIGHT - 1 - y;

                set_cube_pixel(canvas, cube_id, vx, vy,
                               ScaleQuantumToChar(c.redQuantum()),
                               ScaleQuantumToChar(c.greenQuantum()),
                               ScaleQuantumToChar(c.blueQuantum()));
            }
        }
    }
}

/**
 * Initialize cube with image
 */
bool init_cube(CubeID cube_id, const char *image_filename)
{
    if (cube_id >= NUM_CUBES)
        return false;

    cubes[cube_id].images = load_image_for_cube(image_filename);
    cubes[cube_id].current_frame = 0;
    cubes[cube_id].animation_delay = 100; // Default 100ms between frames
    cubes[cube_id].is_animated = cubes[cube_id].images.size() > 1;
    cubes[cube_id].enabled = !cubes[cube_id].images.empty();

    if (cubes[cube_id].enabled)
    {
        printf("Loaded %s to %s cube: %zu frame(s)\n",
               image_filename,
               cube_id == LEFT_CUBE ? "LEFT" : "RIGHT",
               cubes[cube_id].images.size());
    }

    return cubes[cube_id].enabled;
}

/**
 * Update cube animation state
 */
void update_cube_animation(CubeID cube_id)
{
    if (cube_id >= NUM_CUBES || !cubes[cube_id].enabled || !cubes[cube_id].is_animated)
    {
        return;
    }

    cubes[cube_id].current_frame++;
    if (cubes[cube_id].current_frame >= (int)cubes[cube_id].images.size())
    {
        cubes[cube_id].current_frame = 0;
    }
}

/**
 * Render cube current frame
 */
void render_cube(Canvas *canvas, CubeID cube_id)
{
    if (cube_id >= NUM_CUBES || !cubes[cube_id].enabled)
    {
        return;
    }

    const Magick::Image &current_image = cubes[cube_id].images[cubes[cube_id].current_frame];
    copy_image_to_cube(current_image, canvas, cube_id);
}

/**
 * Draw text on cube using virtual coordinates
 */
void draw_text_on_cube(Canvas *canvas, CubeID cube_id, Font &font,
                       int vx, int vy, const Color &color, const char *text)
{
    int px, py;
    if (virtual_to_physical(cube_id, vx, vy, &px, &py))
    {
        DrawText(canvas, font, px, py, color, NULL, text, 0);
    }
}

/**
 * Fill cube with solid color
 */
void fill_cube(Canvas *canvas, CubeID cube_id, uint8_t r, uint8_t g, uint8_t b)
{
    for (int vy = 0; vy < CUBE_HEIGHT; vy++)
    {
        for (int vx = 0; vx < CUBE_WIDTH; vx++)
        {
            set_cube_pixel(canvas, cube_id, vx, vy, r, g, b);
        }
    }
}

/**
 * Draw test pattern to verify coordinate mapping
 * Shows colored quadrants to help debug display arrangement
 */
void draw_test_pattern(Canvas *canvas, CubeID cube_id)
{
    for (int vy = 0; vy < CUBE_HEIGHT; vy++)
    {
        for (int vx = 0; vx < CUBE_WIDTH; vx++)
        {
            uint8_t r = 0, g = 0, b = 0;

            // Create quadrant pattern
            if (vx < 64 && vy < 64)
            {
                // Bottom-left quadrant: Red
                r = 255;
            }
            else if (vx >= 64 && vy < 64)
            {
                // Bottom-right quadrant: Green
                g = 255;
            }
            else if (vx < 64 && vy >= 64)
            {
                // Top-left quadrant: Blue
                b = 255;
            }
            else
            {
                // Top-right quadrant: White
                r = g = b = 255;
            }

            set_cube_pixel(canvas, cube_id, vx, vy, r, g, b);
        }
    }
}

/**
 * Draw rectangle on cube
 */
void draw_rect_on_cube(Canvas *canvas, CubeID cube_id,
                       int vx1, int vy1, int vx2, int vy2,
                       uint8_t r, uint8_t g, uint8_t b)
{
    // Ensure coordinates are in order
    if (vx1 > vx2)
    {
        int temp = vx1;
        vx1 = vx2;
        vx2 = temp;
    }
    if (vy1 > vy2)
    {
        int temp = vy1;
        vy1 = vy2;
        vy2 = temp;
    }

    for (int vy = vy1; vy <= vy2; vy++)
    {
        for (int vx = vx1; vx <= vx2; vx++)
        {
            set_cube_pixel(canvas, cube_id, vx, vy, r, g, b);
        }
    }
}

/**
 * Usage information
 */
int usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [led-matrix-options] [--led-rotate=ANGLE] [left-image] [right-image]\n", progname);
    fprintf(stderr, "  left-image:  Image file for left cube (optional)\n");
    fprintf(stderr, "  right-image: Image file for right cube (optional)\n");
    fprintf(stderr, "\nSupported formats: PNG, JPG, GIF (including animated)\n");
    fprintf(stderr, "\nRequired matrix options: --led-rows=64 --led-cols=128 --led-chain=2\n");
    fprintf(stderr, "This creates a 256×64 logical canvas with two virtual 128×128 cubes\n");
    fprintf(stderr, "\nCustom options:\n");
    fprintf(stderr, "  --led-rotate=ANGLE   Rotate input images by ANGLE degrees (0, 90, 180, 270)\n");
    fprintf(stderr, "                       Useful for correcting orientation or reducing vertical compression\n");
    rgb_matrix::PrintMatrixFlags(stderr);
    return 1;
}

int main(int argc, char **argv)
{
    // Initialize GraphicsMagick
    Magick::InitializeMagick(*argv);

    // Parse matrix options
    RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;

    // Set default options for your setup
    matrix_options.rows = 64;        // Rows per individual panel
    matrix_options.cols = 128;       // Columns per individual panel
    matrix_options.chain_length = 2; // 2 displays chained horizontally (256×64 canvas)
    matrix_options.parallel = 1;     // Single chain
    matrix_options.hardware_mapping = "regular";

    // Parse custom rotation option before standard matrix options
    for (int i = 1; i < argc; i++)
    {
        if (strncmp(argv[i], "--led-rotate=", 13) == 0)
        {
            image_rotation = atoi(argv[i] + 13);

            // Validate rotation value
            if (image_rotation != 0 && image_rotation != 90 &&
                image_rotation != 180 && image_rotation != 270)
            {
                fprintf(stderr, "Invalid rotation angle: %d. Must be 0, 90, 180, or 270.\n", image_rotation);
                return 1;
            }

            // Remove this argument from argv to avoid confusing the matrix parser
            for (int j = i; j < argc - 1; j++)
            {
                argv[j] = argv[j + 1];
            }
            argc--;
            i--; // Recheck this position since we shifted everything
        }
    }

    if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt))
    {
        return usage(argv[0]);
    }

    // Extract image filenames
    const char *left_image = (argc > 1) ? argv[1] : NULL;
    const char *right_image = (argc > 2) ? argv[2] : NULL;

    // Setup signal handlers
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    // Create matrix
    matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == NULL)
    {
        fprintf(stderr, "Failed to create matrix\n");
        return 1;
    }

    printf("Matrix created: %dx%d\n", matrix->width(), matrix->height());
    printf("Virtual cubes: 2 x %dx%d\n", CUBE_WIDTH, CUBE_HEIGHT);
    if (image_rotation != 0)
    {
        printf("Image rotation: %d degrees\n", image_rotation);
    }

    // Initialize cubes
    bool left_loaded = false, right_loaded = false;

    if (left_image)
    {
        left_loaded = init_cube(LEFT_CUBE, left_image);
    }

    if (right_image)
    {
        right_loaded = init_cube(RIGHT_CUBE, right_image);
    }

    // If no images provided, show demo
    if (!left_loaded && !right_loaded)
    {
        printf("No images provided. Showing test pattern...\n");
        printf("This will help verify coordinate mapping:\n");
        printf("Each cube should show 4 colored quadrants:\n");
        printf("  Bottom-left: Red, Bottom-right: Green\n");
        printf("  Top-left: Blue, Top-right: White\n");
        printf("Press Ctrl+C to stop\n");

        // Demo: Show test pattern to verify coordinate mapping
        while (!interrupt_received)
        {
            matrix->Clear();
            draw_test_pattern(matrix, LEFT_CUBE);
            draw_test_pattern(matrix, RIGHT_CUBE);
            usleep(100000); // 100ms
        }
    }
    else
    {
        // Show loaded images
        printf("Displaying images. Press Ctrl+C to stop\n");

        FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();
        int frame_count = 0;

        while (!interrupt_received)
        {
            offscreen_canvas->Clear();

            // Render both cubes
            render_cube(offscreen_canvas, LEFT_CUBE);
            render_cube(offscreen_canvas, RIGHT_CUBE);

            // Swap buffers
            offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);

            // Update animations every 10 frames (adjust as needed)
            if (frame_count % 10 == 0)
            {
                update_cube_animation(LEFT_CUBE);
                update_cube_animation(RIGHT_CUBE);
            }

            frame_count++;
            usleep(10000); // 10ms between frames
        }
    }

    // Cleanup
    matrix->Clear();
    delete matrix;
    return 0;
}
