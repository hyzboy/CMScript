#include "GameHost.h"
#include <cstdio>

// --- GameHost Implementation ---

void GameHost::init() {
    fps = 60;
    frame_counter = 0;
}

void GameHost::shutdown() {
    /* Cleanup resources */
}

uint32_t GameHost::get_fps() const {
    return fps;
}

void GameHost::set_fps(uint32_t new_fps) {
    fps = new_fps;
}

void GameHost::log_message(const char* message) {
    if (message) {
        printf("[WASM Script] %s\n", message);
    }
}

void GameHost::trigger_event(uint32_t event_id, uint32_t value) {
    printf("[Event] ID=%u, Value=%u\n", event_id, value);
}


// --- InputHandler Implementation ---

bool InputHandler::is_key_pressed(uint32_t key_code) const {
    /* This would connect to actual input system */
    return false;
}

int32_t InputHandler::get_mouse_x() const {
    return mouse_x;
}

int32_t InputHandler::get_mouse_y() const {
    return mouse_y;
}

bool InputHandler::is_mouse_clicked() const {
    return mouse_pressed;
}


// --- AudioSystem Implementation ---

void AudioSystem::play_sound(uint32_t sound_id, uint32_t volume) {
    if (volume > 100) volume = 100;
    printf("[Audio] Playing sound %u at volume %u\n", sound_id, volume);
}

void AudioSystem::stop_all_sounds() {
    printf("[Audio] Stopping all sounds\n");
}

void AudioSystem::set_master_volume(uint32_t volume) {
    if (volume > 100) volume = 100;
    master_volume = volume;
    printf("[Audio] Master volume set to %u\n", volume);
}
