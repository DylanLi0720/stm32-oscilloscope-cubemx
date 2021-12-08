#ifndef  __STM32_WaveOutput_H
#define  __STM32_WaveOutput_H

#include "main.h"
#include "math.h"

#define PI  3.1415926
#define Vref 3.3		//0.1~3.3V可调
#define Um  (Vref)
#define N 256

//不需要的波形注释掉即可
#define  Sine_WaveOutput_Enable
#define  Triangular_WaveOutput_Enable

#define  SinWave		0x00	//正弦波
#define  TriangularWave   0x01	//锯齿波
#define  Wave_Channel_1   0x00	//通道1
#define  Wave_Channel_2   0x01	//通道2

//测试波形输出初始化
void TestWave_Init(void);

//1KHz方波初始化
void SquareWave_Init(void);
	
//输出波形初始化
//Wave1：波形1选择
//Wave1_Fre：波形1频率
//NewState1：波形1使能
//Wave2：波形2选择
//Wave2_Fre：波形2频率
//NewState2：波形2使能
void SineWave_Init(uint8_t Wave1,uint16_t Wave1_Fre,uint8_t NewState1,uint8_t Wave2,uint16_t Wave2_Fre,uint8_t NewState2);

//动态设置波形频率
//Wave_Channel：通道选择
//fre：频率
void Set_WaveFre( uint8_t Wave_Channel ,uint16_t fre);


#endif

