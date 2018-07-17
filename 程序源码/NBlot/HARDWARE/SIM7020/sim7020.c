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
static struct sim7020_recv  g_sim7020_recv_desc;
static struct sim7020_send  g_sim7020_send_desc;

//�������ڱ��浱ǰ����ִ�е�ATָ������ṹ��
static at_cmd_info_t g_at_cmd;

//����sim7020�豸�ṹ��
static struct sim7020_dev       g_sim7020_dev;

//����sim7020״̬��Ϣ�ṹ��
static sim7020_status_nest_t    g_sim7020_status;

//����sim7020�̼���Ϣ�ṹ��
static sim020_firmware_info_t   g_firmware_info; 

//����socket��Ϣ�ṹ��
static sim7020_socket_info_t    g_socket_info[5];

//����������������
static int  __sim7020_uart_data_tx (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout);
static int  __sim7020_uart_data_rx (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout);


//�����¼��ص�������
static void __uart_event_cb_handle(void *p_arg);

//�ж����͵�ATָ��ִ�н��
static int8_t at_cmd_result_parse(char* buf);

//ָ����Ӧ�������
uint8_t sim7020_response_handle (sim7020_handle_t sim7020_handle, uint8_t cmd_response);

//������һ��ATָ��
static uint8_t at_cmd_next (void);
   
//����sim7020�¼�֪ͨ
static uint8_t sim7020_notify (sim7020_handle_t sim7020_handle, char *buf);

//������Ϣ��Ӧ�ò㽻��
static void sim7020_msg_send (sim7020_handle_t sim7020_handle, char**buf, int8_t is_ok);

//sim7020����ATָ��
//sim7020_handle   sim7020_handle�豸���
//cmd_handle       ��Ҫ����ָ����Ϣ���
//note ���øú���ǰ�ȹ��������Ĳ���
static int sim7020_at_cmd_send(sim7020_handle_t sim7020_handle, at_cmdhandle cmd_handle);

//sim7020��������
static int sim7020_data_recv(sim7020_handle_t sim7020_handle, uint32_t timeout);

static struct sim7020_drv_funcs drv_funcs = {    
    __sim7020_uart_data_tx,
    __sim7020_uart_data_rx,        
};

//��λ���ջ���
static void sim7020_recv_buf_reset(void)
{   
    memset(&g_sim7020_recv_desc, 0, sizeof(struct sim7020_recv));
}

//��λִ�н���
static void sim7020_status_reset(void)
{
    g_sim7020_status.main_status  = SIM7020_NONE;
    g_sim7020_status.sub_status   = SIM7020_SUB_NONE;
}


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

//�����¼��ص�������
static void __uart_event_cb_handle (void *p_arg)
{    
    sim7020_handle_t  sim7020_handle = (sim7020_handle_t)p_arg; 
    
    uart_dev_t       *p_uart_dev     = sim7020_handle->p_uart_dev;
    
    if (p_uart_dev->uart_event & UART_TX_EVENT) {
               
        printf("sim7020 tx ok %s", g_sim7020_send_desc.buf);  

        lpuart_event_clr(p_uart_dev, UART_TX_EVENT);                 
    }

    if (p_uart_dev->uart_event & UART_RX_EVENT) {
                
        g_sim7020_recv_desc.len = uart_ring_buf_avail_len(p_uart_dev);
        
        //�ӻ��������ж�ȡ����
        if (g_sim7020_recv_desc.len > 0)
        {
                        
            sim7020_data_recv(sim7020_handle, 0);
                       
            //�����첽�¼��ȴ�����
            sim7020_notify(sim7020_handle, g_sim7020_recv_desc.buf);
            
            printf("sim7020 rx ok %s\r\n", g_sim7020_recv_desc.buf);
            
        }
                                                                                 
        lpuart_event_clr(p_uart_dev, UART_RX_EVENT); 
    } 

    //�������ͳ�ʱ�¼���˵��ָ���п���û�з��ͳ�ȥ�����߷��͹����г���
    //����ģ�鹤���쳣��û�л�Ӧ���������
    if (p_uart_dev->uart_event & UART_TX_TIMEOUT_EVENT) {
        
        printf("sim7020 tx timeout %s", g_sim7020_send_desc.buf);  

        sim7020_event_set(sim7020_handle, SIM7020_TIMEOUT_EVENT);
      
        lpuart_event_clr(p_uart_dev, UART_TX_TIMEOUT_EVENT); 
    } 

    //���ʹ�÷ǳ�ʱ��֡�����¼������ϲ��ᷢ��
    if (p_uart_dev->uart_event & UART_RX_TIMEOUT_EVENT) {
        
        g_sim7020_recv_desc.len = uart_ring_buf_avail_len(p_uart_dev);
              

        if (g_sim7020_dev.frame_format == 1) 
        {
            if (g_sim7020_recv_desc.len > 0)
            {
                
                sim7020_data_recv(sim7020_handle, 0);
                
                //�����첽�¼��ȴ�����
                sim7020_notify(sim7020_handle, g_sim7020_recv_desc.buf);
            }
                                        
       //���ڳ�ʱ��֡��״̬�£������ȷ�����˳�ʱ�¼�      
       } else {
         
           sim7020_event_set(sim7020_handle, SIM7020_TIMEOUT_EVENT);
       }
       
       printf("sim7020 rx timeout %s\r\n", g_sim7020_recv_desc.buf);   
       lpuart_event_clr(p_uart_dev, UART_RX_TIMEOUT_EVENT);
    }            
}


