/*
 * Copyright (c) ATK
 * All rights reserved
 */
 
#ifndef _ATK_SOFT_TIMER_H_
#define _ATK_SOFT_TIMER_H_

#include "stdint.h"

typedef struct atk_soft_timer {
    uint32_t timeout;                 //��ʱ
    uint32_t repeat;                  //���¼��� 
    void (*timeout_cb)(void *p_arg);  //�ص�����
    void  *p_arg;                     //�ص���������
    struct atk_soft_timer *next;
}atk_soft_timer_t;

#ifdef __cplusplus  
extern "C" {  
#endif  

void atk_soft_timer_init (struct atk_soft_timer  *handle, 
                          void(*timeout_cb)(void *p_arg),                            
                          void *p_arg, 
                          uint32_t timeout, 
                          uint32_t repeat);
                         
int  atk_soft_timer_start (struct atk_soft_timer *handle);
void atk_soft_timer_stop (struct atk_soft_timer  *handle);
void atk_soft_timer_ticks (void);
void atk_soft_timer_poll (void);
                         

#ifdef __cplusplus
} 
#endif

#endif //end of _ATK_SOFT_TIMER_H_
