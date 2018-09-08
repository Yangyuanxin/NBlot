#include "atk_bc28.h"
#include "atk_delay.h"
#include "atk_bc28_nbiot.h"

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


#define NBIOT_DEBUG                   
#ifdef NBIOT_DEBUG
#define NBIOT_DEBUG_INFO(...)    (int)printf(__VA_ARGS__)
#else
#define NBIOT_DEBUG_INFO(...)
#endif


//��Ҫ�߼�
//���������Ӧ��Ľ��������at_cmd_result_parse�������
//�����ȷ�����������������������Ƿ���Ҫִ����һ������
//������Ϣʱ������nbiot_msg_send����״̬�������ʱ��û�з����ı�
//ֱ������at_cmd_next�������һ��ָ���ʱ��Ż�ı���״̬
//��at_cmd_next����FALSEʱ���������е���״̬���Ѿ������ˣ���ʱ�Ḵ��״̬�����߼�

//����nbiot�����շ������ṹ��
static struct nbiot_recv  g_nbiot_recv_desc;
static struct nbiot_send  g_nbiot_send_desc;

//�������ڱ��浱ǰ����ִ�е�ATָ������ṹ��
static at_cmd_info_t g_at_cmd; 

static char cmd_buf_temp[NBIOT_RECV_BUF_MAX_LEN] = {0};

//����nbiot�豸�ṹ��
static struct nbiot_dev       g_nbiot_dev;

//����nbiot״̬��Ϣ�ṹ��
static nbiot_status_sm_t      g_nbiot_sm_status;

//����״̬��Ϣ�ṹ��
static nbiot_status_connect_t  g_nbiot_connect_status;     

//����nbiot�̼���Ϣ�ṹ��
static nbiot_firmware_info_t   g_firmware_info; 

//����socket��Ϣ�ṹ��
static nbiot_socket_info_t    g_socket_info[5];

//����������������
static int  __nbiot_uart_data_tx (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout);
static int  __nbiot_uart_data_rx (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout);


//�����¼��ص�������
static void __uart_event_cb_handle(void *p_arg);

//ͨ��ATָ�����Ӧ���ݣ��ж�atָ���Ƿ��ͳɹ�
static int8_t at_cmd_result_parse(char* buf);

//����ָ����Ӧ���������ǰ״̬��ָ��ִ�е�����
static uint8_t nbiot_response_handle (nbiot_handle_t nbiot_handle, uint8_t cmd_response);

//������һ��ATָ��
static uint8_t at_cmd_next (void);
   
//nbiot�¼�֪ͨ,�������¼��ص������е���
static uint8_t nbiot_event_notify (nbiot_handle_t nbiot_handle, char *buf);

//������Ϣ��Ӧ�ò㽻��
static void nbiot_msg_send (nbiot_handle_t nbiot_handle, char**buf, int8_t is_ok);


//nbiot��������
static int nbiot_data_recv(nbiot_handle_t nbiot_handle, uint8_t *pData, uint16_t size, uint32_t Timeout);


//���NBIoTģ����������
static struct nbiot_drv_funcs drv_funcs = {    
    __nbiot_uart_data_tx,
    __nbiot_uart_data_rx,        
};

//��λ���ջ���
static void nbiot_recv_buf_reset(void)
{   
    memset(&g_nbiot_recv_desc, 0, sizeof(struct nbiot_recv));
}

//��λִ�н���
static void nbiot_status_reset(void)
{
    g_nbiot_sm_status.main_status  = NBIOT_NONE;
    g_nbiot_sm_status.sub_status   = NBIOT_SUB_NONE;
}

//����ִ�н���
static void nbiot_status_set (nbiot_main_status_t  main_status, nbiot_sub_status_t  sub_status)
{
    g_nbiot_sm_status.main_status  = main_status;
    g_nbiot_sm_status.sub_status   = sub_status;
}

//����nbiot�¼�
void nbiot_event_set (nbiot_handle_t nbiot_handle, int nbiot_event)
{ 
    nbiot_handle->nbiot_event |= nbiot_event;   
}

//��ȡnbiot�¼�
int nbiot_event_get (nbiot_handle_t nbiot_handle,  int nbiot_event)
{ 
    return (nbiot_handle->nbiot_event & nbiot_event); 
}

//���nbiot�¼�
void nbiot_event_clr (nbiot_handle_t nbiot_handle, int nbiot_event)
{ 
    nbiot_handle->nbiot_event &= ~nbiot_event;
}

//�����¼��ص�������
static void __uart_event_cb_handle (void *p_arg)
{    
    nbiot_handle_t  nbiot_handle = (nbiot_handle_t)p_arg; 
    
    uart_dev_t       *p_uart_dev     = nbiot_handle->p_uart_dev;
  
    int size = g_nbiot_recv_desc.len;
    
    if (p_uart_dev->uart_event & UART_TX_EVENT)
    {
               
        NBIOT_DEBUG_INFO("atk_nbiot uart tx ok %s", g_nbiot_send_desc.buf);  

        uart_event_clr(p_uart_dev, UART_TX_EVENT);                 
    }

    if (p_uart_dev->uart_event & UART_RX_EVENT)
    {               
        size = uart_ring_buf_avail_len(p_uart_dev);
        
        //�ӻ��������ж�ȡ����
        if (size > 0)
        {                                  
            nbiot_data_recv(nbiot_handle, (uint8_t*)(&g_nbiot_recv_desc.buf[g_nbiot_recv_desc.len]), size, 0);
                     
            //�����첽�¼��ȴ�����
            nbiot_event_notify(nbiot_handle, g_nbiot_recv_desc.buf);
                        
            NBIOT_DEBUG_INFO("atk_nbiot uart rx ok %s\r\n", &g_nbiot_recv_desc.buf[g_nbiot_recv_desc.len]);
          
            //�ı�������ݵĻ�����λ�ã�ȷ��һ������������������ȫ�յ�
            g_nbiot_recv_desc.len = g_nbiot_recv_desc.len + size;
            
        }
                                                                                 
        uart_event_clr(p_uart_dev, UART_RX_EVENT); 
    } 

    //�������ͳ�ʱ�¼���˵��ָ���п���û�з��ͳ�ȥ�����߷��͹����г���
    //����ģ�鹤���쳣��û�л�Ӧ���������
    if (p_uart_dev->uart_event & UART_TX_TIMEOUT_EVENT) 
    {        
        NBIOT_DEBUG_INFO("atk_nbiot uart tx timeout %s", g_nbiot_send_desc.buf);  

        nbiot_event_set(nbiot_handle, NBIOT_TIMEOUT_EVENT);
      
        uart_event_clr(p_uart_dev, UART_TX_TIMEOUT_EVENT); 
    } 

    //���ʹ�÷ǳ�ʱ��֡�����¼������ϲ��ᷢ��
    if (p_uart_dev->uart_event & UART_RX_TIMEOUT_EVENT) 
    {       
        size = uart_ring_buf_avail_len(p_uart_dev);
        
        //��ʱ��֡      
        if (g_nbiot_dev.frame_format == 1) 
        {
            if (size > 0)
            {               
                nbiot_data_recv(nbiot_handle, (uint8_t*)(&g_nbiot_recv_desc.buf[g_nbiot_recv_desc.len]), size, 0);
                                             
                //�����첽�¼��ȴ�����
                nbiot_event_notify(nbiot_handle, g_nbiot_recv_desc.buf);
              
                g_nbiot_recv_desc.len = g_nbiot_recv_desc.len + size;
            }
                                        
       //���ڳ�ʱ��֡��״̬�£������ȷ�����˳�ʱ�¼�      
       } else {
         
            if (size > 0)
            {               
                nbiot_data_recv(nbiot_handle, (uint8_t*)(&g_nbiot_recv_desc.buf[g_nbiot_recv_desc.len]), size, 0);
                                             
                //�����첽�¼��ȴ�����
                nbiot_event_notify(nbiot_handle, g_nbiot_recv_desc.buf);
              
                //�ı�������ݵĻ�����λ�ã�ȷ��һ������������������ȫ�յ�
                g_nbiot_recv_desc.len = g_nbiot_recv_desc.len + size;
            }          
         
            //������ʱ�¼�����ʱ�¼��Ƿ�ִ�еÿ�����������
            nbiot_event_set(nbiot_handle, NBIOT_TIMEOUT_EVENT);
       }
       
       NBIOT_DEBUG_INFO("atk_nbiot uart rx timeout %s\r\n", g_nbiot_recv_desc.buf);  
       
       uart_event_clr(p_uart_dev, UART_RX_TIMEOUT_EVENT);
    }            
}