//uint8_t tmp_buf[513];
//sim7020�¼�������
int sim7020_event_poll(sim7020_handle_t sim7020_handle)
{
    char *at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX] = {0};
    
    char *p_revc_buf_tmp = g_sim7020_recv_desc.buf;
    
    uint8_t index = 0;
        
    int8_t next_cmd = 0;
    
    int8_t cmd_is_pass = 0;
    
//    memcpy(tmp_buf,g_sim7020_recv_desc.buf, 512); 
        
    if (sim7020_handle->sim7020_event & SIM7020_RECV_EVENT) {
           
        cmd_is_pass = at_cmd_result_parse(g_sim7020_recv_desc.buf);
        
        //���������Ӧ�����ȷ      
        if (cmd_is_pass == AT_CMD_RESULT_OK) 
        {            
            //��ȡATָ��صĲ���,��ʹ��strok�ڼ䣬������ı仺���������ݣ��м�����ٶ��\r\n��Ҳֻ�ᵱ��һ��������
            while((at_response_par[index] = strtok(p_revc_buf_tmp,"\r\n")) != NULL)
            {
                index++;
                p_revc_buf_tmp = NULL;
                
                if (index >= (AT_CMD_RESPONSE_PAR_NUM_MAX - 1))
                {
                    break;
                }  
            }

            if(index == 0)
            {
                //���ݽ����������·��͸�����
                next_cmd = sim7020_response_handle(sim7020_handle, FALSE);
              
                //�建��            
                sim7020_recv_buf_reset();                            
                                        
                sim7020_event_clr(sim7020_handle, SIM7020_RECV_EVENT); 
                                       
                //δ�յ���ȷ������֡
                return  SIM7020_ERROR;
            }            
                                   
        }
        
        if(cmd_is_pass == AT_CMD_RESULT_OK)
        {
            
              printf("%s cmd excute ok \r\n\r\n", g_at_cmd.p_atcmd);
            
              //��������ͳɹ��˲��õ���ȷ����Ӧ
              sim7020_msg_send(sim7020_handle, at_response_par, TRUE);
                
              next_cmd = sim7020_response_handle(sim7020_handle, TRUE);
              
              //�建��            
              sim7020_recv_buf_reset();
        }
        
        else if(cmd_is_pass == AT_CMD_RESULT_ERROR)
        {              
              
              next_cmd = sim7020_response_handle(sim7020_handle, FALSE);     
          
              if (g_at_cmd.cmd_action & ACTION_ERROR_AND_TRY) { 
                                    
                  printf("%s cmd is failed and try\r\n", g_at_cmd.p_atcmd);
                  
                  at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = "error and try\r\n";
                  
                  //֪ͨ�ϲ�Ӧ�ã��˶���ִ��ʧ��,��������
                  sim7020_msg_send(sim7020_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], SIM7020_ERROR_RETRY);
                  
              } else if (g_at_cmd.cmd_action & ACTION_ERROR_BUT_NEXT) {
                  
                  at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = "error but next\r\n";
                 
                  printf("%s cmd is failed and exit\r\n", g_at_cmd.p_atcmd);
                  
                  //֪ͨ�ϲ�Ӧ�ã��˶���ִ��ʧ�ܺ�����������ִ��
                  sim7020_msg_send(sim7020_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], SIM7020_ERROR_NEXT);
                  
              } else {
                  
                  at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = "over max try, but failed\r\n";
                  
                  //֪ͨ�ϲ�Ӧ�ã��˶���ִ��ʧ�ܺ�����������ִ��
                  sim7020_msg_send(sim7020_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], FALSE);                  

              } 
                      
              //�建��            
              sim7020_recv_buf_reset();
        } 

        else if (cmd_is_pass == AT_CMD_RESULT_CONTINUE)
        {
            //����δִ����ɣ���������£��յ��ĵ����������, �������Ļ��ǵ�ǰ������Ӧ���ݵĽ���, �����������ճ�ʱ               
            atk_soft_timer_timeout_change(&sim7020_handle->p_uart_dev->uart_rx_timer, 500);
          
        }       
        else 
        {
            //�����ϲ������е�����,         
            //���������ʱһ��ʱ�䣬��ֹ�������ݹ�������Щ���ݶ�����������

            if (g_sim7020_recv_desc.len > 0) {          
                                    
                next_cmd = sim7020_response_handle(sim7020_handle, FALSE); 
            }              
          
            printf("the err buf %s\r\n",g_sim7020_recv_desc.buf);

            printf("the err buf len %d\r\n",g_sim7020_recv_desc.len);             
        
            //�建��            
            sim7020_recv_buf_reset();          

        }            
        
        sim7020_event_clr(sim7020_handle, SIM7020_RECV_EVENT); 
    }

    if (sim7020_handle->sim7020_event & SIM7020_TIMEOUT_EVENT) {
                          
        at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = "cmd not repsonse or send failed\r\n";
      
        printf("%s cmd not repsonse or send failed\r\n", g_at_cmd.p_atcmd);
        
        //֪ͨ�ϲ�Ӧ�ã��˶���ִ�г�ʱ
        sim7020_msg_send(sim7020_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], SIM7020_ERROR_TIMEOUT);   
          
        sim7020_recv_buf_reset();      
        
        //��ʱ����        
        next_cmd = sim7020_response_handle(sim7020_handle, FALSE);
      
        sim7020_event_clr(sim7020_handle, SIM7020_RECV_EVENT); 
                                          
        sim7020_event_clr(sim7020_handle, SIM7020_TIMEOUT_EVENT); 
    } 

    if (sim7020_handle->sim7020_event & SIM7020_REG_STA_EVENT) {
      
        printf("reg ok\r\n");
        
        at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = (char *)&g_sim7020_status.register_status;        
          
        //֪ͨ�ϲ�Ӧ������ע����
        sim7020_msg_send(sim7020_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], TRUE);        

        //��������Ƿ�ִ����һ������        
        next_cmd = sim7020_response_handle(sim7020_handle, TRUE);
        
        sim7020_event_clr(sim7020_handle,   SIM7020_REG_STA_EVENT); 
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

    //�����next
    if(next_cmd)
    {
        //ִ����һ������
        if (at_cmd_next())
        {
            sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
        }
        //����FALSE��ʾ�ӽ����Ѿ�������
        else
        {
            //�������״̬�����е���״̬�����Ѿ�������
            sim7020_msg_send(sim7020_handle, NULL,TRUE);

            //��λ״̬��־
            sim7020_status_reset();
        }     
    }

    return SIM7020_OK;    
}

