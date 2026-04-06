#pragma once

#include <cstdint>
#include <cstring>

/**
 * Reflectable game script C++ class annotated with WAMR annotations
 * These annotations enable automatic WAMR native symbol registration
 */

/*! WAMR_CLASS */
class GameLogic {
public:
    /**
     * Initialize the game logic
     */
    /*! WAMR_FUNCTION */
    void init();

    /**
     * Update game state
     * @param delta_time Time elapsed since last frame (in milliseconds)
     */
    /*! WAMR_FUNCTION */
    void update(uint32_t delta_time);

    /**
     * Get the current score
     * @return Current score value
     */
    /*! WAMR_FUNCTION */
    uint32_t get_score() const;

    /**
     * Add points to the score
     * @param points Number of points to add
     */
    /*! WAMR_FUNCTION */
    void add_score(uint32_t points);

    /**
     * Reset game state
     */
    /*! WAMR_FUNCTION */
    void reset();

private:
    uint32_t score;        /*! WAMR_PROPERTY */
    uint32_t frame_count;  /*! WAMR_PROPERTY */
};


/*! WAMR_CLASS */
class Vector3 {
public:
    /**
     * Initialize vector with x, y, z values
     */
    /*! WAMR_FUNCTION */
    void set(float x, float y, float z);

    /**
     * Calculate the magnitude (length) of the vector
     * @return Length of the vector
     */
    /*! WAMR_FUNCTION */
    float magnitude() const;

    /**
     * Normalize the vector
     */
    /*! WAMR_FUNCTION */
    void normalize();

    /**
     * Get the X component
     * @return X coordinate
     */
    /*! WAMR_FUNCTION */
    float get_x() const;

private:
    float x;  /*! WAMR_PROPERTY */
    float y;  /*! WAMR_PROPERTY */
    float z;  /*! WAMR_PROPERTY */
};
