#include "atk_sys.h"
#include "atk_delay.h"
#include "atk_usart.h"
#include "atk_led.h"
#include "lcd.h"
#include "demo_entry.h"

/************************************************
 ALIENTEK ������STM32F429������ʵ��12
 TFTLCD��ʾʵ��--HAL�⺯����
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com  
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/
int main(void)
{
    HAL_Init();                     //��ʼ��HAL��   
    Stm32_Clock_Init(360,25,2,8);   //����ʱ��180Mhz
    delay_init(180);                //��ʼ����ʱ����
    uart1_init(9600);               //��ʼ������USART
    led_init();                     //��ʼ��LED 
    LCD_Init();                     //��ʼ��LCD  
    
//    demo_led_entry(); 
    
//    demo_key_entry();
    
//    demo_timer_timing_entry();

//    demo_uart_any_data_len_recv_entry();   

//    demo_nbiot_gprs_attach_entry();  

//    demo_nbiot_info_get_entry(); 

//    demo_nbiot_huaweiiot_entry();    
        
    while(1)
    {              
        delay_ms(1000);
    }
}