//sim7020 ״̬������
//sim7020_main_status��sim7020��������״̬�׶�
void sim7020_app_status_poll(sim7020_handle_t sim7020_handle, int *sim7020_main_status)
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
                
        sim7020_nblot_init(sim7020_handle);        

        *sim7020_main_status = SIM7020_END;
      }
      break;
      
    case SIM7020_NBLOT_INFO:
      {
         printf("sim7020 get signal start\r\n");
        
        
         sim7020_nblot_info_get(sim7020_handle);

         *sim7020_main_status = SIM7020_END;
      }
            
      break;
      
    case SIM7020_SIGNAL:
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
    
    //��ʱ��������ʹ��
    (void)Timeout;
    
    sim7020_handle_t  sim7020_handle = (sim7020_handle_t)p_arg;
    
    uart_handle_t uart_handle = sim7020_handle->p_uart_dev;

    uart_ring_buf_read(uart_handle, pData, size);    
    
//    ret = uart_data_rx_int(uart_handle, pData, size, Timeout); 

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
                              char *argument,
                              cmd_property_t property,
                              uint32_t at_cmd_time_out)
{
    if(cmd_handle == NULL)
    {
      return;
    }
    cmd_handle->cmd_try     = CMD_TRY_TIMES;
    cmd_handle->property    = property;
    cmd_handle->cmd_action  = ACTION_OK_AND_NEXT | ACTION_ERROR_AND_TRY;
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
    memset(g_sim7020_send_desc.buf,0,NB_UART_SEND_BUF_MAX_LEN);
    g_sim7020_send_desc.len = 0;

    if(cmd_handle->property == CMD_TEST)
    {
        cmdLen = snprintf(g_sim7020_send_desc.buf,NB_UART_SEND_BUF_MAX_LEN,
                          "%s=?\r\n",
                          cmd_handle->p_atcmd);
    }    
    else if(cmd_handle->property == CMD_READ)
    {
        cmdLen = snprintf(g_sim7020_send_desc.buf,NB_UART_SEND_BUF_MAX_LEN,
                          "%s?\r\n",
                          cmd_handle->p_atcmd);
    }
    else if(cmd_handle->property == CMD_EXCUTE)
    {
        cmdLen = snprintf(g_sim7020_send_desc.buf,NB_UART_SEND_BUF_MAX_LEN,
                          "%s\r\n",
                          cmd_handle->p_atcmd);    
    }

    else if(cmd_handle->property == CMD_SET)
    {
        cmdLen = snprintf(g_sim7020_send_desc.buf,NB_UART_SEND_BUF_MAX_LEN,
                          "%s=%s\r\n",
                          cmd_handle->p_atcmd,cmd_handle->p_atcmd_arg);    
    }
    
    g_sim7020_send_desc.len = cmdLen;
    
    return cmdLen;
}


