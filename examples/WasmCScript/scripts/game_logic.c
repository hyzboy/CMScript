/**
 * Game script logic - compiled to WebAssembly
 * This code runs inside the WAMR virtual machine
 * 
 * The WASM script can call native functions provided by the host
 * via WAMR's native symbol registration mechanism.
 */

/* Game state (persistent during execution) */
static unsigned int g_score = 0;
static unsigned int g_frame_count = 0;

/**
 * Initialize the game script
 * Called once at startup
 */
void script_init(void) {
    g_score = 0;
    g_frame_count = 0;
}

/**
 * Update the game logic each frame
 * @param delta_time Time elapsed since last frame (milliseconds)
 */
void script_update(unsigned int delta_time) {
    g_frame_count++;
    
    /* Example: increment score every 10 frames */
    if (g_frame_count % 10 == 0) {
        g_score += 10;
    }
}

/**
 * Get current score
 * @return Current score value
 */
unsigned int script_get_score(void) {
    return g_score;
}

/**
 * Add points to score
 * @param points Points to add
 */
void script_add_score(unsigned int points) {
    g_score += points;
}

/**
 * Reset game state
 */
void script_reset(void) {
    g_score = 0;
    g_frame_count = 0;
}
