#ifndef ANIMATION_ENGINE_H
#define ANIMATION_ENGINE_H

#include <stdint.h>

void Animation_Init(void);
void Animation_Update(uint32_t dt_ms);
void Animation_RenderLine(uint16_t y, uint16_t* lineBuffer);

#endif
