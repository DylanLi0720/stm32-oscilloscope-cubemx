#include "dso.h"
#include "lcd.h"
#include "stdio.h"
#include "dso_display.h"

#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

//全局变量
uint8_t run_sta = 0; //示波器运行状态指示
uint8_t key_num = 0;//按键选择
uint8_t key_sta = 1;//按键状态

uint8_t avg = 1;//波形放大系数
uint16_t timN = 3; //时基序号
uint32_t timebase = 0;//时间基准指示
uint32_t max_data = 1024; //触发电平
uint32_t freq = 0; //频率输出
uint16_t Yinit = 5;  //画图数据参数
uint16_t NCollect = 640; //采样数量

uint16_t con_t = 0, con_t1 = 0; //频率和边沿计算
uint16_t buff[600] = {0};  //波形数据存储
uint16_t buff2[600] = {0};  //波形数据存储
uint16_t ADC_Value[900] ;  //AD采集数据
short offset = 0;  //偏移


float Vpp = 0; //峰峰值
uint8_t showstr[20];


void DSO_Init(void)
{
    DSO_SetBackGround();
    DSO_DrawNetwork();

    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&ADC_Value, NCollect);
    __HAL_DMA_DISABLE_IT((&hadc1)->DMA_Handle, DMA_IT_HT);  //关掉DMA半中断

    DSO_SetTimeBase(timN);

    HAL_TIM_Base_Start(&htim3);
}



//频率计算和上升沿触发
void DSO_GetFreq(void)
{
    static uint16_t n = 0;

    for(n = 100; n < NCollect; n++)
    {
        if(ADC_Value[n] < max_data && ADC_Value[n + 2] > max_data)
        {
            if(n > (NCollect - lcddev.width))
            {
                con_t = 100;
            }
            else
            {
                con_t = n;
            }

            break;
        }
    }

    for(n = con_t + 3; n < NCollect; n++)
    {
        if(ADC_Value[n] < max_data && ADC_Value[n + 2] > max_data)
        {
            con_t1 = n;
            break;
        }
    }

    freq = (1 / ((con_t1 - con_t) * timebase / 2000000.0));
}



//获取峰峰值
void DSO_GetVpp(void)
{
    uint16_t max = 0;
    uint16_t min = 4095;
    uint16_t n = 0;

    for(n = 1; n < lcddev.width; n++)
    {
        if(buff[n] > max)
        {
            max = buff[n];
        }

        if(buff[n] < min)
        {
            min = buff[n];
        }
    }

    Vpp = (float)(max - min) * (3.3 / 4096.0);
}


//设置时基
void DSO_SetTimeBase(uint8_t t)
{
    switch(t)
    {
        case 0: //50us
            timebase = 5;
            break;

        case 1: //50us
            timebase = 5;
            break;

        case 2: //0.1ms
            timebase = 10;
            break;

        case 3: //0.2ms
            timebase = 20;
            break;

        case 4: //0.5ms
            timebase = 50;
            break;

        case 5: //1ms
            timebase = 100;
            break;

        case 6: //2ms
            timebase = 200;
            break;

        case 7: //5ms
            timebase = 500;
            break;

        case 8: //10ms
            timebase = 1000;
            break;

        case 9: //20ms
            timebase = 2000;
            break;

        case 10: //20ms
            timebase = 2000;
            break;
    }

    TIM3->ARR = timebase - 1;
}


//偏移量设置
void DSO_OffsetIncrement(uint8_t inc)
{
    uint8_t i = 0;

    if(offset < 100 && offset > -100)
    {
        if(inc)offset++;
        else offset--;
    }
    else
    {
        if(offset == 100)
        {
            offset--;
        }
        else
        {
            offset++;
        }
    }

    while(HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == RESET || HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == RESET)
    {
        for(; i < 4; ++i)
        {
            if(HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == SET && HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == SET)
            {
                return;
            }

            HAL_Delay(50);
        }

        if(offset < 100 && offset > -100)
        {
            if(inc)offset++;
            else offset--;

            HAL_Delay(20);
            key_sta = 1;
        }
        else
        {
            return;
        }
    }
}


//幅值放大系数设置
void DSO_AvgIncrement(uint8_t inc)
{
    if(avg < 4 && inc)
    {
        avg *= 2;
    }

    if(avg > 1 && !inc)
    {
        avg /= 2;
    }

    switch(avg)
    {
        case 1:
            max_data = 1500;
            break;

        case 2:
            max_data = 512;
            break;

        case 4:
            max_data = 128;
            break;
    }
}



