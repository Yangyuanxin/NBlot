#include "timer.h"
#include "led.h"
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
    hlptim1.Init.Clock.Prescaler = psc;                //��Ƶϵ��
 //   hlptim1.Init.Period=arr;                          //�Զ�װ��ֵ



  //  hlptim1.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����   
    
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
  hlptim1.Init.Clock.Prescaler=psc;                       //��Ƶϵ��
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
    if(htim->Instance==LPTIM1)
	{
		__HAL_RCC_LPTIM1_CLK_ENABLE();             //ʹ��LPTIM1ʱ��
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

//�͹��Ķ�ʱ��2�жϷ���������
void HAL_LPTIM_PeriodElapsedCallback(LPTIM_HandleTypeDef *htim)
{
    if(htim==(&hlptim1))
    {
       
    } else if(htim==(&hlptim2)) {
        
    } else {
        
    }
}

