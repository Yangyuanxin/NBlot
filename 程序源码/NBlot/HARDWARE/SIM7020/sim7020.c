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



//����sim7020�����շ������ṹ��
static struct sim7020_recv  sim7020_recv_desc;
static struct sim7020_send  sim7020_send_desc;

//�������ڱ��浱ǰ����ִ�е�ATָ������ṹ��
static at_cmd_info_t g_at_cmd;

//����sim7020�豸�ṹ��
static struct sim7020_dev       g_sim7020_dev;

//����sim7020״̬��Ϣ�ṹ��
static sim7020_status_nest_t    g_sim702_status;

//����sim7020�̼���Ϣ�ṹ��
static sim020_firmware_info_t   g_firmware_info;  

//��������
static int  __sim7020_uart_data_tx (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout);
static int  __sim7020_uart_data_rx (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout);
static void __uart_event_cb_handle(void *p_arg);

static struct sim7020_drv_funcs drv_funcs = {
    
    __sim7020_uart_data_tx,
    __sim7020_uart_data_rx,        
};


//����sim7020�¼�
void sim7020_event_set (sim7020_handle_t sim7020_handle, int sim7020_event)
{ 
    sim7020_handle->sim7020_event |= sim7020_event;   
}

//��ȡsim7020�¼�
int sim7020_event_get (sim7020_handle_t sim7020_handle,  int sim7020_event)
{ 
    return (sim7020_handle->sim7020_event & sim7020_event); 
}

//���sim7020�¼�
void sim7020_event_clr (sim7020_handle_t sim7020_handle, int sim7020_event)
{ 
    sim7020_handle->sim7020_event ^= sim7020_event;
}

static void __uart_event_cb_handle (void *p_arg)
{    
    uart_dev_t *p_uart_dev = (uart_dev_t *)p_arg; 
    
    if (p_uart_dev->uart_event & UART_NONE_EVENT) {
        
    } 

    if (p_uart_dev->uart_event & UART_TX_EVENT) {
        printf("tx data ok\r\n"); 
        lpuart_event_clr(p_uart_dev, UART_TX_EVENT); 
    }

    if (p_uart_dev->uart_event & UART_RX_EVENT) {
        printf("rx data ok\r\n");
        
        lpuart_event_clr(p_uart_dev, UART_RX_EVENT); 
    } 

    if (p_uart_dev->uart_event & UART_TX_TIMEOUT_EVENT) {
        printf("tx data timeout\r\n");
        
        lpuart_event_clr(p_uart_dev, UART_TX_TIMEOUT_EVENT); 
    } 

    if (p_uart_dev->uart_event & UART_RX_TIMEOUT_EVENT) {
        printf("rx data timeout\r\n");
        
        lpuart_event_clr(p_uart_dev, UART_RX_TIMEOUT_EVENT); 
    }            
}


//sim7020�¼�������
void sim7020_event_poll(sim7020_handle_t sim7020_handle)
{
    char buf[16] = {0};
    
    if (sim7020_handle->sim7020_event & SIM7020_NONE_EVENT) {
        
    } 

    if (sim7020_handle->sim7020_event & SIM7020_RECV_EVENT) {                       
        printf("sim7020 recv data ok\r\n"); 
        sim7020_event_clr(sim7020_handle, SIM7020_RECV_EVENT); 
    }

    if (sim7020_handle->sim7020_event & SIM7020_TIMEOUT_EVENT) {
        printf("timeout\r\n");
        
        sim7020_event_clr(sim7020_handle, SIM7020_TIMEOUT_EVENT); 
    } 

    if (sim7020_handle->sim7020_event & SIM7020_REG_STA_EVENT) {
        printf("gprs attach ok\r\n");
        
        sim7020_event_clr(sim7020_handle, SIM7020_REG_STA_EVENT); 
    } 

    if (sim7020_handle->sim7020_event & SIM7020_TCP_RECV_EVENT) {
        printf("tcp recv ok\r\n");
        
        sim7020_event_clr(sim7020_handle,SIM7020_TCP_RECV_EVENT); 
    }

    if (sim7020_handle->sim7020_event & SIM7020_UDP_RECV_EVENT) {
        printf("udp recv ok\r\n");
        
        sim7020_event_clr(sim7020_handle, SIM7020_UDP_RECV_EVENT); 
    }
    
    if (sim7020_handle->sim7020_event & SIM7020_COAP_RECV_EVENT) {
        
        printf("coap recv ok\r\n");
        
        sim7020_event_clr(sim7020_handle, SIM7020_COAP_RECV_EVENT); 
    }    
    
    //�ص�ע�������SIM7020�¼������� 
    sim7020_handle->sim7020_cb(sim7020_handle->p_arg, SIM7020_MSG_INIT, 16, buf);   
}