//uint8_t tmp_buf[513];
//nbiot�¼�������
int nbiot_event_poll(nbiot_handle_t nbiot_handle)
{
    char *at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX] = {0};
    
    char *p_revc_buf_tmp = g_nbiot_recv_desc.buf;
    
    uint8_t index = 0;
        
    int8_t next_cmd = 0;
    
    int8_t cmd_is_pass = 0;
    
    static int8_t recv_cnt = 0;
            
    if (nbiot_handle->nbiot_event & NBIOT_RECV_EVENT) 
    {       
        //�����ڳ�ʱʱ����������ɣ�ֹͣ���ճ�ʱ  
        atk_soft_timer_stop(&nbiot_handle->p_uart_dev->uart_rx_timer); 
        //�����������ڴ��󼴿ɽ������������������볬ʱ�¼�
        nbiot_event_clr(nbiot_handle, NBIOT_TIMEOUT_EVENT); 
        
        NBIOT_DEBUG_INFO("%s recv event\r\n", g_at_cmd.p_atcmd);
      
        cmd_is_pass = at_cmd_result_parse(g_nbiot_recv_desc.buf);
        
        //������Ӧ���     
        if (cmd_is_pass == AT_CMD_RESULT_OK) 
        {
            NBIOT_DEBUG_INFO("RESULT OK\r\n");            
                             
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

            if (index != 0)
            {               
                NBIOT_DEBUG_INFO("%s cmd excute ok\r\n\r\n\r\n", g_at_cmd.p_atcmd);
              
                //��������ͳɹ��˲��õ���ȷ����Ӧ
                nbiot_msg_send(nbiot_handle, at_response_par, TRUE);
                next_cmd = nbiot_response_handle(nbiot_handle, TRUE);                
                                                                    
            } else {
                
                //δ�յ�����\r\n...\r\n��ȷ������֡�����ݽ�����ط������������
                next_cmd = nbiot_response_handle(nbiot_handle, FALSE); 
            }                
                                                                                      
            //�建��            
            nbiot_recv_buf_reset();                        
       
        }       
        else if(cmd_is_pass == AT_CMD_RESULT_ERROR)
        { 
            NBIOT_DEBUG_INFO("RESULT ERROR\r\n");           

            recv_cnt=0;
          
            //���������������Գ����ط������������������ִ����һ������            
            next_cmd = nbiot_response_handle(nbiot_handle, FALSE);     
        
            if (g_at_cmd.cmd_action & ACTION_ERROR_AND_TRY)
            {                                   
                NBIOT_DEBUG_INFO("%s cmd is failed and try\r\n", g_at_cmd.p_atcmd);
                
                at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = (char*)g_at_cmd.p_atcmd;
                
                //֪ͨ�ϲ�Ӧ�ã��˶���ִ��ʧ��,��������
                nbiot_msg_send(nbiot_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], NBIOT_ERROR_RETRY);
                
            } 
            else if (g_at_cmd.cmd_action & ACTION_ERROR_BUT_NEXT)
            {               
                at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = (char*)g_at_cmd.p_atcmd;
               
                NBIOT_DEBUG_INFO("%s cmd is failed and next\r\n", g_at_cmd.p_atcmd);
                
                //֪ͨ�ϲ�Ӧ�ã��˶���ִ��ʧ�ܺ�����������ִ��
                nbiot_msg_send(nbiot_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], NBIOT_ERROR_NEXT);
                
            }
            else 
            {                
                NBIOT_DEBUG_INFO("%s cmd is failed and exit\r\n", g_at_cmd.p_atcmd);        
                at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = (char*)g_at_cmd.p_atcmd;
                
                //֪ͨ�ϲ�Ӧ�ã��˶���ִ��ʧ�ܺ�����������ִ��
                nbiot_msg_send(nbiot_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], FALSE);  

                //��λ״̬��־
                nbiot_status_reset();              

            } 
                    
            //�建��            
            nbiot_recv_buf_reset();            

        } 

        else if (cmd_is_pass == AT_CMD_RESULT_CONTINUE)
        {
          
            NBIOT_DEBUG_INFO("RESULT CONTINUE\r\n");
            //����δִ����ɣ���������£��յ��ĵ����������, �������Ļ��ǵ�ǰ������Ӧ���ݵĽ���, �����������ճ�ʱ               
            atk_soft_timer_timeout_change(&nbiot_handle->p_uart_dev->uart_rx_timer, 500);
            recv_cnt=0;
          
            //����TCP/UDP����״̬ʱ
            if (g_nbiot_sm_status.main_status == NBIOT_TCPUDP_CR) {
                //֪ͨ�ϲ�Ӧ�ã���ȡ��ص���Ϣ
                nbiot_msg_send(nbiot_handle, at_response_par, NBIOT_ERROR_CONTINUE);
            }          
             
            //����δ���            
        }       
        else 
        {
         
            recv_cnt++;

            NBIOT_DEBUG_INFO("recv_cn =%d\r\n", recv_cnt);            
          
            //�����ϲ������е�����, ���е���������������
            //һ���Ǳ�ʾ��������յ������ݶ�������        
            //�ڶ�����IDLE��֡�жϲ�����ô׼ȷ��û������Ҳ��Ϊһ֡������
            if (recv_cnt > (AT_CMD_RESPONSE_PAR_NUM_MAX + 10))                         
            {  
               //�����ڳ�ʱʱ����δ������ɣ�ֹͣ���ճ�ʱ  
               atk_soft_timer_stop(&nbiot_handle->p_uart_dev->uart_rx_timer);   
              
               //�յ���������,ǿ�ƽ��ս�����ͬʱ��������������ж������Ƿ��ط�������������ִ����һ������
               next_cmd = nbiot_response_handle(nbiot_handle, FALSE);
               //�建��            
               nbiot_recv_buf_reset();                
            }
            else 
            {
              
               //����δ���,�յ��ĵ���������Ե������е�һ����                
               atk_soft_timer_timeout_change(&nbiot_handle->p_uart_dev->uart_rx_timer, 8000);
             
            }              
                               
        } 
            
        nbiot_event_clr(nbiot_handle, NBIOT_RECV_EVENT); 
    }
       
    if (nbiot_handle->nbiot_event & NBIOT_REG_STA_EVENT) 
    {      
        NBIOT_DEBUG_INFO("reg event\r\n");
                             
        at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = "reg event";        
          
        //֪ͨ�ϲ�Ӧ������ע����
        nbiot_msg_send(nbiot_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], TRUE); 


        if (g_nbiot_connect_status.register_status == 1) 
        {            
            //�������ִ����һ������        
            next_cmd = AT_CMD_NEXT;
        }
        
        nbiot_event_clr(nbiot_handle,   NBIOT_REG_STA_EVENT); 
      
        //�����������    
        nbiot_recv_buf_reset();   
    }
    
    if (nbiot_handle->nbiot_event & NBIOT_TCP_RECV_EVENT) 
    {
        NBIOT_DEBUG_INFO("tcp recv event ok\r\n");
      
        //֪ͨ�ϲ�Ӧ�ý��յ�TCP����
        nbiot_msg_send(nbiot_handle, NULL, TRUE);     
        
        nbiot_event_clr(nbiot_handle,NBIOT_TCP_RECV_EVENT);
      
        //�����������    
        nbiot_recv_buf_reset();       
    }

    if (nbiot_handle->nbiot_event & NBIOT_UDP_RECV_EVENT) 
    {
        NBIOT_DEBUG_INFO("udp recv event ok\r\n");
      
        //֪ͨ�ϲ�Ӧ������ע����
        nbiot_msg_send(nbiot_handle, NULL, TRUE);    
        
        nbiot_event_clr(nbiot_handle, NBIOT_UDP_RECV_EVENT); 
      
        //�����������    
        nbiot_recv_buf_reset();  
    }
        
    if (nbiot_handle->nbiot_event & NBIOT_COAP_RECV_EVENT) 
    {
        
        NBIOT_DEBUG_INFO("coap recv event ok\r\n");
      
        //֪ͨ�ϲ�Ӧ�ý��յ�CoAP����
        nbiot_msg_send(nbiot_handle, NULL, TRUE);          
             
        nbiot_event_clr(nbiot_handle, NBIOT_COAP_RECV_EVENT); 
      
        //�����������    
        nbiot_recv_buf_reset();  
    }
    
    if (nbiot_handle->nbiot_event & NBIOT_CM2M_RECV_EVENT) 
    {
        
        NBIOT_DEBUG_INFO("cm2m recv event ok\r\n");
      
        //֪ͨ�ϲ�Ӧ�ý��յ�CM2M����
        nbiot_msg_send(nbiot_handle, NULL, TRUE);          
             
        nbiot_event_clr(nbiot_handle, NBIOT_CM2M_RECV_EVENT); 
      
        //�����������    
        nbiot_recv_buf_reset();  
    }

    if (nbiot_handle->nbiot_event & NBIOT_CM2M_STATUS_EVENT) 
    {
        
        NBIOT_DEBUG_INFO("cm2m status event ok\r\n");
      
        //֪ͨ�ϲ�Ӧ��CM2M״̬���ӷ����仯
        nbiot_msg_send(nbiot_handle, NULL, TRUE);          
             
        nbiot_event_clr(nbiot_handle, NBIOT_CM2M_STATUS_EVENT); 
      
        //�����������    
        nbiot_recv_buf_reset();  
    }      
    
    
    if (nbiot_handle->nbiot_event & NBIOT_SOCKET_ERR_EVENT) 
    {       
        NBIOT_DEBUG_INFO("socket err event\r\n");
      
        //֪ͨ�ϲ�Ӧ��SOCKETʧ����
        nbiot_msg_send(nbiot_handle, NULL, TRUE);        
              
        nbiot_event_clr(nbiot_handle, NBIOT_SOCKET_ERR_EVENT); 
      
        //�����������    
        nbiot_recv_buf_reset();  
    }

    if (nbiot_handle->nbiot_event & NBIOT_TIMEOUT_EVENT) 
    {        
        //��ʱ����������������Գ����ط������������������ִ����һ������        
        next_cmd = nbiot_response_handle(nbiot_handle, FALSE);
      
        //֪ͨ�ϲ�Ӧ�ã��˶���ִ�г�ʱ
        if (g_at_cmd.cmd_action & ACTION_ERROR_BUT_NEXT) 
        {           
            at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = (char *)g_at_cmd.p_atcmd;
           
            NBIOT_DEBUG_INFO("%s cmd not repsonse or send failed\r\n", g_at_cmd.p_atcmd);
                               
            //֪ͨ�ϲ�Ӧ�ã��˶�����ʱִ��ʧ�ܺ�����������ִ��
            nbiot_msg_send(nbiot_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], NBIOT_ERROR_NEXT);            
        }        
        else if (g_at_cmd.cmd_action & ACTION_ERROR_AND_TRY) 
        {           
            at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = (char *)g_at_cmd.p_atcmd;
           
            NBIOT_DEBUG_INFO("%s cmd not repsonse or send failed\r\n", g_at_cmd.p_atcmd);
                               
            //֪ͨ�ϲ�Ӧ�ã��˶�����ʱִ��ʧ�ܺ�����������ִ��
            nbiot_msg_send(nbiot_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], NBIOT_ERROR_RETRY);            
        }
        else        
        {            
            NBIOT_DEBUG_INFO("%s cmd is failed and exit\r\n", g_at_cmd.p_atcmd);        
            at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1] = (char*)g_at_cmd.p_atcmd;
            
            //֪ͨ�ϲ�Ӧ�ã��˶�����ʱִ��ʧ�ܺ��������ִ��
            nbiot_msg_send(nbiot_handle, &at_response_par[AT_CMD_RESPONSE_PAR_NUM_MAX - 1], FALSE);  

            //��λ״̬��־
            nbiot_status_reset();              
        } 
        
        //�����������      
        nbiot_recv_buf_reset();      
                                                    
        nbiot_event_clr(nbiot_handle, NBIOT_TIMEOUT_EVENT); 
    } 
    
    //�����¼���״̬�ж��Ƿ���Ҫִ����һ������
    if(next_cmd == AT_CMD_NEXT)
    {
        //ִ����һ������
        if (at_cmd_next())
        {
            nbiot_at_cmd_send(nbiot_handle, &g_at_cmd);
        }
        
        //����FALSE��ʾ�ӽ����Ѿ�������
        else
        {
            //�������״̬�����е���״̬�����Ѿ��ɹ�ִ�������
            nbiot_msg_send(nbiot_handle, NULL,TRUE);

            //��λsm״̬��־
            nbiot_status_reset();
        }     
    }
   
    return NBIOT_OK;    
}

