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

/*
 * AT�������������
 */
#define AT_SYNC        "AT"
#define AT_CMEE        "AT+CMEE"
#define AT_ATI         "ATI"
#define AT_CPIN        "AT+CPIN"
#define AT_CSQ         "AT+CSQ"
#define AT_CFUN        "AT+CFUN"
#define AT_CGREG       "AT+CGREG"
#define AT_CGACT       "AT+CGACT"
#define AT_CGATT       "AT+CGATT"
#define AT_COPS        "AT+COPS"
#define AT_CGCONTRDP   "AT+CGCONTRDP"

#define AT_CSCON       "AT+CSCON"
#define AT_CLAC        "AT+CLAC"

#define AT_CGPADDR     "AT+CGPADDR"
#define AT_COPS        "AT+COPS"
#define AT_CGMI        "AT+CGMI"
#define AT_CGMM        "AT+CGMM"
#define AT_CGMR        "AT+CGMR"

#define AT_CIMI        "AT+CIMI"
#define AT_CGSN        "AT+CGSN"

#define AT_CGDCONT     "AT+CGDCONT"
#define AT_CCLK        "AT+CCLK"
#define AT_CPSMS       "AT+CPSMS"
#define AT_CEDRXS      "AT+CEDRXS"
#define AT_CEER        "AT+CEER"
#define AT_CEDRXRDP    "AT+CEDRXRDP"
#define AT_CTZR        "AT+CTZR"


/* ETSI Commands* (127.005)  <Under development> */
/*
#define AT_CSMS       "AT+CSMS"
#define AT_CNMA       "AT+CNMA"
#define AT_CSCA       "AT+CSCA"
#define AT_CMGS       "AT+CMGS"
#define AT_CMGC       "AT+CMGC"
#define AT_CSODCP     "AT+CSODCP"
#define AT_CRTDCP     "AT+CRTDCP"
*/
#define AT_NMGS       "AT+NMGS"
#define AT_NMGR       "AT+NMGR"
#define AT_NNMI       "AT+NNMI"
#define AT_NSMI       "AT+NSMI"
#define AT_NQMGR      "AT+NQMGR"
#define AT_NQMGS      "AT+NQMGS"
#define AT_NMSTATUS   "AT+NMSTATUS"
#define AT_NRB        "AT+NRB"
#define AT_NCDP       "AT+NCDP"

#define AT_NUESTATS   "AT+NUESTATS"

#define AT_NEARFCN    "AT+NEARFCN"
#define AT_NSOCR      "AT+NSOCR"
#define AT_NSOST      "AT+NSOST"
#define AT_NSOSTF     "AT+NSOSTF"
#define AT_NSORF      "AT+NSORF"
#define AT_NSOCL      "AT+NSOCL"  
      
#define  AT_NPING          "AT+NPING"
#define  AT_NBAND          "AT+NBAND"
#define  AT_NLOGLEVEL      "AT+NLOGLEVEL"
#define  AT_NCONFIG        "AT+NCONFIG"
#define  AT_NATSPEED       "AT+NATSPEED"
#define  AT_NCCID          "AT+NCCID"
#define  AT_NFWUPD         "AT+NFWUPD"
#define  AT_NRDCTRL        "AT+NRDCTRL"
#define  AT_NCHIPINFO      "AT+NCHIPINFO"
#define  AT_NTSETID        "AT+NTSETID"


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


//ATָ����Ӧ������������
#define AT_CMD_RESPONSE_PAR_NUM_MAX   16