//sim7020 ״̬������
//sim7020_main_status��sim7020��������״̬�׶�
void sim7020_app_status_poll(int  *sim7020_main_status)
{    
    switch(*sim7020_main_status)
    {
    case SIM7020_NONE:
      {
        
      }
      break;
      
    case SIM7020_NBLOT_INIT:
      {
        printf("sim7020 init start\r\n");

        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_NBLOT_INFO:
      {
         printf("sim7020 get signal start\r\n");

         *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_SIGN:
      {
        printf("sim7020 module info start\r\n");
        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_UDP_CR:
      {
        printf("udp socket creat start\r\n");  
        //do nothing
        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_UDP_CL:
      {          
        //do nothing
        printf("udp socket close start\r\n");   
          
        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_UDP_SEND:
      {
          
        printf("udp send start\r\n");     
        //do nothing
        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_UDP_RECV:
      {
          
        printf("udp recv start\r\n");    
        //do nothing
        *sim7020_main_status = SIM7020_END; 
      }
      break;
      
    case SIM7020_TCP_CR:
      {
          
        printf("tcp socket creat start\r\n");  
          
        //do nothing
        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_TCP_CL:
      {
          
        printf("tcp socket close start\r\n");    
        //do nothing
        *sim7020_main_status = SIM7020_END;
      }
      break; 
      
    case SIM7020_TCP_SEND:
      {
        //do nothing
        printf("tcp send start\r\n");    
        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_TCP_RECV:
      {
          
        printf("tcp recv start\r\n");  
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


static int  __sim7020_uart_data_tx (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout)
{  
    int ret = 0;
    
    sim7020_handle_t  sim7020_handle = (sim7020_handle_t)p_arg;
    
    uart_handle_t uart_handle = sim7020_handle->p_uart_dev; 
    
    ret = uart_data_tx_poll(uart_handle, pData, size, Timeout); 

    return ret;    
}

static int  __sim7020_uart_data_rx (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout)
{  
    int ret = 0;
    
    sim7020_handle_t  sim7020_handle = (sim7020_handle_t)p_arg;
    
    uart_handle_t uart_handle = sim7020_handle->p_uart_dev; 
    
    ret = uart_data_rx_int(uart_handle, pData, size, Timeout); 

    return ret;    
}

//��1���ַ�ת��Ϊ16��������
//chr:�ַ�,0~9/A~F/a~F
//����ֵ:chr��Ӧ��16������ֵ
u8 sim7020_chr2hex(u8 chr)
{
    if(chr>='0'&&chr<='9')return chr-'0';
    if(chr>='A'&&chr<='F')return (chr-'A'+10);
    if(chr>='a'&&chr<='f')return (chr-'a'+10); 
    return 0;
}

//��1��16��������ת��Ϊ�ַ�
//hex:16��������,0~15;
//����ֵ:�ַ�
u8 sim7020_hex2chr(u8 hex)
{
    if(hex<=9)return hex+'0';
    if(hex>=10&&hex<=15)return (hex-10+'A'); 
    return '0';
}


//sim7020 atָ���ʼ��
static void at_cmd_param_init(at_cmdhandle cmd_handle,
                              const char *at_cmd,
                              char* argument,
                              cmd_property_t property,
                              uint32_t at_cmd_time_out)
{
    if(cmd_handle == NULL)
    {
      return;
    }
    cmd_handle->cmd_try     = CMD_TRY_TIMES;
    cmd_handle->property    = property;
    cmd_handle->cmd_action  = ACTION_ERROR_AND_TRY;
    cmd_handle->p_atcmd_arg = argument;
    cmd_handle->p_expectres = NULL;
    cmd_handle->have_tried  = 0;
    cmd_handle->max_timeout = at_cmd_time_out;
    cmd_handle->p_atcmd     = at_cmd;
}

//����sim7020 atָ����ַ��������Ӧ�ĳ���
static int cmd_generate(at_cmdhandle cmd_handle)
{
    int cmdLen = 0;
    if(cmd_handle == NULL)
    {
       return cmdLen;
    }
    memset(sim7020_send_desc.buf,0,NB_UART_SEND_BUF_MAX_LEN);
    sim7020_send_desc.len = 0;

    if(cmd_handle->property == CMD_TEST)
    {
        cmdLen = snprintf(sim7020_send_desc.buf,NB_UART_SEND_BUF_MAX_LEN,
                         "%s=?\r\n",
                         cmd_handle->p_atcmd);
    }    
    else if(cmd_handle->property == CMD_READ)
    {
        cmdLen = snprintf(sim7020_send_desc.buf,NB_UART_SEND_BUF_MAX_LEN,
                          "%s?\r\n",
                          cmd_handle->p_atcmd);
    }
    else if(cmd_handle->property == CMD_EXCUTE)
    {
        cmdLen = snprintf(sim7020_send_desc.buf,NB_UART_SEND_BUF_MAX_LEN,
                          "%s\r\n",
                          cmd_handle->p_atcmd);    
    }

    else if(cmd_handle->property == CMD_SET)
    {
        cmdLen = snprintf(sim7020_send_desc.buf,NB_UART_SEND_BUF_MAX_LEN,
                      "%s=%s\r\n",
                      cmd_handle->p_atcmd,cmd_handle->p_atcmd_arg);    
    }
    
    sim7020_send_desc.len = cmdLen;
    
    return cmdLen;
}


//�ж�atָ���Ƿ��ͳɹ�
static int8_t at_cmd_is_ok(char* buf)
{
  int8_t result = -1;
     
  if(g_at_cmd.p_atcmd == NULL)
  {
    if (strstr(buf,"OK"))
    {
      result = TRUE; ;
    }
    else if (strstr(buf,"ERROR"))
    {
      result = FALSE;
    }
  }
  else
  {
    if(strstr(buf,"OK"))
    {
      if(strstr(buf,g_at_cmd.p_atcmd))
      {
        result = TRUE;
      }
      else
      {
        result = FALSE;
      }
        
    }
    else if(strstr(buf,"ERROR"))
    {
      //+CMEE ERROR
      result = FALSE;
    }
  }

  return result;
}

//������һ��ATָ��
static uint8_t at_cmd_next (void)
{ 
    if (g_sim702_status.main_status == SIM7020_NBLOT_INIT)
    {
        g_sim702_status.sub_status++;
      
        if (g_sim702_status.sub_status == SIM7020_SUB_END)
        {
            return FALSE;
        }

        switch(g_sim702_status.sub_status)
        {
            
        //��ѯNB��״̬�Ƿ�׼����    
        case SIM7020_SUB_CPIN:
          {
            at_cmd_param_init(&g_at_cmd, AT_CPIN, NULL, CMD_READ, 2000);
          }
          break;
          
        //��ѯ��Ƶģ���ź�����   
        case SIM7020_SUB_CSQ:
          {
            at_cmd_param_init(&g_at_cmd, AT_CSQ, NULL, CMD_EXCUTE, 2000);
          }
          break;

        //ʹ��ģ����Ƶ�ź�,��Ӧ�ȴ����ʱ��Ϊ10S      
        case SIM7020_SUB_CFUN:                                   
          {
            at_cmd_param_init(&g_at_cmd, AT_CFUN,"1",CMD_SET, 11000);
          }
          break;
          
        // ʹ��NBlot����ע��   
        case SIM7020_SUB_CEREG:
          {
            at_cmd_param_init(&g_at_cmd, AT_CGREG, "1", CMD_SET, 2000);
          }
          break;      
          
        // ʹ��PDN     
        case SIM7020_SUB_CGACT:
          {
            at_cmd_param_init(&g_at_cmd, AT_CGACT,"1",CMD_SET, 151000);
          }
          break;
               
        case SIM7020_SUB_CGACT_QUERY:
          {
            at_cmd_param_init(&g_at_cmd, AT_CGATT, NULL, CMD_READ, 151000);
            g_at_cmd.p_expectres = "+CGACT:1,1"; //���������ظ���Ϣ�����ָ��ִ�����
                                                 //û������������Ϣƥ�䣬����Ϊ����
                                                 //�����г�����
          }
          break;
          
        //ʹ�����總��,�����Ӧʱ�䲻��      
        case SIM7020_SUB_CGAAT:
          {
            at_cmd_param_init(&g_at_cmd, AT_CGATT, "1", CMD_SET, 3000);
          }
          break;

        //��ѯ���總��,�����Ӧʱ�䲻��       
        case SIM7020_SUB_CGATT_QUERY:
          {
            at_cmd_param_init(&g_at_cmd, AT_CGATT, NULL, CMD_READ, 3000);
            
            //���������ظ���Ϣ�����ָ��ִ�����
            //û������������Ϣƥ�䣬����Ϊ����                                             
            //�����г�����               
            g_at_cmd.p_expectres = "CGATT:1";     
          }
          break;
          
        default:        
          break;   
         }
    }
    else if (g_sim702_status.main_status == SIM7020_NBLOT_INFO)
    {
        g_sim702_status.sub_status++;
      
        if (g_sim702_status.sub_status == SIM7020_SUB_END)
        {
          return FALSE;
        }
        switch(g_sim702_status.sub_status)
        {
        case SIM7020_SUB_CGMM:
          {
            at_cmd_param_init(&g_at_cmd,AT_CGMM,NULL,CMD_EXCUTE,2000);
          }
          break;
        case SIM7020_SUB_CGMR:
          {
            at_cmd_param_init(&g_at_cmd,AT_CGMR,NULL,CMD_EXCUTE,2000);
          }
          break;
        case SIM7020_SUB_NBAND:
          {
            at_cmd_param_init(&g_at_cmd,AT_NBAND,NULL,CMD_READ,2000);
          }
          break;
        }
    }
    else if (g_sim702_status.main_status == SIM7020_SIGN)
    {
        g_sim702_status.sub_status++;
        return FALSE;
    } 
    else  
    {
      
    }  
    return TRUE;
}


//sim7020����ATָ��
//sim7020_handle   sim7020_handle�豸���
//cmd_handle       ��Ҫ����ָ����Ϣ���
//note ���øú���ǰ�ȹ��������Ĳ���
static int sim7020_at_cmd_send(sim7020_handle_t sim7020_handle, at_cmdhandle cmd_handle)
{
    int strLen = 0;
    
    int ret = 0;
        
    if (sim7020_handle == NULL || cmd_handle == NULL)
    {
       return -1;
    }
        
    strLen = cmd_generate(cmd_handle);

    ret = sim7020_handle->p_drv_funcs->sim7020_send_data(sim7020_handle, 
                                                   (uint8_t*)sim7020_send_desc.buf, 
                                                   strLen,                                                    
                                                   cmd_handle->max_timeout);
    
    return ret;
}

//sim7020��������
//sim7020_handle   sim7020_handle�豸���
//cmd_handle       ��Ҫ����ָ����Ϣ���
//note ���øú���ǰ�ȹ��������Ĳ���
static int sim7020_data_recv(sim7020_handle_t sim7020_handle, at_cmdhandle cmd_handle)
{
    int strLen = 0;
        
    if (sim7020_handle == NULL || cmd_handle == NULL)
    {
       return -1;
    }
        
    strLen = cmd_generate(cmd_handle);

    sim7020_handle->p_drv_funcs->sim7020_recv_data(sim7020_handle, 
                                                   (uint8_t*)sim7020_recv_desc.buf, 
                                                   strLen,                                                                                                     
                                                   cmd_handle->max_timeout);
}


//sim7020��ʼ�� 
sim7020_handle_t sim7020_init(uart_handle_t lpuart_handle)
{
     //����豸�ṹ��
     g_sim7020_dev.p_uart_dev    = lpuart_handle;
     g_sim7020_dev.p_drv_funcs   = &drv_funcs; 
     g_sim7020_dev.firmware_info = &g_firmware_info;
     g_sim7020_dev.sim702_status = &g_sim702_status;    
    
     /* ע��sim7020�����շ��¼��ص����� */
     lpuart_event_registercb(lpuart_handle, __uart_event_cb_handle, lpuart_handle);     
    
     return &g_sim7020_dev;    
}

//ע��sim7020�¼��ص�����
void sim7020_event_registercb(sim7020_handle_t sim7020_handle, sim7020_cb cb, void *p_arg)
{  
    if(cb != 0)
    {
        sim7020_handle->sim7020_cb  = cb;
        sim7020_handle->p_arg       = p_arg;
    }
}

int sim7020_nblot_init(sim7020_handle_t sim7020_handle)
{
    
  if (g_sim702_status.main_status != SIM7020_NONE)
  {
      return SIM7020_ERROR;
  }
  
  at_cmd_param_init(&g_at_cmd, AT_SYNC, NULL,CMD_EXCUTE, 2000);
  
  //����SIM7020_NBLOT_INIT״̬
  g_sim702_status.main_status = SIM7020_NBLOT_INIT;
  g_sim702_status.sub_status  = SIM7020_SUB_SYNC;
   
  sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
    
  return SIM7020_OK;
}

   





