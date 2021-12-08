#include "STM32_WaveOutput.h"


//正弦波数据
void SineWave_Data( uint16_t cycle, uint16_t *D)
{
    uint16_t i;

    for(i = 0; i < cycle; i++)
    {
        D[i] = (uint16_t)((sin((1.0 * i / cycle) * 2 * PI) + 1.0) * 2048); //(uint16_t)((sin((1.0*i/(cycle))*2*PI)+1.0)*4095/2.0);//
    }
}

//三角波数据
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

//正弦波形表
#ifdef Sine_WaveOutput_Enable
    uint16_t SineWave_Value[256];
#endif

//锯齿波形表
#ifdef Triangular_WaveOutput_Enable
    uint16_t Triangular_Value[256];
#endif

//DAC寄存器地址声明
#define DAC_DHR12R1 (uint32_t)&(DAC->DHR12R1)   //DAC通道1输出寄存器地址
#define DAC_DHR12R2 (uint32_t)&(DAC->DHR12R2)   //DAC通道2输出寄存器地址

//引脚初始化
void SineWave_GPIO_Config(uint8_t NewState1, uint8_t NewState2)
{
    RCC->APB2ENR |= 1 << 2; //使能PORTA时钟

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

//DAC初始化
void SineWave_DAC_Config(uint8_t NewState1, uint8_t NewState2)
{
    RCC->APB2ENR |= 1 << 2; //使能PORTA时钟
    RCC->APB1ENR |= 1 << 29; //使能DAC时钟

//	GPIOA->CRL&=0XFFF0FFFF;
//	GPIOA->CRL|=0X00000000;

    if( NewState1 != DISABLE)
    {
        DAC->CR |= 1 << 0;	//使能DAC通道1
        DAC->CR |= 1 << 1;	//DAC1输出缓存不使能 BOFF1=1
        DAC->CR |= 1 << 2;	//使用触发功能 TEN1=1

        DAC->CR |= 0 << 3;	//3、4、5=100时为TIM2 TRGO事件触发
        DAC->CR |= 0 << 4;	//
        DAC->CR |= 1 << 5;	//

        DAC->CR |= 0 << 6;	//不使用波形发生
        DAC->CR |= 0 << 8;	//屏蔽、幅值设置
        DAC->CR |= 1 << 12;	//DAC1 DMA使能
    }

    if( NewState2 != DISABLE)
    {
        DAC->CR |= 1 << 16;	//使能DAC通道2
        DAC->CR |= 1 << 17;	//DAC2输出缓存不使能 BOFF1=1
        DAC->CR |= 1 << 18;	//使用触发功能 TEN2=1

        DAC->CR |= 0 << 19;	//3、4、5=100时为TIM2 TRGO事件触发
        DAC->CR |= 0 << 20;	//
        DAC->CR |= 0 << 21;	//

        DAC->CR |= 0 << 22;	//不使用波形发生
        DAC->CR |= 0 << 24;	//屏蔽、幅值设置
        DAC->CR |= 1 << 28;	//DAC1 DMA使能

        DAC->DHR12R1 = 0; //使能通道1
        DAC->DHR12R2 = 0; //使能通道2
    }
}

//定时器配置
void SineWave_TIM_Config( uint32_t Wave1_Fre, uint8_t NewState1, uint32_t Wave2_Fre, uint8_t NewState2)
{
    if(NewState1 != DISABLE)RCC->APB1ENR |= 1 << 0;	//TIM2时钟使能

    if(NewState2 != DISABLE)RCC->APB1ENR |= 1 << 4;	//TIM6时钟使能

    TIM2->PSC = 0x0; //预分频器不分频
    TIM2->CR1 |= 0 << 4; //向上计数模式
    TIM6->PSC = 0x0;
    TIM6->CR1 |= 0 << 4; //向上计数模式

    if( NewState1 != DISABLE)
    {
        TIM2->ARR = Wave1_Fre - 1;
        TIM2->CR1 |= 0x01;  //使能定时器6
        TIM2->CR2 &= (uint16_t)~((uint16_t)0x0070);//设置TIM2输出触发为更新模式
        TIM2->CR2 |= 0x0020; //设置TIM2输出触发为更新模式
    }

    if( NewState2 != DISABLE)
    {
        TIM6->ARR = Wave2_Fre - 10;
        TIM6->CR1 |= 0x01;  //使能定时器2
        TIM6->CR2 &= (uint16_t)~((uint16_t)0x0070);//设置TIM6输出触发为更新模式
        TIM6->CR2 |= 0x0020; //设置TIM6输出触发为更新模式
    }
}

//DMA配置
void SineWave_DMA_Config( uint16_t *Wave1_Mem, uint8_t NewState1, uint16_t *Wave2_Mem, uint8_t NewState2)
{
    RCC->AHBENR |= 1 << 1;			//开启DMA2时钟
    HAL_Delay(5);				//等待DMA时钟稳定

    if(NewState1 != DISABLE)
    {
        DMA2_Channel3->CPAR = DAC_DHR12R1; 	 	//DMA1 外设地址
        DMA2_Channel3->CMAR = (uint32_t)Wave1_Mem; 	//DMA1,存储器地址
        DMA2_Channel3->CNDTR = 256;    	//DMA2,传输数据量
        DMA2_Channel3->CCR = 0X00000000;	//复位
        DMA2_Channel3->CCR |= 1 << 4;  		//从存储器读
        DMA2_Channel3->CCR |= 0 << 6; 		//外设地址非增量模式
        DMA2_Channel3->CCR |= 1 << 7; 	 	//存储器增量模式
        DMA2_Channel3->CCR |= 1 << 8; 	 	//外设数据宽度为16位
        DMA2_Channel3->CCR |= 1 << 10; 		//存储器数据宽度16位
        DMA2_Channel3->CCR |= 1 << 12; 		//最高优先级
        DMA2_Channel3->CCR |= 1 << 13; 		//最高优先级
        DMA2_Channel3->CCR |= 0 << 14; 		//非存储器到存储器模式
        DMA2_Channel3->CCR |= 1 << 5;  		//循环发送模式
        DMA2_Channel3->CCR |= 1 << 0;      //开启DMA传输
    }

    if(NewState2 != DISABLE)
    {
        DMA2_Channel4->CPAR = DAC_DHR12R2; 	 	//DMA1 外设地址
        DMA2_Channel4->CMAR = (uint32_t)Wave2_Mem; 	//DMA1,存储器地址
        DMA2_Channel4->CNDTR = 256;    	//DMA2,传输数据量
        DMA2_Channel4->CCR = 0X00000000;	//复位
        DMA2_Channel4->CCR |= 1 << 4;  		//从存储器读
        DMA2_Channel4->CCR |= 0 << 6; 		//外设地址非增量模式
        DMA2_Channel4->CCR |= 1 << 7; 	 	//存储器增量模式
        DMA2_Channel4->CCR |= 1 << 8; 	 	//外设数据宽度为16位
        DMA2_Channel4->CCR |= 1 << 10; 		//存储器数据宽度16位
        DMA2_Channel4->CCR |= 1 << 12; 		//最高优先级
        DMA2_Channel4->CCR |= 1 << 13; 		//最高优先级
        DMA2_Channel4->CCR |= 0 << 14; 		//非存储器到存储器模式
        DMA2_Channel4->CCR |= 1 << 5;  		//循环发送模式
        DMA2_Channel4->CCR |= 1 << 0;      //开启DMA传输
    }
}


void MyTIM_SetAutoreload(TIM_TypeDef* TIMx, uint16_t Autoreload)
{
    /* Check the parameters */
//  assert_param(IS_TIM_ALL_PERIPH(TIMx));
    /* Set the Autoreload Register value */
    TIMx->ARR = Autoreload;
}

//*****输出波形初始化***************/
//uint8_t Wave1 波形1
//uint16_t Wave1_Fre 波形1频率(Hz)
//uint8_t NewState1 波形1状态（忽略）
//uint8_t Wave2 波形2
//uint16_t Wave2_Fre 波形2频率(Hz)
//uint8_t NewState2 波形2状态（忽略）
//调用exp：  SineWave_Init( SawToothWave ,10 ,ENABLE ,SinWave ,10 ,ENABLE);//三角波，10Hz，正弦波，10Hz
void SineWave_Init(uint8_t Wave1, uint16_t Wave1_Fre, uint8_t NewState1, uint8_t Wave2, uint16_t Wave2_Fre, uint8_t NewState2)
{
    uint16_t *add1, *add2;
    uint16_t f1 = (uint16_t)(72000000 / sizeof(SineWave_Value) * 2 / Wave1_Fre);
    uint16_t f2 = (uint16_t)(72000000 / sizeof(SineWave_Value) * 2 / Wave2_Fre);
    SineWave_Data( N, SineWave_Value);		//生成波形表1
    Triangular_Data( N, Triangular_Value);//生成波形表2

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

    SineWave_GPIO_Config(ENABLE, ENABLE);			 //初始化引脚
    SineWave_TIM_Config(f1, NewState1, f2, NewState2);			 //初始化定时器
    SineWave_DAC_Config(NewState1, NewState2);			 //初始化DAC
    SineWave_DMA_Config(add1, NewState1, add2, NewState2);			 //初始化DMA

    if(NewState1 != DISABLE) TIM2->CR1 |= 0x01; //使能定时器2;			 //使能TIM2,开始产生波形

    if(NewState2 != DISABLE) TIM6->CR1 |= 0x01; //使能定时器2;			 //使能TIM6,开始产生波形
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
    RCC->APB2ENR |= 1 << 0; 	//AFIO使能
    RCC->APB2ENR |= 1 << 2; //使能PORTA时钟
    RCC->APB1ENR |= 1 << 3; 	//TIM5时钟使能

    GPIOA->CRL &= 0XFFFF0FFF;	//PA3清除之前的设置
    GPIOA->CRL |= 0X0000B000;	//复用功能输出
    GPIOA->ODR |= 1 << 3;   //PA3 输出高

    TIM5->ARR = 999;			//设定计数器自动重装值
    TIM5->PSC = 71;			//预分频器设置

    TIM5->CCMR2 |= 7 << 12;  	//CH4 PWM2模式
    TIM5->CCMR2 |= 1 << 11; 	//CH4预装载使能
    TIM5->CCER |= 1 << 12;   	//OC4 输出使能
    TIM5->BDTR |= 1 << 15;   	//MOE 主输出使能

    TIM5->CR1 = 0x0080;   	//ARPE使能
    TIM5->CR1 |= 0x01;    	//使能定时器5

    TIM5->CCR4 = 499;
}

void TestWave_Init(void)
{
    SineWave_Init(TriangularWave, 1000, ENABLE, SinWave, 1000, ENABLE);
    SquareWave_Init();
}
