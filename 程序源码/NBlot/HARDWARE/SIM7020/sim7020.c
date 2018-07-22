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


#define SIM7020_DEBUG                   
#ifdef SIM7020_DEBUG
#define SIM7020_DEBUG_INFO(...)    (int)printf(__VA_ARGS__)
#else
#define SIM7020_DEBUG_INFO(...)
#endif


//��Ҫ�߼�
//���������Ӧ��Ľ��������at_cmd_result_parse�������
//�����ȷ�����������������������Ƿ���Ҫִ����һ������
//������Ϣʱ������sim7020_msg_send����״̬�������ʱ��û�з����ı�
//ֱ������at_cmd_next�������һ��ָ���ʱ��Ż�ı���״̬
//��at_cmd_next����FALSEʱ���������е���״̬���Ѿ������ˣ���ʱ�Ḵ��״̬�����߼�

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
   
//sim7020�¼�֪ͨ
static uint8_t sim7020_event_notify (sim7020_handle_t sim7020_handle, char *buf);

//������Ϣ��Ӧ�ò㽻��
static void sim7020_msg_send (sim7020_handle_t sim7020_handle, char**buf, int8_t is_ok);

//sim7020����ATָ��
//sim7020_handle   sim7020_handle�豸���
//cmd_handle       ��Ҫ����ָ����Ϣ���
//note ���øú���ǰ�ȹ��������Ĳ���
static int sim7020_at_cmd_send(sim7020_handle_t sim7020_handle, at_cmdhandle cmd_handle);

//sim7020��������
static int sim7020_data_recv(sim7020_handle_t sim7020_handle, uint8_t *pData, uint16_t size, uint32_t Timeout);

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

