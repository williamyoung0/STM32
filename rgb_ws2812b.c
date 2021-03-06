#include "rgb_ws2812b.h"

volatile u16 RGB_buffer[258] = {0};


void RGB_WS2812B_GPIO_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Set_RGB_Colour(uint8_t red, uint8_t green, uint8_t blue)
{
	uint32_t rgb_value = green<<16 | red<<8 | blue;
	
	while(DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET);
	DMA_ClearFlag(DMA1_FLAG_TC1);
	
	DMA_Cmd(DMA1_Channel2,DISABLE);
	DMA_SetCurrDataCounter(DMA1_Channel2,sizeof(RGB_buffer));
	for(int i = 0; i<192; ++i)
	{
		if((rgb_value<<i)&0x800000)
		{
			RGB_buffer[i+45] = 61;
			RGB_buffer[i+69] = 61;
			RGB_buffer[i+93] = 61;
			RGB_buffer[i+117] = 61;
			RGB_buffer[i+141] = 61;
			RGB_buffer[i+165] = 61;
			RGB_buffer[i+213] = 61;

		}
		else
		{
			RGB_buffer[i+69] = 28;
			RGB_buffer[i+93] = 28;
			RGB_buffer[i+117] = 28;
			RGB_buffer[i+141] = 28;
			RGB_buffer[i+165] = 28;
			RGB_buffer[i+213] = 28;
		}
	}
	DMA_Cmd(DMA1_Channel2,ENABLE);
}

void RGB_DMA_Transmission(void)
{
   	DMA_InitTypeDef DMA_InitStructure;

   	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_DeInit(DMA1_Channel2);
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&TIM3->CCR3);                //外设地址  
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RGB_buffer;                       //存储器地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                            //数据方向为存储器到外设
	DMA_InitStructure.DMA_BufferSize = sizeof(RGB_buffer);                        //单次发送数据个数
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;              //外设地址不自增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                       //内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;   //设置外设数据位宽为半字(16位)
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;       //设置存储器数据位宽为半字(16位)
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                                 //正常在工作模式，另外一个是循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;                           //设置最高优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                  //失能存储器到存储器传输
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);                                  //初始化配置生效

	
  	DMA_Cmd(DMA1_Channel2, ENABLE);
}

void RGB_TIM3OC3_PWM(void)
{
	TIM_OCInitTypeDef TIM_OCInitStructure;
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM3, ENABLE);
	TIM_DMACmd(TIM3, TIM_DMA_CC3,ENABLE);
}

void TIM3_PWM_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_APB2PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_Period = 0;
	TIM_TimeBaseStructure.TIM_Prescaler = 89;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
	
	TIM_Cmd(TIM3, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
}

void RGB_WS2812B_Init(void)
{
	RGB_WS2812B_GPIO_Config();
	RGB_DMA_Transmission();
	TIM3_PWM_Init();
	RGB_TIM3OC3_PWM();
}
