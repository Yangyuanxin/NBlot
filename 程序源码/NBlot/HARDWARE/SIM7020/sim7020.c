#include "sim7020.h"
#include "delay.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2015/9/7
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************


//����ȫ�ֻ������
#define NB_UART_RECE_BUF_MAX_LEN    512
#define NB_UART_SEND_BUF_MAX_LEN    512

//���ջ���ռ�
struct sim7020_recv
{
  char      buf[NB_UART_RECE_BUF_MAX_LEN];    //�������ݻ�����
  uint16_t  len;                              //��Ч���ݳ���
}sim7020_recv_t;

//���ջ���ռ�
struct sim7020_send
{
  char      buf[NB_UART_SEND_BUF_MAX_LEN];    //�������ݻ�����
  uint16_t  len;                              //��Ч���ݳ���
}sim7020_send_t;



void sim7020_sm_event (int sim7020_event)
{
    
    switch(sim7020_event) {
    case 0: 

        break;


    case 1:

        break; 
    
   default:
        
        break;
   }    
    
}