/*
 * ATָ������ö��
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
    ACTION_OK_EXIT        = 0X01,              //����ִ�гɹ����˳� 
    ACTION_OK_AND_NEXT    = 0X02,              //����ִ�гɹ���ִ����һ��ָ��      
    ACTION_ERROR_BUT_NEXT = 0X04,              //����ִ�д��������������ִ����һ��ָ��
    ACTION_ERROR_AND_TRY  = 0X08,              //����ִ�д������г���
    ACTION_ERROR_EXIT     = 0X10               //����ִ�д�����˳�    
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
    uint8_t         cmd_action;    // ATָ����Ϊ
    uint32_t        max_timeout;   // ���ʱʱ��
}at_cmd_info_t;


//����AT cmd�ṹָ������
typedef at_cmd_info_t *at_cmdhandle;


//ָ��ִ�н��
#define AT_CMD_RESULT_OK           0
#define AT_CMD_RESULT_ERROR        1
#define AT_CMD_RESULT_CONTINUE     -5
#define AT_CMD_RESULT_RANDOM_CODE  -6


//������붨�� 
#define   SIM7020_OK               0
#define   SIM7020_ERROR           -1
#define   SIM7020_ERROR_TIMEOUT   -2
#define   SIM7020_ERROR_RETRY     -3
#define   SIM7020_ERROR_NEXT      -4

//sim7020��״̬���壬app����ʹ�ø���Ϣ
typedef enum sim7020_main_status
{
    SIM7020_NONE,
    SIM7020_NBLOT_INIT,   // ��ʼ������
    SIM7020_NBLOT_INFO,   // ��ȡ NB ģ�鳧�̼��̼���Ƶ�ε���Ϣ
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
    SIM7020_CoAP_SEND,    //  ������Ϣ
    SIM7020_CoAP_RECV,    // CoAP������Ϣ
    SIM7020_RESET,        // ��λNB
    SIM7020_END
    
}sim7020_main_status_t;


//sim7020��״̬����
typedef enum sim7020_sub_status
{
    SIM7020_SUB_NONE,
    SIM7020_SUB_SYNC,
    SIM7020_SUB_CMEE,    
    SIM7020_SUB_ATI,
    SIM7020_SUB_CPIN,
    SIM7020_SUB_CSQ,
    SIM7020_SUB_CFUN,
    SIM7020_SUB_CEREG,
//    SIM7020_SUB_CGACT_DISABLE,
//    SIM7020_SUB_CGACT,
    SIM7020_SUB_CGACT_QUERY,    
    SIM7020_SUB_CGATT, 
    SIM7020_SUB_CGATT_QUERY,
    SIM7020_SUB_COPS_QUERY,
    SIM7020_SUB_CGCONTRDP_QUERY,    
    SIM7020_SUB_CEREG_QUERY,
    
    SIM7020_SUB_CIMI,
    SIM7020_SUB_CGSN,
  
    SIM7020_SUB_NSMI,
    SIM7020_SUB_NNMI,
    SIM7020_SUB_CGMI,
    SIM7020_SUB_CGMM,
    SIM7020_SUB_CGMR,
    SIM7020_SUB_NBAND,
    
    SIM7020_SUB_CSCON, 
    SIM7020_SUB_UDP_CR,
    SIM7020_SUB_UDP_CL,
    SIM7020_SUB_UDP_ST,
    SIM7020_SUB_UDP_RE,
    SIM7020_SUB_END   
    
}sim7020_sub_status_t;



//sim7020 ״̬��Ϣ
typedef struct sim7020_status_nest
{
    sim7020_main_status_t  main_status;         //�����������׶�
    int                    sub_status;          //���������ӽ׶Σ�����״̬Ƕ��  
    uint8_t                connect_status;      //���ӵ�״̬
    uint8_t                connect_type;        //���ӵ�����
    int8_t                 rssi;                //�źŵ�����    
        
    uint8_t                register_status;     //����ע��
    
}sim7020_status_nest_t;


typedef struct sim7020_socket_info {
    uint8_t                socket_type;         //ָʾsocket_type������
    uint8_t                socket_id;           //ָʾ��Ӧ��socket id
    uint16_t               data_len;            //��ʾ���ݳ���   
}sim7020_socket_info_t;

//SIM7020G�̼���Ϣ
typedef struct sim020_firmware_info
{   char         name[32];
    uint8_t      IMSI[16];
    uint8_t      IMEI[16];      
}sim020_firmware_info_t;


//�����շ����ݻ���������
#define NB_UART_RECE_BUF_MAX_LEN    (RING_BUF_LEN + 1)
#define NB_UART_SEND_BUF_MAX_LEN    (RING_BUF_LEN + 1)

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

#define    SIM7020_NONE_EVENT          0x0000           //û���¼�����
#define    SIM7020_RECV_EVENT          0x0001           //�յ���Ӧ�����¼�
#define    SIM7020_TIMEOUT_EVENT       0x0002           //�����ʱ���������ʱ�¼����ͺ�ò�����Ӧ��������ʱ�¼�
#define    SIM7020_REG_STA_EVENT       0x0004           //NBIOT���總���¼�
#define    SIM7020_TCP_RECV_EVENT      0x0008           //TCP�����¼�
#define    SIM7020_UDP_RECV_EVENT      0x0010           //UDP�����¼�
#define    SIM7020_COAP_RECV_EVENT     0X0020           //COAP�����¼�
#define    SIM7020_MQTT_RECV_EVENT     0X0040           //MQTT�����¼�
#define    SIM7020_LWM2M_RECV_EVENT    0X0080           //LWM2M�����¼�


//sim7020��Ϣid, �ص�������ʹ��
typedef enum sim7020_msg_id
{
    SIM7020_MSG_NONE,

    SIM7020_MSG_NBLOT_INIT,

    SIM7020_MSG_NBLOT_INFO,
    
    SIM7020_MSG_REG,

    SIM7020_MSG_IMSI,
    SIM7020_MSG_IMEI,       //�ƶ��豸�����    
    SIM7020_MSG_MID,        //������ID
    SIM7020_MSG_MMODEL,     //�����ͺ�
    SIM7020_MSG_MREV,       //���Ұ汾��
    SIM7020_MSG_BAND,       //����Ƶ��


    SIM7020_MSG_SIGN,       //�ź�ǿ��
    

    SIM7020_MSG_UDP_CREATE,
    SIM7020_MSG_UDP_CLOSE,
    SIM7020_MSG_UDP_SEND,
    SIM7020_MSG_UDP_RECV,

    SIM7020_MSG_COAP,
    SIM7020_MSG_COAP_SEND,
    SIM7020_MSG_COAP_RECV,

    SIM7020_MSG_END
  
}sim7020_msg_id_t;

//���崮���¼��ص�����ָ��
typedef void (*sim7020_cb)(void *p_arg, sim7020_msg_id_t, int ,char*);


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
    
    //sim7020ָ��ִ��״̬��Ϣ
    at_cmd_info_t            *p_sim702_cmd; 

    //sim7020ָ��ִ��״̬��Ϣ
    sim7020_socket_info_t    *p_socket_info;    

    //sim7020״̬��Ϣ
    sim7020_status_nest_t    *sim702_status;
  
    //sim7020�̼���Ϣ
    sim020_firmware_info_t   *firmware_info; 

    //֡��ʽ����ʱ��֡����ILDE��֡
    uint8_t                   frame_format;  
  
             
}sim7020_dev_t;

//sim7020�豸���
typedef sim7020_dev_t *sim7020_handle_t;

//����sim7020�¼�
void sim7020_event_set (sim7020_handle_t sim7020_handle, int sim7020_event);

//��ȡsim7020�¼�
int sim7020_event_get (sim7020_handle_t sim7020_handle, int sim7020_event);

//���sim7020�¼�
void sim7020_event_clr (sim7020_handle_t sim7020_handle, int sim7020_event);

//sim7020��ʼ�� 
sim7020_handle_t sim7020_init(uart_handle_t lpuart_handle);

//ע��sim7020�¼��ص�������
void sim7020_event_registercb (sim7020_handle_t sim7020_handle, sim7020_cb cb, void *p_arg);

//sim7020Ӧ��״̬������
void sim7020_app_status_poll (sim7020_handle_t sim7020_handle, int *sim702_main_status);

//sim7020�¼�������
int sim7020_event_poll (sim7020_handle_t sim7020_handle);

//sim7020 nblot��ʼ�����������ע��
int sim7020_nblot_init (sim7020_handle_t sim7020_handle);

#endif