static int  __nbiot_uart_data_tx (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout)
{  
    int ret = 0;
    
    nbiot_handle_t  nbiot_handle = (nbiot_handle_t)p_arg;
    
    uart_handle_t uart_handle = nbiot_handle->p_uart_dev; 
    
    ret = uart_data_tx_poll(uart_handle, pData, size, Timeout); 

    return ret;    
}

static int  __nbiot_uart_data_rx (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout)
{  
    int ret = 0;
    
    //��ʱ��������ʹ��
    (void)Timeout;
    
    nbiot_handle_t  nbiot_handle = (nbiot_handle_t)p_arg;
    
    uart_handle_t uart_handle = nbiot_handle->p_uart_dev;

    uart_ring_buf_read(uart_handle, pData, size);    
    
    return ret;    
}


//��1���ַ�ת��Ϊ10��������
//chr:�ַ�,0~9/A~F/a~F
//����ֵ:chr��Ӧ��10������ֵ
static u8 nbiot_chr2hex (u8 chr)
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
static u8 nbiot_hex2chr (u8 hex)
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
      return '0';
    }              
}

//��hexת����һ�������ַ�,��0x12345678ת�����ַ���"12345678"
void nbiot_hex2chrbuf (char *src_p_buf, char *dest_buf, int len)
{
    int i = 0; 
   
    char tmp  = 0;
    char tmp1 = 0;
     
    for (i = 0; i < len; i=i+2)
    {
        tmp = nbiot_chr2hex( src_p_buf[i]);       
        tmp1 = nbiot_chr2hex(src_p_buf[i + 1]);      
        dest_buf[i / 2] = (tmp << 4)  |  tmp1;                    
    } 
    
    dest_buf[i / 2] = 0;      
}

//����������ÿ�����ֽ����һ��ʮ����������2���ֽڻ����1���ַ�
//hex:16��������,0~15;
//����ֵ:�ַ�
void nbiot_buf2chr (char *p_buf, int len)
{
    int i = 0; 
   
    char tmp  = 0;
    char tmp1 = 0;
     
    for (i = 0; i < len; i=i+2)
    {
        tmp  = nbiot_chr2hex(p_buf[i]);       
        tmp1 = nbiot_chr2hex(p_buf[i + 1]);      
        p_buf[i / 2] = (tmp << 4)  |  tmp1;                    
    } 
    
    p_buf[i / 2] = 0;      
}

