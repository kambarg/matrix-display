/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *
 * Blue Eyes Animation Example using LED Matrix
 */
#include "led-matrix-c.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

// Function to calculate distance between two points
float distance(int x1, int y1, int x2, int y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

// Function to get blue gradient value based on distance from center
uint8_t get_blue_gradient(float dist, float max_dist) {
    float ratio = dist / max_dist;
    if (ratio > 1.0) ratio = 1.0;
    // Invert ratio to make center darker
    ratio = 1.0 - ratio;
    // Make gradient more pronounced
    ratio = ratio * ratio;
    // Scale to blue range (50-255)
    return (uint8_t)(50 + (ratio * 205));
}

int main(int argc, char **argv) {
    struct RGBLedMatrixOptions options;
    struct RGBLedMatrix *matrix;
    struct LedCanvas *offscreen_canvas;
    int width, height;
    int x, y;

    // Initialize matrix options
    memset(&options, 0, sizeof(options));
    options.rows = 32;
    options.chain_length = 1;

    // Create matrix
    matrix = led_matrix_create_from_options(&options, &argc, &argv);
    if (matrix == NULL)
        return 1;

    // Create canvas for double-buffering
    offscreen_canvas = led_matrix_create_offscreen_canvas(matrix);
    led_canvas_get_size(offscreen_canvas, &width, &height);

    // Define eye centers (assuming 32x32 matrix)
    int eye1_x = width / 4;
    int eye2_x = (2 * width) / 4;
    int eye_y = height / 2;
    float eye_radius = height / 4;

    // Animation loop
    while (1) {
        // Clear the canvas
        for (y = 0; y < height; ++y) {
            for (x = 0; x < width; ++x) {
                // Calculate distances to both eye centers
                float dist1 = distance(x, y, eye1_x, eye_y);
                float dist2 = distance(x, y, eye2_x, eye_y);
                
                // Take the minimum distance to determine which eye we're in
                float dist = dist1 < dist2 ? dist1 : dist2;
                
                if (dist <= eye_radius) {
                    // Inside eye area - apply blue gradient
                    uint8_t blue = get_blue_gradient(dist, eye_radius);
                    // Add a slight white tint for better visibility
                    uint8_t white = (uint8_t)(blue / 5);
                    led_canvas_set_pixel(offscreen_canvas, x, y, white, white, blue);
                } else {
                    // Outside eye area - black
                    led_canvas_set_pixel(offscreen_canvas, x, y, 0, 0, 0);
                }
            }
        }

        // Swap buffers
        offscreen_canvas = led_matrix_swap_on_vsync(matrix, offscreen_canvas);
        
        // Small delay to control animation speed
        usleep(50000);  // 50ms delay
    }

    // Cleanup
    led_matrix_delete(matrix);
    return 0;
} 