//�ж�atָ���Ƿ��ͳɹ�
//����ֵ�����-1������յ������ݿ���Ϊ����
//����ֵ�����0�� ָ��ͨ���������1��ָ�����
static int8_t at_cmd_result_parse(char* buf)
{
    int8_t result = -1;
       
    if(g_at_cmd.p_expectres == NULL)
    {
        if (strstr(buf,"\r\nOK\r\n"))
        {
            result = AT_CMD_RESULT_OK;
        }
        else if (strstr(buf,"\r\nERROR\r\n"))
        {
            result = AT_CMD_RESULT_ERROR;
          
        } else if (strstr(buf,g_at_cmd.p_atcmd)) {
          
           //������Ļ���
           result = AT_CMD_RESULT_CONTINUE;
          
        } else {
          
           //����
           result = AT_CMD_RESULT_RANDOM_CODE;
        }
    }
    else
    {
        if (strstr(buf,"\r\nOK\r\n"))
        {
            //��õ�������ֵһ��
            if(strstr(buf,g_at_cmd.p_expectres))
            {
                result = AT_CMD_RESULT_OK;
            }
            else
            {
                result = AT_CMD_RESULT_ERROR;
            }
            
        }
        else if(strstr(buf,"\r\nERROR\r\n"))
        {
            //ERROR
            result = AT_CMD_RESULT_ERROR;
        }
         
        else if (strstr(buf, g_at_cmd.p_atcmd)) {
          
             result = AT_CMD_RESULT_CONTINUE;
          
        } else {
          
             //����
             result = AT_CMD_RESULT_RANDOM_CODE;      
          
        }    
    }

    return result;
}


//����sim7020�¼�֪ͨ
static uint8_t sim7020_notify (sim7020_handle_t sim7020_handle, char *buf)
{
   char *target_pos_start = NULL;
    
   if((target_pos_start = strstr(buf, "+CGREG:")) != NULL)
    {
        char *p_colon = strchr(target_pos_start,':');
      
        if (p_colon)
        {
            p_colon = p_colon + 4;
            
            //���������ֱ�Ӹ�������ע��״̬����Ϣ�����ַ�����ʾ�ģ�Ҫת��������        
            g_sim7020_status.register_status = (*p_colon - '0');
        }
        
        //����ע���¼�
        if (g_sim7020_status.register_status == 1) {

            sim7020_event_set(sim7020_handle, SIM7020_REG_STA_EVENT);
        }
        else 
        {
            
            //����յ��ظ���������������Ӧ������
            sim7020_event_set(sim7020_handle, SIM7020_RECV_EVENT);  
        }
    }
    
    else if((target_pos_start = strstr(buf,"+CSONMI")) != NULL)
    {
        //�յ��������˷�����SOCKET������
        char* p_colon = strchr(target_pos_start,':');
        
        if(p_colon)
        {
          p_colon++;
          g_socket_info[0].socket_id = strtoul(p_colon,0,10);
        }
        
        char* pComma = strchr(p_colon,',');

        if(pComma)
        {
          pComma++;
          g_socket_info[0].data_len = strtoul(pComma,0,10);
        }
        
        if (g_socket_info[0].socket_type == 2) {

           sim7020_event_set(sim7020_handle, SIM7020_UDP_RECV_EVENT); 
            
        } else if (g_socket_info[0].socket_type == 1) {

           sim7020_event_set(sim7020_handle, SIM7020_TCP_RECV_EVENT); 
        }            
    } 
    
    //�յ�Coap���ݰ�
    else if((target_pos_start = strstr(buf,"+CCOAPNMI")) != NULL)
    {

        sim7020_event_set(sim7020_handle, SIM7020_COAP_RECV_EVENT);  
    }      
    
    else if((target_pos_start = strstr(buf,"+CLMOBSERVE")) != NULL)
    {

        sim7020_event_set(sim7020_handle, SIM7020_LWM2M_RECV_EVENT);  
        
    } 

    //�յ�MQTT���ݰ�    
    else if ((target_pos_start = strstr(buf,"+CMQPUB")) != NULL)
    {
        
        sim7020_event_set(sim7020_handle, SIM7020_MQTT_RECV_EVENT);  

    }
    else 
    {
        //����յ��ظ���������������Ӧ������
        sim7020_event_set(sim7020_handle, SIM7020_RECV_EVENT);  
    }        

    return 0;
}