//����ִ�н���
static void sim7020_status_set (sim7020_main_status_t  main_status, int  sub_status)
{
    g_sim7020_status.main_status  = main_status;
    g_sim7020_status.sub_status   = sub_status;
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
  
    int size = g_sim7020_recv_desc.len;
    
    if (p_uart_dev->uart_event & UART_TX_EVENT)
    {
               
        SIM7020_DEBUG_INFO("sim7020 uart tx ok %s", g_sim7020_send_desc.buf);  

        lpuart_event_clr(p_uart_dev, UART_TX_EVENT);                 
    }

    if (p_uart_dev->uart_event & UART_RX_EVENT)
    {               
        size = uart_ring_buf_avail_len(p_uart_dev);
        
        //�ӻ��������ж�ȡ����
        if (size > 0)
        {                                  
            sim7020_data_recv(sim7020_handle, (uint8_t*)(&g_sim7020_recv_desc.buf[g_sim7020_recv_desc.len]), size, 0);
                     
            //�����첽�¼��ȴ�����
            sim7020_event_notify(sim7020_handle, g_sim7020_recv_desc.buf);
            
            SIM7020_DEBUG_INFO("sim7020 uart rx ok %s\r\n", &g_sim7020_recv_desc.buf[g_sim7020_recv_desc.len]);
          
            g_sim7020_recv_desc.len = g_sim7020_recv_desc.len + size;
            
        }
                                                                                 
        lpuart_event_clr(p_uart_dev, UART_RX_EVENT); 
    } 

    //�������ͳ�ʱ�¼���˵��ָ���п���û�з��ͳ�ȥ�����߷��͹����г���
    //����ģ�鹤���쳣��û�л�Ӧ���������
    if (p_uart_dev->uart_event & UART_TX_TIMEOUT_EVENT) 
    {
        
        SIM7020_DEBUG_INFO("sim7020 uart tx timeout %s", g_sim7020_send_desc.buf);  

        sim7020_event_set(sim7020_handle, SIM7020_TIMEOUT_EVENT);
      
        lpuart_event_clr(p_uart_dev, UART_TX_TIMEOUT_EVENT); 
    } 

    //���ʹ�÷ǳ�ʱ��֡�����¼������ϲ��ᷢ��
    if (p_uart_dev->uart_event & UART_RX_TIMEOUT_EVENT) 
    {       
        size = uart_ring_buf_avail_len(p_uart_dev);
        
        //��ʱ��֡      
        if (g_sim7020_dev.frame_format == 1) 
        {
            if (size > 0)
            {               
                sim7020_data_recv(sim7020_handle, (uint8_t*)(&g_sim7020_recv_desc.buf[g_sim7020_recv_desc.len]), size, 0);
                                             
                //�����첽�¼��ȴ�����
                sim7020_event_notify(sim7020_handle, g_sim7020_recv_desc.buf);
              
                g_sim7020_recv_desc.len = g_sim7020_recv_desc.len + size;
            }
                                        
       //���ڳ�ʱ��֡��״̬�£������ȷ�����˳�ʱ�¼�      
       } else {
         
           sim7020_event_set(sim7020_handle, SIM7020_TIMEOUT_EVENT);
       }
       
       SIM7020_DEBUG_INFO("sim7020 uart rx timeout %s\r\n", &g_sim7020_recv_desc.buf[g_sim7020_recv_desc.len]);  
       
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
    
    static int8_t recv_cnt = 0;
            
    if (sim7020_handle->sim7020_event & SIM7020_RECV_EVENT) 
    {   
        SIM7020_DEBUG_INFO("%s recv event\r\n", g_at_cmd.p_atcmd);
      
        cmd_is_pass = at_cmd_result_parse(g_sim7020_recv_desc.buf);
        
        //������Ӧ���     
        if (cmd_is_pass == AT_CMD_RESULT_OK) 
        {

            //�����ڳ�ʱʱ����������ɣ�ֹͣ���ճ�ʱ  
            atk_soft_timer_stop(&sim7020_handle->p_uart_dev->uart_rx_timer); 
          
            recv_cnt=0; 
          
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

            if (index == 0)
            {
                //���ݽ����������·��͸�����
                next_cmd = sim7020_response_handle(sim7020_handle, FALSE);
              
                //�建��            
                sim7020_recv_buf_reset();                            
                                        
                sim7020_event_clr(sim7020_handle, SIM7020_RECV_EVENT); 
                            
                //δ�յ���ȷ������֡
                return  SIM7020_ERROR;
            }            
                                                            
            SIM7020_DEBUG_INFO("%s cmd excute ok\r\n", g_at_cmd.p_atcmd);
          
            //��������ͳɹ��˲��õ���ȷ����Ӧ
            sim7020_msg_send(sim7020_handle, at_response_par, TRUE);
              
            next_cmd = sim7020_response_handle(sim7020_handle, TRUE);
            
            //�建��            
            sim7020_recv_buf_reset();
       
        }       
        else if(cmd_is_pass == AT_CMD_RESULT_ERROR)
        {                            
            next_cmd = sim7020_response_handle(sim7020_handle, FALSE);     
        
            if (g_at_cmd.cmd_action & ACTION_ERROR_AND_TRY)
            {                                   
                SIM7020_DEBUG_INFO("%s cmd is failed and try\r\n", g_at_cmd.p_atcmd);
                
                at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = (char*)g_at_cmd.p_atcmd;
                
                //֪ͨ�ϲ�Ӧ�ã��˶���ִ��ʧ��,��������
                sim7020_msg_send(sim7020_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], SIM7020_ERROR_RETRY);
                
            } 
            else if (g_at_cmd.cmd_action & ACTION_ERROR_BUT_NEXT)
            {               
                at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = "error but next\r\n";
               
                SIM7020_DEBUG_INFO("%s cmd is failed and next\r\n", g_at_cmd.p_atcmd);
                
                //֪ͨ�ϲ�Ӧ�ã��˶���ִ��ʧ�ܺ�����������ִ��
                sim7020_msg_send(sim7020_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], SIM7020_ERROR_NEXT);
                
            }
            else 
            {                
                SIM7020_DEBUG_INFO("%s cmd is failed and exit\r\n", g_at_cmd.p_atcmd);        
                at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = (char*)g_at_cmd.p_atcmd;
                
                //֪ͨ�ϲ�Ӧ�ã��˶���ִ��ʧ�ܺ�����������ִ��
                sim7020_msg_send(sim7020_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], FALSE);  

                //��λ״̬��־
                sim7020_status_reset();              

            } 
                    
            //�建��            
            sim7020_recv_buf_reset();
            
            recv_cnt=0; 
            
            //�����ڳ�ʱʱ����������ɣ�ֹͣ���ճ�ʱ  
            atk_soft_timer_stop(&sim7020_handle->p_uart_dev->uart_rx_timer); 
        } 

        else if (cmd_is_pass == AT_CMD_RESULT_CONTINUE)
        {
          
            //����δִ����ɣ���������£��յ��ĵ����������, �������Ļ��ǵ�ǰ������Ӧ���ݵĽ���, �����������ճ�ʱ               
            atk_soft_timer_timeout_change(&sim7020_handle->p_uart_dev->uart_rx_timer, 1000);
          
            //����TCP/UDP����״̬ʱ
            if (g_sim7020_status.main_status == SIM7020_TCPUDP_CR) {
                //֪ͨ�ϲ�Ӧ�ã���ȡ��ص���Ϣ
               sim7020_msg_send(sim7020_handle, at_response_par, SIM7020_ERROR_CONTINUE);
            }          
            recv_cnt=0; 

            //����δ���            
        }       
        else 
        {
         
            recv_cnt++;          
          
            //�����ϲ������е�����, ���е���������������
            //һ���Ǳ�ʾ��������յ������ݶ�������        
            //�ڶ�����IDLE��֡�жϲ�����ô׼ȷ��û������Ҳ��Ϊһ֡������
            if (recv_cnt > AT_CMD_RESPONSE_PAR_NUM_MAX)                         
            {              
               //�յ���������,ǿ�ƽ��ս���
               next_cmd = sim7020_response_handle(sim7020_handle, FALSE);
               //�建��            
               sim7020_recv_buf_reset();                
            }
            else 
            {
              
               //����δ���,�յ��ĵ���������Ե������е�һ����                
//               atk_soft_timer_timeout_change(&sim7020_handle->p_uart_dev->uart_rx_timer, 1000);
             
            }              
                               
        }            
        
        sim7020_event_clr(sim7020_handle, SIM7020_RECV_EVENT); 
    }

    if (sim7020_handle->sim7020_event & SIM7020_TIMEOUT_EVENT) 
    {        
        //��ʱ���������ط�����        
        next_cmd = sim7020_response_handle(sim7020_handle, FALSE);
      
        //֪ͨ�ϲ�Ӧ�ã��˶���ִ�г�ʱ
        if (g_at_cmd.cmd_action & ACTION_ERROR_AND_TRY) 
        {           
            at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = (char *)g_at_cmd.p_atcmd;
           
            SIM7020_DEBUG_INFO("%s cmd not repsonse or send failed\r\n", g_at_cmd.p_atcmd);
                               
            //֪ͨ�ϲ�Ӧ�ã��˶���ִ��ʧ�ܺ�����������ִ��
            sim7020_msg_send(sim7020_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], SIM7020_ERROR_NEXT);            
        } 
        else 
        {            
            SIM7020_DEBUG_INFO("%s cmd is failed and exit\r\n", g_at_cmd.p_atcmd);        
            at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = (char*)g_at_cmd.p_atcmd;
            
            //֪ͨ�ϲ�Ӧ�ã��˶���ִ��ʧ�ܺ�����������ִ��
            sim7020_msg_send(sim7020_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], FALSE);  

            //��λ״̬��־
            sim7020_status_reset();              
        } 
        
        //�����������      
        sim7020_recv_buf_reset();      
              
        //������¼�
        sim7020_event_clr(sim7020_handle, SIM7020_RECV_EVENT);                                         
        sim7020_event_clr(sim7020_handle, SIM7020_TIMEOUT_EVENT); 
    } 

    if (sim7020_handle->sim7020_event & SIM7020_REG_STA_EVENT) 
    {      
        SIM7020_DEBUG_INFO("creg event ok\r\n");
                             
        at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = "creg ok";        
          
        //֪ͨ�ϲ�Ӧ������ע����
        sim7020_msg_send(sim7020_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], TRUE);        

        //��������Ƿ�ִ����һ������        
        next_cmd = sim7020_response_handle(sim7020_handle, TRUE);
        
        sim7020_event_clr(sim7020_handle,   SIM7020_REG_STA_EVENT); 
      
        //�����������    
        sim7020_recv_buf_reset();   
    }
    
    if (sim7020_handle->sim7020_event & SIM7020_TCP_RECV_EVENT) 
    {
        SIM7020_DEBUG_INFO("tcp recv event ok\r\n");
      
        //֪ͨ�ϲ�Ӧ�ý��յ�TCP����
        sim7020_msg_send(sim7020_handle, NULL, TRUE);     
        
        sim7020_event_clr(sim7020_handle,SIM7020_TCP_RECV_EVENT);
      
        //�����������    
        sim7020_recv_buf_reset();       
    }

    if (sim7020_handle->sim7020_event & SIM7020_UDP_RECV_EVENT) 
    {
        SIM7020_DEBUG_INFO("udp recv event ok\r\n");
      
        //֪ͨ�ϲ�Ӧ������ע����
        sim7020_msg_send(sim7020_handle, NULL, TRUE);    
        
        sim7020_event_clr(sim7020_handle, SIM7020_UDP_RECV_EVENT); 
      
        //�����������    
        sim7020_recv_buf_reset();  
    }
        
    if (sim7020_handle->sim7020_event & SIM7020_COAP_RECV_EVENT) 
    {
        
        SIM7020_DEBUG_INFO("coap recv event ok\r\n");
             
        sim7020_event_clr(sim7020_handle, SIM7020_COAP_RECV_EVENT); 
    }
    
    if (sim7020_handle->sim7020_event & SIM7020_SOCKET_ERR_EVENT) 
    {       
        SIM7020_DEBUG_INFO("socket err event\r\n");
      
        //֪ͨ�ϲ�Ӧ��SOCKETʧ����
        sim7020_msg_send(sim7020_handle, NULL, TRUE);        
              
        sim7020_event_clr(sim7020_handle, SIM7020_SOCKET_ERR_EVENT); 
    }    

    //�����¼���״̬�ж��Ƿ���Ҫִ����һ������
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
    
