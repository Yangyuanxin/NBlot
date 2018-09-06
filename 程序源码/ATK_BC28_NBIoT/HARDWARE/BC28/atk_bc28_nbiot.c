
////////////////////////////////////////////////////////////////////////////////// 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//����1��ʼ��           
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2018/7/11
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
#include "atk_bc28_nbiot.h"
#include "atk_delay.h"

#define SIM7020_NBLOT_DEBUG                   
#ifdef SIM7020_NBLOT_DEBUG
#define SIM7020_NBLOT_DEBUG_INFO(...)    (int)printf(__VA_ARGS__)
#else
#define SIM7020_NBLOT_DEBUG_INFO(...)
#endif

static char cmd_buf_temp[SIM7020_RECV_BUF_MAX_LEN] = {0};

//sim7020 nblot��ʼ�����������ע��
int sim7020_nblot_init (sim7020_handle_t sim7020_handle)
{
    
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }

    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_SYNC, NULL, CMD_EXCUTE, 3000);

    //����SIM7020_NBLOT_INIT״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_NBLOT_INIT;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_SYNC;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);

    return SIM7020_OK;
}

//��ȡNBģ�����Ϣ
int sim7020_nblot_info_get(sim7020_handle_t sim7020_handle)
{

    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
    
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CGREG, NULL, CMD_READ, 3000);

    //����SIM7020_NBLOT_INFO״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_NBLOT_INFO;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_CEREG_QUERY;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
    
    return SIM7020_OK;
}


//��ȡNBģ����ź�����
int sim7020_nblot_signal_get(sim7020_handle_t sim7020_handle)
{

    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
        
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CSQ, NULL, CMD_EXCUTE, 3000);

    //����SIM7020_SIGNAL״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_SIGNAL;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_CSQ;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
    
    return SIM7020_OK;
}

//����tcpudp 
int sim7020_nblot_tcpudp_create(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{
    char *p_tcpudp = NULL;
  
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
    if (type == SIM7020_TCP)
    {
        sim7020_handle->p_socket_info[0].socket_type = SIM7020_TCP;
        p_tcpudp = "1,1,1";
    }
    
    else if(type == SIM7020_UDP)
    {   
        sim7020_handle->p_socket_info[0].socket_type = SIM7020_UDP;
        p_tcpudp = "1,2,1";
    } 
    else 
    {
       return SIM7020_NOTSUPPORT;
      
    }
            
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CSOC, p_tcpudp, CMD_SET, 3000);
    
    //���봴��TCP/UDP״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_TCPUDP_CR;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_TCPUDP_CR;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
    
    return SIM7020_OK;
}


//�ر�tcpudp
int sim7020_nblot_tcpudp_close(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{

    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
       
    (void)type;
    
    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط���������ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));
       
    int16_t msg_len = snprintf(cmd_buf_temp,
                                sizeof(cmd_buf_temp),
                                "%d",
                                sim7020_handle->p_socket_info[0].socket_id);
                                
    if (msg_len < 0) {
      
        return SIM7020_ERROR;
    }
                                                   
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CSOCL, cmd_buf_temp, CMD_SET, 3000);

    //����tcp/udp�ر�״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_TCPUDP_CL;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_TCPUDP_CL;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
        
    return SIM7020_OK;
}


//��hex���ݸ�ʽ��������,������ż��������
int sim7020_nblot_tcpudp_send_hex(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type)
{
  
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
  
    //�ж�SOCKET ID�Ƿ���ȷ
    if (sim7020_handle->p_socket_info[0].socket_id  < 0 || sim7020_handle->p_socket_info[0].socket_id > 5 )
    {
        return SIM7020_ERROR;
    }
  
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (SIM7020_SEND_BUF_MAX_LEN - 20) ;

    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));


    uint16_t msg_len = snprintf(cmd_buf_temp,
                                str_len,
                                "%d,%d,",
                                 sim7020_handle->p_socket_info[0].socket_id, 
                                len);
                                
    for(uint16_t i = 0 ; i < len ; i++)
    {
        sprintf(&cmd_buf_temp[msg_len + (i << 1)],"%02X",(uint8_t)msg[i]);
    }                             
 
    //����TCP/UDP���ݷ�����������Ӧʱ�䲻��
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CSOSEND, cmd_buf_temp, CMD_SET, 3000);
    
    //����tcp/udp���ݷ���״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_TCPUDP_SEND;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_TCPUDP_SEND;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);

    return SIM7020_OK;
}

//���ַ�����ʽ��������
int sim7020_nblot_tcpudp_send_str(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type)
{
  
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
  
    //�ж�SOCKET ID�Ƿ���ȷ
    if (sim7020_handle->p_socket_info[0].socket_id  < 0 || sim7020_handle->p_socket_info[0].socket_id > 5 )
    {
        return SIM7020_ERROR;
    }
      

    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (SIM7020_SEND_BUF_MAX_LEN - 20) ;

    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));
    

    int16_t msg_len = snprintf(cmd_buf_temp,
                               str_len,
                               "%d,%d,%s%s%s",
                                sim7020_handle->p_socket_info[0].socket_id,
                                0,
                                "\"",                                  
                                msg,
                                "\"");
    
    if (msg_len < 0) {
      
        return SIM7020_ERROR;
    }
    
                                                          
    //����TCP/UDP���ݷ�����������Ӧʱ�䲻��
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CSOSEND, cmd_buf_temp, CMD_SET, 3000);
    
    //����tcp/udp���ݷ���״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_TCPUDP_SEND;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_TCPUDP_SEND;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
    
    return SIM7020_OK;
}   

