/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *
 * Typewriter Text Animation Example using LED Matrix
 */
#include "led-matrix.h"
#include "graphics.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

using namespace rgb_matrix;

// Text lines to display
#define NUM_LINES 4
const char* text_lines[NUM_LINES] = {
    "Hello,",
    "I am",
    "Roamio!",
    "by RMC Labs"
};

// Font paths (adjust these paths based on your font location)
const char* font_path = "../fonts/6x10.bdf";

// Structure to hold typewriter state
typedef struct {
    int current_line;
    int current_char;
    int animation_complete;
} TypewriterState;

// Global variables for cleanup
RGBMatrix *matrix = NULL;

// Signal handler for clean shutdown
void signal_handler(int signum) {
    if (matrix) {
        delete matrix;
    }
    exit(signum);
}

// Function to draw text line by line with typewriter effect
void draw_typewriter_text(RGBMatrix *canvas, Font *font, 
                         TypewriterState *state, 
                         int width, int height) {
    
    // Clear canvas
    canvas->Clear();
    
    // Text color
    Color color(255, 255, 255); // White text
    
    // Line positions and spacing
    int line_height = 10;
    int start_y = 8;
    
    // Draw each line up to current progress
    for (int line = 0; line <= state->current_line && line < NUM_LINES; line++) {
        int y_pos = start_y + (line * line_height);
        
        // Calculate how many characters to show for this line
        int chars_to_show;
        if (line < state->current_line) {
            // Previous lines: show all characters
            chars_to_show = strlen(text_lines[line]);
        } else if (line == state->current_line) {
            // Current line: show up to current character
            chars_to_show = state->current_char + 1;
        } else {
            // Future lines: show nothing
            chars_to_show = 0;
        }
        
        // Create substring to display
        if (chars_to_show > 0) {
            char display_text[256];
            int len = strlen(text_lines[line]);
            int actual_chars = chars_to_show > len ? len : chars_to_show;
            
            strncpy(display_text, text_lines[line], actual_chars);
            display_text[actual_chars] = '\0';
            
            // Center the text horizontally
            int text_width = strlen(display_text) * 6; // Approximate character width
            int x_pos = (width - text_width) / 2;
            if (x_pos < 0) x_pos = 0;
            
            // Draw the text
            DrawText(canvas, *font, x_pos, y_pos, color, NULL, display_text, 0);
        }
    }
}

// Function to update typewriter state
void update_typewriter_state(TypewriterState *state) {
    if (state->animation_complete) return;
    
    // Move to next character
    state->current_char++;
    
    // Check if we've finished the current line
    if (state->current_char >= (int)strlen(text_lines[state->current_line])) {
        // Move to next line
        state->current_line++;
        state->current_char = -1; // Will be incremented to 0 next time
        
        // Check if we've finished all lines
        if (state->current_line >= NUM_LINES) {
            state->animation_complete = 1;
        }
    }
}

int main(int argc, char **argv) {
    RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;
    
    // Set default options
    matrix_options.rows = 32;
    matrix_options.chain_length = 1;
    matrix_options.parallel = 1;
    matrix_options.hardware_mapping = "regular";

    // Parse command line options
    if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
        fprintf(stderr, "Failed to parse options\n");
        return 1;
    }

    // Create matrix
    matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == NULL) {
        fprintf(stderr, "Failed to create matrix\n");
        return 1;
    }

    // Setup signal handlers for clean exit
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    // Get matrix dimensions
    int width = matrix->width();
    int height = matrix->height();

    // Load font
    Font font;
    
    if (!font.LoadFont(font_path)) {
        fprintf(stderr, "Failed to load font from %s, trying alternatives...\n", font_path);
        // Try alternative font paths
        if (!font.LoadFont("../fonts/7x13.bdf") && 
            !font.LoadFont("../fonts/5x8.bdf") &&
            !font.LoadFont("../fonts/6x9.bdf")) {
            fprintf(stderr, "Could not load any font\n");
            delete matrix;
            return 1;
        }
    }

    // Initialize typewriter state
    TypewriterState state = {0, -1, 0};

    printf("Starting typewriter animation...\n");
    printf("Press Ctrl+C to stop\n");

    // Animation loop
    while (1) {
        // Update typewriter state
        update_typewriter_state(&state);
        
        // Draw current state
        draw_typewriter_text(matrix, &font, &state, width, height);
        
        // If animation is complete, restart after a pause
        if (state.animation_complete) {
            sleep(3); // Wait 3 seconds before restarting
            state.current_line = 0;
            state.current_char = -1;
            state.animation_complete = 0;
            printf("Restarting animation...\n");
        } else {
            // 0.1 second delay between characters
            usleep(100000); // 100,000 microseconds = 0.1 seconds
        }
    }

    // Cleanup (though we'll never reach here due to infinite loop)
    delete matrix;
    return 0;
} 