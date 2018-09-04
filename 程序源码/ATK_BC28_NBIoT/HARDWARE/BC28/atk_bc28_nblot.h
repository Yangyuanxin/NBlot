#ifndef _SIM7020_NBIOT_H
#define _SIM7020_NBIOT_H

#include "sim7020.h"
#include "delay.h"

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




//sim7020 app״̬����
typedef enum sim7020_app_status
{
    SIM7020_APP_NONE,        
    SIM7020_APP_NBLOT_INIT  = SIM7020_NBLOT_INIT,   // ��ʼ������
    SIM7020_APP_NBLOT_INFO  = SIM7020_NBLOT_INFO,   // ��ȡ NB ģ�鳧�̼��̼���Ƶ�ε���Ϣ
    SIM7020_APP_SIGNAL      = SIM7020_SIGNAL,       // ��ȡ�ź�����
    SIM7020_APP_TCPUDP_CR   = SIM7020_TCPUDP_CR,    // ���� TCP/UDP
    SIM7020_APP_TCPUDP_CL   = SIM7020_TCPUDP_CL,    // �ر� TCP/UDP
    SIM7020_APP_TCPUDP_SEND = SIM7020_TCPUDP_SEND,  // �����Ѿ�������TCP/UDP��������
    SIM7020_APP_TCPUDP_RECV = SIM7020_TCPUDP_RECV,  // TCP/UDP������Ϣ 
    SIM7020_APP_SOCKET_ERR  = SIM7020_SOCKET_ERR,   // SOCKET���ӷ�������
    SIM7020_APP_CoAP_SEVER  = SIM7020_CoAP_SEVER,   // CoAPԶ�̷���������
    SIM7020_APP_CoAP_CLIENT = SIM7020_CoAP_CLIENT,  // CoAP�ͻ������ӵ�ַ����
    SIM7020_APP_CoAP_SEND   = SIM7020_CoAP_SEND,    // CoAP������Ϣ
    SIM7020_APP_CoAP_RECV   = SIM7020_CoAP_RECV,    // CoAP������Ϣ
    SIM7020_APP_CoAP_CL     = SIM7020_CoAP_CL,      // �ر�CoAP
    SIM7020_APP_CM2M_CLIENT = SIM7020_CM2M_CLIENT,  // CM2M�ͻ������ӵ�ַ����
    SIM7020_APP_CM2M_SEND   = SIM7020_CM2M_SEND,    // CM2M������Ϣ
    SIM7020_APP_CM2M_RECV   = SIM7020_CM2M_RECV,    // CM2M������Ϣ
    SIM7020_APP_CM2M_STATUS = SIM7020_CM2M_STATUS,  // CM2M����״̬��Ϣ
    SIM7020_APP_CM2M_CL     = SIM7020_CM2M_CL,      // �ر�CM2M 
    SIM7020_APP_RESET       = SIM7020_RESET,        // ��λNB
    SIM7020_APP_END          
}sim7020_app_status_t;


//sim7020��Ϣid, �ص�������ʹ��
typedef enum sim7020_msg_id
{
    SIM7020_MSG_NONE,

    SIM7020_MSG_NBLOT_INIT,

    SIM7020_MSG_NBLOT_INFO,
    
    SIM7020_MSG_REG,
  
    SIM7020_MSG_IMEI,       //�ƶ��豸�����   
    SIM7020_MSG_IMSI,
  
    SIM7020_MSG_MID,        //����ID
    SIM7020_MSG_MMODEL,     //ģ���ͺ�
    SIM7020_MSG_MREV,       //����汾��
    SIM7020_MSG_BAND,       //����Ƶ��
  
    SIM7020_MSG_CSQ,        
    SIM7020_MSG_SIGNAL,     //�ź�ǿ��
    

    SIM7020_MSG_TCPUDP_CREATE,
    SIM7020_MSG_TCPUDP_CLOSE,
    SIM7020_MSG_TCPUDP_SEND,
    SIM7020_MSG_TCPUDP_RECV,
    
    SIM7020_MSG_SOCKET_ERROR, //socket����    

    SIM7020_MSG_COAP_SERVER,
    SIM7020_MSG_COAP_CLIENT,
    SIM7020_MSG_COAP_SEND,
    SIM7020_MSG_COAP_RECV,
    SIM7020_MSG_COAP_CLOSE,
    
    SIM7020_MSG_CM2M_CLIENT,
    SIM7020_MSG_CM2M_SEND,
    SIM7020_MSG_CM2M_RECV,
    SIM7020_MSG_CM2M_STATUS,
    SIM7020_MSG_CM2M_CLOSE,   


    SIM7020_MSG_CMD_RETRY,
    
    SIM7020_MSG_CMD_NEXT,
    
    SIM7020_MSG_CMD_FAIL,    

    SIM7020_MSG_END
  
}sim7020_msg_id_t;


//sim7020 nblot��ʼ�����������ע��
int sim7020_nblot_init (sim7020_handle_t sim7020_handle);

//��ȡNBģ�����Ϣ
int sim7020_nblot_info_get(sim7020_handle_t sim7020_handle);

//��ȡNBģ����ź�����
int sim7020_nblot_signal_get(sim7020_handle_t sim7020_handle);


//����tcpudp 
int sim7020_nblot_tcpudp_create(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type);


//�ر�tcpudp
int sim7020_nblot_tcpudp_close(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type);


//��hex���ݸ�ʽ��������
int sim7020_nblot_tcpudp_send_hex(sim7020_handle_t sim7020_handle, 
                                  int len, 
                                  char *msg, 
                                  sim7020_connect_type_t type);

//���ַ�����ʽ��������
int sim7020_nblot_tcpudp_send_str(sim7020_handle_t sim7020_handle, 
                                  int len, 
                                  char *msg, 
                                  sim7020_connect_type_t type);
                                  
//����coap������ 
int sim7020_nblot_coap_server_create(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type);

//����coap�ͻ���
int sim7020_nblot_coap_client_create(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type);


//�ر�coap
int sim7020_nblot_coap_close(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type);


//��hex���ݸ�ʽ����coapЭ������,������ż��������
int sim7020_nblot_coap_send_hex(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type);

//���ַ�����ʽ��������coapЭ������
int sim7020_nblot_coap_send_str(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type);                                                                  

//����cm2m�ͻ���
int sim7020_nblot_cm2m_client_create(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type);


//�ر�cm2m
int sim7020_nblot_cm2m_close(sim7020_handle_t sim7020_handle, sim7020_connect_type_t type);

//��hex���ݸ�ʽ����cm2mЭ������,������ż��������
int sim7020_nblot_cm2m_send_hex(sim7020_handle_t sim7020_handle, int len, char *msg, sim7020_connect_type_t type); 



#endif