void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    HAL_TIM_Base_MspDeInit(&htim3);

    if(run_sta == 1)
    {
        HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
        DSO_ShowString_COLOR(5, 1, 100, "Running", YELLOW);
        DSO_ShowGUI();
    }
    else
    {
        DSO_ShowString_COLOR(5, 1, 100, "HOLD   ", RED);
    }

    HAL_TIM_Base_MspInit(&htim3);
}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t i = 0;
    HAL_Delay(10);    //消抖

    if(GPIO_Pin == WK_UP_Pin)
    {
        if(HAL_GPIO_ReadPin(WK_UP_GPIO_Port, WK_UP_Pin) == SET)
        {
            for(; i < 5 && HAL_GPIO_ReadPin(WK_UP_GPIO_Port, WK_UP_Pin) == SET; ++i)
            {
                HAL_Delay(50);
            }

            if(i == 5)
            {
                run_sta = !run_sta;
            }
            else
            {
                key_num++;
            }

            key_sta = 1;
        }
    }
    else if(GPIO_Pin == KEY0_Pin)
    {

        if(HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == RESET)
        {
            if(key_num % KEYNUM == 0 && timN > 0)
            {
                DSO_SetTimeBase(--timN % TIMEBASENUM);
            }
            else if(key_num % KEYNUM == 1)
            {
                DSO_OffsetIncrement(INCREASE);
            }
            else if(key_num % KEYNUM == 2)
            {
                DSO_AvgIncrement(INCREASE);
            }

            key_sta = 1;
        }
    }
    else if(GPIO_Pin == KEY1_Pin)
    {
        if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == RESET)
        {
            if(key_num % KEYNUM == 0 && timN < TIMEBASENUM - 1)
            {
                DSO_SetTimeBase(++timN % TIMEBASENUM);
            }
            else if(key_num % KEYNUM == 1)
            {
                DSO_OffsetIncrement(REDUCE);
            }
            else if(key_num % KEYNUM == 2)
            {
                DSO_AvgIncrement(REDUCE);
            }

            key_sta = 1;
        }
    }
}





////ADC初始化
//void DSO_ADC_Init(void)
//{
//
//	GPIO_InitTypeDef GPIO_InitStructure;
//	ADC_InitTypeDef ADC_InitStructure;

//	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
//
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA,ENABLE);
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  //设置为模拟输入
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//

//	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;   //独立ADC模式
//	ADC_InitStructure.ADC_ScanConvMode = DISABLE ;       //不使能扫描模式
//	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;   //开启连续转换，就是不停转换
//	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
//	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//	ADC_InitStructure.ADC_NbrOfChannel = 1;
//	ADC_Init(ADC1, &ADC_InitStructure);

//
//	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5);
//	ADC_DMACmd(ADC1, ENABLE);
//	ADC_Cmd(ADC1, ENABLE);
////	ADC_ResetCalibration(ADC1);
////	while(ADC_GetResetCalibrationStatus(ADC1));
////	ADC_StartCalibration(ADC1);
////	while(ADC_GetCalibrationStatus(ADC1));
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

//}

////DMA初始化
//void DSO_DMA_Init(void)
//{

//	DMA_InitTypeDef DMA_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

//	DMA_DeInit(DMA1_Channel1);  //复位初始化，配置为复位模式，下面我们再重新配置（不知道有用没）
//	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&ADC1->DR);    //设置源地址
//	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_Value; //设置内存地址
//	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  // 设置传输方向
//	DMA_InitStructure.DMA_BufferSize = NCollect;
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
//	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;          //循环模式
//	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;    //高优先级
//	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;     //禁止内存间传输数据
//
//	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

//	DMA_ClearITPendingBit(DMA1_IT_GL1);
//	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
//
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;  //DMA2_Stream0中断
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;  //抢占优先级1
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;        //子优先级1
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQ通道使能
//	NVIC_Init(&NVIC_InitStructure);    //根据指定的参数初始化NVIC寄存器
//
//	DMA_Cmd(DMA1_Channel1, ENABLE);

//}

////定时器3初始化
//void DSO_TIMER_Init(void)
//{
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

//	TIM_TimeBaseStructure.TIM_Prescaler = 55; //分频系数
//	TIM_TimeBaseStructure.TIM_Period = 1;
//	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //时钟分频因子
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up ; //向上计数模式
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //初始化定时器3

//	//使能定时器中断
//	TIM_ARRPreloadConfig(TIM3, ENABLE); //允许TIM3定时重载
//	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);  //选择TIM3的UPDATA事件更新为触发源
//
//	//使能TIM3
//	TIM_Cmd(TIM3, ENABLE);
//}

////DMA中断
//void DMA1_Channel1_IRQHandler(void)
//{
//	if(DMA_GetITStatus(DMA1_IT_TC1))
//	{
//		TIM_Cmd(TIM3, DISABLE);
//		if(run_sta==1)
//		{
//			LED0=!LED0;
//			DSO_ShowString_COLOR(5,1,100,"Running",YELLOW);
//			DSO_ShowGUI();
//		}
//		else
//		{
//			DSO_ShowString_COLOR(5,1,100,"HOLD   ",RED);
//		}
//		TIM_Cmd(TIM3, ENABLE);
//		DMA_ClearITPendingBit(DMA1_IT_GL1); //清除全部中断标志
//	}
//}





