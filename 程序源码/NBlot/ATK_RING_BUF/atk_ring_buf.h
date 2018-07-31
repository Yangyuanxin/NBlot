/*
 * Copyright (c) ATK
 * All rights reserved
 */
 
#ifndef _ATK_RING_BUF_H_
#define _ATK_RING_BUF_H_

#include "stdint.h"

#ifdef __cplusplus  
extern "C" {  
#endif  

//���廷�λ������������ֽ���                               
#define RING_BUF_LEN                  512         

//���λ������ṹ��
typedef struct atk_ring_buf_t
{
    uint32_t head;           
    uint32_t tail;
    uint32_t lenght;
    uint8_t  ring_buf[RING_BUF_LEN];  
    
}atk_ring_buf_t;
  

/**
 * @brief ��ʼ�����λ������������Ϣ
 */
int atk_ring_buf_init (atk_ring_buf_t *p_ring_buf);

/**
 * @brief �����λ�����д����
 */
int atk_ring_buf_write(atk_ring_buf_t *p_ring_buf, uint8_t data);


 /**
 * @brief  ��ȡ���λ�����������
 */
int atk_ring_buf_read(atk_ring_buf_t *p_ring_buf, uint8_t *data);

/**
 * @brief ���λ���������Ч���ݵĸ���
 */
int atk_ring_buf_avail_len (atk_ring_buf_t *p_ring_buf);

 /**
 * @brief  �ӽ��ջ������ȡָ�����ȵ����ݣ����ͷ�ռ�õĿռ�
 */
int atk_ring_buf_size_read(atk_ring_buf_t *p_ring_buf, uint8_t *data, int len);


 /**
 * @brief  д����ջ������ָ�����ȵ����ݣ���ռ�õĿռ�
 */
int atk_ring_buf_size_write(atk_ring_buf_t *p_ring_buf, uint8_t *data, int len);  
                         

#ifdef __cplusplus
} 
#endif

#endif //end of _ATK_RING_BUF_H_
