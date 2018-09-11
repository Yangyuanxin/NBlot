
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

#define NBIOT_DEBUG                   
#ifdef NBIOT_DEBUG
#define NBIOT_DEBUG_INFO(...)    (int)printf(__VA_ARGS__)
#else
#define NBIOT_DEBUG_INFO(...)
#endif

static char cmd_buf_temp[NBIOT_RECV_BUF_MAX_LEN] = {0};

//NBģ���ʼ�����������ע��
int nbiot_init (nbiot_handle_t nbiot_handle)
{    
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }

    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_SYNC, NULL, CMD_EXCUTE, 3000);

    //����NBIOT_INIT״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_INIT;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_SYNC;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
  
    return NBIOT_OK;
}


//����nbiotģ��
int nbiot_reboot(nbiot_handle_t nbiot_handle)
{

    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
    
    //�������ʱʱ��Ϊ300ms��Ϊ�����������ﳬʱ����Ϊ500ms    
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_NRB, NULL, CMD_EXCUTE, 500);

    //����NBIOT_REBOOT״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_RESET;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_RESET;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
    
    return NBIOT_OK;
}

//ʹ��/�����Զ�����
int nbiot_nconfig(nbiot_handle_t nbiot_handle, uint8_t auto_flag)
{
    char *p_auto_nconfig = NULL;
    
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
        
    if (auto_flag == 0) 
    {
        
        p_auto_nconfig = "AUTOCONNECT,FALSE";
    }    
    else 
    {
        p_auto_nconfig = "AUTOCONNECT,TRUE"; 
    }
        
    //�������ʱʱ��Ϊ300ms��Ϊ�����������ﳬʱ����Ϊ500ms    
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_NCONFIG, p_auto_nconfig, CMD_SET, 500);

    //����NBIOT_REBOOT״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_NCONFIG;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_NCONFIG;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
    
    return NBIOT_OK;
}



//��ȡNBģ�����Ϣ
int nbiot_info_get(nbiot_handle_t nbiot_handle)
{

    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
    
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_CEREG, NULL, CMD_READ, 3000);

    //����NBIOT_INFO״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_INFO;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_CEREG_QUERY;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
    
    return NBIOT_OK;
}


//��ȡNBģ����ź�����
int nbiot_signal_get(nbiot_handle_t nbiot_handle)
{

    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
        
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_CSQ, NULL, CMD_EXCUTE, 3000);

    //����NBIOT_SIGNAL״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_SIGNAL;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_CSQ;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
    
    return NBIOT_OK;
}

//����tcpudp 
int nbiot_tcpudp_create(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type)
{
    char *p_tcpudp = NULL;
  
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
    
    if (type == NBIOT_TCP)
    {
        nbiot_handle->p_socket_info[0].socket_type = NBIOT_TCP;
        p_tcpudp = "1,1,1";
    }
    
    else if(type == NBIOT_UDP)
    {   
        nbiot_handle->p_socket_info[0].socket_type = NBIOT_UDP;
        p_tcpudp = "1,2,1";
    } 
    else 
    {
       return NBIOT_NOTSUPPORT;
      
    }
            
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_CSOC, p_tcpudp, CMD_SET, 3000);
    
    //���봴��TCP/UDP״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_TCPUDP_CR;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_TCPUDP_CR;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
    
    return NBIOT_OK;
}


//�ر�tcpudp
int nbiot_tcpudp_close(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type)
{

    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
    
       
    (void)type;
    
    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط���������ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));
       
    int16_t msg_len = snprintf(cmd_buf_temp,
                                sizeof(cmd_buf_temp),
                                "%d",
                                nbiot_handle->p_socket_info[0].socket_id);
                                
    if (msg_len < 0) {
      
        return NBIOT_ERROR;
    }
                                                   
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_CSOCL, cmd_buf_temp, CMD_SET, 3000);

    //����tcp/udp�ر�״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_TCPUDP_CL;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_TCPUDP_CL;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
        
    return NBIOT_OK;
}


