#ifndef __DSO_DISPLAY_H
#define __DSO_DISPLAY_H	 
#include "main.h"

void DSO_ShowString_COLOR(uint16_t x, uint16_t y,uint16_t width,uint8_t *p,uint16_t color);
void DSO_DrawLine_COLOR(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);
void DSO_ShowGUI(void);
void DSO_DrawWare(void);//»­²¨ÐÎÍ¼
void DSO_DrawNetwork(void);
void DSO_SetBackGround(void);

#endif
