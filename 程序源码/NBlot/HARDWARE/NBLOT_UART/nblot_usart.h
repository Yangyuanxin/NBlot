#ifndef _NB_USART_H
#define _NB_USART_H
#include "sys.h"
#include "stdio.h"	
#include "atk_soft_timer.h"

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

#define UART_TX_EVENT                0X0001
#define UART_RX_EVENT                0X0002
#define UART_TX_TIMEOUT_EVENT        0X0004
#define UART_RX_TIMEOUT_EVENT        0X0008


//���尴���ص�����ָ��
typedef void (*uart_cb)(u32 key_event, void *p_arg);

//���������ṹ��
typedef struct uart_dev
{  
    int     uart_event;   
    int     start_tick;   
    int     time_out;
    
    struct Timer uart_timer;
    
    /* �����¼��ص����� */
    uart_cb  lpuart_cb;
    
    void   *p_arg;       
    
}uart_dev_t;


#define LPUSART_REC_LEN  	    512  	    //�����������ֽ��� 200
#define EN_LPUART1_RX 			1		    //ʹ�ܣ�1��/��ֹ��0�����ڽ���
	  	
extern u8  NBLOT_RxBuffer[LPUSART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 NBLOT_USART_RX_STA;         	    //����״̬���	
extern UART_HandleTypeDef hlpuart1;         //UART���

//��ʼ��IO LPUART1
//bound:������
void lpuart1_init (u32 bound);


//��ѯ���ʹ�������
int uart_data_tx_poll(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout);

//��ѯ���մ�������
int uart_data_rx_poll(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout);


#endif
