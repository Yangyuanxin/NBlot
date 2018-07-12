#ifndef _NBLOT_USART_H
#define _NBLOT_USART_H
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

#define UART_NONE_EVENT              0X0000
#define UART_TX_EVENT                0X0001
#define UART_RX_EVENT                0X0002
#define UART_TX_TIMEOUT_EVENT        0X0004
#define UART_RX_TIMEOUT_EVENT        0X0008

                               
#define RING_BUFF_LEN            512         //���廷�λ������������ֽ���

//
typedef struct atk_ring_buf_t
{
    uint32_t head;           
    uint32_t tail;
    uint32_t lenght;
    uint8_t  ring_buff[RING_BUFF_LEN];    
    
}atk_ring_buf_t;


extern atk_ring_buf_t g_uart_ring_buff;      //����һ��ringBuff�Ļ����� 

//���崮���¼��ص�����ָ��
typedef void (*uart_cb)(void *p_arg);

//�����豸�ṹ��
typedef struct uart_dev
{  
    UART_HandleTypeDef *p_huart; 
    
    struct atk_soft_timer uart_rx_timer;  
    struct atk_soft_timer uart_tx_timer;
    
    /* �����¼��ص����� */
    uart_cb  uart_cb;
    
    void     *p_arg;   

    int       uart_event;  

    uint32_t  bound;    
    
}uart_dev_t;

#define EN_LPUART1_RX             1           //ʹ�ܣ�1��/��ֹ��0�����ڽ���

//uart�豸���
typedef uart_dev_t *uart_handle_t;
          
extern UART_HandleTypeDef hlpuart1;            //UART���
                   
//��ʼ��IO LPUART1
//bound:������
uart_dev_t *lpuart1_init (u32 bound);


//��ѯ���ʹ�������
int uart_data_tx_poll(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout);

//��ѯ���մ�������
int uart_data_rx_poll(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout);


//�жϽ��մ�������
int uart_data_rx_int(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout);

//�жϷ��ʹ������ݣ� ����ʹ��
int uart_data_tx_int(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout);


//ע�ᴮ���¼��ص�����
void lpuart_event_registercb(uart_cb cb, void *p_arg);


//���ô����¼�
void lpuart_event_set (int uart_event);

//��ȡ�����¼�
int lpuart_event_get (int uart_event);

//��������¼�
void lpuart_event_clr (int uart_event);


//��ѯ�����¼�
void uart_event_poll(uart_handle_t uart_handle);

#endif