//������һ��ATָ��
static uint8_t at_cmd_next (void)
{ 
    if (g_sim7020_status.main_status == SIM7020_NBLOT_INIT)
    {
        g_sim7020_status.sub_status++;
      
        if (g_sim7020_status.sub_status == SIM7020_SUB_END)
        {
            return FALSE;
        }

        switch(g_sim7020_status.sub_status)
        {
          
        case SIM7020_SUB_SYNC:
            
            break;
        
        case SIM7020_SUB_CMEE:
          
            at_cmd_param_init(&g_at_cmd, AT_CMEE, "1", CMD_SET, 3000);
          
            break;
        
        case SIM7020_SUB_ATI:
          
            at_cmd_param_init(&g_at_cmd, AT_ATI, NULL, CMD_EXCUTE, 3000);
            g_at_cmd.p_expectres = "SIM";         //���������ظ���Ϣ�����ָ��ִ�����
                                                  //û������������Ϣƥ�䣬����Ϊ����
                                                  //�����г�����           
            break;
                      
        //��ѯNB��״̬�Ƿ�׼����    
        case SIM7020_SUB_CPIN:
          {
            at_cmd_param_init(&g_at_cmd, AT_CPIN, NULL, CMD_READ, 3000);
            g_at_cmd.p_expectres = "+CPIN: READY"; //���������ظ���Ϣ�����ָ��ִ�����
                                                  //û������������Ϣƥ�䣬����Ϊ����
                                                  //�����г�����              
          }
          break;
                    
          
        //��ѯ��Ƶģ���ź�����   
        case SIM7020_SUB_CSQ:
          {
             at_cmd_param_init(&g_at_cmd, AT_CSQ, NULL, CMD_EXCUTE, 3000);
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
            at_cmd_param_init(&g_at_cmd, AT_CGREG, "1", CMD_SET, 3000);
          }
          break;  
          
//        //�Ƚ���PDN�� ������150S֮�ڻ��л�Ӧ     
//        case SIM7020_SUB_CGACT_DISABLE:
//          {
//            at_cmd_param_init(&g_at_cmd, AT_CGACT,"0,1",CMD_SET, 151000);
//          }
//          break;          
//    
//          
        //��ʹ��PDN�� ������150S֮�ڻ��л�Ӧ�����Ƚ��ܵĻ�����ʹ��״ִ̬��ִ��������     
//        case SIM7020_SUB_CGACT:
//          {
//            at_cmd_param_init(&g_at_cmd, AT_CGACT,"1,1",CMD_SET, 151000);
//          }
//          break;
        
        //��ѯPDN������Ϣ          
        case SIM7020_SUB_CGACT_QUERY:
          {
            at_cmd_param_init(&g_at_cmd, AT_CGACT, NULL, CMD_READ, 151000);
            g_at_cmd.p_expectres = "+CGACT: 1,1"; //���������ظ���Ϣ�����ָ��ִ�����
                                                  //û������������Ϣƥ�䣬����Ϊ����
                                                  //�����г�����
          }
          break;          
               
          
        //ʹ�����總��,�����Ӧʱ�䲻��      
        case SIM7020_SUB_CGATT:
          {
            at_cmd_param_init(&g_at_cmd, AT_CGATT, "1", CMD_SET, 3000);
          }
          break;

        //��ѯ���總����Ϣ,�����Ӧʱ�䲻��       
        case SIM7020_SUB_CGATT_QUERY:
          {
            at_cmd_param_init(&g_at_cmd, AT_CGATT, NULL, CMD_READ, 3000);
            
            //���������ظ���Ϣ�����ָ��ִ�����
            //û������������Ϣƥ�䣬����Ϊ����                                             
            //�����г�����               
            g_at_cmd.p_expectres = "CGATT: 1";     
          }
          break;
          
        //��ѯ�Ƿ���ʹ��NBlot����,�����Ӧʱ�䲻��       
        case SIM7020_SUB_COPS_QUERY:
          {
            at_cmd_param_init(&g_at_cmd, AT_COPS, NULL, CMD_READ, 3000);
            
            //���������ظ���ϢΪ9��������NBloT����
            //���ָ��ִ�����,û������������Ϣƥ��                                            
            //����Ϊ���������г�����               
            g_at_cmd.p_expectres = "9";     
          }
          break;
          
 
        //��ѯ�����APN��Ϣ��IP��ַ     
        case SIM7020_SUB_CGCONTRDP_QUERY:
          {
            at_cmd_param_init(&g_at_cmd, AT_CGCONTRDP, NULL, CMD_EXCUTE, 3000);
               
          }
          break;
                    
          
        //��ѯNBlot�����Ƿ�ע��,�����Ӧʱ�䲻��       
        case SIM7020_SUB_CEREG_QUERY:
          {
            at_cmd_param_init(&g_at_cmd, AT_CGREG, NULL, CMD_READ, 3000); 
            //���������ظ���ϢΪ��
            //���ָ��ִ�����,û������������Ϣƥ��                                            
            //����Ϊ���������г�����               
            g_at_cmd.p_expectres = "CGREG: 1,1";              

          }
          break;
                    
        default: 
          
          //ǿ�Ʊ�ʾ�ӽ��̽���
          g_sim7020_status.sub_status = SIM7020_SUB_END;
        
          return FALSE;
                   
         }
    }
    
    else if (g_sim7020_status.main_status == SIM7020_NBLOT_INFO)
    {
        g_sim7020_status.sub_status++;
      
        if (g_sim7020_status.sub_status == SIM7020_SUB_END)
        {
            return FALSE;
        }
        
        switch(g_sim7020_status.sub_status)
        {
          
        case  SIM7020_SUB_CGMI:
          
          {
            at_cmd_param_init(&g_at_cmd,AT_CGMI,NULL,CMD_EXCUTE,3000);
          }
          break;

                   
        case SIM7020_SUB_CGMM:
          {
            at_cmd_param_init(&g_at_cmd,AT_CGMM,NULL,CMD_EXCUTE,3000);
          }
          break;
          
        case SIM7020_SUB_CGMR:
          {
            at_cmd_param_init(&g_at_cmd,AT_CGMR,NULL,CMD_EXCUTE,3000);
          }
          break;
          
        case SIM7020_SUB_CIMI:
          {
            at_cmd_param_init(&g_at_cmd,AT_CIMI,NULL,CMD_EXCUTE,3000);
          }
          break;

        case SIM7020_SUB_CGSN:
          {
            at_cmd_param_init(&g_at_cmd, AT_CGSN, NULL, CMD_EXCUTE, 3000);
            
            //���������ظ���ϢΪ��
            //���ָ��ִ�����,û������������Ϣƥ��                                            
            //����Ϊ���������г�����               
            g_at_cmd.p_expectres = "CGSN";
          }
          break;             
          
          
        case SIM7020_SUB_CBAND:
          {
            at_cmd_param_init(&g_at_cmd,AT_CBAND,NULL,CMD_READ,3000);
            //���������ظ���ϢΪ��
            //���ָ��ִ�����,û������������Ϣƥ��                                            
            //����Ϊ���������г�����               
            g_at_cmd.p_expectres = "CBAND";
          }
          break;
          
        default: 
          
          //ǿ�Ʊ�ʾ�ӽ��̽���
          g_sim7020_status.sub_status = SIM7020_SUB_END;
        
          return FALSE;          
        
        }
    }
    else if (g_sim7020_status.main_status == SIM7020_SIGNAL)
    {
        g_sim7020_status.sub_status = SIM7020_SUB_END;
        return FALSE;
    } 
    else  
    {
      
    }  
    return TRUE;
}