//����coap������ 
int sim7020_nblot_coap_server_create(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{
   
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
    if (type != SIM7020_COAP)
    {
        return SIM7020_NOTSUPPORT;
    }
    
    sim7020_handle->sim7020_connect_status->connect_type = SIM7020_COAP;  
    sim7020_handle->sim7020_connect_status->cid          = 1;
    sim7020_handle->sim7020_connect_status->connect_id   = 1;
    
    
    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));    
               
                       
    //��������ķ���ֵΪ��Ҫ   ��ʽ��д��ĳ��ȣ��������ַ������������Զ���������ַ�
    uint16_t coap_cn_len = snprintf(cmd_buf_temp,
                                    sizeof(cmd_buf_temp) -1,"%s,%s,%d",                                                                                             
                                    REMOTE_SERVER_IP,
                                    REMOTE_COAP_PORT,
                                    sim7020_handle->sim7020_connect_status->cid);
                                        
                     
            
    //�����Ӧʱ�䲻��                                              
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CCOAPSTA, cmd_buf_temp, CMD_SET, 3000);
    sim7020_handle->p_sim7020_cmd->cmd_action = ACTION_OK_AND_NEXT | ACTION_ERROR_BUT_NEXT;                                
    
    //��������COAP������״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_CoAP_SEVER;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_CoAP_SEVER;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
    
    return SIM7020_OK;
}

//����coap�ͻ���
int sim7020_nblot_coap_client_create(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{
     
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
    if (type != SIM7020_COAP)
    {
        return SIM7020_NOTSUPPORT;
    }
    
    sim7020_handle->sim7020_connect_status->connect_type = SIM7020_COAP;
    sim7020_handle->sim7020_connect_status->cid          = 1;  

    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));       
                
    //��������ķ���ֵΪ��Ҫ��ʽ��д��ĳ��ȣ��������ַ������������Զ���������ַ�
    uint16_t coap_cn_len = snprintf(cmd_buf_temp,
                                    sizeof(cmd_buf_temp) -1,"%s,%s,%d",                                                                                             
                                    REMOTE_SERVER_IP,
                                    REMOTE_COAP_PORT,
                                    sim7020_handle->sim7020_connect_status->cid);
                                                                       
    //�����Ӧʱ�䲻��                                              
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CCOAPNEW, cmd_buf_temp, CMD_SET, 3000);
    sim7020_handle->p_sim7020_cmd->cmd_action = ACTION_OK_AND_NEXT | ACTION_ERROR_BUT_NEXT; 
                                       
    //���봴���ͻ���״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_CoAP_CLIENT;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_CoAP_CLIENT;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
    
    return SIM7020_OK;
}


//�ر�coap
int sim7020_nblot_coap_close(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{
  
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
           
    if (type != SIM7020_COAP)
    {
        return SIM7020_NOTSUPPORT;
    }
    
    sim7020_handle->sim7020_connect_status->connect_type = SIM7020_COAP;  

    //ͨ���ͻ���id������COAP����
    cmd_buf_temp[0] = sim7020_handle->sim7020_connect_status->connect_id + '0'; 
    cmd_buf_temp[1] = 0;
    cmd_buf_temp[2] = 0;     
                                                                                   
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CCOAPDEL, cmd_buf_temp, CMD_SET, 3000);

    //����coap�ر�״̬,�����Ӧʱ�䲻��
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_CoAP_CL;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_CoAP_CL;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
    
    
    return SIM7020_OK;
}


//��hex���ݸ�ʽ����coapЭ������,������ż��������
int sim7020_nblot_coap_send_hex(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type)
{ 
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
  
    //�ж�coap id�Ƿ���ȷ
    if (sim7020_handle->sim7020_connect_status->connect_id  < 0)
    {
        return SIM7020_ERROR;
    }
    
    if (type != SIM7020_COAP)
    {
        return SIM7020_NOTSUPPORT;
    }    
  
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (SIM7020_SEND_BUF_MAX_LEN - 24) ;

    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));    


    uint16_t msg_len = snprintf(cmd_buf_temp,
                                str_len,
                                "%d,%d,",
                                 sim7020_handle->sim7020_connect_status->connect_id, 
                                len);
    
    cmd_buf_temp[msg_len] = '\"';
    
    
    
    for(uint16_t i = 0 ; i < len ; i++)
    {
        //��ʾһ���ַ���ʮ�����Ʊ�ʾ 
        sprintf(&cmd_buf_temp[msg_len + 1 + (i << 1)],"%02X",(uint8_t)msg[i]);
    }

    cmd_buf_temp[msg_len + 1 + (len << 1)] = '\"';    
 
    //����coap���ݷ�����������Ӧʱ�䲻��
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CCOAPSEND, cmd_buf_temp, CMD_SET, 3000);
    
    //����coap���ݷ���״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_CoAP_SEND;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_CoAP_SEND;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);

    return SIM7020_OK;
}

