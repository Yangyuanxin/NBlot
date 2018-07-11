#ifndef _SIM7020_H
#define _SIM7020_H
#include "sys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nblot_usart.h"
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


#define    NB_SP_RECE_EVENT       0x0001           //�յ���������
#define    NB_TIMEOUT_EVENT       0x0002           //��ʱ�¼�
#define    NB_UDPRECE_EVENT       0x0004           //UDP�����¼�
#define    NB_COAP_RE_EVENT       0X0008           //COAP�����¼�
#define    NB_REG_STA_EVENT       0x0010           //NB IOT���總���¼�

/*
 * AT�������������
 */
#define AT_SYNC        "AT"
#define AT_ATI         "ATI"
#define AT_CGMI        "AT+CGMI"
#define AT_CGMM        "AT+CGMM"
#define AT_CGMR        "AT+CGMR"
#define AT_CGSN        "AT+CGSN"
#define AT_CEREG       "AT+CEREG"
#define AT_CSCON       "AT+CSCON"
#define AT_CLAC        "AT+CLAC"
#define AT_CSQ         "AT+CSQ"
#define AT_CGPADDR     "AT+CGPADDR"
#define AT_COPS        "AT+COPS"
#define AT_CGATT       "AT+CGATT"
#define AT_CGACT       "AT+CGACT"
#define AT_CIMI        "AT+CIMI"
#define AT_CGDCONT     "AT+CGDCONT"
#define AT_CFUN        "AT+CFUN"
#define AT_CMEE        "AT+CMEE"
#define AT_CCLK        "AT+CCLK";
#define AT_CPSMS       "AT+CPSMS";
#define AT_CEDRXS      "AT+CEDRXS";
#define AT_CEER        "AT+CEER";
#define AT_CEDRXRDP    "AT+CEDRXRDP";
#define AT_CTZR        "AT+CTZR";

/* ETSI Commands* (127.005)  <Under development> */
/*
#define AT_CSMS       "AT+CSMS";
#define AT_CNMA       "AT+CNMA";
#define AT_CSCA       "AT+CSCA";
#define AT_CMGS       "AT+CMGS";
#define AT_CMGC       "AT+CMGC";
#define AT_CSODCP     "AT+CSODCP";
#define AT_CRTDCP     "AT+CRTDCP";
*/
#define AT_NMGS       "AT+NMGS";
#define AT_NMGR       "AT+NMGR";
#define AT_NNMI       "AT+NNMI";
#define AT_NSMI       "AT+NSMI";
#define AT_NQMGR      "AT+NQMGR";
#define AT_NQMGS      "AT+NQMGS";
#define AT_NMSTATUS   "AT+NMSTATUS";
#define AT_NRB        "AT+NRB";
#define AT_NCDP       "AT+NCDP";

#define AT_NUESTATS   "AT+NUESTATS";

#define AT_NEARFCN    "AT+NEARFCN";
#define AT_NSOCR      "AT+NSOCR";
#define AT_NSOST      "AT+NSOST";
#define AT_NSOSTF     "AT+NSOSTF";
#define AT_NSORF      "AT+NSORF";
#define AT_NSOCL      "AT+NSOCL";  
      
#define  AT_NPING          "AT+NPING";
#define  AT_NBAND          "AT+NBAND";
#define  AT_NLOGLEVEL      "AT+NLOGLEVEL";
#define  AT_NCONFIG        "AT+NCONFIG";
#define  AT_NATSPEED       "AT+NATSPEED";
#define  AT_NCCID          "AT+NCCID";
#define  AT_NFWUPD         "AT+NFWUPD";
#define  AT_NRDCTRL        "AT+NRDCTRL";
#define  AT_NCHIPINFO      "AT+NCHIPINFO";
#define  AT_NTSETID        "AT+NTSETID";

//                                                                              
//

#define CMD_TRY_TIMES           10
#define CMD_READ_ARGUMENT       "?"
#define CMD_TEST_ARGUMENT       "=?"

#define CMD_OK_RES              "OK"

#define REMOTE_SERVER_IP        "115.29.240.46"
#define REMOTE_SERVER_PORT      "6000"


#define REMOTE_COAP_INFO        "115.29.240.46,5683"

#define LOCAL_UDP_SET           "DGRAM,17,10000,1"

#define BAND_850MHZ_ID           5
#define BAND_850MHZ_STR          "850"

#define BAND_900MHZ_ID           8
#define BAND_900MHZ_STR          "900"

#define BAND_800MHZ_ID           20
#define BAND_800MHZ_STR          "800"

#define BAND_700MHZ_ID           28
#define BAND_700MHZ_STR          "700"


/*
 * cmd ����ö��
 */
typedef enum
{
   CMD_TEST,         //����TEST����
   CMD_READ,         //����READ����
   CMD_SET,          //����SET����
   CMD_EXCUTE        //����EXCUTE����
}cmd_property_t;

/*
 * ATָ�����Ϊö��
 */
typedef enum
{
  ACTION_OK,                      //����ɹ�ִ��
  ACTION_ERROR,                   //����ִ�д���
    
  ACTION_ERROR_NEXT,              //����ִ�д��󽫼���ִ����һ��ָ��
  ACTION_ERROR_TRY                //����ִ�д������г���
}cmd_action_t;

