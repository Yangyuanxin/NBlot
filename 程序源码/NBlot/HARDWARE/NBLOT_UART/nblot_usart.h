#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.csom
//�޸�����:2015/6/23
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************
//V1.0�޸�˵�� 
////////////////////////////////////////////////////////////////////////////////// 	
#define LPUSART_REC_LEN  			200  	  //�����������ֽ��� 200
#define EN_LPUART1_RX 			1		  //ʹ�ܣ�1��/��ֹ��0�����ڽ���
	  	
extern u8  NBLOT_RxBuffer[LPUSART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 NBLOT_USART_RX_STA;         	  //����״̬���	
extern UART_HandleTypeDef hlpuart1;       //UART���

//��ʼ��IO LPUART1
//bound:������
void lpuart1_init (u32 bound);


//��ѯ���ʹ�������
int uart_data_tx_poll(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size);

//��ѯ���մ�������
int uart_data_rx_poll(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size);


#endif
