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

//���ڱ��浱ǰ����ִ�е�ATָ��
static at_cmd_info_t g_at_cmd;

static struct sim7020_recv  sim7020_recv_desc;
static struct sim7020_send  sim7020_send_desc;

static struct sim7020_dev   g_sim7020_dev;
static struct sim020_status g_sim020_status;  


static struct sim7020_drv_funcs drv_funcs = {
    
    uart_data_tx_poll,
    uart_data_rx_int        
};

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
    cmd_handle->cmd_action  = ACTION_OK;
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


//******************************************************************************
// fn : cmd_isPass
//
// brief : �ж�SIM7020ִ�е�ATָ���Ƿ�ִ�гɹ�
//
// param : none
//
// return : none
static int8_t at_cmd_isPass(char* buf)
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


//******************************************************************************
// fn : NB_SendCmd
//
// brief : ͨ��ע��Ĵ��ں������ⷢ��atָ��
//
// param : hw_handle ->  Ӳ����������ָ��
//         cmdHandle -> ��Ҫ����ָ����Ϣ
//
// return : none
static void sim7020_send_at_cmd(sim7020_handle_t sim7020_handle,at_cmdhandle cmd_handle)
{
    int strLen = 0;
    
    UART_HandleTypeDef *p_huart = sim7020_handle->p_uart_dev->p_huart; 
    
    if (sim7020_handle == NULL || cmd_handle == NULL)
    {
       return;
    }
        
    strLen = cmd_generate(cmd_handle);

    sim7020_handle->p_funcs->sim7020_send_data(p_huart, 
                                               (uint8_t*)sim7020_send_desc.buf, 
                                                strLen, 
                                                cmd_handle->max_timeout);
}

//sim7020��ʼ�� 
sim7020_handle_t sim7020_init(uart_handle_t lpuart_handle)
{

     g_sim7020_dev.p_uart_dev = lpuart_handle;
     g_sim7020_dev.p_funcs    = &drv_funcs; 
     
     return &g_sim7020_dev;    
}

//ע��sim7020�¼��ص�����
void lpuart_event_registercb(sim7020_cb cb, void *p_arg)
{  
    if(cb != 0)
    {
        g_sim7020_dev.sim7020_cb  = cb;
        g_sim7020_dev.p_arg       = p_arg;
    }
}


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




