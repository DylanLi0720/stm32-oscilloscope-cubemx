#ifndef __DSO_H
#define __DSO_H	 
#include "main.h"

#define INCREASE 1
#define REDUCE 0
#define KEYNUM 3

#define TIMEBASENUM 11

//全局变量
extern uint8_t run_sta; //示波器运行状态指示
extern uint8_t key_num;//按键选择
extern uint8_t key_sta;

extern uint8_t avg;
extern uint16_t timN;
extern uint32_t timebase;//时间基准指示
extern uint32_t max_data; //触发电平
extern uint32_t freq; //频率输出
extern uint16_t Yinit;  //画图数据参数

extern uint16_t con_t;
extern uint16_t buff[600];  //波形数据存储
extern uint16_t buff2[600];  //波形数据存储
extern uint16_t ADC_Value[900];  //AD采集数据
extern short offset;  //偏移

extern uint16_t NCollect;  
extern float Vpp;
extern uint8_t showstr[20];


void DSO_Init(void);
void DSO_TIMER_Init(void);
void DSO_ADC_Init(void);
void DSO_DMA_Init(void);
void DSO_GetFreq(void);//频率计算和下降沿触发
void DSO_GetVpp(void);
void DSO_SetTimeBase(uint8_t t);
void DSO_OffsetIncrement(uint8_t inc);
void DSO_AvgIncrement(uint8_t inc);

#endif
