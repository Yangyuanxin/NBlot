 /************************************************
 Copyright (c) ������������ӿƼ����޹�˾ 2014-2024
 All rights reserved 
 ALIENTEK ������STM32F429������ 
 NBIOT ���λ�����ʵ��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/ 

#include "atk_sys.h"
#include "atk_ring_buf.h"

/**
  * @brief  The demo ring buf entry entry point.
  *
  * @retval None
  */
void demo_atk_ring_buf_entry(void)
{
    int err = 0;
         
    atk_ring_buf_t g_uart_ring_buf; 
    
    
    //���ս��ջ��λ�����
    err = atk_ring_buf_init(&g_uart_ring_buf);    

      
    while(1) {        
        ;
    }
}
