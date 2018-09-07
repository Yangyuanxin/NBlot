#ifndef _NBIOT_H
#define _NBIOT_H

#include "atk_bc28.h"
#include "atk_delay.h"

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

     
     
//nbiot app״̬����
typedef enum nbiot_app_status
{
    NBIOT_APP_NONE,        
    NBIOT_APP_INIT        = NBIOT_INIT,         // ��ʼ������
    NBIOT_APP_INFO        = NBIOT_INFO,         // ��ȡ NB ģ�鳧�̼��̼���Ƶ�ε���Ϣ
    NBIOT_APP_SIGNAL      = NBIOT_SIGNAL,       // ��ȡ�ź�����
    NBIOT_APP_TCPUDP_CR   = NBIOT_TCPUDP_CR,    // ���� TCP/UDP
    NBIOT_APP_TCPUDP_CL   = NBIOT_TCPUDP_CL,    // �ر� TCP/UDP
    NBIOT_APP_TCPUDP_SEND = NBIOT_TCPUDP_SEND,  // �����Ѿ�������TCP/UDP��������
    NBIOT_APP_TCPUDP_RECV = NBIOT_TCPUDP_RECV,  // TCP/UDP������Ϣ 
    NBIOT_APP_SOCKET_ERR  = NBIOT_SOCKET_ERR,   // SOCKET���ӷ�������
    NBIOT_APP_CoAP_SEVER  = NBIOT_CoAP_SEVER,   // CoAPԶ�̷���������
    NBIOT_APP_CoAP_CLIENT = NBIOT_CoAP_CLIENT,  // CoAP�ͻ������ӵ�ַ����
    NBIOT_APP_CoAP_SEND   = NBIOT_CoAP_SEND,    // CoAP������Ϣ
    NBIOT_APP_CoAP_RECV   = NBIOT_CoAP_RECV,    // CoAP������Ϣ
    NBIOT_APP_CoAP_CL     = NBIOT_CoAP_CL,      // �ر�CoAP
    NBIOT_APP_CM2M_CLIENT = NBIOT_CM2M_CLIENT,  // CM2M�ͻ������ӵ�ַ����
    NBIOT_APP_CM2M_SEND   = NBIOT_CM2M_SEND,    // CM2M������Ϣ
    NBIOT_APP_CM2M_RECV   = NBIOT_CM2M_RECV,    // CM2M������Ϣ
    NBIOT_APP_CM2M_STATUS = NBIOT_CM2M_STATUS,  // CM2M����״̬��Ϣ
    NBIOT_APP_CM2M_CL     = NBIOT_CM2M_CL,      // �ر�CM2M 
    NBIOT_APP_RESET       = NBIOT_RESET,        // ��λNB
    NBIOT_APP_NCONFIG     = NBIOT_NCONFIG,      // ʹ��/�����Զ���������
    NBIOT_APP_END          
}nbiot_app_status_t;


//nbiot��Ϣid, �ص�������ʹ��
typedef enum nbiot_msg_id
{
    NBIOT_MSG_NONE,

    NBIOT_MSG_INIT,       //��ʼ�������Ϣ

    NBIOT_MSG_INFO,       //�̼���Ϣ��ȡ�����Ϣ
    
    NBIOT_MSG_REG,        //ע����Ϣ��ȡ�����Ϣ      
  
    NBIOT_MSG_IMEI,       //�ƶ��豸������ȡ�����Ϣ   
    NBIOT_MSG_IMSI,
  
    NBIOT_MSG_MID,        //����ID��ȡ�����Ϣ
    NBIOT_MSG_MMODEL,     //ģ���ͺŻ�ȡ�����Ϣ
    NBIOT_MSG_MREV,       //����汾�Ż�ȡ�����Ϣ
    NBIOT_MSG_BAND,       //����Ƶ�λ�ȡ�����Ϣ
  
    NBIOT_MSG_CSQ,        
    NBIOT_MSG_SIGNAL,     //�ź�ǿ�Ȼ�ȡ�����Ϣ
    

    NBIOT_MSG_TCPUDP_CREATE,
    NBIOT_MSG_TCPUDP_CLOSE,
    NBIOT_MSG_TCPUDP_SEND,
    NBIOT_MSG_TCPUDP_RECV,
    
    NBIOT_MSG_SOCKET_ERROR, //�յ�socket������Ϣ    

    NBIOT_MSG_COAP_SERVER,
    NBIOT_MSG_COAP_CLIENT,
    NBIOT_MSG_COAP_SEND,
    NBIOT_MSG_COAP_RECV,
    NBIOT_MSG_COAP_CLOSE,
    
    NBIOT_MSG_CM2M_CLIENT,
    NBIOT_MSG_CM2M_SEND,
    NBIOT_MSG_CM2M_RECV,
    NBIOT_MSG_CM2M_STATUS,
    NBIOT_MSG_CM2M_CLOSE,

    NBIOT_MSG_RESET,        //NB��λ�����Ϣ
    NBIOT_MSG_NCONFIG,      //�Զ��������������Ϣ

    NBIOT_MSG_CMD_RETRY,    //�������������Ϣ
    
    NBIOT_MSG_CMD_NEXT,     //��������������������ִ����һ��������Ϣ
    
    NBIOT_MSG_CMD_FAIL,     //���������˳�ִ����Ϣ 

    NBIOT_MSG_END           
  
}nbiot_msg_id_t;

//NBģ���ʼ�����������ע��
int nbiot_init (nbiot_handle_t nbiot_handle);

//��ȡNBģ�����Ϣ
int nbiot_info_get(nbiot_handle_t nbiot_handle);

//��ȡNBģ����ź�����
int nbiot_signal_get(nbiot_handle_t nbiot_handle);


//����tcpudp 
int nbiot_tcpudp_create(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type);


//�ر�tcpudp
int nbiot_tcpudp_close(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type);


//��hex���ݸ�ʽ��������
int nbiot_tcpudp_send_hex(nbiot_handle_t nbiot_handle, 
                                  int len, 
                                  char *msg, 
                                  nbiot_connect_type_t type);

//���ַ�����ʽ��������
int nbiot_tcpudp_send_str(nbiot_handle_t nbiot_handle, 
                                  int len, 
                                  char *msg, 
                                  nbiot_connect_type_t type);
                                  
//����coap������ 
int nbiot_coap_server_create(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type);

//����coap�ͻ���
int nbiot_coap_client_create(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type);


//�ر�coap
int nbiot_coap_close(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type);


//��hex���ݸ�ʽ����coapЭ������,������ż��������
int nbiot_coap_send_hex(nbiot_handle_t nbiot_handle, int len, char *msg, nbiot_connect_type_t type);

//���ַ�����ʽ��������coapЭ������
int nbiot_coap_send_str(nbiot_handle_t nbiot_handle, int len, char *msg, nbiot_connect_type_t type);                                                                  

//����cm2m�ͻ���
int nbiot_cm2m_client_create(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type);


//�ر�cm2m
int nbiot_cm2m_close(nbiot_handle_t nbiot_handle, nbiot_connect_type_t type);

//��hex���ݸ�ʽ����cm2mЭ������,������ż��������
int nbiot_cm2m_send_hex(nbiot_handle_t nbiot_handle, int len, char *msg, nbiot_connect_type_t type); 



#endif