//����������ÿ�����ֽ����һ��ʮ����������2���ֽڻ���һ���ֽڵ�ʮ��������
//hex:16��������,0~15;
//����ֵ:�ַ�
void nbiot_buf2hex (char *p_buf , int len)
{
    int i = 0; 
   
    char tmp  = 0;
    char tmp1 = 0;
     
    for (i = 0; i < len; i=i+2)
    {
        tmp = nbiot_chr2hex(p_buf[i]);       
        tmp1 = nbiot_chr2hex(p_buf[i + 1]);      
        p_buf[i / 2] = (tmp << 4)  |  tmp1;                    
    } 
    
    p_buf[i / 2] = 0;      
}


//����������ÿ�����ֽ����һ��ʮ����������2���ֽڻ����1���ַ�
//hex:16��������,0~15;
//����ֵ:�ַ�
void nbiot_srcbuf2chr (char *src_buf, char *dest_buf, int len)
{
    int i = 0; 
   
    char tmp  = 0;
    char tmp1 = 0;
     
    for (i = 0; i < len; i=i+2)
    {
        tmp  = nbiot_chr2hex(src_buf[i]);       
        tmp1 = nbiot_chr2hex(src_buf[i + 1]);      
        dest_buf[i / 2] = (tmp << 4)  |  tmp1;                    
    } 
    
    dest_buf[i / 2] = 0;      
}

//����������ÿ�����ֽ����һ��ʮ����������2���ֽڻ���һ���ֽڵ�ʮ��������
//hex:16��������,0~15;
//����ֵ:�ַ�
void nbiot_srcbuf2hex (char *src_buf ,char *dest_buf, int len)
{
    int i = 0; 
   
    char tmp  = 0;
    char tmp1 = 0;
     
    for (i = 0; i < len; i=i+2)
    {
        tmp = nbiot_chr2hex(src_buf[i]);       
        tmp1 = nbiot_chr2hex(src_buf[i + 1]);      
        dest_buf[i / 2] = (tmp << 4)  |  tmp1;                    
    } 
    
    dest_buf[i / 2] = 0;      
}


//nbiot atָ���ʼ��
void nbiot_at_cmd_param_init (at_cmdhandle cmd_handle,
                              const char *at_cmd,
                              char *argument,
                              cmd_property_t property,
                              uint32_t at_cmd_time_out)
{
    if (cmd_handle == NULL)
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

//����nbiot atָ����ַ��������Ӧ�ĳ���
static int cmd_generate(at_cmdhandle cmd_handle)
{
    int cmdLen = 0;
    
    if (cmd_handle == NULL)
    {
        return cmdLen;
    }
    memset(g_nbiot_send_desc.buf,0,NBIOT_SEND_BUF_MAX_LEN);
    g_nbiot_send_desc.len = 0;

    if(cmd_handle->property == CMD_TEST)
    {
        cmdLen = snprintf(g_nbiot_send_desc.buf,NBIOT_SEND_BUF_MAX_LEN,
                          "%s=?\r\n",
                          cmd_handle->p_atcmd);
    }    
    else if(cmd_handle->property == CMD_READ)
    {
        cmdLen = snprintf(g_nbiot_send_desc.buf,NBIOT_SEND_BUF_MAX_LEN,
                          "%s?\r\n",
                          cmd_handle->p_atcmd);
    }
    else if(cmd_handle->property == CMD_EXCUTE)
    {
        cmdLen = snprintf(g_nbiot_send_desc.buf,NBIOT_SEND_BUF_MAX_LEN,
                          "%s\r\n",
                          cmd_handle->p_atcmd);    
    }

    else if(cmd_handle->property == CMD_SET)
    {
        cmdLen = snprintf(g_nbiot_send_desc.buf,NBIOT_SEND_BUF_MAX_LEN,
                          "%s=%s\r\n",
                          cmd_handle->p_atcmd,cmd_handle->p_atcmd_arg);    
    }
    
    //cmdlen����Ч�����ݳ��ȣ��������ַ���������Ƿ�
    g_nbiot_send_desc.len = cmdLen;
    
    return cmdLen;
}


//ͨ��ATָ�����Ӧ���ݣ��ж�atָ���Ƿ��ͳɹ�
//����ֵ�����-1������յ������ݿ���Ϊ����
//����ֵ�����0�� ָ��ͨ���������1��ָ�����
static int8_t at_cmd_result_parse (char *buf)
{
    int8_t result = -1;
  
//    NBIOT_DEBUG_INFO("respones buf%s\r\n",buf); 
//  
//    NBIOT_DEBUG_INFO("respones buf len %d \r\n", strlen(buf)); 
  
    //ȷ���յ��������Ƿ�Ϊ������Ӧ�Ĳ���(+) 
    char *p_colon = strchr(buf,'+');
    
    //ȷ���յ��������Ƿ�Ϊ������Ӧ�Ĳ���(:)
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
          
            NBIOT_DEBUG_INFO("p_colon %s  p_colon_temp %s \r\n", p_colon, p_colon_temp);
            //ǰ�߳���������Ļ��� || ���߳���OK֮ǰ���������
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
         
        else if (strstr(buf, g_at_cmd.p_atcmd) || ((p_colon && p_colon_temp))) {
          
             result = AT_CMD_RESULT_CONTINUE;
          
        } else {
          
             //����
             result = AT_CMD_RESULT_RANDOM_CODE;      
          
        }    
    }

    return result;
}


