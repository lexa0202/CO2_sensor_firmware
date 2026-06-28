#ifndef ANIMATION_ENGINE_H
#define ANIMATION_ENGINE_H

/******************************************************************************
 * animation_engine.h
 *
 * Simple animation framework.
 *
 * Responsible for:
 * - animation timing
 * - animation state updates
 * - frame rendering
 *
 * Current implementation is a placeholder and will be replaced
 * by dashboard character animations.
 *
 ******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdint.h>

/******************************************************************************
 * Public functions
 *****************************************************************************/

/*
 * Initialize animation subsystem.
 */
void Animation_Init(void);

/*
 * Update animation state.
 *
 * dt_ms - elapsed time in milliseconds.
 */
void Animation_Update(
    uint32_t dt_ms
);

/*
 * Render one display line.
 */
void Animation_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer
);

#endif /* ANIMATION_ENGINE_H */
