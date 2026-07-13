/******************************************************************************
 * face_animation.c
 *
 * Dashboard face animation subsystem.
 *
 * Renders expressive eyes and mouth using scanline primitives.
 *
 ******************************************************************************/

#include "face_animation.h"

#include <math.h>

/******************************************************************************
 * Configuration
 *****************************************************************************/

#define FACE_EYE_RADIUS           11
#define FACE_PUPIL_RADIUS          5
#define FACE_HIGHLIGHT_RADIUS      2

#define FACE_EYE_SPACING_X        28
#define FACE_EYE_OFFSET_Y          4
#define FACE_PUPIL_MAX_OFFSET      4

#define FACE_MOUTH_OFFSET_Y        36
#define FACE_MOUTH_RADIUS          16
#define FACE_MOUTH_THICKNESS        2

#define FACE_SAD_MOUTH_DROP         4

/******************************************************************************
 * Static data
 *****************************************************************************/

static FaceAnimationState_t faceState =
{
    .gazeMode = FACE_GAZE_CENTER,
    .expression = FACE_EXPRESSION_SMILE,
    .gazeTargetX = 0,
    .gazeTargetY = 0
};

/******************************************************************************
 * Private helpers
 *****************************************************************************/

static void FaceAnimation_SetGazeTarget(
    int16_t targetX,
    int16_t targetY
)
{
    faceState.gazeTargetX = targetX;
    faceState.gazeTargetY = targetY;
}

static void FaceAnimation_ComputePupilOffset(
    int16_t eyeX,
    int16_t eyeY,
    int8_t* offsetX,
    int8_t* offsetY
)
{
    if(faceState.gazeMode == FACE_GAZE_CENTER)
    {
        *offsetX = 0;
        *offsetY = 0;
        return;
    }

    const float dx =
        (float)(faceState.gazeTargetX - eyeX);

    const float dy =
        (float)(faceState.gazeTargetY - eyeY);

    const float dist =
        sqrtf(dx * dx + dy * dy);

    if(dist < 0.5f)
    {
        *offsetX = 0;
        *offsetY = 0;
        return;
    }

    const float scale =
        (float)FACE_PUPIL_MAX_OFFSET /
        dist;

    *offsetX =
        (int8_t)(dx * scale);

    *offsetY =
        (int8_t)(dy * scale);
}

static void FaceAnimation_DrawFilledDisc(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    int16_t centerX,
    int16_t centerY,
    int16_t radius,
    uint16_t color
)
{
    const int16_t dy =
        (int16_t)y - centerY;

    if(dy < -radius ||
       dy > radius)
    {
        return;
    }

    const int32_t dy2 =
        (int32_t)dy * dy;

    const int32_t radius2 =
        (int32_t)radius * radius;

    const int16_t span =
        (int16_t)sqrtf(
            (float)(radius2 - dy2)
        );

    const int16_t xStart =
        centerX - span;

    const int16_t xEnd =
        centerX + span;

    for(int16_t x = xStart;
        x <= xEnd;
        x++)
    {
        if(x >= 0 &&
           x < (int16_t)width)
        {
            lineBuffer[x] = color;
        }
    }
}

static void FaceAnimation_DrawRingStroke(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    int16_t centerX,
    int16_t centerY,
    int16_t radius,
    int16_t thickness,
    uint16_t color
)
{
    const int16_t dy =
        (int16_t)y - centerY;

    const int32_t outerR =
        radius + thickness;

    if(dy < -outerR ||
       dy > outerR)
    {
        return;
    }

    const int32_t dy2 =
        (int32_t)dy * dy;

    const int32_t outerR2 =
        (int32_t)outerR * outerR;

    const int32_t innerR2 =
        (int32_t)radius * radius;

    const int16_t outerSpan =
        (int16_t)sqrtf(
            (float)(outerR2 - dy2)
        );

    const int16_t innerSpan =
        (dy2 <= innerR2) ?
        (int16_t)sqrtf(
            (float)(innerR2 - dy2)
        ) :
        0;

    for(int16_t x = centerX - outerSpan;
        x <= centerX + outerSpan;
        x++)
    {
        if(x < centerX - innerSpan ||
           x > centerX + innerSpan)
        {
            if(x >= 0 &&
               x < (int16_t)width)
            {
                lineBuffer[x] = color;
            }
        }
    }
}