//����nbiot�¼�֪ͨ
static uint8_t nbiot_event_notify (nbiot_handle_t nbiot_handle, char *buf)
{
    char *target_pos_start = NULL;
       
    if((target_pos_start = strstr(buf, "+CEREG:")) != NULL)
    {
        char *p_colon = strchr(target_pos_start,',');
      
        if (p_colon)
        {
            p_colon = p_colon + 1;
            
            //���������ֱ�Ӹ�������ע��״̬����Ϣ�����ַ�����ʾ�ģ�Ҫת��������        
            g_nbiot_connect_status.register_status = (*p_colon - '0');
        }        
        else 
        {
           p_colon = strchr(target_pos_start,':');
           p_colon = p_colon + 1; 
            
           //���������ֱ�Ӹ�������ע��״̬����Ϣ�����ַ�����ʾ�ģ�Ҫת��������        
           g_nbiot_connect_status.register_status = (*p_colon - '0');
        }
        
        nbiot_event_set(nbiot_handle, NBIOT_REG_STA_EVENT);                                   
    }    
    else if((target_pos_start = strstr(buf,"+CSONMI")) != NULL)
    {
        //�յ��������˷���TCP/UDP����
        char *p_colon = strchr(target_pos_start, ':');
      
//        int8_t socket_id = 0;
        
        //�õ����ĸ�socket�յ�����
        if (p_colon)
        {
            p_colon++;
//            socket_id = strtoul(p_colon,0,10);
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
            nbiot_event_set(nbiot_handle, NBIOT_UDP_RECV_EVENT);             
        }         
        else if (g_socket_info[0].socket_type == NBIOT_TCP)
        {

            nbiot_event_set(nbiot_handle, NBIOT_TCP_RECV_EVENT); 
        } 
        else 
        {
            //������Ĭ�ϲ��������¼�
            nbiot_event_set(nbiot_handle, NBIOT_RECV_EVENT);            
        }
        
        nbiot_status_set(NBIOT_TCPUDP_RECV, NBIOT_SUB_TCPUDP_RECV);        
    }  
    else if((target_pos_start = strstr(buf,"+CSOERR")) != NULL)
    {
        //�յ��������˷���TCP/UDP������
        char *p_colon = strchr(target_pos_start,':');
      
//        int8_t socket_id = 0;
      
        int8_t socket_err = 0;
        
        //�õ����ĸ�socket�յ�����
        if (p_colon)
        {
            p_colon++;          
//            socket_id = strtoul(p_colon,0,10);
        }
        
        //�õ��յ���socket������
        char *pComma = strchr(p_colon,',');
        
        if(pComma)
        {
          pComma++;
          socket_err = strtoul(pComma, 0, 10);          
          g_socket_info[0].socket_errcode = socket_err; 
        }
                
        nbiot_event_set(nbiot_handle, NBIOT_SOCKET_ERR_EVENT); 
        nbiot_status_set(NBIOT_SOCKET_ERR, NBIOT_SUB_SOCKET_ERR);        
    }
    
    //�յ�Coap���ݰ�
    else if((target_pos_start = strstr(buf,"+CCOAPNMI")) != NULL)
    {
        //�յ��������˷���CoAP����
        char *p_colon = strchr(target_pos_start, ':');
      
//        int8_t coap_id = 0;
        
        //�õ����ĸ�coap�յ�������
        if (p_colon)
        {
            p_colon++;
//            coap_id = strtoul(p_colon,0,10);
        } 
        
        //�õ��յ������ݳ���
        char *pComma = strchr(p_colon,',');

        if (pComma)
        {
            pComma++;
            g_nbiot_connect_status.data_len = strtoul(pComma,0,10);
        }     
       
        //�õ���Ч���ݵ���ʼ��ַ
        char *p_data_offest = strchr(pComma,',');

        if (p_data_offest)
        {
            p_data_offest++;
            g_nbiot_connect_status.data_offest = p_data_offest;
        }                   
      
        nbiot_event_set(nbiot_handle, NBIOT_COAP_RECV_EVENT);  
        nbiot_status_set(NBIOT_CoAP_RECV, NBIOT_SUB_CoAP_RECV);  
    } 
    else if ((target_pos_start = strstr(buf,"+CM2MCLI:")) != NULL)
    {
        //�յ��������˷���CM2M״̬����
        char *p_colon = strchr(target_pos_start, ':');
              
        //�õ�CM2M��ǰ��״̬
        if (p_colon)
        {
            p_colon++;
            g_nbiot_connect_status.m2m_status = strtoul(p_colon,0,10);
        } 
                              
        nbiot_event_set(nbiot_handle, NBIOT_CM2M_STATUS_EVENT);  
        nbiot_status_set(NBIOT_CM2M_STATUS, NBIOT_SUB_CM2M_STATUS);          
    }

    else if ((target_pos_start = strstr(buf,"+CM2MCLIRECV")) != NULL)
    {
        //�յ��������˷���CM2M����
        char *p_colon = strchr(target_pos_start, ':');
             
        //�õ���Ч���ݵ���ʼ��ַ
        if (p_colon)
        {
            p_colon =  p_colon + 2;
            g_nbiot_connect_status.data_offest = p_colon;
                     
        } 
                              
        nbiot_event_set(nbiot_handle, NBIOT_CM2M_RECV_EVENT);  
        nbiot_status_set(NBIOT_CM2M_RECV, NBIOT_SUB_CM2M_RECV);           
    }    
    else if((target_pos_start = strstr(buf,"+CLMOBSERVE")) != NULL)
    {
        nbiot_event_set(nbiot_handle, NBIOT_LWM2M_RECV_EVENT);          
    } 

    //�յ�MQTT���ݰ�    
    else if ((target_pos_start = strstr(buf,"+CMQPUB")) != NULL)
    {        
        nbiot_event_set(nbiot_handle, NBIOT_MQTT_RECV_EVENT);  
    }    
    else 
    {
        //����յ��ظ���������������Ӧ������
        nbiot_event_set(nbiot_handle, NBIOT_RECV_EVENT);  
    }
    
    return 0;
}

