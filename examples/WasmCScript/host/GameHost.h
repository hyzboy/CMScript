#pragma once

#include <cstdint>
#include "hgl/Reflect.h"

/**
 * Host-side game interface
 * These classes are exposed to WASM scripts via native symbol registration
 *
 * Macro annotations are automatically detected by clang AST parser:
 * - H_CLASS: marks a class for reflection
 * - H_FUNCTION: marks a method for native export
 * - H_PROPERTY: marks a member variable for documentation
 */

class H_CLASS GameHost {
public:
    /**
     * Initialize the host game system
     */
    H_FUNCTION void init();

    /**
     * Shutdown cleanly
     */
    H_FUNCTION void shutdown();

    /**
     * Get the frame rate (frames per second)
     * Scripts can query this to adjust behavior
     */
    H_FUNCTION uint32_t get_fps() const;

    /**
     * Set the frame rate
     * @param fps Desired frames per second
     */
    H_FUNCTION void set_fps(uint32_t fps);

    /**
     * Log a message to the host console
     * Useful for script debugging
     * @param message Message to log
     */
    H_FUNCTION void log_message(const char* message);

    /**
     * Trigger an event in the host
     * @param event_id Event identifier
     * @param value Associated event value
     */
    H_FUNCTION void trigger_event(uint32_t event_id, uint32_t value);

private:
    H_PROPERTY uint32_t fps;
    H_PROPERTY uint32_t frame_counter;
};


class H_CLASS InputHandler {
public:
    /**
     * Check if a key is pressed
     * @param key_code Key code to check
     * @return True if key is currently pressed
     */
    H_FUNCTION bool is_key_pressed(uint32_t key_code) const;

    /**
     * Get mouse X position
     */
    H_FUNCTION int32_t get_mouse_x() const;

    /**
     * Get mouse Y position
     */
    H_FUNCTION int32_t get_mouse_y() const;

    /**
     * Check if mouse is clicked
     */
    H_FUNCTION bool is_mouse_clicked() const;

private:
    H_PROPERTY int32_t mouse_x;
    H_PROPERTY int32_t mouse_y;
    H_PROPERTY bool mouse_pressed;
};


class H_CLASS AudioSystem {
public:
    /**
     * Play a sound effect
     * @param sound_id ID of the sound to play
     * @param volume Volume (0-100)
     */
    H_FUNCTION void play_sound(uint32_t sound_id, uint32_t volume);

    /**
     * Stop all sounds
     */
    H_FUNCTION void stop_all_sounds();

    /**
     * Set master volume
     * @param volume Master volume (0-100)
     */
    H_FUNCTION void set_master_volume(uint32_t volume);

private:
    H_PROPERTY uint32_t master_volume;
};