//������Ϣ��Ӧ�ò㽻��
static void sim7020_msg_send (sim7020_handle_t sim7020_handle, char**buf, int8_t is_ok)
{
  if (sim7020_handle == NULL)
  {
      return;
  }
  
  if ((is_ok == SIM7020_ERROR_RETRY) || 
      (is_ok == SIM7020_ERROR_NEXT)  ||
      (is_ok == SIM7020_ERROR_TIMEOUT)) {
        
      sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)g_sim7020_status.main_status, strlen(buf[0]), buf[0]);  
        
      return;      
  }
      
  //�������ϱ�������ִ��ʧ��
  else if(is_ok == FALSE)
  {
    sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)g_sim7020_status.main_status,1,"F");
    return;
  }
  
  if (g_sim7020_status.main_status == SIM7020_NBLOT_INIT)
  {
    switch(g_sim7020_status.sub_status)
    {
        
    case SIM7020_SUB_SYNC:
        
      break;
    
    case SIM7020_SUB_CMEE:
      
      break;    
    
    case SIM7020_SUB_ATI:
      
      //�õ�ģ�������
      memcpy(g_firmware_info.name, buf[1], strlen(buf[1])); 
        
      break;    
    
    case SIM7020_SUB_CPIN:
        
      break;
    
    
    case SIM7020_SUB_CSQ:
    {
        char *pColon = strchr(buf[0],':');
        pColon++;
      
        //ת����10��������
        uint8_t lqi =strtoul(pColon,0, 10);
      
        printf("rssi = %d\r\n",lqi); 
        //����ȡ��ÿ����ֵ��Ӧ��dbm��Χ
        int8_t rssi = -110 + (lqi << 1);
        g_sim7020_status.rssi = rssi;       
        break;
    }  
            
    
    case SIM7020_SUB_CFUN:
        break;
    

    case SIM7020_SUB_CEREG:
        
    
        break;
    
    
//    case SIM7020_SUB_CGACT_DISABLE:
//           
//    
//    case SIM7020_SUB_CGACT:
//        
//        break;
//    
//    
    case SIM7020_SUB_CGACT_QUERY:
        
        break;
    
        
    case SIM7020_SUB_CGATT:
        
        break;
    
    
    case SIM7020_SUB_CGATT_QUERY:
        
        break;
    
    case SIM7020_SUB_COPS_QUERY:
        
        break;  

    case SIM7020_SUB_CGCONTRDP_QUERY:
        
        break;       
       
    case SIM7020_SUB_END:
        
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_NBLOT_INIT,1,"S");
    
        break;
    
    default:
      
        break;
    }
  }
  else if(g_sim7020_status.main_status == SIM7020_NBLOT_INFO)
  {
    switch(g_sim7020_status.sub_status)
    {
        
    //��ѯ����ע��״̬    
    case SIM7020_SUB_CEREG_QUERY:

      sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_REG, 1, buf[0]);
    
      break;        
        
                 
    case SIM7020_SUB_CGMI:
      {
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_MID, strlen(buf[0]), buf[0]);
      }
      break;
      
    case SIM7020_SUB_CGMM:
      {
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_MMODEL,strlen(buf[0]),buf[0]);
      }
      break;
      
    case SIM7020_SUB_CGMR:
      {

         sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_MREV,strlen(buf[0]),buf[0]);
        
      }
      break;
      
      
    case SIM7020_SUB_CIMI:
      {
        memcpy(g_firmware_info.IMSI,buf[0],15);
        g_firmware_info.IMSI[15] = 0;
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_IMSI,strlen(buf[0]),buf[0]);
      }
      break;
      
    
    case SIM7020_SUB_CGSN:
      {
        char* pColon = strchr(buf[0],':');
        if(pColon)
        {
          pColon++;
          memcpy(g_firmware_info.IMEI ,pColon,15);
          g_firmware_info.IMEI[15] = 0;
          sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_IMEI,15,(char*)g_firmware_info.IMEI);
        }
      }
      break;            
      
    case SIM7020_SUB_CBAND:
      {
        char *pColon = strchr(buf[0],':');
        char *pFreq = NULL;
        if(pColon)
        {
          pColon++;
          uint8_t hz_id = strtoul(pColon,0,10);
          if(hz_id == BAND_850MHZ_ID)
          {
            //850MHZ
            pFreq = BAND_850MHZ_STR;
          }
          else if(hz_id == BAND_900MHZ_ID)
          {
            //900MHZ
            pFreq = BAND_900MHZ_STR;
          }
          else if(hz_id == BAND_800MHZ_ID)
          {
            //800MHZ 
            pFreq = BAND_800MHZ_STR;
          }
          else 
          {
            //700MHZ
            pFreq = BAND_700MHZ_STR;
          }
          sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_BAND,strlen(pFreq),pFreq);
        }
      }
      break;

    case SIM7020_SUB_END:
      {
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_NBLOT_INFO, 1, "S");
      }
      break;
    }
  }
  else if(g_sim7020_status.main_status == SIM7020_SIGNAL)
  {
    if(g_sim7020_status.sub_status == 1)
    {
      char* pColon = strchr(buf[0],':');
      pColon++;
      //ת����10��������
      uint8_t lqi =strtoul(pColon,0, 10);
      //����ȡ��ÿ����ֵ��Ӧ��dbm��Χ
      int8_t rssi = -110 + (lqi << 1);
      uint8_t len = snprintf(buf[0],10,"%d",rssi);
      *(buf[0]+len) = 0;
      
      g_sim7020_status.rssi = rssi;
      sim7020_handle->sim7020_cb(sim7020_handle->p_arg,(sim7020_msg_id_t)SIM7020_MSG_SIGN,len,buf[0]);
    }

  }
  else if(g_sim7020_status.main_status == SIM7020_UDP_CR)
  {
    switch(g_sim7020_status.sub_status)
    {
    case SIM7020_SUB_UDP_CR:
      {
//        memcpy(g_firmware_info.nb95_udp_id,buf[0],2);
        
        if(g_sim7020_status.sub_status == 1)
        {
          sim7020_handle->sim7020_cb(sim7020_handle->p_arg,(sim7020_msg_id_t)SIM7020_UDP_CR,1,"S");
        }
      }
      break;
      
    case SIM7020_SUB_END:
      {
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_UDP_CREATE,1,"S");
      }
      break;
    }
  }
  else if(g_sim7020_status.main_status == SIM7020_UDP_CL)
  {
    switch(g_sim7020_status.sub_status)
    {
    case SIM7020_SUB_UDP_CL:
      {
//        g_firmware_info.nb95_udp_id[0] = 0;
//        g_firmware_info.nb95_udp_id[1] = 0;
      }
      break;
    case SIM7020_SUB_END:
      {
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_UDP_CLOSE,1,"S");
      }
      break;
    } 

  }
  else if(g_sim7020_status.main_status == SIM7020_UDP_SEND)
  {
    switch(g_sim7020_status.sub_status)
    {
    case SIM7020_SUB_UDP_ST:
      {
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_UDP_SEND,1,"S");
      }
      break;
    }
  }
  else if(g_sim7020_status.main_status == SIM7020_UDP_RECV)
  {
    if(g_sim7020_status.sub_status == 1)
    {
      //
      char* param[6];
      uint16_t index = 0;
      char* tmp_buf = buf[0];
      while(( param[index] = strtok(tmp_buf,",")) != NULL)
      {
        index++;
        tmp_buf = NULL;
        if(index >= 6)
        {
          break;
        }
      }
      if(index != 6)
      {
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_UDP_RECV,1,"F");
        return;
      }
      
      tmp_buf = param[4];
      index =  0; //NB_HexStrToNum(tmp_buf);
      
      sim7020_handle->sim7020_cb(sim7020_handle->p_arg,(sim7020_msg_id_t)SIM7020_MSG_UDP_RECV,index,tmp_buf);
    }
    
  }
  else if(g_sim7020_status.main_status == SIM7020_CoAP_SEVER)
  {
    if(g_sim7020_status.sub_status == 1)
    {
      char* tmp_buf = NULL;
      if(strstr(buf[0],"OK"))
      {
        tmp_buf = "S";
      }
      else
      {
        tmp_buf = strchr(buf[1],':');
        if(tmp_buf)
        {
          tmp_buf++;
        }
      }
      
      sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_COAP,strlen(tmp_buf),tmp_buf);
    }
  }
  else if(g_sim7020_status.main_status == SIM7020_CoAP_SEND)
  {
    if(g_sim7020_status.sub_status == 1)
    {
      sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_COAP_SEND,1,"S");
    }
  }
  else if(g_sim7020_status.main_status == SIM7020_CoAP_RECV)
  {
    if(g_sim7020_status.sub_status == 1)
    {
      uint16_t index = 0;
      char* tmp_buf = NULL;
     
      tmp_buf = strchr(buf[0],',');
      if(tmp_buf)
      {
        tmp_buf++;
        index = 0;//NB_HexStrToNum(tmp_buf);
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_COAP_RECV, index,tmp_buf); 
      }
   
    }
  }    
}

