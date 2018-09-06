#include "lptimer.h"
#include "atk_led.h"
//////////////////////////////////////////////////////////////////////////////////     
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//��ʱ���ж���������       
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/11/27
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved                                      
//////////////////////////////////////////////////////////////////////////////////     

//�͹��Ķ�ʱ�����      
LPTIM_HandleTypeDef hlptim1;
LPTIM_HandleTypeDef hlptim2;


//�͹��Ķ�ʱ��1�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��ǵ͹��Ķ�ʱ��1(��ʱ��1����APB1�ϣ�ʱ��ΪHCLK/2)
void LPTIM1_Init(u32 psc, u32 arr)
{
    hlptim1.Instance = LPTIM1;
    
    hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;               
    
    hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
    hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
    hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
    hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
    
       
    if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
    
    
    /* ### - 3 - Start the Timeout function in interrupt mode ################# */
    /*
    *  arr = 65535
    *  psc  = 32767
    *  According to this configuration (LPTIMER clocked by LSE & compare = 32767,
    *  the Timeout period = (compare + 1)/LSE_Frequency = 1s
    */
    if (HAL_LPTIM_TimeOut_Start_IT(&hlptim1, arr, psc) != HAL_OK)
    {
        Error_Handler();
    }   
}


//�͹��Ķ�ʱ��2�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��ǵ͹��Ķ�ʱ��1(��ʱ��1����APB1�ϣ�ʱ��ΪHCLK/2)
void LPTIM2_Init(u32 psc, u32 arr)
{
          
  hlptim2.Instance = LPTIM2;
  hlptim2.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler=psc;                   //��Ƶϵ��
//  hlptim1.Init.CounterMode=TIM_COUNTERMODE_UP;      //���ϼ�����
//  hlptim1.Init.Period=arr;                          //�Զ�װ��ֵ
//  hlptim1.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����   
  hlptim2.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim2.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim2.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim2.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim2.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim2.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  if (HAL_LPTIM_Init(&hlptim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

//�͹��Ķ�ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
//�˺����ᱻHAL_TIM_Base_Init()��������
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef *htim)
{
    /* Clocks structure declaration */
    RCC_PeriphCLKInitTypeDef        RCC_PeriphCLKInitStruct;
    
    if(htim->Instance==LPTIM1)
    {
        __HAL_RCC_LPTIM1_CLK_ENABLE();             //ʹ��LPTIM1ʱ��
        
        /* ### - 1 - Re-target the LSE to Clock the LPTIM Counter ################# */
        /* Select the LSE clock as LPTIM peripheral clock */
        RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
        RCC_PeriphCLKInitStruct.Lptim2ClockSelection = RCC_LPTIM2CLKSOURCE_PCLK1;
        HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);       
        
        HAL_NVIC_SetPriority(LPTIM1_IRQn,14,0);    //�����ж����ȼ�����ռ���ȼ�1�������ȼ�3
        HAL_NVIC_EnableIRQ(LPTIM1_IRQn);           //����ITM3�ж�   
    }  
}

//�͹��Ķ�ʱ��1�жϷ�����
void LPTIM1_IRQHandler(void)
{
    HAL_LPTIM_IRQHandler(&hlptim1);
}


//�͹��Ķ�ʱ��2�жϷ�����
void LPTIM2_IRQHandler(void)
{
    HAL_LPTIM_IRQHandler(&hlptim2);
}


/**
  * @brief  Compare match callback in non blocking mode 
  * @param  hlptim : LPTIM handle
  * @retval None
  */
void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
    if(hlptim==(&hlptim1)) {
        
        
        printf("lptim1 timeout reach\r\n");
       
    } else if(hlptim==(&hlptim2)) {
        
        printf("lptim2 timeout reach\r\n");
        
    } else {
        
    }
}

