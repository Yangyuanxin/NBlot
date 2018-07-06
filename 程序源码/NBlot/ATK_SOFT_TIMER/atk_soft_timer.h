/*
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * All rights reserved
 */
 
#ifndef _MULTI_TIMER_H_
#define _MULTI_TIMER_H_

#include "stdint.h"

typedef struct Timer {
    uint32_t timeout;                 //��ʱ
    uint32_t repeat;                  //���¼��� 
    void (*timeout_cb)(void *p_arg);  //�ص�����
    void  *p_arg;                     //�ص���������
    struct Timer* next;
}Timer;

#ifdef __cplusplus  
extern "C" {  
#endif  

void atk_soft_timer_init(struct Timer* handle, 
                         void(*timeout_cb)(void *p_arg),                            
                         void *p_arg, 
                         uint32_t timeout, 
                         uint32_t repeat);
                         
int  atk_soft_timer_start(struct Timer* handle);
void atk_soft_timer_stop(struct Timer* handle);
void atk_soft_timer_ticks(void);
void atk_soft_timer_loop(void);

// void timer_again(struct Timer* handle);
// void timer_set_repeat(struct Timer* handle, uint32_t repeat);

#ifdef __cplusplus
} 
#endif

#endif
