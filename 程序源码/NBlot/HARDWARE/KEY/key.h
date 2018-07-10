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

#define KEY0_PRES   1
#define KEY1_PRES   2
#define KEY2_PRES   4
#define WKUP_PRES   8

//��������ʱ��
#define KEY_DELAY_TICK   15

//���尴���ص�����ָ��
typedef void (*key_cb)(u32 key_event, void *p_arg);

//�����ṹ��
typedef struct key_dev
{
    uint8_t key_event;
    
    int     start_tick;
    
    key_cb  pFkey_cb;
    
    void   *p_arg;       
    
}key_dev_t;

void key_init(int mode);

u8 KEY_Scan(u8 mode);

//**************************************
// fn : KEY_RegisterCb
//
// brief : ע�ᰴť�¼��ص�
//
// param : cb -> ����ť�¼�����ָ��
//
// return : none
void key_registercb(key_cb cb, void *p_arg);

//**************************************
// fn : key_poll
//
// brief : ��ѯ��ť�¼�
//
// param : none
//
// return : none
void key_poll(void);

#endif
