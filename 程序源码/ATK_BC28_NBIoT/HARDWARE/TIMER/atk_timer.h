#ifndef _TIMER_H
#define _TIMER_H

#include "atk_sys.h"

#include "stm32l4xx_hal_tim.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//��ʱ����������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/11/27
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 


extern TIM_HandleTypeDef TIM7_Handler; //��ʱ����� 

void TIM7_Init(u16 arr,u16 psc);

#endif

