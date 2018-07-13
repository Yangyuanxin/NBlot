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

//�������ⳤ���豸���
#define UART_ANY_DATA_LEN_RECV       1


#define UART_NONE_EVENT              0X0000
#define UART_TX_EVENT                0X0001
#define UART_RX_EVENT                0X0002
#define UART_TX_TIMEOUT_EVENT        0X0004
#define UART_RX_TIMEOUT_EVENT        0X0008

//���廷�λ������������ֽ���                               
#define RING_BUF_LEN                  512         

//���λ������ṹ��
typedef struct atk_ring_buf_t
{
    uint32_t head;           
    uint32_t tail;
    uint32_t lenght;
    uint8_t  ring_buf[RING_BUF_LEN];  
    
}atk_ring_buf_t;


//���崮���¼��ص�����ָ��
typedef void (*uart_cb)(void *p_arg);

//�����豸�ṹ��
typedef struct uart_dev
{  
    UART_HandleTypeDef *p_huart; 
    
    struct atk_soft_timer uart_rx_timer;  
    struct atk_soft_timer uart_tx_timer;
    
    //�����ʹ�����λ���������ֵΪNULL
    atk_ring_buf_t *p_uart_ring_buff;  
    
    //�¼��ص�����
    uart_cb  uart_cb;
    
    //�ص�����
    void     *p_arg;   

    //�¼������
    int       uart_event;  

    //��ǰʹ�õĲ�����
    uint32_t  bound;    
    
}uart_dev_t;

//uart�豸���
typedef uart_dev_t *uart_handle_t;


/**
 * @brief ��ʼ�����λ������������Ϣ
 */
int atk_ring_buf_init (atk_ring_buf_t *p_ring_buf);

/**
 * @brief �����λ�����д����
 */
int atk_ring_buf_write(atk_ring_buf_t *p_ring_buf, uint8_t data);


 /**
 * @brief  ��ȡ���λ�����������
 */
int atk_ring_buf_read(atk_ring_buf_t *p_ring_buf, uint8_t *data);

/**
 * @brief ���λ���������Ч���ݵĸ���
 */
int atk_ring_buf_avail_len (atk_ring_buf_t *p_ring_buf);

 /**
 * @brief  �ӽ��ջ������ȡָ�����ȵ����ݣ����ͷ�ռ�õĿռ�
 */
int atk_ring_buf_size_read(atk_ring_buf_t *p_ring_buf, uint8_t *data, int len);


 /**
 * @brief  д����ջ������ָ�����ȵ����ݣ���ռ�õĿռ�
 */
int atk_ring_buf_size_write(atk_ring_buf_t *p_ring_buf, uint8_t *data, int len);


 /**
 * @brief  ��UART���ջ������ȡָ�����ȵ����ݣ����ͷ�ռ�õĿռ�
 */
int uart_ring_buf_read(uart_handle_t uart_handle, uint8_t *data, int len);

 /**
 * @brief  ��ȡ���ڻ��λ���������Ч���ݵĸ���
 */
int uart_ring_buf_avail_len(uart_handle_t uart_handle);

 /**
 * @brief  д��uart���ջ������ָ�����ȵ����ݣ���ռ�õĿռ�
 */
int uart_ring_buf_write(uart_handle_t uart_handle, uint8_t *data, int len);

                   
//��ʼ��IO LPUART1
//bound:������
uart_dev_t *lpuart1_init (u32 bound);


//��ѯ���ʹ�������
int uart_data_tx_poll(uart_handle_t uart_handle, uint8_t *pData, uint16_t size, uint32_t Timeout);

//��ѯ���մ�������
int uart_data_rx_poll(uart_handle_t uart_handle, uint8_t *pData, uint16_t size, uint32_t Timeout);


//�жϽ��մ�������
int uart_data_rx_int(uart_handle_t uart_handle, uint8_t *pData, uint16_t size, uint32_t Timeout);

//�жϷ��ʹ������ݣ� ����ʹ��
int uart_data_tx_int(uart_handle_t uart_handle, uint8_t *pData, uint16_t size, uint32_t Timeout);


//ע�ᴮ���¼��ص�����
void lpuart_event_registercb(uart_handle_t uart_handle, uart_cb cb, void *p_arg);


//���ô����¼�
void lpuart_event_set (uart_handle_t uart_handle,int uart_event);

//��ȡ�����¼�
int lpuart_event_get (uart_handle_t uart_handle, int uart_event);

//��������¼�
void lpuart_event_clr (uart_handle_t uart_handle, int uart_event);

//��ѯ�����¼�
void uart_event_poll(uart_handle_t uart_handle);

#endif