//��hex���ݸ�ʽ��������,������ż��������
int nbiot_tcpudp_send_hex(nbiot_handle_t nbiot_handle, int len, char *msg, nbiot_connect_type_t type)
{
  
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
  
    //�ж�SOCKET ID�Ƿ���ȷ
    if (nbiot_handle->p_socket_info[0].socket_id  < 0 || nbiot_handle->p_socket_info[0].socket_id > 5 )
    {
        return NBIOT_ERROR;
    }
  
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (NBIOT_SEND_BUF_MAX_LEN - 20) ;

    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));


    uint16_t msg_len = snprintf(cmd_buf_temp,
                                str_len,
                                "%d,%d,",
                                 nbiot_handle->p_socket_info[0].socket_id, 
                                len);
                                
    for(uint16_t i = 0 ; i < len ; i++)
    {
        sprintf(&cmd_buf_temp[msg_len + (i << 1)],"%02X",(uint8_t)msg[i]);
    }                             
 
    //����TCP/UDP���ݷ�����������Ӧʱ�䲻��
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_CSOSEND, cmd_buf_temp, CMD_SET, 3000);
    
    //����tcp/udp���ݷ���״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_TCPUDP_SEND;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_TCPUDP_SEND;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);

    return NBIOT_OK;
}

//���ַ�����ʽ��������
int nbiot_tcpudp_send_str(nbiot_handle_t nbiot_handle, int len, char *msg, nbiot_connect_type_t type)
{
  
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
  
    //�ж�SOCKET ID�Ƿ���ȷ
    if (nbiot_handle->p_socket_info[0].socket_id  < 0 || nbiot_handle->p_socket_info[0].socket_id > 5 )
    {
        return NBIOT_ERROR;
    }
      

    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (NBIOT_SEND_BUF_MAX_LEN - 20) ;

    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));
    

    int16_t msg_len = snprintf(cmd_buf_temp,
                               str_len,
                               "%d,%d,%s%s%s",
                                nbiot_handle->p_socket_info[0].socket_id,
                                0,
                                "\"",                                  
                                msg,
                                "\"");
    
    if (msg_len < 0) {
      
        return NBIOT_ERROR;
    }
    
                                                          
    //����TCP/UDP���ݷ�����������Ӧʱ�䲻��
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_CSOSEND, cmd_buf_temp, CMD_SET, 3000);
    
    //����tcp/udp���ݷ���״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_TCPUDP_SEND;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_TCPUDP_SEND;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
    
    return NBIOT_OK;
}   

//����coap������ 
int nbiot_coap_server_create(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type)
{  
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
    
    if (type != NBIOT_COAP)
    {
        return NBIOT_NOTSUPPORT;
    }
    
    nbiot_handle->p_connect_status->connect_type = NBIOT_COAP;  
    nbiot_handle->p_connect_status->cid          = 1;
    nbiot_handle->p_connect_status->connect_id   = 1;
    
    
    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));    
               
                       
    //��������ķ���ֵΪ��Ҫ   ��ʽ��д��ĳ��ȣ��������ַ������������Զ���������ַ�
    uint16_t coap_cn_len = snprintf(cmd_buf_temp,
                                    sizeof(cmd_buf_temp) -1,"%s,%s,%d",                                                                                             
                                    REMOTE_SERVER_IP,
                                    REMOTE_COAP_PORT,
                                    nbiot_handle->p_connect_status->cid);
                                        
                     
            
    //�����Ӧʱ�䲻��                                              
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_CCOAPSTA, cmd_buf_temp, CMD_SET, 3000);
    nbiot_handle->p_nbiot_cmd->cmd_action = ACTION_OK_AND_NEXT | ACTION_ERROR_BUT_NEXT;                                
    
    //��������COAP������״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_CoAP_SEVER;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_CoAP_SEVER;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
    
    return NBIOT_OK;
}


