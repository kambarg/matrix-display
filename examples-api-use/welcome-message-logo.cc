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

// Animation phases
typedef enum {
    PHASE_TEXT_ANIMATION,
    PHASE_LOGO_ANIMATION
} AnimationPhase;

// Structure to hold typewriter state
typedef struct {
    int current_line;
    int current_char;
    int animation_complete;
} TypewriterState;

// Structure to hold logo animation state
typedef struct {
    int blink_counter;
    int eyes_open;
    int display_cycles;
} LogoState;

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

// Function to draw the R logo with animated eyes
void draw_logo(RGBMatrix *canvas, LogoState *state, int width, int height) {
    // Clear canvas
    canvas->Clear();
    
    // Logo color (white)
    Color logo_color(255, 255, 255);
    
    // Calculate center position
    int center_x = width / 2;
    int center_y = height / 2;
    
    // Draw the RMC Labs R logo: semi-oval top + diagonal leg
    // Proportions: wider than tall (~2:1 ratio)
    
    // Top horizontal curve of the semi-oval
    for (int x = -10; x <= 10; x++) {
        canvas->SetPixel(center_x + x, center_y - 8, 
                        logo_color.r, logo_color.g, logo_color.b);
        canvas->SetPixel(center_x + x, center_y - 7, 
                        logo_color.r, logo_color.g, logo_color.b);
    }
    
    // Right side of semi-oval (curves down)
    for (int y = -6; y <= 0; y++) {
        canvas->SetPixel(center_x + 10, center_y + y, 
                        logo_color.r, logo_color.g, logo_color.b);
        canvas->SetPixel(center_x + 11, center_y + y, 
                        logo_color.r, logo_color.g, logo_color.b);
    }
    
    // Bottom horizontal part of semi-oval
    for (int x = -10; x <= 10; x++) {
        canvas->SetPixel(center_x + x, center_y + 1, 
                        logo_color.r, logo_color.g, logo_color.b);
        canvas->SetPixel(center_x + x, center_y + 2, 
                        logo_color.r, logo_color.g, logo_color.b);
    }
    
    // Diagonal line/leg extending down and to the right
    // This is the characteristic diagonal of the R
    for (int i = 0; i <= 10; i++) {
        int x = center_x - 4 + i;
        int y = center_y + 3 + i;
        canvas->SetPixel(x, y, logo_color.r, logo_color.g, logo_color.b);
        canvas->SetPixel(x + 1, y, logo_color.r, logo_color.g, logo_color.b);
    }
    
    // Draw the "eyes" - two circles
    // Only draw them if eyes are open (for blinking animation)
    if (state->eyes_open) {
        Color eye_color(255, 255, 255);
        
        // Left eye (circle in the semi-oval)
        int eye1_x = center_x - 5;
        int eye1_y = center_y - 3;
        
        // Draw a filled circle for left eye
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                canvas->SetPixel(eye1_x + dx, eye1_y + dy,
                               eye_color.r, eye_color.g, eye_color.b);
            }
        }
        
        // Right eye (circle in the semi-oval)
        int eye2_x = center_x + 5;
        int eye2_y = center_y - 3;
        
        // Draw a filled circle for right eye
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                canvas->SetPixel(eye2_x + dx, eye2_y + dy,
                               eye_color.r, eye_color.g, eye_color.b);
            }
        }
    }
}

// Function to update logo animation state (blinking eyes)
void update_logo_state(LogoState *state) {
    state->blink_counter++;
    
    // Blink pattern: eyes open for 20 frames, closed for 2 frames
    if (state->eyes_open) {
        if (state->blink_counter >= 20) {
            state->eyes_open = 0; // Close eyes
            state->blink_counter = 0;
        }
    } else {
        if (state->blink_counter >= 2) {
            state->eyes_open = 1; // Open eyes
            state->blink_counter = 0;
        }
    }
    
    state->display_cycles++;
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

    // Initialize animation states
    AnimationPhase current_phase = PHASE_TEXT_ANIMATION;
    TypewriterState text_state = {0, -1, 0};
    LogoState logo_state = {0, 1, 0}; // Start with eyes open

    printf("Starting animation sequence...\n");
    printf("Press Ctrl+C to stop\n");

    // Animation loop
    while (1) {
        if (current_phase == PHASE_TEXT_ANIMATION) {
            // Update typewriter state
            update_typewriter_state(&text_state);
            
            // Draw current state
            draw_typewriter_text(matrix, &font, &text_state, width, height);
            
            // If animation is complete, transition to logo phase
            if (text_state.animation_complete) {
                sleep(2); // Brief pause before showing logo
                current_phase = PHASE_LOGO_ANIMATION;
                logo_state.blink_counter = 0;
                logo_state.eyes_open = 1;
                logo_state.display_cycles = 0;
                printf("Switching to logo animation...\n");
            } else {
                // 0.1 second delay between characters
                usleep(100000); // 100,000 microseconds = 0.1 seconds
            }
        } 
        else if (current_phase == PHASE_LOGO_ANIMATION) {
            // Update logo animation state
            update_logo_state(&logo_state);
            
            // Draw logo with blinking eyes
            draw_logo(matrix, &logo_state, width, height);
            
            // Display logo for about 10 seconds (100 cycles at 0.1s each)
            if (logo_state.display_cycles >= 100) {
                sleep(1); // Brief pause before restarting
                current_phase = PHASE_TEXT_ANIMATION;
                text_state.current_line = 0;
                text_state.current_char = -1;
                text_state.animation_complete = 0;
                printf("Restarting animation sequence...\n");
            } else {
                // 0.1 second delay between frames
                usleep(100000); // 100,000 microseconds = 0.1 seconds
            }
        }
    }

    // Cleanup (though we'll never reach here due to infinite loop)
    delete matrix;
    return 0;
} 