//ָ����Ӧ�����������������������״̬���ԣ�������״̬��ִ�С�
uint8_t sim7020_response_handle (sim7020_handle_t sim7020_handle, uint8_t cmd_response)
{
    uint8_t next_cmd = 0;
      
    if (cmd_response)
    {
        if (g_at_cmd.cmd_action & ACTION_OK_AND_NEXT)
        {
            next_cmd = TRUE;
        }
        
        else   
        {

            //��������ִ�гɹ����˳�
             g_at_cmd.cmd_action = ACTION_OK_EXIT;
             
        }
    }
    else
    {
        if(g_at_cmd.cmd_action & ACTION_ERROR_AND_TRY)
        {
            g_at_cmd.have_tried++;

            if (g_at_cmd.have_tried < g_at_cmd.cmd_try)
            {
              sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
            }
            else
            {
                  
               //��������ִ�д�����˳�
                g_at_cmd.cmd_action = ACTION_ERROR_EXIT;
                
            }
        }
        else if (!(g_at_cmd.cmd_action & ACTION_ERROR_EXIT))  
        {
            //ACTION_ERROR_BUT_NEXT
            next_cmd = TRUE;
        }
        else 
        {
            //nerver reach here  
        }
    }
    
    return next_cmd;
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
       return SIM7020_ERROR;
    }
        
    strLen = cmd_generate(cmd_handle);

    ret = sim7020_handle->p_drv_funcs->sim7020_send_data(sim7020_handle, 
                                                         (uint8_t*)g_sim7020_send_desc.buf, 
                                                         strLen,                                                    
                                                         cmd_handle->max_timeout);
    
    return ret;
}