//����coap�ͻ���
int nbiot_coap_client_create(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type)
{
     
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
    
    if (type != NBIOT_COAP)
    {
        return NBIOT_NOTSUPPORT;
    }
    
    nbiot_handle->p_connect_status->connect_type = NBIOT_COAP;
    nbiot_handle->p_connect_status->cid          = 1;  

    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));       
                
    //��������ķ���ֵΪ��Ҫ��ʽ��д��ĳ��ȣ��������ַ������������Զ���������ַ�
    uint16_t coap_cn_len = snprintf(cmd_buf_temp,
                                    sizeof(cmd_buf_temp) -1,"%s,%s,%d",                                                                                             
                                    REMOTE_SERVER_IP,
                                    REMOTE_COAP_PORT,
                                    nbiot_handle->p_connect_status->cid);
                                                                       
    //�����Ӧʱ�䲻��                                              
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_CCOAPNEW, cmd_buf_temp, CMD_SET, 3000);
    nbiot_handle->p_nbiot_cmd->cmd_action = ACTION_OK_AND_NEXT | ACTION_ERROR_BUT_NEXT; 
                                       
    //���봴���ͻ���״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_CoAP_CLIENT;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_CoAP_CLIENT;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
    
    return NBIOT_OK;
}


//�ر�coap
int nbiot_coap_close(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type)
{
  
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
           
    if (type != NBIOT_COAP)
    {
        return NBIOT_NOTSUPPORT;
    }
    
    nbiot_handle->p_connect_status->connect_type = NBIOT_COAP;  

    //ͨ���ͻ���id������COAP����
    cmd_buf_temp[0] = nbiot_handle->p_connect_status->connect_id + '0'; 
    cmd_buf_temp[1] = 0;
    cmd_buf_temp[2] = 0;     
                                                                                   
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_CCOAPDEL, cmd_buf_temp, CMD_SET, 3000);

    //����coap�ر�״̬,�����Ӧʱ�䲻��
    nbiot_handle->p_sm_status->main_status = NBIOT_CoAP_CL;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_CoAP_CL;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
    
    
    return NBIOT_OK;
}


//��hex���ݸ�ʽ����coapЭ������,������ż��������
int nbiot_coap_send_hex(nbiot_handle_t nbiot_handle, int len, char *msg, nbiot_connect_type_t type)
{ 
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
  
    //�ж�coap id�Ƿ���ȷ
    if (nbiot_handle->p_connect_status->connect_id  < 0)
    {
        return NBIOT_ERROR;
    }
    
    if (type != NBIOT_COAP)
    {
        return NBIOT_NOTSUPPORT;
    }    
  
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (NBIOT_SEND_BUF_MAX_LEN - 24) ;

    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));    


    uint16_t msg_len = snprintf(cmd_buf_temp,
                                str_len,
                                "%d,%d,",
                                 nbiot_handle->p_connect_status->connect_id, 
                                len);
    
    cmd_buf_temp[msg_len] = '\"';
    
    
    
    for(uint16_t i = 0 ; i < len ; i++)
    {
        //��ʾһ���ַ���ʮ�����Ʊ�ʾ 
        sprintf(&cmd_buf_temp[msg_len + 1 + (i << 1)],"%02X",(uint8_t)msg[i]);
    }

    cmd_buf_temp[msg_len + 1 + (len << 1)] = '\"';    
 
    //����coap���ݷ�����������Ӧʱ�䲻��
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_CCOAPSEND, cmd_buf_temp, CMD_SET, 3000);
    
    //����coap���ݷ���״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_CoAP_SEND;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_CoAP_SEND;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);

    return NBIOT_OK;
}

//���ַ�����ʽ��������coapЭ������
int nbiot_coap_send_str(nbiot_handle_t nbiot_handle, int len, char *msg, nbiot_connect_type_t type)
{ 
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
  
    //�ж�coap id�Ƿ���ȷ
    if (nbiot_handle->p_connect_status->connect_id  < 0)
    {
        return NBIOT_ERROR;
    }
    
    if (type != NBIOT_COAP)
    {
        return NBIOT_NOTSUPPORT;
    } 
      
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (NBIOT_SEND_BUF_MAX_LEN - 24) ;


    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));   

    int16_t msg_len = snprintf(cmd_buf_temp,
                               str_len,
                               "%d,%d,%s%s%s",
                                nbiot_handle->p_connect_status->connect_id ,
                                len,
                                "\"",                                  
                                msg,
                                "\"");
    
    if (msg_len < 0) {
      
        return NBIOT_ERROR;
    }
    
                                                          
    //����coap���ݷ�����������Ӧʱ�䲻��
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_CCOAPSEND, cmd_buf_temp, CMD_SET, 3000);
    
    //����coap���ݷ���״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_CoAP_SEND;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_CoAP_SEND;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
         
    return NBIOT_OK;
}