//������һ��ATָ��
static uint8_t at_cmd_next (void)
{ 
    if (g_nbiot_sm_status.main_status == NBIOT_INIT)
    {
        g_nbiot_sm_status.sub_status++;
      
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
        {
            return FALSE;
        }

        switch(g_nbiot_sm_status.sub_status)
        {
          
        case NBIOT_SUB_SYNC:
            
            break;
        
        case NBIOT_SUB_CMEE:
          
            nbiot_at_cmd_param_init(&g_at_cmd, AT_CMEE, "1", CMD_SET, 500);
          
            break;        
        
        case NBIOT_SUB_BAND:
          
            nbiot_at_cmd_param_init(&g_at_cmd, AT_NBAND, "5", CMD_SET, 500);
          
            break;
        
        case NBIOT_SUB_ATI:
          
            nbiot_at_cmd_param_init(&g_at_cmd, AT_ATI, NULL, CMD_EXCUTE, 3000);
            g_at_cmd.p_expectres = "Quectel";     //���������ظ���Ϣ�����ָ��ִ�����
                                                  //û������������Ϣƥ�䣬����Ϊ����
                                                  //�����г�����           
            break;
                      
        //��ѯNB��״̬�Ƿ�׼����    
        case NBIOT_SUB_CPIN:
          {
            nbiot_at_cmd_param_init(&g_at_cmd, AT_NCCID, NULL, CMD_READ, 3000);
            g_at_cmd.p_expectres = "+NCCID:"; //���������ظ���Ϣ�����ָ��ִ�����
                                              //û������������Ϣƥ�䣬����Ϊ����
                                              //�����г�����              
          }
          break;
                    
          
        //��ѯ��Ƶģ���ź�����   
        case NBIOT_SUB_CSQ:
          {
             nbiot_at_cmd_param_init(&g_at_cmd, AT_CSQ, NULL, CMD_EXCUTE, 3000);
          }
          break;

        //ʹ��ģ����Ƶ�ź�,��Ӧ�ȴ����ʱ��Ϊ6S      
        case NBIOT_SUB_CFUN:                                   
          {
            nbiot_at_cmd_param_init(&g_at_cmd, AT_CFUN,"1",CMD_SET, 6000);
          }
          
          break;
          
        // ʹ��nbiot����ע��   
        case NBIOT_SUB_CEREG:
          {
            nbiot_at_cmd_param_init(&g_at_cmd, AT_CEREG, "1", CMD_SET, 500);
            g_at_cmd.cmd_action  = ACTION_OK_WAIT | ACTION_ERROR_AND_TRY;  
              
          }
          break;  
          
                 
        //��ѯPDP������Ϣ          
        case NBIOT_SUB_CIPCA_QUERY:
          {
            nbiot_at_cmd_param_init(&g_at_cmd, AT_CIPCA, NULL, CMD_READ, 500);

          }
          break;          
               
          
        //ʹ�����總��,�����Ӧʱ��Ϊ1s,������������Ϊ3s     
        case NBIOT_SUB_CGATT:
          {
            nbiot_at_cmd_param_init(&g_at_cmd, AT_CGATT, "1", CMD_SET, 3000);            
          }
          break;
          
          
        //��ѯģ�������״̬��Ϣ     
        case NBIOT_SUB_NUESTATS:
          {
            nbiot_at_cmd_param_init(&g_at_cmd, AT_NUESTATS, NULL, CMD_EXCUTE, 500);
          }
          break;
          
        //��ѯ�����APN��Ϣ��IP��ַ     
        case NBIOT_SUB_CGPADDR:
          {
            nbiot_at_cmd_param_init(&g_at_cmd, AT_CGPADDR, NULL, CMD_EXCUTE, 500);
               
          }
          break;          
          
          
        //��ѯ���總����Ϣ,�����Ӧʱ�䲻��       
        case NBIOT_SUB_CGATT_QUERY:
          {
            nbiot_at_cmd_param_init(&g_at_cmd, AT_CGATT, NULL, CMD_READ, 3000);
            
            //���������ظ���Ϣ�����ָ��ִ�����
            //û������������Ϣƥ�䣬����Ϊ����                                             
            //�����г�����               
            g_at_cmd.p_expectres = "CGATT:1";     
          }
          break;
          
                                      
        //��ѯnbiot�����Ƿ�ע��,�����Ӧʱ�䲻��       
        case NBIOT_SUB_CEREG_QUERY:
          {
            nbiot_at_cmd_param_init(&g_at_cmd, AT_CEREG, NULL, CMD_READ, 500);             

          }
          break;
                    
        default: 
          
          //ǿ�Ʊ�ʾ�ӽ��̽���
          g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
        
          return FALSE;
                   
         }
    }
    
    else if (g_nbiot_sm_status.main_status == NBIOT_INFO)
    {
        g_nbiot_sm_status.sub_status++;
      
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
        {
            return FALSE;
        }
        
        switch(g_nbiot_sm_status.sub_status)
        {
          
        case  NBIOT_SUB_CGMI:
          
          {
            nbiot_at_cmd_param_init(&g_at_cmd,AT_CGMI,NULL,CMD_EXCUTE,3000);
          }
          break;

                   
        case NBIOT_SUB_CGMM:
          {
            nbiot_at_cmd_param_init(&g_at_cmd,AT_CGMM,NULL,CMD_EXCUTE,3000);
          }
          break;
          
        case NBIOT_SUB_CGMR:
          {
            nbiot_at_cmd_param_init(&g_at_cmd,AT_CGMR,NULL,CMD_EXCUTE,3000);
          }
          break;
          
        case NBIOT_SUB_CIMI:
          {
            nbiot_at_cmd_param_init(&g_at_cmd,AT_CIMI,NULL,CMD_EXCUTE,3000);
          }
          break;

        case NBIOT_SUB_CGSN:
          {
            nbiot_at_cmd_param_init(&g_at_cmd, AT_CGSN, NULL, CMD_EXCUTE, 3000);
            
            //���������ظ���ϢΪ��
            //���ָ��ִ�����,û������������Ϣƥ��                                            
            //����Ϊ���������г�����               
            g_at_cmd.p_expectres = "CGSN";
          }
          break;             
          
          
        case NBIOT_SUB_CBAND:
          {
            nbiot_at_cmd_param_init(&g_at_cmd,AT_CBAND,NULL,CMD_READ,3000);
            //���������ظ���ϢΪ��
            //���ָ��ִ�����,û������������Ϣƥ��                                            
            //����Ϊ���������г�����               
            g_at_cmd.p_expectres = "CBAND";
          }
          break;
          
        default: 
          
          //ǿ�Ʊ�ʾ�ӽ��̽���
          g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
        
          return FALSE;          
        
        }
    }
    
    else if (g_nbiot_sm_status.main_status == NBIOT_SIGNAL)
    {
        
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
        return FALSE;
    }    
    else if (g_nbiot_sm_status.main_status == NBIOT_RESET)
    {
        
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
        return FALSE;
    }
    else if (g_nbiot_sm_status.main_status == NBIOT_NCONFIG)
    {
        
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
        return FALSE;
    }     
    else if (g_nbiot_sm_status.main_status == NBIOT_TCPUDP_CR)
    {
        
        g_nbiot_sm_status.sub_status++;
      
      
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
        {
            return FALSE;
        }
        
        switch(g_nbiot_sm_status.sub_status)
        {
          
        case NBIOT_SUB_TCPUDP_CONNECT:
          
          {
          
            char *p_remote_port = NULL;

            //����ʹ��ջ�ϵ��ڴ�����������
            memset(cmd_buf_temp,0,sizeof(cmd_buf_temp));
            
            if (g_socket_info[0].socket_type == NBIOT_TCP)
            { 
                p_remote_port = REMOTE_TCP_PORT;
            }
            else
            {
                p_remote_port = REMOTE_UDP_PORT;   
            }              

    
            //��������ķ���ֵΪ��Ҫ��ʽ��д��ĳ��ȣ��������ַ������������Զ���������ַ�
            uint16_t tcpudp_cn_len = snprintf(cmd_buf_temp,
                                        sizeof(cmd_buf_temp) -1,"%d,%s,%s",                                          
                                        g_socket_info[0].socket_id,
                                        p_remote_port,                                         
                                        REMOTE_SERVER_IP);
                                        
//            NBIOT_DEBUG_INFO("tcpudp_cn_len = %d\r\n", tcpudp_cn_len);
//                                        
//            NBIOT_DEBUG_INFO("tcpudp_cn_buf = %s\r\n", buf);                            
            
            //�����Ӧʱ�䲻��                                        
            nbiot_at_cmd_param_init(&g_at_cmd,AT_CSOCON, cmd_buf_temp, CMD_SET, 3000);
                                        
          }
          break;

                                     
        default: 
          
          //ǿ�Ʊ�ʾ�ӽ��̽���
          g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
        
          return FALSE;          
        
        }
    } 
    else if (g_nbiot_sm_status.main_status == NBIOT_TCPUDP_CL) 
    {
      
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
      
        return FALSE;    
      
    }  
    else if (g_nbiot_sm_status.main_status == NBIOT_TCPUDP_SEND) 
    {
      
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
      
        return FALSE;     
    }

    else if (g_nbiot_sm_status.main_status == NBIOT_CoAP_SEVER) 
    {
      
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
      
        return FALSE;    
      
    } 
    
    else if (g_nbiot_sm_status.main_status == NBIOT_CoAP_CLIENT) 
    {
      
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
      
        return FALSE;     
    }
   
     else if (g_nbiot_sm_status.main_status == NBIOT_CoAP_SEND) 
    {
      
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
      
        return FALSE;    
      
    } 
    
    else if (g_nbiot_sm_status.main_status == NBIOT_CoAP_CL) 
    {
      
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
      
        return FALSE;     
    }

    else if (g_nbiot_sm_status.main_status == NBIOT_CM2M_CLIENT) 
    {
      
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
      
        return FALSE;     
    }
   
     else if (g_nbiot_sm_status.main_status == NBIOT_CM2M_SEND) 
    {
      
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
      
        return FALSE;    
      
    } 
    
    else if (g_nbiot_sm_status.main_status == NBIOT_CM2M_CL) 
    {
      
        g_nbiot_sm_status.sub_status = NBIOT_SUB_END;
      
        return FALSE;     
    } 
       
    else if (g_nbiot_sm_status.main_status == NBIOT_NONE)   
    {  //��ֹ�����ط�
       return FALSE; 
    }
    
    else 
    {
      
    }
    
    return TRUE;
}

