#include "STM32_WaveOutput.h"


//���Ҳ�����
void SineWave_Data( uint16_t cycle, uint16_t *D)
{
    uint16_t i;

    for(i = 0; i < cycle; i++)
    {
        D[i] = (uint16_t)((sin((1.0 * i / cycle) * 2 * PI) + 1.0) * 2048); //(uint16_t)((sin((1.0*i/(cycle))*2*PI)+1.0)*4095/2.0);//
    }
}

//���ǲ�����
void Triangular_Data( uint16_t cycle, uint16_t *D)
{
    uint16_t i;

    for(i = 0; i < cycle; i++)
    {
        if(i < cycle / 2)
        {
            D[i] = (uint16_t)(1.0 * i / 127 * 4095);
        }
        else
        {
            D[i] = D[255 - i];
        }
    }
}

//���Ҳ��α�
#ifdef Sine_WaveOutput_Enable
    uint16_t SineWave_Value[256];
#endif

//��ݲ��α�
#ifdef Triangular_WaveOutput_Enable
    uint16_t Triangular_Value[256];
#endif

//DAC�Ĵ�����ַ����
#define DAC_DHR12R1 (uint32_t)&(DAC->DHR12R1)   //DACͨ��1����Ĵ�����ַ
#define DAC_DHR12R2 (uint32_t)&(DAC->DHR12R2)   //DACͨ��2����Ĵ�����ַ

//���ų�ʼ��
void SineWave_GPIO_Config(uint8_t NewState1, uint8_t NewState2)
{
    RCC->APB2ENR |= 1 << 2; //ʹ��PORTAʱ��

    if( NewState1 != DISABLE)
    {
        GPIOA->CRL &= 0xFFF0FFFF;
        GPIOA->CRL |= 0x00030000;
        GPIOA->ODR |= (1 << 4);
    }

    if( NewState2 != DISABLE)
    {
        GPIOA->CRL &= 0xFF0FFFFF;
        GPIOA->CRL |= 0x00300000;
        GPIOA->ODR |= (1 << 5);
    }
}

//DAC��ʼ��
void SineWave_DAC_Config(uint8_t NewState1, uint8_t NewState2)
{
    RCC->APB2ENR |= 1 << 2; //ʹ��PORTAʱ��
    RCC->APB1ENR |= 1 << 29; //ʹ��DACʱ��

//	GPIOA->CRL&=0XFFF0FFFF;
//	GPIOA->CRL|=0X00000000;

    if( NewState1 != DISABLE)
    {
        DAC->CR |= 1 << 0;	//ʹ��DACͨ��1
        DAC->CR |= 1 << 1;	//DAC1������治ʹ�� BOFF1=1
        DAC->CR |= 1 << 2;	//ʹ�ô������� TEN1=1

        DAC->CR |= 0 << 3;	//3��4��5=100ʱΪTIM2 TRGO�¼�����
        DAC->CR |= 0 << 4;	//
        DAC->CR |= 1 << 5;	//

        DAC->CR |= 0 << 6;	//��ʹ�ò��η���
        DAC->CR |= 0 << 8;	//���Ρ���ֵ����
        DAC->CR |= 1 << 12;	//DAC1 DMAʹ��
    }

    if( NewState2 != DISABLE)
    {
        DAC->CR |= 1 << 16;	//ʹ��DACͨ��2
        DAC->CR |= 1 << 17;	//DAC2������治ʹ�� BOFF1=1
        DAC->CR |= 1 << 18;	//ʹ�ô������� TEN2=1

        DAC->CR |= 0 << 19;	//3��4��5=100ʱΪTIM2 TRGO�¼�����
        DAC->CR |= 0 << 20;	//
        DAC->CR |= 0 << 21;	//

        DAC->CR |= 0 << 22;	//��ʹ�ò��η���
        DAC->CR |= 0 << 24;	//���Ρ���ֵ����
        DAC->CR |= 1 << 28;	//DAC1 DMAʹ��

        DAC->DHR12R1 = 0; //ʹ��ͨ��1
        DAC->DHR12R2 = 0; //ʹ��ͨ��2
    }
}