//����cdp������
int nbiot_ncdp_update(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type)
{     
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
    
    if (type != NBIOT_NCDP)
    {
        return NBIOT_NOTSUPPORT;
    }
    
    nbiot_handle->p_connect_status->connect_type = NBIOT_NCDP;

    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));       
                
    //��������ķ���ֵΪ��Ҫ��ʽ��д��ĳ��ȣ��������ַ������������Զ���������ַ�
    uint16_t ncdp_cn_len = snprintf(cmd_buf_temp,
                                    sizeof(cmd_buf_temp) -1,"%s,%s",                                                                                             
                                    REMOTE_SERVER_IP,
                                    REMOTE_COAP_PORT);
                                                                       
    //�����Ӧʱ��Ϊ300ms�����������ﱣ��Ϊ500ms                                           
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_NCDP, cmd_buf_temp, CMD_SET, 500);
                                    
//    nbiot_handle->p_nbiot_cmd->cmd_action = ACTION_OK_AND_NEXT | ACTION_ERROR_BUT_NEXT; 
                                       
    //���봴���ͻ���״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_NCDP_SERVER;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_NCDP_SERVER;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
    
    return NBIOT_OK;
}


//�ر�ncdp
int nbiot_ncdp_close(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type)
{
  
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
           
    if (type != NBIOT_NCDP)
    {
        return NBIOT_NOTSUPPORT;
    }   
                                                                                   
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, AT_QLWSREGIND, "1", CMD_SET, 500);  

    //����ncdp�ر�״̬,�����Ӧʱ�䲻��
    nbiot_handle->p_sm_status->main_status = NBIOT_NCDP_CL;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_NCDP_CL;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
        
    return NBIOT_OK;
}

//��hex���ݸ�ʽ����ncdpЭ������,������ż��������
int nbiot_ncdp_send_hex(nbiot_handle_t nbiot_handle, int len, char *msg, nbiot_connect_type_t type, char *mode)
{ 
    
    int16_t msg_len = 0;
    
    char *p_at_cmd = NULL;
    
    if (nbiot_handle->p_sm_status->main_status != NBIOT_NONE)
    {
        return NBIOT_ERROR;
    }
  
    //�����Ͳ�ΪNCDPʱ���򶼷��͵����ݸ�����Ϊż����ʱ
    if ((type != NBIOT_NCDP) || ((strlen(msg) % 2) != 0))
    {
        return NBIOT_NOTSUPPORT;
    } 
      
    //������ݳ���Ϊ��Ч���ݼ���ͷ��
    int16_t str_len = (NBIOT_SEND_BUF_MAX_LEN - 24) ;

    //����ʹ��ջ�ϵ��ڴ�������ݣ�Ҫ��Ȼ�ط�������ջ�ϵ��ڴ������ͷŵ�ʱ�����   
    memset(cmd_buf_temp, 0, sizeof(cmd_buf_temp));
    
    
    if (mode == NULL) 
    {

        msg_len = snprintf(cmd_buf_temp,
                               str_len,
                               "%d,%s",
                               (strlen(msg) / 2),                                                  
                                msg);
        
        p_at_cmd = AT_QLWULDATA;
    }
    
    else 
    {
        msg_len = snprintf(cmd_buf_temp,
                               str_len,
                               "%d,%s,%s",
                               (strlen(msg) / 2),                                                  
                                msg,
                                mode); 

        p_at_cmd = AT_QLWULDATAEX;        
    }
    
    if (msg_len < 0) 
    {
      
        return NBIOT_ERROR;
    }
    
                                                          
    //����ncdp���ݷ�����������Ӧʱ�䲻��
    nbiot_at_cmd_param_init(nbiot_handle->p_nbiot_cmd, p_at_cmd, cmd_buf_temp, CMD_SET, 3000);
    
    //����ncdp���ݷ���״̬
    nbiot_handle->p_sm_status->main_status = NBIOT_NCDP_SEND;
    nbiot_handle->p_sm_status->sub_status  = NBIOT_SUB_NCDP_SEND;

    nbiot_at_cmd_send(nbiot_handle, nbiot_handle->p_nbiot_cmd);
         
    return NBIOT_OK;
}