//    if ((!sim7020_handle->sim7020_event))
//    {
//        SIM7020_DEBUG_INFO("g_recv_buf %s \r\n", g_sim7020_recv_desc.buf);
//    }

    return SIM7020_OK;    
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
    
    return ret;    
}

//��1���ַ�ת��Ϊ10��������
//chr:�ַ�,0~9/A~F/a~F
//����ֵ:chr��Ӧ��10������ֵ
u8 sim7020_chr2hex(u8 chr)
{
    if(chr>='0'&&chr<='9')
    {
      return chr - '0';
    }
    else if(chr>='A'&&chr<='F')
    {
      return (chr-'A'+10);
    }
    else if(chr>='a'&&chr<='f')
    {
      return (chr-'a'+10); 
    }
    else
    {
      
      return 0;
    }
                
}



//��1��16��������ת��Ϊ�ַ�
//hex:16��������,0~15;
//����ֵ:�ַ�
u8 sim7020_hex2chr(u8 hex)
{
    if(hex<=9)
    {
      return hex+'0';
    }
    else if(hex>=10&&hex<=15)
    {
      return (hex-10+'A'); 
    }
    else 
    {
        
    }      
    
    return '0';
}


////��ʮ�������ַ���ת������
//uint16_t sim7020_hexstr2num(char* str)
//{
//  uint16_t i = 0;
//  uint8_t tmp = 0;
//  uint8_t tmp1 = 0;
//  
//  uint16_t len = strlen(str);
//  
//  
//  
//  
//  return (i >> 1);
//}



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
    memset(g_sim7020_send_desc.buf,0,SIM7020_SEND_BUF_MAX_LEN);
    g_sim7020_send_desc.len = 0;

    if(cmd_handle->property == CMD_TEST)
    {
        cmdLen = snprintf(g_sim7020_send_desc.buf,SIM7020_SEND_BUF_MAX_LEN,
                          "%s=?\r\n",
                          cmd_handle->p_atcmd);
    }    
    else if(cmd_handle->property == CMD_READ)
    {
        cmdLen = snprintf(g_sim7020_send_desc.buf,SIM7020_SEND_BUF_MAX_LEN,
                          "%s?\r\n",
                          cmd_handle->p_atcmd);
    }
    else if(cmd_handle->property == CMD_EXCUTE)
    {
        cmdLen = snprintf(g_sim7020_send_desc.buf,SIM7020_SEND_BUF_MAX_LEN,
                          "%s\r\n",
                          cmd_handle->p_atcmd);    
    }

    else if(cmd_handle->property == CMD_SET)
    {
        cmdLen = snprintf(g_sim7020_send_desc.buf,SIM7020_SEND_BUF_MAX_LEN,
                          "%s=%s\r\n",
                          cmd_handle->p_atcmd,cmd_handle->p_atcmd_arg);    
    }
    
    //cmdlen����Ч�����ݳ��ȣ��������ַ���������Ƿ�
    g_sim7020_send_desc.len = cmdLen;
    
    return cmdLen;
}