static void FaceAnimation_DrawEye(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    int16_t eyeX,
    int16_t eyeY,
    const FaceAnimationConfig_t* config
)
{
    int8_t pupilOffsetX = 0;
    int8_t pupilOffsetY = 0;

    FaceAnimation_ComputePupilOffset(
        eyeX,
        eyeY,
        &pupilOffsetX,
        &pupilOffsetY
    );

    FaceAnimation_DrawFilledDisc(
        y,
        lineBuffer,
        width,
        eyeX,
        eyeY,
        FACE_EYE_RADIUS,
        config->eyeColor
    );

    const int16_t pupilX =
        eyeX + pupilOffsetX;

    const int16_t pupilY =
        eyeY + pupilOffsetY;

    FaceAnimation_DrawFilledDisc(
        y,
        lineBuffer,
        width,
        pupilX,
        pupilY,
        FACE_PUPIL_RADIUS,
        config->pupilColor
    );

    FaceAnimation_DrawFilledDisc(
        y,
        lineBuffer,
        width,
        pupilX + 1,
        pupilY - 1,
        FACE_HIGHLIGHT_RADIUS,
        config->highlightColor
    );
}

static void FaceAnimation_DrawArcStroke(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    int16_t centerX,
    int16_t centerY,
    int16_t radius,
    int16_t thickness,
    uint16_t color,
    uint8_t lowerHalf
)
{
    if(lowerHalf != 0U)
    {
        if(y < centerY)
        {
            return;
        }
    }
    else
    {
        if(y > centerY)
        {
            return;
        }
    }

    FaceAnimation_DrawRingStroke(
        y,
        lineBuffer,
        width,
        centerX,
        centerY,
        radius,
        thickness,
        color
    );
}

static void FaceAnimation_DrawMouth(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    const FaceAnimationConfig_t* config
)
{
    const int16_t mouthCenterX =
        config->originX;

    const int16_t mouthBaseY =
        config->originY +
        FACE_MOUTH_OFFSET_Y;

    const int16_t mouthRadius =
        FACE_MOUTH_RADIUS;

    if(faceState.expression == FACE_EXPRESSION_SMILE)
    {
        const int16_t arcCenterY =
            mouthBaseY -
            mouthRadius;

        FaceAnimation_DrawArcStroke(
            y,
            lineBuffer,
            width,
            mouthCenterX,
            arcCenterY,
            mouthRadius,
            FACE_MOUTH_THICKNESS,
            config->mouthColor,
            1U
        );
        return;
    }

    if(faceState.expression == FACE_EXPRESSION_SAD)
    {
        const int16_t arcCenterY =
            mouthBaseY +
            mouthRadius -
            FACE_SAD_MOUTH_DROP;

        FaceAnimation_DrawArcStroke(
            y,
            lineBuffer,
            width,
            mouthCenterX,
            arcCenterY,
            mouthRadius,
            FACE_MOUTH_THICKNESS,
            config->mouthColor,
            0U
        );
        return;
    }

    FaceAnimation_DrawFilledDisc(
        y,
        lineBuffer,
        width,
        mouthCenterX,
        mouthBaseY,
        2,
        config->mouthColor
    );
}

/******************************************************************************
 * Public functions
 *****************************************************************************/

void FaceAnimation_Init(void)
{
    faceState.gazeMode = FACE_GAZE_CENTER;
    faceState.expression = FACE_EXPRESSION_SMILE;
    faceState.gazeTargetX = 0;
    faceState.gazeTargetY = 0;
}

const FaceAnimationState_t* FaceAnimation_GetState(void)
{
    return &faceState;
}

void FaceAnimation_Update(
    const FaceAnimationInput_t* input
)
{
    switch(input->co2Status)
    {
    case SENSOR_CO2_STATUS_WARMUP:

        faceState.gazeMode =
            FACE_GAZE_TRACK_TARGET;

        faceState.expression =
            FACE_EXPRESSION_SMILE;

        FaceAnimation_SetGazeTarget(
            input->markerX,
            input->markerY
        );

        break;

    case SENSOR_CO2_STATUS_OK:

        faceState.gazeMode =
            FACE_GAZE_CENTER;

        faceState.expression =
            FACE_EXPRESSION_SMILE;

        break;

    default:

        faceState.gazeMode =
            FACE_GAZE_CENTER;

        faceState.expression =
            FACE_EXPRESSION_NEUTRAL;

        break;
    }
}

void FaceAnimation_RenderLine(
    uint16_t y,
    uint16_t* lineBuffer,
    uint16_t width,
    const FaceAnimationConfig_t* config
)
{
    const int16_t leftEyeX =
        config->originX -
        FACE_EYE_SPACING_X;

    const int16_t rightEyeX =
        config->originX +
        FACE_EYE_SPACING_X;

    const int16_t eyeY =
        config->originY +
        FACE_EYE_OFFSET_Y;

    FaceAnimation_DrawEye(
        y,
        lineBuffer,
        width,
        leftEyeX,
        eyeY,
        config
    );

    FaceAnimation_DrawEye(
        y,
        lineBuffer,
        width,
        rightEyeX,
        eyeY,
        config
    );

    FaceAnimation_DrawMouth(
        y,
        lineBuffer,
        width,
        config
    );
}