//��ʱ������
void SineWave_TIM_Config( uint32_t Wave1_Fre, uint8_t NewState1, uint32_t Wave2_Fre, uint8_t NewState2)
{
    if(NewState1 != DISABLE)RCC->APB1ENR |= 1 << 0;	//TIM2ʱ��ʹ��

    if(NewState2 != DISABLE)RCC->APB1ENR |= 1 << 4;	//TIM6ʱ��ʹ��

    TIM2->PSC = 0x0; //Ԥ��Ƶ������Ƶ
    TIM2->CR1 |= 0 << 4; //���ϼ���ģʽ
    TIM6->PSC = 0x0;
    TIM6->CR1 |= 0 << 4; //���ϼ���ģʽ

    if( NewState1 != DISABLE)
    {
        TIM2->ARR = Wave1_Fre - 1;
        TIM2->CR1 |= 0x01;  //ʹ�ܶ�ʱ��6
        TIM2->CR2 &= (uint16_t)~((uint16_t)0x0070);//����TIM2�������Ϊ����ģʽ
        TIM2->CR2 |= 0x0020; //����TIM2�������Ϊ����ģʽ
    }

    if( NewState2 != DISABLE)
    {
        TIM6->ARR = Wave2_Fre - 10;
        TIM6->CR1 |= 0x01;  //ʹ�ܶ�ʱ��2
        TIM6->CR2 &= (uint16_t)~((uint16_t)0x0070);//����TIM6�������Ϊ����ģʽ
        TIM6->CR2 |= 0x0020; //����TIM6�������Ϊ����ģʽ
    }
}

//DMA����
void SineWave_DMA_Config( uint16_t *Wave1_Mem, uint8_t NewState1, uint16_t *Wave2_Mem, uint8_t NewState2)
{
    RCC->AHBENR |= 1 << 1;			//����DMA2ʱ��
    HAL_Delay(5);				//�ȴ�DMAʱ���ȶ�

    if(NewState1 != DISABLE)
    {
        DMA2_Channel3->CPAR = DAC_DHR12R1; 	 	//DMA1 �����ַ
        DMA2_Channel3->CMAR = (uint32_t)Wave1_Mem; 	//DMA1,�洢����ַ
        DMA2_Channel3->CNDTR = 256;    	//DMA2,����������
        DMA2_Channel3->CCR = 0X00000000;	//��λ
        DMA2_Channel3->CCR |= 1 << 4;  		//�Ӵ洢����
        DMA2_Channel3->CCR |= 0 << 6; 		//�����ַ������ģʽ
        DMA2_Channel3->CCR |= 1 << 7; 	 	//�洢������ģʽ
        DMA2_Channel3->CCR |= 1 << 8; 	 	//�������ݿ��Ϊ16λ
        DMA2_Channel3->CCR |= 1 << 10; 		//�洢�����ݿ��16λ
        DMA2_Channel3->CCR |= 1 << 12; 		//������ȼ�
        DMA2_Channel3->CCR |= 1 << 13; 		//������ȼ�
        DMA2_Channel3->CCR |= 0 << 14; 		//�Ǵ洢�����洢��ģʽ
        DMA2_Channel3->CCR |= 1 << 5;  		//ѭ������ģʽ
        DMA2_Channel3->CCR |= 1 << 0;      //����DMA����
    }

    if(NewState2 != DISABLE)
    {
        DMA2_Channel4->CPAR = DAC_DHR12R2; 	 	//DMA1 �����ַ
        DMA2_Channel4->CMAR = (uint32_t)Wave2_Mem; 	//DMA1,�洢����ַ
        DMA2_Channel4->CNDTR = 256;    	//DMA2,����������
        DMA2_Channel4->CCR = 0X00000000;	//��λ
        DMA2_Channel4->CCR |= 1 << 4;  		//�Ӵ洢����
        DMA2_Channel4->CCR |= 0 << 6; 		//�����ַ������ģʽ
        DMA2_Channel4->CCR |= 1 << 7; 	 	//�洢������ģʽ
        DMA2_Channel4->CCR |= 1 << 8; 	 	//�������ݿ��Ϊ16λ
        DMA2_Channel4->CCR |= 1 << 10; 		//�洢�����ݿ��16λ
        DMA2_Channel4->CCR |= 1 << 12; 		//������ȼ�
        DMA2_Channel4->CCR |= 1 << 13; 		//������ȼ�
        DMA2_Channel4->CCR |= 0 << 14; 		//�Ǵ洢�����洢��ģʽ
        DMA2_Channel4->CCR |= 1 << 5;  		//ѭ������ģʽ
        DMA2_Channel4->CCR |= 1 << 0;      //����DMA����
    }
}