//�ж�atָ���Ƿ��ͳɹ�
//����ֵ�����-1������յ������ݿ���Ϊ����
//����ֵ�����0�� ָ��ͨ���������1��ָ�����
static int8_t at_cmd_result_parse (char *buf)
{
    int8_t result = -1;
  
//    SIM7020_DEBUG_INFO("respones buf%s\r\n",buf); 
//  
//    SIM7020_DEBUG_INFO("respones buf len %d \r\n", strlen(buf)); 
  
    char *p_colon = strchr(g_at_cmd.p_atcmd,'+');
    char *p_colon_temp = strchr(buf,':');
       
    if(g_at_cmd.p_expectres == NULL)
    {
        if (strstr(buf,"\r\nOK\r\n"))
        {
            result = AT_CMD_RESULT_OK;
        }
        else if (strstr(buf,"\r\nERROR\r\n"))
        {
            result = AT_CMD_RESULT_ERROR;
          
        } else if ((strstr(buf,g_at_cmd.p_atcmd)) || ((p_colon && p_colon_temp))) {
          
           //������Ļ���,����OK֮ǰ���������
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
static uint8_t sim7020_event_notify (sim7020_handle_t sim7020_handle, char *buf)
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
           
        //ͬʱ�����������¼�
        sim7020_event_set(sim7020_handle, SIM7020_RECV_EVENT);  
        
    }    
    else if((target_pos_start = strstr(buf,"+CSONMI")) != NULL)
    {
        //�յ��������˷���TCP/UDP����
        char *p_colon = strchr(target_pos_start, ':');
      
        int8_t socket_id = 0;
        
        //�õ����ĸ�socket�յ�����
        if (p_colon)
        {
            p_colon++;
            socket_id = strtoul(p_colon,0,10);
        } 
        
        //�õ��յ������ݳ���
        char *pComma = strchr(p_colon,',');

        if (pComma)
        {
            pComma++;
            g_socket_info[0].data_len = strtoul(pComma,0,10);
        }     
       
        //�õ���Ч���ݵ���ʼ��ַ
        char *p_data_offest = strchr(pComma,',');

        if (p_data_offest)
        {
            p_data_offest++;
            g_socket_info[0].data_offest = p_data_offest;
        }               
        if (g_socket_info[0].socket_type == 2) 
        {
            sim7020_event_set(sim7020_handle, SIM7020_UDP_RECV_EVENT);             
        }         
        else if (g_socket_info[0].socket_type == SIM7020_TCP)
        {

            sim7020_event_set(sim7020_handle, SIM7020_TCP_RECV_EVENT); 
        } 
        else 
        {
            //������Ĭ�ϲ��������¼�
            sim7020_event_set(sim7020_handle, SIM7020_RECV_EVENT);            
        }
        
        sim7020_status_set(SIM7020_TCPUDP_RECV, SIM7020_SUB_TCPUDP_RECV);        
    }  
    else if((target_pos_start = strstr(buf,"+CSOERR")) != NULL)
    {
        //�յ��������˷���TCP/UDP������
        char *p_colon = strchr(target_pos_start,':');
      
        int8_t socket_id = 0;
      
        int8_t socket_err = 0;
        
        //�õ����ĸ�socket�յ�����
        if (p_colon)
        {
            p_colon++;          
            socket_id = strtoul(p_colon,0,10);
        }
        
        //�õ��յ���socket������
        char *pComma = strchr(p_colon,',');
        
        if(pComma)
        {
          pComma++;
          socket_err = strtoul(pComma, 0, 10);          
          g_socket_info[0].socket_errcode = socket_err; 
        }
                
        sim7020_event_set(sim7020_handle, SIM7020_SOCKET_ERR_EVENT); 
        sim7020_status_set(SIM7020_SOCKET_ERR, SIM7020_SUB_SOCKET_ERR);        
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
    
    else if (g_sim7020_status.main_status == SIM7020_TCPUDP_CR)
    {
        
        g_sim7020_status.sub_status++;
      
      
        if (g_sim7020_status.sub_status == SIM7020_SUB_END)
        {
            return FALSE;
        }
        
        switch(g_sim7020_status.sub_status)
        {
          
        case SIM7020_SUB_TCPUDP_CONNECT:
          
          {
          
            char *p_remote_port = NULL;

            static char buf[64];
          
            memset(buf,0,sizeof(buf));
            
            if (g_socket_info[0].socket_type == SIM7020_TCP)
            { 
                p_remote_port = REMOTE_TCP_PORT;
            }
            else
            {
                p_remote_port = REMOTE_UDP_PORT;   
            }              

    
            //��������ķ���ֵΪ��Ҫ��ʽ��д��ĳ��ȣ��������ַ������������Զ���������ַ�
            uint16_t tcpudp_cn_len = snprintf(buf,
                                        sizeof(buf) -1,"%d,%s,%s",                                          
                                        g_socket_info[0].socket_id,
                                        p_remote_port,                                         
                                        REMOTE_SERVER_IP);
                                        
//            SIM7020_DEBUG_INFO("tcpudp_cn_len = %d\r\n", tcpudp_cn_len);
//                                        
//            SIM7020_DEBUG_INFO("tcpudp_cn_buf = %s\r\n", buf);                            
            
            //�����Ӧʱ�䲻��                                        
            at_cmd_param_init(&g_at_cmd,AT_CSOCON, buf, CMD_SET, 3000);
                                        
          }
          break;

                                     
        default: 
          
          //ǿ�Ʊ�ʾ�ӽ��̽���
          g_sim7020_status.sub_status = SIM7020_SUB_END;
        
          return FALSE;          
        
        }
    } 
    else if (g_sim7020_status.main_status == SIM7020_TCPUDP_CL) 
    {
      
        g_sim7020_status.sub_status = SIM7020_SUB_END;
      
        return FALSE;    
      
    }  
    else if (g_sim7020_status.main_status == SIM7020_TCPUDP_SEND) 
    {
      
        g_sim7020_status.sub_status = SIM7020_SUB_END;
      
        return FALSE;     
    }
        
    else if (g_sim7020_status.main_status == SIM7020_NONE)   
    {  //��ֹ�����ط�
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
        
      sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_RETRY, strlen(buf[0]), buf[0]);  
        
      return;      
  }
      
  //�������ϱ�������ִ��ʧ��
  else if(is_ok == FALSE)
  {
    sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_FAIL, strlen(buf[0]), buf[0]);
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
        char *p_colon = strchr(buf[1],':');
        p_colon++;
      
        //ת����10��������
        uint8_t lqi =strtoul(p_colon,0, 10);
      
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

      sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_REG, strlen(buf[0]), buf[0]);
    
      break;        
        
                 
    case SIM7020_SUB_CGMI:
      {
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_MID, strlen(buf[1]), buf[1]);
      }
      break;
      
    case SIM7020_SUB_CGMM:
      {
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_MMODEL,strlen(buf[1]),buf[1]);
      }
      break;
      
    case SIM7020_SUB_CGMR:
      {

         sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_MREV,strlen(buf[1]),buf[1]);
        
      }
      break;
      
      
    case SIM7020_SUB_CIMI:
      {
        memcpy(g_firmware_info.IMSI,buf[1],15);
        g_firmware_info.IMSI[15] = 0;
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_IMSI,strlen(buf[1]),buf[1]);
      }
      break;
      
    
    case SIM7020_SUB_CGSN:
      {
        char *p_colon = strchr(buf[1],':');
        if(p_colon)
        {
          p_colon++;
          memcpy(g_firmware_info.IMEI ,p_colon,15);
          g_firmware_info.IMEI[15] = 0;
          sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_IMEI,15,(char*)g_firmware_info.IMEI);
        }
      }
      break;            
      
    case SIM7020_SUB_CBAND:
      {
        char *p_colon = strchr(buf[1],':');
        char *pFreq = NULL;
        if(p_colon)
        {
          p_colon++;
          uint8_t hz_id = strtoul(p_colon,0,10);
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
    switch(g_sim7020_status.sub_status) 
    { 
    case SIM7020_SUB_CSQ:
    {
        char *p_colon = strchr(buf[1],':');
        if (p_colon != NULL) 
        {            
          p_colon++;
          //ת����10��������
          uint8_t lqi =strtoul(p_colon,0, 10);
          //����ȡ��ÿ����ֵ��Ӧ��dbm��Χ
          int8_t rssi = -110 + (lqi << 1);
          uint8_t len = snprintf(buf[1],10,"%d",rssi);
          *(buf[1]+len) = 0;
          
          g_sim7020_status.rssi = rssi;
          
          sim7020_handle->sim7020_cb(sim7020_handle->p_arg,(sim7020_msg_id_t)SIM7020_MSG_CSQ,len,buf[1]);
          
        }
      
        break;
    } 
      
    case SIM7020_SUB_END:
    {
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg,(sim7020_msg_id_t)SIM7020_MSG_SIGNAL,1,"S");
        break;
    }
    
    default:
      
        break;
    }
    
  }
  else if(g_sim7020_status.main_status == SIM7020_TCPUDP_CR)
  {
    switch(g_sim7020_status.sub_status)
    {
    case SIM7020_SUB_TCPUDP_CR:
      {

        char *p_colon = strchr(buf[1],':');
                    
        if (p_colon != NULL) 
        {                
            p_colon++;
            
            //ת����10��������,�õ���ǰ������socket id
            g_socket_info[0].socket_id =strtoul(p_colon,0, 10);
        }
                
      }
      break;
      
    case SIM7020_SUB_TCPUDP_CONNECT:
      {                                             
      }
      break;
      
    case SIM7020_SUB_END:
      {              
         char *p_buf_tmep = NULL;
              
         g_sim7020_status.connect_status = 1;  
               
         if (g_socket_info[0].socket_type == SIM7020_TCP)
         {
             p_buf_tmep = "tcp create ok";
         }
        
         else 
         {   
             p_buf_tmep = "udp create ok";
         } 
                                                       
         sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_TCPUDP_CREATE, strlen(p_buf_tmep), p_buf_tmep);
                
      }
      break;
    }
  } 
  else if(g_sim7020_status.main_status == SIM7020_TCPUDP_CL)
  {
    if (g_sim7020_status.sub_status == SIM7020_SUB_END)
    {
         g_sim7020_status.connect_status = 0;  
      
         char *p_buf_tmep = NULL;
                                    
         if (g_socket_info[0].socket_type == SIM7020_TCP)
         {
             p_buf_tmep = "tcp close";
         }         
         else 
         {   
             p_buf_tmep = "udp close";
         }         
         sim7020_handle->sim7020_cb(sim7020_handle->p_arg,(sim7020_msg_id_t)SIM7020_MSG_TCPUDP_CLOSE, strlen(p_buf_tmep), p_buf_tmep);
     }
  }   
  else if(g_sim7020_status.main_status == SIM7020_TCPUDP_SEND)
  {
    switch(g_sim7020_status.sub_status)
    {
      
    case SIM7020_SUB_END:
      {
        char *p_buf_tmep = g_sim7020_send_desc.buf;
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_TCPUDP_SEND,strlen(p_buf_tmep),p_buf_tmep);
      }
      break;
      
    default:
      
      break;
    }
  }
  else if(g_sim7020_status.main_status == SIM7020_TCPUDP_RECV)
  {
    if(g_sim7020_status.sub_status == SIM7020_SUB_TCPUDP_RECV)
    {
      char *data_buf = g_socket_info[0].data_offest; 

//      SIM7020_DEBUG_INFO("data_buf = %s", data_buf);      
           
      sim7020_handle->sim7020_cb(sim7020_handle->p_arg,(sim7020_msg_id_t)SIM7020_MSG_TCPUDP_RECV,strlen(data_buf),data_buf);
      
      //��λ״̬��־
      sim7020_status_reset();
    }
    
  }
  
  else if(g_sim7020_status.main_status == SIM7020_SOCKET_ERR)
  {
    if (g_sim7020_status.sub_status == SIM7020_SUB_SOCKET_ERR)
    {

        SIM7020_DEBUG_INFO("the socket err, the err code is %d\r\n", g_socket_info[0].socket_errcode); 
             
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg,
                                   (sim7020_msg_id_t)SIM7020_MSG_SOCKET_ERROR, 
                                    1, 
                                    (char *)&g_socket_info[0].socket_errcode);

        //��λ״̬��־
        sim7020_status_reset();
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
        index = 0;
        sim7020_handle->sim7020_cb(sim7020_handle->p_arg, (sim7020_msg_id_t)SIM7020_MSG_COAP_RECV, index,tmp_buf); 
      }
   
    }
  }

  else
  {

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
//sim7020_handle �豸���
static int sim7020_data_recv(sim7020_handle_t sim7020_handle, uint8_t *pData, uint16_t size, uint32_t Timeout)
{   
    int ret = 0;
        
    if (sim7020_handle == NULL)
    {
       return SIM7020_ERROR;
    }
        
    ret = sim7020_handle->p_drv_funcs->sim7020_recv_data(sim7020_handle, pData, size, Timeout);      
    //�ں�������ַ���������
//    pData[size]=0;
    
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
    
    return SIM7020_OK;
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
    
    return SIM7020_OK;
}

//����tcpudp 
int sim7020_nblot_tcpudp_create(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{
    char *p_tcpudp = NULL;
  
    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
    if (type == SIM7020_TCP)
    {
        g_socket_info[0].socket_type = SIM7020_TCP;
        p_tcpudp = "1,1,1";
    }
    
    else if(type == SIM7020_UDP)
    {   
        g_socket_info[0].socket_type = SIM7020_UDP;
        p_tcpudp = "1,2,1";
    } 
    else 
    {
       return SIM7020_NOTSUPPORT;
      
    }
            
    at_cmd_param_init(&g_at_cmd, AT_CSOC, p_tcpudp, CMD_SET, 3000);
//    g_at_cmd.cmd_action  = ACTION_OK_AND_NEXT | ACTION_ERROR_EXIT;
    
    //����SIM7020_SIGNAL״̬
    g_sim7020_status.main_status = SIM7020_TCPUDP_CR;
    g_sim7020_status.sub_status  = SIM7020_SUB_TCPUDP_CR;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
    
    return SIM7020_OK;
}


//�ر�tcpudp
int sim7020_nblot_tcpudp_close(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{

    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
       
    (void)type;
    
    //������ݳ���Ϊ��Ч���ݼ���ͷ��

    char  buf[36];
    
    memset(buf, 0, sizeof(buf));
    
    
    int16_t msg_len = snprintf(buf,
                                sizeof(buf),
                                "%d",
                                g_socket_info[0].socket_id);
                                
    if (msg_len < 0) {
      
        return SIM7020_ERROR;
    }
    
//    SIM7020_DEBUG_INFO("close len %d\r\n",msg_len); 
    
                                    
        
    at_cmd_param_init(&g_at_cmd, AT_CSOCL, buf, CMD_SET, 3000);

    //����tcp/udp�ر�״̬
    g_sim7020_status.main_status = SIM7020_TCPUDP_CL;
    g_sim7020_status.sub_status  = SIM7020_SUB_TCPUDP_CL;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
    
//    SIM7020_DEBUG_INFO("close buf %s", g_sim7020_send_desc.buf);
    
    return SIM7020_OK;
}


//��hex���ݸ�ʽ��������,������ż��������
int sim7020_nblot_tcpudp_send_hex(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type)
{
  
    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
  
    //�ж�SOCKET ID�Ƿ���ȷ
    if (g_socket_info[0].socket_id  < 0 || g_socket_info[0].socket_id > 5 )
    {
        return SIM7020_ERROR;
    }
  
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (SIM7020_SEND_BUF_MAX_LEN - 20) ;

    char  buf[(SIM7020_SEND_BUF_MAX_LEN - 20)];
    
    memset(buf, 0, str_len);


    uint16_t msg_len = snprintf(buf,
                                str_len,
                                "%d,%d,",
                                 g_socket_info[0].socket_id, 
                                len);
                                
    for(uint16_t i = 0 ; i < len ; i++)
    {
        sprintf(&buf[msg_len + (i << 1)],"%02X",(uint8_t)msg[i]);
    }                             
 
    //����TCP/UDP���ݷ�����������Ӧʱ�䲻��
    at_cmd_param_init(&g_at_cmd, AT_CSOSEND, buf, CMD_SET, 3000);
    
    //����tcp/udp���ݷ���״̬
    g_sim7020_status.main_status = SIM7020_TCPUDP_SEND;
    g_sim7020_status.sub_status  = SIM7020_SUB_TCPUDP_SEND;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);

    return SIM7020_OK;
}

//���ַ�����ʽ��������
int sim7020_nblot_tcpudp_send_str(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type)
{
  
    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
  
    //�ж�SOCKET ID�Ƿ���ȷ
    if (g_socket_info[0].socket_id  < 0 || g_socket_info[0].socket_id > 5 )
    {
        return SIM7020_ERROR;
    }
      
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (SIM7020_SEND_BUF_MAX_LEN - 20) ;


    char  buf[SIM7020_SEND_BUF_MAX_LEN - 20];
    
    memset(buf, 0, str_len);

    int16_t msg_len = snprintf(buf,
                               str_len,
                               "%d,%d,%s%s%s",
                                g_socket_info[0].socket_id,
                                0,
                                "\"",                                  
                                msg,
                                "\"");
    
    if (msg_len < 0) {
      
        return SIM7020_ERROR;
    }
    
//    SIM7020_DEBUG_INFO("send buf len %d\r\n",msg_len); 
                                                          
    //����TCP/UDP���ݷ�����������Ӧʱ�䲻��
    at_cmd_param_init(&g_at_cmd, AT_CSOSEND, buf, CMD_SET, 3000);
    
    //����tcp/udp���ݷ���״̬
    g_sim7020_status.main_status = SIM7020_TCPUDP_SEND;
    g_sim7020_status.sub_status  = SIM7020_SUB_TCPUDP_SEND;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
    
    
//    SIM7020_DEBUG_INFO("send buf %s", g_sim7020_send_desc.buf);    

    return SIM7020_OK;
}   

//����coap������ 
int sim7020_nblot_coap_server_create(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{
  
    char buf[64] = {0};
  
    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
    if (type != SIM7020_COAP)
    {
        return SIM7020_NOTSUPPORT;
    }
    
    g_sim7020_status.connect_type = SIM7020_COAP;  
    g_sim7020_status.cid          = 1;
               
                       
    //��������ķ���ֵΪ��Ҫ��ʽ��д��ĳ��ȣ��������ַ������������Զ���������ַ�
    uint16_t coap_cn_len = snprintf(buf,
                                    sizeof(buf) -1,"%s,%s,%d",                                                                                             
                                    REMOTE_SERVER_IP,
                                    REMOTE_COAP_PORT,
                                    g_sim7020_status.cid);
                                        
                     
            
    //�����Ӧʱ�䲻��                                          
    
    at_cmd_param_init(&g_at_cmd, AT_CCOAPSTA, buf, CMD_SET, 3000);
    
    //��������COAP������״̬
    g_sim7020_status.main_status = SIM7020_CoAP_SEVER;
    g_sim7020_status.sub_status  = SIM7020_SUB_CoAP_SEVER;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
    
    return SIM7020_OK;
}

//����coap�ͻ���
int sim7020_nblot_coap_client_create(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{
    char *p_tcpudp = NULL;
  
    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
    if (type == SIM7020_TCP)
    {
        g_socket_info[0].socket_type = SIM7020_TCP;
        p_tcpudp = "1,1,1";
    }
    
    else if(type == SIM7020_UDP)
    {   
        g_socket_info[0].socket_type = SIM7020_UDP;
        p_tcpudp = "1,2,1";
    } 
    else 
    {
        return SIM7020_NOTSUPPORT;
      
    }
            
    at_cmd_param_init(&g_at_cmd, AT_CSOC, p_tcpudp, CMD_SET, 3000);
    
    //����SIM7020_SIGNAL״̬
    g_sim7020_status.main_status = SIM7020_TCPUDP_CR;
    g_sim7020_status.sub_status  = SIM7020_SUB_TCPUDP_CR;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
    
    return SIM7020_OK;
}


//�ر�coap
int sim7020_nblot_coap_close(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{

    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
       
    (void)type;
    
    //������ݳ���Ϊ��Ч���ݼ���ͷ��

    char  buf[36];
    
    memset(buf, 0, sizeof(buf));
    
    
    int16_t msg_len = snprintf(buf,
                                sizeof(buf),
                                "%d",
                                g_socket_info[0].socket_id);
                                
    if (msg_len < 0) {
      
        return SIM7020_ERROR;
    }
    
//    SIM7020_DEBUG_INFO("close len %d\r\n",msg_len); 
    
                                    
        
    at_cmd_param_init(&g_at_cmd, AT_CSOCL, buf, CMD_SET, 3000);

    //����tcp/udp�ر�״̬
    g_sim7020_status.main_status = SIM7020_TCPUDP_CL;
    g_sim7020_status.sub_status  = SIM7020_SUB_TCPUDP_CL;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
    
//    SIM7020_DEBUG_INFO("close buf %s", g_sim7020_send_desc.buf);
    
    return SIM7020_OK;
}


//��hex���ݸ�ʽ��������,������ż��������
int sim7020_nblot_coap_send_hex(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type)
{
  
    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
  
    //�ж�SOCKET ID�Ƿ���ȷ
    if (g_socket_info[0].socket_id  < 0 || g_socket_info[0].socket_id > 5 )
    {
        return SIM7020_ERROR;
    }
  
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (SIM7020_SEND_BUF_MAX_LEN - 20) ;

    char  buf[(SIM7020_SEND_BUF_MAX_LEN - 20)];
    
    memset(buf, 0, str_len);


    uint16_t msg_len = snprintf(buf,
                                str_len,
                                "%d,%d,",
                                 g_socket_info[0].socket_id, 
                                len);
                                
    for(uint16_t i = 0 ; i < len ; i++)
    {
        sprintf(&buf[msg_len + (i << 1)],"%02X",(uint8_t)msg[i]);
    }                             
 
    //����TCP/UDP���ݷ�����������Ӧʱ�䲻��
    at_cmd_param_init(&g_at_cmd, AT_CSOSEND, buf, CMD_SET, 3000);
    
    //����tcp/udp���ݷ���״̬
    g_sim7020_status.main_status = SIM7020_TCPUDP_SEND;
    g_sim7020_status.sub_status  = SIM7020_SUB_TCPUDP_SEND;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);

    return SIM7020_OK;
}

//���ַ�����ʽ��������coapЭ��
int sim7020_nblot_coap_send_str(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type)
{ 
    if (g_sim7020_status.main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
  
    //�ж�SOCKET ID�Ƿ���ȷ
    if (g_socket_info[0].socket_id  < 0 || g_socket_info[0].socket_id > 5 )
    {
        return SIM7020_ERROR;
    }
      
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (SIM7020_SEND_BUF_MAX_LEN - 20) ;


    char  buf[SIM7020_SEND_BUF_MAX_LEN - 20];
    
    memset(buf, 0, str_len);

    int16_t msg_len = snprintf(buf,
                               str_len,
                               "%d,%d,%s%s%s",
                                g_socket_info[0].socket_id,
                                0,
                                "\"",                                  
                                msg,
                                "\"");
    
    if (msg_len < 0) {
      
        return SIM7020_ERROR;
    }
    
//    SIM7020_DEBUG_INFO("send buf len %d\r\n",msg_len); 
                                                          
    //����TCP/UDP���ݷ�����������Ӧʱ�䲻��
    at_cmd_param_init(&g_at_cmd, AT_CSOSEND, buf, CMD_SET, 3000);
    
    //����tcp/udp���ݷ���״̬
    g_sim7020_status.main_status = SIM7020_TCPUDP_SEND;
    g_sim7020_status.sub_status  = SIM7020_SUB_TCPUDP_SEND;

    sim7020_at_cmd_send(sim7020_handle, &g_at_cmd);
    
    
//    SIM7020_DEBUG_INFO("send buf %s", g_sim7020_send_desc.buf);    

    return SIM7020_OK;
}   