//sim7020��������
//sim7020_handle   sim7020_handle�豸���
static int sim7020_data_recv(sim7020_handle_t sim7020_handle, uint32_t timeout)
{   
    int ret = 0;
        
    if (sim7020_handle == NULL)
    {
       return SIM7020_ERROR;
    }
        

    ret = sim7020_handle->p_drv_funcs->sim7020_recv_data(sim7020_handle, 
                                                         (uint8_t*)g_sim7020_recv_desc.buf, 
                                                         g_sim7020_recv_desc.len,                                                                                                     
                                                         timeout);
    
    //�ں�������ַ���������
    g_sim7020_recv_desc.buf[g_sim7020_recv_desc.len]=0;
    
    return ret;
}


//sim7020��ʼ�� 
sim7020_handle_t sim7020_init(uart_handle_t lpuart_handle)
{
     //����豸�ṹ��
     g_sim7020_dev.p_uart_dev    = lpuart_handle;
     g_sim7020_dev.p_drv_funcs   = &drv_funcs; 

     g_sim7020_dev.p_sim702_cmd  = &g_at_cmd;    
     g_sim7020_dev.p_socket_info = g_socket_info;
     g_sim7020_dev.firmware_info = &g_firmware_info;
     g_sim7020_dev.sim702_status = &g_sim7020_status;

     g_sim7020_dev.frame_format  = 0;  
    
     /* ע��sim7020�����շ��¼��ص����� */
     lpuart_event_registercb(lpuart_handle, __uart_event_cb_handle, &g_sim7020_dev);     
    
     return &g_sim7020_dev;    
}

//ע��sim7020�¼��ص�����
void sim7020_event_registercb (sim7020_handle_t sim7020_handle, sim7020_cb cb, void *p_arg)
{  
    if(cb != 0)
    {
        sim7020_handle->sim7020_cb  = cb;
        sim7020_handle->p_arg       = p_arg;
    }
}

//sim7020 nblot��ʼ�����������ע��
int sim7020_nblot_init (sim7020_handle_t sim7020_handle)
{
    
    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }

    at_cmd_param_init(&g_at_cmd, AT_SYNC, NULL, CMD_EXCUTE, 3000);

    //����SIM7020_NBLOT_INIT״̬
    g_sim7020_status.main_status = SIM7020_NBLOT_INIT;
    g_sim7020_status.sub_status  = SIM7020_SUB_SYNC;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);

    return SIM7020_OK;
}



//��ȡNBģ�����Ϣ
int sim7020_nblot_info_get(sim7020_handle_t sim7020_handle)
{

    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
    
    at_cmd_param_init(&g_at_cmd, AT_CGREG, NULL, CMD_READ, 3000);

    //����SIM7020_NBLOT_INFO״̬
    g_sim7020_status.main_status = SIM7020_NBLOT_INFO;
    g_sim7020_status.sub_status  = SIM7020_SUB_CEREG_QUERY;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
    
    return SUCCESS;
}


//��ȡNBģ����ź�����
int sim7020_nblot_signal_get(sim7020_handle_t sim7020_handle)
{

    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
        
    at_cmd_param_init(&g_at_cmd, AT_CSQ, NULL, CMD_EXCUTE, 3000);

    //����SIM7020_SIGNAL״̬
    g_sim7020_status.main_status = SIM7020_SIGNAL;
    g_sim7020_status.sub_status  = SIM7020_SUB_CSQ;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
    
    return SUCCESS;
}

   





