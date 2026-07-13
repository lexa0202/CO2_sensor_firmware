#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

/******************************************************************************
 * screen_manager.h
 *
 * Display orchestration and screen state machine.
 *
 ******************************************************************************/

/******************************************************************************
 * Types
 *****************************************************************************/

/*
 * Screen manager states.
 */
typedef enum
{
    SCREEN_BOOT,
    SCREEN_DRAWING_RAW,
    SCREEN_DASHBOARD,
    SCREEN_ANIMATING,
    SCREEN_USB

} ScreenState_t;

/******************************************************************************
 * Public functions
 *****************************************************************************/

/*
 * Initialize display subsystem.
 */
void Screen_Init(void);

/*
 * Main screen processing task.
 */
void Screen_Process(void);

/*
 * Show boot splash screen.
 */
void Screen_ShowBoot(void);

/*
 * Show USB Mass Storage screen.
 */
void Screen_ShowUsb(void);

/*
 * Fill display with black color.
 */
void Screen_Black(void);

/*
 * Return to default application screen.
 */
void Screen_ShowDefault(void);

#endif /* SCREEN_MANAGER_H */