//ATָ��ṹ����
typedef struct at_cmd_info
{
    const char*     p_atcmd;       // ATָ��
    cmd_property_t  property;      // ָ�ǰ����(TEST,READ,SET,EXCUTE)
    char*           p_atcmd_arg;   // ָ�����
    char*           p_expectres;   // �����õ��ظ�
    unsigned char   cmd_try;       // �����Դ���
    unsigned char   have_tried;    // �Ѿ����γ��ԵĴ���
    cmd_action_t    cmd_action;    // ATָ����Ϊ
    uint32_t        max_timeout;   // ���ʱʱ��
}at_cmd_info_t;


//����AT cmd�ṹָ������
typedef at_cmd_info_t *at_cmdhandle;

#define    SIM7020_NONE_EVENT          0x0000           //û�������¼�����
#define    SIM7020_RECV_EVENT          0x0001           //�յ����������¼�
#define    SIM7020_TIMEOUT_EVENT       0x0002           //��ʱ�¼�
#define    SIM7020_REG_STA_EVENT       0x0004           //NBIOT���總���¼�
#define    SIM7020_TCP_RECV_EVENT      0x0008           //TCP�����¼�
#define    SIM7020_UDP_RECV_EVENT      0x0010           //UDP�����¼�
#define    SIM7020_COAP_RECV_EVENT     0X0020           //COAP�����¼�


//sim7020 ��״̬���壬app����ʹ�ø���Ϣ
typedef enum sim7020_main_status
{
    SIM7020_NONE,
    SIM7020_INIT,         // ��ʼ������
    SIM7020_MODULE_INFO,  // ��ȡ NB ģ�鳧�̼��̼���Ƶ�ε���Ϣ
    SIM7020_SIGN,         // ��ȡ�ź�����
    SIM7020_UDP_CR,       // ���� UDP
    SIM7020_UDP_CL,       // �ر� UDP
    SIM7020_TCP_CR,       // ���� TCP
    SIM7020_TCP_CL,       // �ر� TCP
    SIM7020_UDP_SEND,     // �����Ѿ�������UDP��������
    SIM7020_UDP_RECV,     // UDP������Ϣ
    SIM7020_TCP_SEND,     // �����Ѿ�������TCP��������
    SIM7020_TCP_RECV,     // TCP������Ϣ    
    SIM7020_CoAP_SEVER,   // CoAPԶ�̵�ַ�������ȡ
    SIM7020_CoAP_SEND,    // ����CoAP������Ϣ
    SIM7020_CoAP_RECV,    // CoAP������Ϣ
    SIM7020_RESET,        // ��λNB
    SIM7020_END
    
}sim7020_main_status_t;


//sim7020 ״̬��Ϣ
typedef struct sim7020_status_nest
{
    sim7020_main_status_t  main_status;         //���׶�
    int                    sub_status;          //�ӽ׶Σ�����״̬Ƕ��    
    uint8_t                connect_status;
    uint8_t                register_status;  
    uint8_t                socket_id;           //ָʾ��Ӧ��socket id
    uint16_t               data_len;            //��ʾ���ݳ���     
}sim7020_status_nest_t;

//SIM7020G�̼���Ϣ
typedef struct sim020_firmware_info
{
    uint8_t      IMSI[16];
    uint8_t      IMEI[16];      
}sim020_firmware_info_t;


//�����շ����ݻ���������
#define NB_UART_RECE_BUF_MAX_LEN    512
#define NB_UART_SEND_BUF_MAX_LEN    512

//���ջ���ռ�
typedef struct sim7020_recv
{
    char      buf[NB_UART_RECE_BUF_MAX_LEN];    //�������ݻ�����
    uint16_t  len;                              //��Ч���ݳ���
}sim7020_recv_t;

//���ջ���ռ�
typedef struct sim7020_send
{
    char      buf[NB_UART_SEND_BUF_MAX_LEN];    //�������ݻ�����
    uint16_t  len;                              //��Ч���ݳ���
}sim7020_send_t;


//sim7020���������ṹ��
struct sim7020_drv_funcs {
    
    //sim7020��������
    int (*sim7020_send_data) (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout);
    //sim7020��������
    int (*sim7020_recv_data) (void *p_arg, uint8_t *pData, uint16_t size, uint32_t Timeout);    
};

//���崮���¼��ص�����ָ��
typedef void (*sim7020_cb)(void *p_arg);

//sim7020�豸�ṹ��
typedef struct sim7020_dev
{     
    struct sim7020_drv_funcs *p_drv_funcs;
    
    uart_dev_t               *p_uart_dev;
    
    //sim7020�豸�¼��ص�����
    sim7020_cb                sim7020_cb;  
    
    void                     *p_arg;

    //�¼����
    int                       sim7020_event; 

    //sim7020״̬��Ϣ
    sim7020_status_nest_t    *sim702_status;
    //sim7020�̼���Ϣ
    sim020_firmware_info_t   *firmware_info;      
             
}sim7020_dev_t;

//sim7020�豸���
typedef sim7020_dev_t *sim7020_handle_t;


//����sim7020�¼�
void sim7020_event_set (int sim7020_event);

//��ȡsim7020�¼�
int sim7020_event_get (int sim7020_event);

//���sim7020�¼�
void sim7020_event_clr (int sim7020_event);


//sim7020��ʼ�� 
sim7020_handle_t sim7020_init(uart_handle_t lpuart_handle);

//ע��sim7020�¼��ص�������
void sim7020_event_registercb(sim7020_cb cb, void *p_arg);

//sim7020��Ϣ�¼�������
void sim7020_app_status_poll(int *sim702_main_status);

#endif