//������Ϣ��Ӧ�ò㽻��
static void nbiot_msg_send (nbiot_handle_t nbiot_handle, char**buf, int8_t is_ok)
{
    if (nbiot_handle == NULL)
    {
      return;
    }

    if ((is_ok == NBIOT_ERROR_RETRY) || 
        (is_ok == NBIOT_ERROR_TIMEOUT)) {
        
         nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_CMD_RETRY, strlen(buf[0]), buf[0]);  
        
         return;      
    }

    //����������ָ��ִ��  
    else if (is_ok == NBIOT_ERROR_NEXT)
    {

        nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_CMD_NEXT, strlen(buf[0]), buf[0]);  
        
        return;
    }    
           
    //�������ϱ�������ִ��ʧ��
    else if(is_ok == FALSE)
    {
        nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_CMD_FAIL, strlen(buf[0]), buf[0]);
        return;
    }

    if (g_nbiot_sm_status.main_status == NBIOT_INIT)
    {
        switch(g_nbiot_sm_status.sub_status)
        {
            
        case NBIOT_SUB_SYNC:
            
          break;

        case NBIOT_SUB_CMEE:
          
          break;    

        case NBIOT_SUB_ATI:
          
          //�õ�ģ�������
          memcpy(g_firmware_info.name, buf[1], strlen(buf[1])); 
            
          break;

        //��ѯNB��״̬�Ƿ�׼����    
        case NBIOT_SUB_CPIN:
          {
       
          }
          break;        

          
        case NBIOT_SUB_CSQ:
        {
            char *p_colon = strchr(buf[0],':');
            p_colon++;
          
            //ת����10��������
            uint8_t lqi =strtoul(p_colon,0, 10);
          
            //����ȡ��ÿ����ֵ��Ӧ��dbm��Χ
            int8_t rssi = -110 + (lqi << 1);
          
            g_nbiot_connect_status.rssi = rssi; 
          
            break;
        }  
                  
        case NBIOT_SUB_CFUN:
            break;


        case NBIOT_SUB_CEREG:
           
            NBIOT_DEBUG_INFO("reg status=%d\r\n", g_nbiot_connect_status.register_status);           
            break;


   
        //    case NBIOT_SUB_CIPCA:
        //        
        //        break;
        //    
        //    
        case NBIOT_SUB_CIPCA_QUERY:
        {
            char *p_colon = strchr(buf[0],':');
                        
            if (p_colon != NULL) 
            {                
                p_colon++;
                
                //ת����10��������,�õ���ǰ������cid
                g_nbiot_connect_status.cid =strtoul(p_colon,0, 10);
            }
            
            break;
         }
               
        case NBIOT_SUB_CGATT:
            
            break;


        case NBIOT_SUB_CGATT_QUERY:
            
            break;

        case NBIOT_SUB_CGPADDR:
            
            break;       
           
        case NBIOT_SUB_END:
            
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_INIT,1,"S");

            break;

        default:
          
            break;
        }
    }
    else if(g_nbiot_sm_status.main_status == NBIOT_NCONFIG)  
    {          
      if (g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
      {
          nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_NCONFIG,1,"S"); 
      }        

    }

    else if(g_nbiot_sm_status.main_status == NBIOT_RESET)  
    {
        
   
      if (g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
      {
          nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_RESET,1,"S"); 
      }        

    }    
    
    else if(g_nbiot_sm_status.main_status == NBIOT_INFO)
    {
        switch(g_nbiot_sm_status.sub_status)
        {
            
        //��ѯ����ע��״̬    
        case NBIOT_SUB_CEREG_QUERY:

          nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_REG, strlen(buf[1]), buf[1]);

          break;        
            
                     
        case NBIOT_SUB_CGMI:
          {
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_MID, strlen(buf[1]), buf[1]);
          }
          break;
          
        case NBIOT_SUB_CGMM:
          {
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_MMODEL,strlen(buf[1]),buf[1]);
          }
          break;
          
        case NBIOT_SUB_CGMR:
          {

             nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_MREV,strlen(buf[1]),buf[1]);
            
          }
          break;
          
          
        case NBIOT_SUB_CIMI:
          {
            memcpy(g_firmware_info.IMSI,buf[1],15);
            g_firmware_info.IMSI[15] = 0;
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_IMSI,strlen(buf[1]),buf[1]);
          }
          break;
          

        case NBIOT_SUB_CGSN:
          {
            char *p_colon = strchr(buf[1],':');
            
            if(p_colon)
            {
              p_colon = p_colon + 2;
              memcpy(g_firmware_info.IMEI ,p_colon,15);
              g_firmware_info.IMEI[15] = 0;
              nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_IMEI,15,(char*)g_firmware_info.IMEI);
            }
          }
          break;            
          
        case NBIOT_SUB_CBAND:
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
              nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_BAND,strlen(pFreq),pFreq);
            }
          }
          break;

        case NBIOT_SUB_END:
          {
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_INFO, 1, "S");
          }
          break;
        }
    }
    else if(g_nbiot_sm_status.main_status == NBIOT_SIGNAL)
    {
        switch(g_nbiot_sm_status.sub_status) 
        { 
        case NBIOT_SUB_CSQ:
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
              
              g_nbiot_connect_status.rssi = rssi;
              
              nbiot_handle->nbiot_cb(nbiot_handle->p_arg,(nbiot_msg_id_t)NBIOT_MSG_CSQ,len,buf[1]);
              
            }
          
            break;
        } 
          
        case NBIOT_SUB_END:
        {
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg,(nbiot_msg_id_t)NBIOT_MSG_SIGNAL,1,"S");
            break;
        }

        default:
          
            break;
        }

    }
    else if(g_nbiot_sm_status.main_status == NBIOT_TCPUDP_CR)
    {
        switch(g_nbiot_sm_status.sub_status)
        {
        case NBIOT_SUB_TCPUDP_CR:
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
          
        case NBIOT_SUB_TCPUDP_CONNECT:
          {                                             
          }
          break;
          
        case NBIOT_SUB_END:
          {              
             char *p_buf_tmep = NULL;
                  
             g_nbiot_connect_status.connect_status = 1;  
                   
             if (g_socket_info[0].socket_type == NBIOT_TCP)
             {
                 p_buf_tmep = "tcp create ok";
             }
            
             else 
             {   
                 p_buf_tmep = "udp create ok";
             } 
                                                           
             nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_TCPUDP_CREATE, strlen(p_buf_tmep), p_buf_tmep);
                    
          }
          break;
        }
    } 
    else if(g_nbiot_sm_status.main_status == NBIOT_TCPUDP_CL)
    {
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
        {
             g_nbiot_connect_status.connect_status = 0;  
          
             char *p_buf_tmep = NULL;
                                        
             if (g_socket_info[0].socket_type == NBIOT_TCP)
             {
                 p_buf_tmep = "tcp close";
             }         
             else 
             {   
                 p_buf_tmep = "udp close";
             }         
             nbiot_handle->nbiot_cb(nbiot_handle->p_arg,(nbiot_msg_id_t)NBIOT_MSG_TCPUDP_CLOSE, strlen(p_buf_tmep), p_buf_tmep);
         }
    }   
    else if(g_nbiot_sm_status.main_status == NBIOT_TCPUDP_SEND)
    {
        switch(g_nbiot_sm_status.sub_status)
        {
          
        case NBIOT_SUB_END:
          {
            char *p_buf_tmep = g_nbiot_send_desc.buf;
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_TCPUDP_SEND,strlen(p_buf_tmep),p_buf_tmep);
          }
          break;
          
        default:
          
          break;
        }
    }
    else if(g_nbiot_sm_status.main_status == NBIOT_TCPUDP_RECV)
    {
        if(g_nbiot_sm_status.sub_status == NBIOT_SUB_TCPUDP_RECV)
        {
          char *data_buf = g_socket_info[0].data_offest; 
          //��λ״̬��־
          nbiot_status_reset();      

        //      NBIOT_DEBUG_INFO("data_buf = %s", data_buf);      
               
          nbiot_handle->nbiot_cb(nbiot_handle->p_arg,(nbiot_msg_id_t)NBIOT_MSG_TCPUDP_RECV,strlen(data_buf),data_buf);
          
        }

    }

    else if(g_nbiot_sm_status.main_status == NBIOT_SOCKET_ERR)
    {
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_SOCKET_ERR)
        {

            NBIOT_DEBUG_INFO("the socket err, the err code is %d\r\n", g_socket_info[0].socket_errcode); 
          
            //��λ״̬��־
            nbiot_status_reset();      
                 
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg,
                                       (nbiot_msg_id_t)NBIOT_MSG_SOCKET_ERROR, 
                                        1, 
                                        (char *)&g_socket_info[0].socket_errcode);

        }

    }

    else if(g_nbiot_sm_status.main_status == NBIOT_CoAP_SEVER)
    {
        if(g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
        {     
          nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_COAP_SERVER, 1, "S");
        }
    }
    else if(g_nbiot_sm_status.main_status == NBIOT_CoAP_CLIENT)
    {
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_CoAP_CLIENT)
        { 
            //��¼���ӵ�id
            char *p_colon = strchr(buf[1],':');
                        
            if (p_colon != NULL) 
            {                
                p_colon++;
                
                //ת����10��������,�õ���ǰ������socket id
                g_nbiot_connect_status.connect_id =strtoul(p_colon, 0, 10);
            }
        }    

        if(g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
        { 
          g_nbiot_connect_status.connect_status = 1;         
          nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_COAP_CLIENT, 1, "S");
        }
    }  

    else if(g_nbiot_sm_status.main_status == NBIOT_CoAP_SEND)
    {
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
        {
            char *p_buf_tmep = g_nbiot_send_desc.buf;
          
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_COAP_SEND,strlen(p_buf_tmep),p_buf_tmep);
        }
    }
    else if(g_nbiot_sm_status.main_status == NBIOT_CoAP_RECV)
    {
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_CoAP_RECV)
        {
            char *data_buf = g_nbiot_connect_status.data_offest; 
          
            //��λ״̬��־
            nbiot_status_reset();      
                       
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg,(nbiot_msg_id_t)NBIOT_MSG_COAP_RECV,strlen(data_buf),data_buf);
                   
        }
    }

    else if(g_nbiot_sm_status.main_status == NBIOT_CoAP_CL)
    {
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
        {
             g_nbiot_connect_status.connect_status = 0;  
          
             char *p_buf_tmep = NULL;
                                        
             if (g_nbiot_connect_status.connect_type == NBIOT_COAP)
             {
                 p_buf_tmep = "coap close";
             }         
            
             nbiot_handle->nbiot_cb(nbiot_handle->p_arg,(nbiot_msg_id_t)NBIOT_MSG_COAP_CLOSE, strlen(p_buf_tmep), p_buf_tmep);
         }
    }

    else if(g_nbiot_sm_status.main_status == NBIOT_CM2M_CLIENT)
    {
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_CM2M_CLIENT)
        { 
            
        }    

        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
        { 
            g_nbiot_connect_status.connect_status = 1;         
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_CM2M_CLIENT, 1, "S");
        }
    }  

    else if(g_nbiot_sm_status.main_status == NBIOT_CM2M_SEND)
    {
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
        {
            char *p_buf_tmep = g_nbiot_send_desc.buf;
          
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg, (nbiot_msg_id_t)NBIOT_MSG_CM2M_SEND, strlen(p_buf_tmep), p_buf_tmep);
        }
    }
    else if(g_nbiot_sm_status.main_status == NBIOT_CM2M_RECV)
    {
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_CM2M_RECV)
        {
            char *data_buf = g_nbiot_connect_status.data_offest; 
          
            //��λ״̬��־
            nbiot_status_reset();      
                        
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg,(nbiot_msg_id_t)NBIOT_MSG_CM2M_RECV,strlen(data_buf),data_buf);
          
             
        }
    }

    else if(g_nbiot_sm_status.main_status == NBIOT_CM2M_STATUS)
    {
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_CM2M_STATUS)
        {
            //��λ״̬��־
            nbiot_status_reset();                         
            nbiot_handle->nbiot_cb(nbiot_handle->p_arg,(nbiot_msg_id_t)NBIOT_MSG_CM2M_STATUS,1, (char *)&g_nbiot_connect_status.m2m_status);
                   
        }
    }  

    else if(g_nbiot_sm_status.main_status == NBIOT_CM2M_CL)
    {
        if (g_nbiot_sm_status.sub_status == NBIOT_SUB_END)
        {
             g_nbiot_connect_status.connect_status = 0;  
          
             char *p_buf_tmep = NULL;
                                        
             if (g_nbiot_connect_status.connect_type == NBIOT_CM2M)
             {
                 p_buf_tmep = "cm2m close";
             }         
            
             nbiot_handle->nbiot_cb(nbiot_handle->p_arg,(nbiot_msg_id_t)NBIOT_MSG_CM2M_CLOSE, strlen(p_buf_tmep), p_buf_tmep);
        }
    }

    else
    {

    }    
}

