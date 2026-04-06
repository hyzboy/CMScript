#include "GameScript.h"
#include <cmath>

// --- GameLogic Implementation ---

void GameLogic::init() {
    score = 0;
    frame_count = 0;
}

void GameLogic::update(uint32_t delta_time) {
    frame_count += delta_time;
}

uint32_t GameLogic::get_score() const {
    return score;
}

void GameLogic::add_score(uint32_t points) {
    score += points;
}

void GameLogic::reset() {
    score = 0;
    frame_count = 0;
}

// --- Vector3 Implementation ---

void Vector3::set(float x_val, float y_val, float z_val) {
    x = x_val;
    y = y_val;
    z = z_val;
}

float Vector3::magnitude() const {
    return std::sqrt(x * x + y * y + z * z);
}

void Vector3::normalize() {
    float len = magnitude();
    if (len > 0.0f) {
        x /= len;
        y /= len;
        z /= len;
    }
}

float Vector3::get_x() const {
    return x;
}
