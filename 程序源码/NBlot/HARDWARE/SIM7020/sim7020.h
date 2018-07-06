#ifndef _SIM7020_H
#define _SIM7020_H
#include "sys.h"
#include "stdio.h"	
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.csom
//�޸�����:2015/6/23
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************
//V1.0�޸�˵�� 
////////////////////////////////////////////////////////////////////////////////// 	


typedef enum sim7020state
{
    SIM7020NONE,
    SIM7020INIT,       // ��ʼ������
    SIM7020MODULE,     // ��ȡ NB ģ�鳧�̼��̼���Ƶ�ε���Ϣ
    SIM7020SIGN,
    SIM7020UDP_CR,     // ���� UDP
    SIM7020UDP_CL,     // �ر� UDP
    SIM7020TCP_CR,     // ���� TCP
    SIM7020TCP_CL,     // �ر� TCP
    SIM7020UDP_ST,     // �����Ѿ������� UDP ��������
    SIM7020UDP_RE,     // UDP ������Ϣ
    SIM7020CoAP_SEVER, // CoAP Զ�̵�ַ�������ȡ
    SIM7020CoAP_ST,    // ���� CoAP ������Ϣ
    SIM7020CoAP_RE,    // CoAP ������Ϣ
    SIM7020RESET,      // ��λ NB
    SIM7020END
    
}sim7020state_t;


#endif