//���ַ�����ʽ��������coapЭ������
int sim7020_nblot_coap_send_str(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type)
{ 
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
  
    //�ж�coap id�Ƿ���ȷ
    if (sim7020_handle->sim7020_connect_status->connect_id  < 0)
    {
        return SIM7020_ERROR;
    }
    
    if (type != SIM7020_COAP)
    {
        return SIM7020_NOTSUPPORT;
    } 
      
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (SIM7020_SEND_BUF_MAX_LEN - 24) ;


    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));   

    int16_t msg_len = snprintf(cmd_buf_temp,
                               str_len,
                               "%d,%d,%s%s%s",
                                sim7020_handle->sim7020_connect_status->connect_id ,
                                len,
                                "\"",                                  
                                msg,
                                "\"");
    
    if (msg_len < 0) {
      
        return SIM7020_ERROR;
    }
    
                                                          
    //����coap���ݷ�����������Ӧʱ�䲻��
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CCOAPSEND, cmd_buf_temp, CMD_SET, 3000);
    
    //����coap���ݷ���״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_CoAP_SEND;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_CoAP_SEND;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
         
    return SIM7020_OK;
}

//����cm2m�ͻ���
int sim7020_nblot_cm2m_client_create(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{     
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
    
    if (type != SIM7020_CM2M)
    {
        return SIM7020_NOTSUPPORT;
    }
    
    sim7020_handle->sim7020_connect_status->connect_type = SIM7020_CM2M;

    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));       
                
    //��������ķ���ֵΪ��Ҫ��ʽ��д��ĳ��ȣ��������ַ������������Զ���������ַ�
    uint16_t cm2m_cn_len = snprintf(cmd_buf_temp,
                                    sizeof(cmd_buf_temp) -1,"%s,%s%s%s,%s%s%s,%d",                                                                                             
                                    REMOTE_SERVER_IP,
                                    "\"",
                                    REMOTE_COAP_PORT,
                                    "\"",
                                    "\"",
                                    sim7020_handle->firmware_info->IMEI,
                                    "\"",
                                    100);
                                                                       
    //�����Ӧʱ�䲻��                                              
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CM2MCLINEW, cmd_buf_temp, CMD_SET, 15000);
                                    
    sim7020_handle->p_sim7020_cmd->cmd_action = ACTION_OK_AND_NEXT | ACTION_ERROR_BUT_NEXT; 
                                       
    //���봴���ͻ���״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_CM2M_CLIENT;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_CM2M_CLIENT;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
    
    return SIM7020_OK;
}


//�ر�cm2m
int sim7020_nblot_cm2m_close(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type)
{
  
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
           
    if (type != SIM7020_CM2M)
    {
        return SIM7020_NOTSUPPORT;
    }   
                                                                                   
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CM2MCLIDEL, NULL, CMD_EXCUTE, 15000);
    sim7020_handle->p_sim7020_cmd->cmd_action = ACTION_OK_AND_NEXT | ACTION_ERROR_BUT_NEXT;     

    //����cm2m�ر�״̬,�����Ӧʱ�䲻��
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_CM2M_CL;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_CM2M_CL;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
        
    return SIM7020_OK;
}

//��hex���ݸ�ʽ����cm2mЭ������,������ż��������
int sim7020_nblot_cm2m_send_hex(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type)
{ 
    if (sim7020_handle->sim7020_sm_status->main_status != SIM7020_NONE)
    {
        return SIM7020_ERROR;
    }
  
    //�����Ͳ�ΪCM2Mʱ���򶼷��͵����ݸ�����Ϊż����ʱ
    if ((type != SIM7020_CM2M) || ((strlen(msg) % 2) != 0))
    {
        return SIM7020_NOTSUPPORT;
    } 
      
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (SIM7020_SEND_BUF_MAX_LEN - 24) ;


    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));   

    int16_t msg_len = snprintf(cmd_buf_temp,
                               str_len,
                               "%s%s%s",                                
                                "\"",                                  
                                msg,
                                "\"");
    
    if (msg_len < 0) {
      
        return SIM7020_ERROR;
    }
    
                                                          
    //����cm2m���ݷ�����������Ӧʱ�䲻��
    at_cmd_param_init(sim7020_handle->p_sim7020_cmd, AT_CM2MCLISEND, cmd_buf_temp, CMD_SET, 3000);
    
    //����cm2m���ݷ���״̬
    sim7020_handle->sim7020_sm_status->main_status = SIM7020_CM2M_SEND;
    sim7020_handle->sim7020_sm_status->sub_status  = SIM7020_SUB_CM2M_SEND;

    sim7020_at_cmd_send(sim7020_handle, sim7020_handle->p_sim7020_cmd);
         
    return SIM7020_OK;
}


