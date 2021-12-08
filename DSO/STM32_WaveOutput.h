#ifndef  __STM32_WaveOutput_H
#define  __STM32_WaveOutput_H

#include "main.h"
#include "math.h"

#define PI  3.1415926
#define Vref 3.3		//0.1~3.3V�ɵ�
#define Um  (Vref)
#define N 256

//����Ҫ�Ĳ���ע�͵�����
#define  Sine_WaveOutput_Enable
#define  Triangular_WaveOutput_Enable

#define  SinWave		0x00	//���Ҳ�
#define  TriangularWave   0x01	//��ݲ�
#define  Wave_Channel_1   0x00	//ͨ��1
#define  Wave_Channel_2   0x01	//ͨ��2

//���Բ��������ʼ��
void TestWave_Init(void);

//1KHz������ʼ��
void SquareWave_Init(void);
	
//������γ�ʼ��
//Wave1������1ѡ��
//Wave1_Fre������1Ƶ��
//NewState1������1ʹ��
//Wave2������2ѡ��
//Wave2_Fre������2Ƶ��
//NewState2������2ʹ��
void SineWave_Init(uint8_t Wave1,uint16_t Wave1_Fre,uint8_t NewState1,uint8_t Wave2,uint16_t Wave2_Fre,uint8_t NewState2);

//��̬���ò���Ƶ��
//Wave_Channel��ͨ��ѡ��
//fre��Ƶ��
void Set_WaveFre( uint8_t Wave_Channel ,uint16_t fre);


#endif

