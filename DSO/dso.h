#ifndef __DSO_H
#define __DSO_H	 
#include "main.h"

#define INCREASE 1
#define REDUCE 0
#define KEYNUM 3

#define TIMEBASENUM 11

//ȫ�ֱ���
extern uint8_t run_sta; //ʾ��������״ָ̬ʾ
extern uint8_t key_num;//����ѡ��
extern uint8_t key_sta;

extern uint8_t avg;
extern uint16_t timN;
extern uint32_t timebase;//ʱ���׼ָʾ
extern uint32_t max_data; //������ƽ
extern uint32_t freq; //Ƶ�����
extern uint16_t Yinit;  //��ͼ���ݲ���

extern uint16_t con_t;
extern uint16_t buff[600];  //�������ݴ洢
extern uint16_t buff2[600];  //�������ݴ洢
extern uint16_t ADC_Value[900];  //AD�ɼ�����
extern short offset;  //ƫ��

extern uint16_t NCollect;  
extern float Vpp;
extern uint8_t showstr[20];


void DSO_Init(void);
void DSO_TIMER_Init(void);
void DSO_ADC_Init(void);
void DSO_DMA_Init(void);
void DSO_GetFreq(void);//Ƶ�ʼ�����½��ش���
void DSO_GetVpp(void);
void DSO_SetTimeBase(uint8_t t);
void DSO_OffsetIncrement(uint8_t inc);
void DSO_AvgIncrement(uint8_t inc);

#endif