void MyTIM_SetAutoreload(TIM_TypeDef* TIMx, uint16_t Autoreload)
{
    /* Check the parameters */
//  assert_param(IS_TIM_ALL_PERIPH(TIMx));
    /* Set the Autoreload Register value */
    TIMx->ARR = Autoreload;
}

//*****������γ�ʼ��***************/
//uint8_t Wave1 ����1
//uint16_t Wave1_Fre ����1Ƶ��(Hz)
//uint8_t NewState1 ����1״̬�����ԣ�
//uint8_t Wave2 ����2
//uint16_t Wave2_Fre ����2Ƶ��(Hz)
//uint8_t NewState2 ����2״̬�����ԣ�
//����exp��  SineWave_Init( SawToothWave ,10 ,ENABLE ,SinWave ,10 ,ENABLE);//���ǲ���10Hz�����Ҳ���10Hz
void SineWave_Init(uint8_t Wave1, uint16_t Wave1_Fre, uint8_t NewState1, uint8_t Wave2, uint16_t Wave2_Fre, uint8_t NewState2)
{
    uint16_t *add1, *add2;
    uint16_t f1 = (uint16_t)(72000000 / sizeof(SineWave_Value) * 2 / Wave1_Fre);
    uint16_t f2 = (uint16_t)(72000000 / sizeof(SineWave_Value) * 2 / Wave2_Fre);
    SineWave_Data( N, SineWave_Value);		//���ɲ��α�1
    Triangular_Data( N, Triangular_Value);//���ɲ��α�2

    if(NewState1 != DISABLE)
    {
        if( Wave1 == 0x00) add1 = SineWave_Value;
        else add1 = Triangular_Value;
    }

    if( NewState2 != DISABLE)
    {
        if(Wave2 == 0x00) add2 = SineWave_Value;
        else add2 = Triangular_Value;
    }

    SineWave_GPIO_Config(ENABLE, ENABLE);			 //��ʼ������
    SineWave_TIM_Config(f1, NewState1, f2, NewState2);			 //��ʼ����ʱ��
    SineWave_DAC_Config(NewState1, NewState2);			 //��ʼ��DAC
    SineWave_DMA_Config(add1, NewState1, add2, NewState2);			 //��ʼ��DMA

    if(NewState1 != DISABLE) TIM2->CR1 |= 0x01; //ʹ�ܶ�ʱ��2;			 //ʹ��TIM2,��ʼ��������

    if(NewState2 != DISABLE) TIM6->CR1 |= 0x01; //ʹ�ܶ�ʱ��2;			 //ʹ��TIM6,��ʼ��������
}

void Set_WaveFre( uint8_t Wave_Channel, uint16_t fre)
{
    TIM_TypeDef* TIMX;
    uint16_t reload;

    if( Wave_Channel == 0x00) TIMX = TIM2;
    else if(Wave_Channel == 0x01) TIMX = TIM6;

    reload = (uint16_t)(72000000 / 512 / fre);
    MyTIM_SetAutoreload(TIMX, reload);
}

void SquareWave_Init(void)
{
    RCC->APB2ENR |= 1 << 0; 	//AFIOʹ��
    RCC->APB2ENR |= 1 << 2; //ʹ��PORTAʱ��
    RCC->APB1ENR |= 1 << 3; 	//TIM5ʱ��ʹ��

    GPIOA->CRL &= 0XFFFF0FFF;	//PA3���֮ǰ������
    GPIOA->CRL |= 0X0000B000;	//���ù������
    GPIOA->ODR |= 1 << 3;   //PA3 �����

    TIM5->ARR = 999;			//�趨�������Զ���װֵ
    TIM5->PSC = 71;			//Ԥ��Ƶ������

    TIM5->CCMR2 |= 7 << 12;  	//CH4 PWM2ģʽ
    TIM5->CCMR2 |= 1 << 11; 	//CH4Ԥװ��ʹ��
    TIM5->CCER |= 1 << 12;   	//OC4 ���ʹ��
    TIM5->BDTR |= 1 << 15;   	//MOE �����ʹ��

    TIM5->CR1 = 0x0080;   	//ARPEʹ��
    TIM5->CR1 |= 0x01;    	//ʹ�ܶ�ʱ��5

    TIM5->CCR4 = 499;
}

void TestWave_Init(void)
{
    SineWave_Init(TriangularWave, 1000, ENABLE, SinWave, 1000, ENABLE);
    SquareWave_Init();
}
