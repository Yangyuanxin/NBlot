#ifndef _KEY_H
#define _KEY_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//KEY��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/11/27
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
#define KEY0        HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3)  //KEY0����PC3
#define KEY1        HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_2)  //KEY1����PC2
#define KEY2        HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1)  //KEY2����PC1
#define WK_UP       HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)  //WKUP����PC0

#define KEY0_PRES 	1
#define KEY1_PRES	2
#define KEY2_PRES	3
#define WKUP_PRES   4

void key_init(void);

u8 KEY_Scan(u8 mode);

#endif
