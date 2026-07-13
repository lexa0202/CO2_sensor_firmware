#ifndef FACE_ANIMATION_H
#define FACE_ANIMATION_H

/******************************************************************************
 * face_animation.h
 *
 * Dashboard face animation subsystem.
 *
 * Renders a simple expressive face (eyes + mouth) using scanline primitives.
 * Designed for incremental extension: new expressions and gaze modes can be
 * added without changing the dashboard screen code.
 *
 ******************************************************************************/

#include <stdint.h>

#include "sensor_model.h"

/******************************************************************************
 * Types
 *****************************************************************************/

typedef enum
{
    FACE_EXPRESSION_SMILE,
    FACE_EXPRESSION_NEUTRAL,
    FACE_EXPRESSION_SAD

} FaceExpression_t;

typedef enum
{
    FACE_GAZE_TRACK_TARGET,
    FACE_GAZE_CENTER

} FaceGazeMode_t;

typedef struct
{
    int16_t originX;
    int16_t originY;

    uint16_t eyeColor;
    uint16_t pupilColor;
    uint16_t highlightColor;
    uint16_t mouthColor;

} FaceAnimationConfig_t;

typedef struct
{
    FaceGazeMode_t gazeMode;
    FaceExpression_t expression;

    int16_t gazeTargetX;
    int16_t gazeTargetY;

} FaceAnimationState_t;

typedef struct
{
    SensorCO2_Status_t co2Status;

    int16_t markerX;
    int16_t markerY;

} FaceAnimationInput_t;

/******************************************************************************
 * Public functions
 *****************************************************************************/

void FaceAnimation_Init(void);

const FaceAnimationState_t* FaceAnimation_GetState(void);

void FaceAnimation_Update(
    const FaceAnimationInput_t* input
);

void FaceAnimation_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    const FaceAnimationConfig_t* config
);

#endif /* FACE_ANIMATION_H */