//ָ����Ӧ�����������������������״̬���ԣ�������״̬��ִ�С�
uint8_t nbiot_response_handle (nbiot_handle_t nbiot_handle, uint8_t cmd_response)
{
    uint8_t next_cmd = AT_CMD_OK;
      
    if (cmd_response)
    {
        if (g_at_cmd.cmd_action & ACTION_OK_AND_NEXT)
        {
            next_cmd = AT_CMD_NEXT;
        }        
        else if (g_at_cmd.cmd_action & ACTION_OK_WAIT)   
        {
            //��������ִ�гɹ����ȴ�
            next_cmd = AT_CMD_WAIT;             
        }
        else  
        {
            //��������ִ�гɹ����˳��ô�״̬���������������������
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
                //�����ط�������
                nbiot_at_cmd_send(nbiot_handle, &g_at_cmd);
            }
            else
            {
                  
               //���Դﵽ���������ñ��������ԣ���������ִ�д�����˳��ô�״̬���������������������
                g_at_cmd.cmd_action = ACTION_ERROR_EXIT;
                
            }
        }        
        else if (g_at_cmd.cmd_action & ACTION_OK_WAIT)   
        {
            //��������ִ�гɹ����ȴ�
            next_cmd = AT_CMD_WAIT;             
        }
                
        else if (!(g_at_cmd.cmd_action & ACTION_ERROR_EXIT))  
        {
            //����ִ�д�������������ִ����һ������
            next_cmd = TRUE;
        }
        else 
        {
            //nerver reach here  
        }
    }
    
    return next_cmd;
}


//��nbiotģ�鷢��ATָ��
//nbiot_handle     nbiot_handleģ���豸���
//cmd_handle       ��Ҫ����ָ����Ϣ���
//note ���øú���ǰ�ȹ��������Ĳ���
int nbiot_at_cmd_send(nbiot_handle_t nbiot_handle, at_cmdhandle cmd_handle)
{
    int strLen = 0;
    
    int ret = 0;
        
    if (nbiot_handle == NULL || cmd_handle == NULL)
    {
       return NBIOT_ERROR;
    }
        
    strLen = cmd_generate(cmd_handle);

    ret = nbiot_handle->p_drv_funcs->nbiot_send_data(nbiot_handle, 
                                                     (uint8_t*)g_nbiot_send_desc.buf, 
                                                     strLen,                                                    
                                                     cmd_handle->max_timeout);   
    return ret;
}

//nbiotģ���������
//nbiot_handleģ���豸���
static int nbiot_data_recv(nbiot_handle_t nbiot_handle, uint8_t *pData, uint16_t size, uint32_t Timeout)
{   
    int ret = 0;
        
    if (nbiot_handle == NULL)
    {
       return NBIOT_ERROR;
    }
        
    ret = nbiot_handle->p_drv_funcs->nbiot_recv_data(nbiot_handle, pData, size, Timeout);      
    
    return ret;
}


//nbiotģ���ʼ�� 
nbiot_handle_t nbiot_dev_init(uart_handle_t nbiot_handle)
{
     //����豸�ṹ��
     g_nbiot_dev.p_uart_dev       = nbiot_handle;
     g_nbiot_dev.p_drv_funcs      = &drv_funcs; 

     g_nbiot_dev.p_nbiot_cmd      = &g_at_cmd;    
     g_nbiot_dev.p_socket_info    = g_socket_info;
     g_nbiot_dev.p_firmware_info  = &g_firmware_info;
     g_nbiot_dev.p_sm_status      = &g_nbiot_sm_status;
     g_nbiot_dev.p_connect_status = &g_nbiot_connect_status;
  
     g_nbiot_dev.frame_format     = 0;  
    
     /* ע��nbiot�����շ��¼��ص����� */
     uart_event_registercb(nbiot_handle, __uart_event_cb_handle, &g_nbiot_dev);     
    
     return &g_nbiot_dev;    
}

//ע��nbiotģ���¼��ص�����
void nbiot_event_registercb (nbiot_handle_t nbiot_handle, nbiot_cb cb, void *p_arg)
{  
    if(cb != 0)
    {
        nbiot_handle->nbiot_cb  = (nbiot_cb)cb;
        nbiot_handle->p_arg       = p_arg;
    }
}








