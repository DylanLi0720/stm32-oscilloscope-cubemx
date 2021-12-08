#include "dso.h"
#include "lcd.h"
#include "stdio.h"
#include "dso_display.h"

#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

void DSO_DrawLine_COLOR(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    POINT_COLOR = color;
    LCD_DrawLine(x1, y1, x2, y2);
}

void DSO_ShowString_COLOR(uint16_t x, uint16_t y, uint16_t width, uint8_t *p, uint16_t color)
{
    POINT_COLOR = color;
    LCD_ShowString(x, y, width, 16, 16, p);
}


void DSO_ShowGUI(void)
{
    static float temp = 0;

    DSO_ShowString_COLOR(240, 1, 100, "Trigged", YELLOW);

    DSO_GetFreq();
    DSO_DrawNetwork();
    DSO_DrawWare();
    //delay_ms(30);

    DSO_ShowString_COLOR(240, 1, 100, "Waiting", BLUE);

    POINT_COLOR = YELLOW;

    //显示频率
    LCD_ShowNum(145, 222, freq, 8, 16);

    //显示峰峰值
    sprintf((char*)showstr, "Vpp:%.2fV", Vpp);
    LCD_ShowString(240, 222, 200, 16, 16, showstr);

    if(key_sta == 1)
    {
        //显示时基-div
        temp = timebase / 100.0;

        if(temp < 0.1)
        {
            sprintf((char*)showstr, "%dus  ", timebase * 10);
        }
        else if(temp < 1.0)
        {
            sprintf((char*)showstr, "%.1fms  ", temp);
        }
        else
        {
            sprintf((char*)showstr, "%dms  ", (uint8_t)temp);
        }

        LCD_ShowString(74, 222, 200, 16, 16, showstr);

        //显示幅值-div
        sprintf((char*)showstr, "%.1fV ", 0.4 / avg);
        LCD_ShowString(10, 222, 50, 16, 16, showstr);

        //显示偏移
        LCD_ShowString(108, 2, 200, 16, 16, "[-----------]");

        if(key_num % KEYNUM == 1)
        {
            POINT_COLOR = WHITE;
        }

        LCD_ShowChar(155 + offset * 0.42, 2, '|', 16, 1);
        LCD_ShowChar(157 + offset * 0.42, 2, '|', 16, 1);


        //显示选择
        if(key_num % KEYNUM == 0)
        {
            POINT_COLOR = WHITE;
            LCD_DrawRectangle(70, 222, 116, 238);
        }
        else
        {
            POINT_COLOR = BLACK;
            LCD_DrawRectangle(70, 222, 116, 238);
        }

        if(key_num % KEYNUM == 2)
        {
            POINT_COLOR = WHITE;
            LCD_DrawRectangle(8, 222, 50, 238);
        }
        else
        {
            POINT_COLOR = BLACK;
            LCD_DrawRectangle(8, 222, 50, 238);
        }

        key_sta = 0;
    }
}


void DSO_DrawWare(void)//画波形图
{
    uint16_t Ypos1 = 0, Ypos2 = 0;
    static uint16_t n = 0;

    for(n = 0; n < lcddev.width; n++)
    {
        buff[n] = ADC_Value[offset + con_t + n];
    }

    DSO_GetVpp();//计算峰峰值

    buff[0] = Ypos1 = ((lcddev.height - 35) - (Yinit + buff[0] * 0.0397 * avg)); //转换坐标

    for(n = 1; n < (lcddev.width - 2); n++)
    {
        buff[n] = Ypos2 = ((lcddev.height - 35) - (Yinit + (double)(buff[n] * 0.0397 * avg))); //转换坐标


        if(Ypos2 >= 220) //限幅
        {
            buff[n] = Ypos2 = 219;
        }

        if(Ypos2 <= 20) //限幅
        {
            buff[n] = Ypos2 = 21;
        }

        DSO_DrawLine_COLOR(n, buff2[n], n + 1, buff2[n + 1], BLACK); //清除上一条线
        DSO_DrawLine_COLOR(n, Ypos1, n + 1, Ypos2, YELLOW);
        Ypos1 = Ypos2 ;
    }

    for(n = 1; n < lcddev.width; n++)
    {
        buff2[n] = buff[n - 1];
    }

}

void DSO_DrawNetwork(void)
{
    uint16_t y = 0;
    uint16_t x = 0;

    //画列点
    for(x = 20; x < lcddev.width; x += 20)
    {
        for(y = 20; y < (lcddev.height - 20); y += 5)
        {
            LCD_Fast_DrawPoint(x, y, 0XAAAA);
        }
    }

    //画行点
    for(y = 20; y < (lcddev.height - 20); y += 20)
    {
        for(x = 0 ; x < lcddev.width ; x += 5)
        {
            LCD_Fast_DrawPoint(x, y, 0xAAAA); //0X534c);
        }
    }

    DSO_DrawLine_COLOR(0, lcddev.height / 2, lcddev.width, lcddev.height / 2, 0X534c);
    DSO_DrawLine_COLOR(lcddev.width / 2, 20, lcddev.width / 2, (lcddev.height - 20), 0X534c);
    POINT_COLOR = 0X534c;
    LCD_DrawRectangle(0, 20, lcddev.width, (lcddev.height - 20)); //矩形
}

void DSO_SetBackGround(void)
{
    BACK_COLOR = BLACK;
    LCD_Display_Dir(1);  //显示方向
    LCD_Clear(BLACK);
    POINT_COLOR = YELLOW;
    DSO_ShowString_COLOR(212, 222, 16, "Hz", YELLOW);
}










