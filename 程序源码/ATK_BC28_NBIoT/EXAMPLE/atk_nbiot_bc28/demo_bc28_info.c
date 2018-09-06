/**
  *
  * @file           : demo_sim7020_gprs_attach_entry.c
  * @brief          : sim7020 ���總��ʵ��
  */
/* Includes ------------------------------------------------------------------*/
#include "atk_sys.h"
#include "atk_led.h"
#include "atk_delay.h"
#include "atk_bc28_nbiot.h"
#include "atk_bc28.h"
#include "atk_bc28_nbiot.h"
#include "stm32l4xx_hal.h"


static int sm7020_main_status =  SIM7020_NBLOT_INFO;
static sim7020_handle_t  sim7020_handle = NULL;  


//sim7020��Ϣ�¼�������
static void __sim7020_event_cb_handler (void *p_arg, int msg_id, int len, char *msg)
{ 
    sim7020_handle_t sim7020_handle = (sim7020_handle_t)p_arg; 
    
    (void)sim7020_handle;
    
    switch(msg_id)
    {
        case SIM7020_MSG_CMD_RETRY:
          printf("%s cmd error and retry\r\n",msg);      
        break;  

        case SIM7020_MSG_CMD_FAIL:
        {
          printf("%s cmd failed\r\n",msg);
          
          break;                     
        }          
      
        case SIM7020_MSG_NBLOT_INIT:
        {
          printf("init=%s\r\n",msg);
                     
        }
        break;

        case SIM7020_MSG_IMSI:
        {
           printf("\r\nIMSI=%s\r\n",msg);
        }
        break;
        
        case SIM7020_MSG_REG:
        {       
             printf("\r\nmsg creg status=%s\r\n", msg);
                                        
        }
        break;
                  
        case SIM7020_MSG_CSQ:
          printf("rssi=%sdbm\r\n",msg);      
        break;        
        
        case SIM7020_MSG_SIGNAL:
          printf("signal=%s\r\n",msg);      
        break;
        
        case SIM7020_MSG_NBLOT_INFO:
          
        {
          printf("info get=%s\r\n",msg);
          
          sm7020_main_status =  SIM7020_SIGNAL;                     
        }

        break;

        case SIM7020_MSG_BAND:
             printf("\r\nfreq=%s\r\n",msg);
        break;
        
        //����ID
        case SIM7020_MSG_MID:
        {
            printf("\r\nmid=%s\r\n",msg);
        }
        break;
        
        //ģ���ͺ�
        case SIM7020_MSG_MMODEL:
        {
            printf("\r\nmmodel=%s\r\n",msg);
        }
        break;        

        //����汾��
        case SIM7020_MSG_MREV:
        {
            printf("\r\nmrev=%s\r\n",msg);
        }
        break;        
        
        case SIM7020_MSG_IMEI:
        {
            printf("\r\nimei=%s\r\n",msg);
        }
        break;
        
        case SIM7020_MSG_TCPUDP_CREATE:
        {
          printf("\r\nudp_cr=%s\r\n",msg);
        }
        break;
        
        case SIM7020_MSG_TCPUDP_CLOSE:
        {
          printf("\r\nUDP_CL=%s\r\n",msg);
        }
        break;
        
        case SIM7020_MSG_TCPUDP_SEND:
        {
          printf("\r\nUDP_SEND=%s\r\n",msg);
        }
        break;
        
        case SIM7020_MSG_TCPUDP_RECV:
        {
          printf("\r\nUDP_RECE=%s\r\n",msg);
        }
        break;
        
        case SIM7020_MSG_COAP_SERVER:
        {
          printf("\r\nCOAP=%s\r\n",msg);
        }
        break;
        
        case SIM7020_MSG_COAP_SEND:
        {
          printf("\r\nCOAP_SENT=%s\r\n",msg);
        }
        break;

        case SIM7020_MSG_COAP_RECV:
        {
            printf("\r\nCOAP_RECE=%s\r\n",msg);
        }
        break;

        default :
        {
            break;
        }
    }             
}


//sim7020 ״̬������
//sim7020_main_status��sim7020��������״̬�׶�
static void sim7020_app_status_poll(sim7020_handle_t sim7020_handle, int *sim7020_main_status)
{    
    switch(*sim7020_main_status)
    {
    case SIM7020_NONE:
      {
        
      }
      break;
           
    case SIM7020_NBLOT_INIT:
      {
        printf("atk_sim7020 init start\r\n");
                
        sim7020_nblot_init(sim7020_handle);        

        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_NBLOT_INFO:
      {
         printf("atk_sim7020 get signal start\r\n");
                
         sim7020_nblot_info_get(sim7020_handle);

         *sim7020_main_status = SIM7020_END;
      }
            
      break;
      
    case SIM7020_SIGNAL:
      {
        printf("atk_sim7020 rssi(db) start\r\n");
        
        sim7020_nblot_signal_get(sim7020_handle);
        
        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_TCPUDP_CR:
      {
        printf("tcp/udp socket creat start\r\n");  
        
//        sim7020_nblot_tcpudp_create(sim7020_handle, SIM7020_UDP);
        
        sim7020_nblot_tcpudp_create(sim7020_handle, SIM7020_TCP);
        
        //do nothing
        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_TCPUDP_CL:
      {          
        //do nothing
        printf("tcp/udp socket close start\r\n");  

//        sim7020_nblot_tcpudp_close(sim7020_handle, SIM7020_UDP); 

//        sim7020_nblot_tcpudp_close(sim7020_handle, SIM7020_TCP);        
          
        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_TCPUDP_SEND:
      {
          
        printf("tcp/udp send start\r\n"); 

//        sim7020_nblot_tcpudp_send_str(sim7020_handle, sizeof("hello world"), "hello world", SIM7020_UDP);
        
        sim7020_nblot_tcpudp_send_str(sim7020_handle, sizeof("hello world"), "hello world", SIM7020_TCP);        
        
        //do nothing
        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_TCPUDP_RECV:
      {         
        printf("tcp/udp recv start\r\n");    
        //do nothing
        *sim7020_main_status = SIM7020_END; 
      }
      break;
      
    case SIM7020_CoAP_SEVER:
      {
        printf("CoAP Server set start\r\n");

        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_CoAP_SEND:
      {
        printf("CoAP send start\r\n");
        *sim7020_main_status = SIM7020_END;
        
        
      }
      break;
      
    case SIM7020_CoAP_RECV:
      {
        printf("CoAP recv start\r\n");
        *sim7020_main_status = SIM7020_END;        
      }
      break;  
      
    default:
      {
        
      }
      break;
    }
}


/**
  * @brief  The demo sim7020 nblot info get application entry point.
  *
  * @retval None
  */
void demo_sim7020_nblot_info_get_entry(void)
{ 
       
    uart_handle_t nbiot_handle = NULL; 
 
    nbiot_handle = atk_nbiot_uart_init(115200);  
    
    sim7020_handle = sim7020_init(nbiot_handle);
     
    sim7020_event_registercb(sim7020_handle, __sim7020_event_cb_handler, sim7020_handle);
    
    //sim7020�ϵ���Ҫ�ȴ�10s
    delay_ms(1000);
             
    while (1)
    {   
        sim7020_app_status_poll(sim7020_handle, &sm7020_main_status);      
        sim7020_event_poll(sim7020_handle);      
        uart_event_poll(nbiot_handle);       
    }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

