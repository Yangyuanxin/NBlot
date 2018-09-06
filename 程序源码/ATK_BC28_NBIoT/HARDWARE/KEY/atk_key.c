#include "atk_key.h"
#include "atk_delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//KEY��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/11/27
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

static key_dev_t key_dev = {0,0, NULL, NULL};


//������ʼ������
//mode: 0 ��ѯģʽ�� 1���ж�ģʽ
key_handle_t key_init(int mode)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOC_CLK_ENABLE();                                  //����GPIOCʱ�� 
    
    GPIO_Initure.Pin =GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3; //PC0,1,2,3
    
    if (mode == 1) {
        GPIO_Initure.Mode=GPIO_MODE_IT_FALLING;    //����, �½��ش��� 
    } else {
        GPIO_Initure.Mode=GPIO_MODE_INPUT;        //����, �½��ش���
    }
    
    GPIO_Initure.Pull=GPIO_PULLUP;             //����
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;        //����
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);
    
    if (mode == 1) {
        
        //�ж���0-PC0
        HAL_NVIC_SetPriority(EXTI0_IRQn,12,0);       //��ռ���ȼ�Ϊ12�������ȼ�Ϊ0
        HAL_NVIC_EnableIRQ(EXTI0_IRQn);             //ʹ���ж���0
        
        //�ж���1-PC1
        HAL_NVIC_SetPriority(EXTI1_IRQn,12,0);       //��ռ���ȼ�Ϊ12�������ȼ�Ϊ0
        HAL_NVIC_EnableIRQ(EXTI1_IRQn);           
        
        //�ж���2-PC2
        HAL_NVIC_SetPriority(EXTI2_IRQn,12,0);       //��ռ���ȼ�Ϊ12�������ȼ�Ϊ0
        HAL_NVIC_EnableIRQ(EXTI2_IRQn);              //ʹ���ж���2
           
        //�ж���3-PC3
        HAL_NVIC_SetPriority(EXTI3_IRQn,12,0);       //��ռ���ȼ�Ϊ12�������ȼ�Ϊ0
        HAL_NVIC_EnableIRQ(EXTI3_IRQn);              //ʹ���ж���13          
    }
		
    return &key_dev;
}

//����������,���ж�����µ���
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//0��û���κΰ�������
//1��WKUP���� WK_UP
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY2>WK_UP!!
u8 key_scan(u8 mode)
{
    static u8 key_up=1;     //�����ɿ���־
    if(mode==1)key_up=1;    //֧������
    if(key_up&&(KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
    {
        delay_ms(10);
        key_up=0;
        if(KEY0==0)       return KEY0_PRES;
        else if(KEY1==0)  return KEY1_PRES;
        else if(KEY2==0)  return KEY2_PRES;
        else if(WK_UP==0) return WKUP_PRES;          
    }else if(KEY0==1&&KEY1==1&&KEY2==1&&WK_UP==1)key_up=1;
    return 0;   //�ް�������
}


//�жϷ�����
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);//�����жϴ����ú���
}

void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);//�����жϴ����ú���
}

void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);//�����жϴ����ú���
}

void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);//�����жϴ����ú���
}

//�жϷ����������Ҫ��������
//��HAL�������е��ⲿ�жϷ�����������ô˺���
//GPIO_Pin:�ж����ź�
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch(GPIO_Pin)
    {
        case GPIO_PIN_0:
            if(WK_UP==0) 
            {
                key_dev.key_event  = WKUP_PRES;
                key_dev.start_tick = HAL_GetTick();
            }
            break;
            
        case GPIO_PIN_1:
            if (KEY2==0)  //
            {
                key_dev.key_event  = KEY2_PRES;
                key_dev.start_tick = HAL_GetTick();           
                
            }
            break;
            
        case GPIO_PIN_2:
            if(KEY1==0)  
            {
                key_dev.key_event  = KEY1_PRES;
                key_dev.start_tick = HAL_GetTick();
                
            }
            break;
            
        case GPIO_PIN_3:
            
            if(KEY0==0)  //
            {
                
                key_dev.key_event  = KEY0_PRES;
                key_dev.start_tick = HAL_GetTick();
            }
            break;
    }
}


//ע�ᰴ���¼��ص�
void atk_key_registercb (key_handle_t key_handle, key_cb cb, void *p_arg)
{
    if ((cb != NULL) && (key_handle != NULL))
    {
        key_handle->key_cb = cb;
        key_handle->p_arg  = p_arg;
    }
}



//��ѯ��ť�¼�
void atk_key_event_poll(key_handle_t key_handle)
{
  uint8_t key_event = 0;
    
  if(key_dev.key_event)
  { 
    if(HAL_GetTick() - key_dev.start_tick >= KEY_DELAY_TICK )
    {
      if(key_dev.key_event & WKUP_PRES)
      {
        if(WK_UP == GPIO_PIN_RESET)
        {
          key_event |= WKUP_PRES;
        }

        key_dev.key_event ^= WKUP_PRES;
      }
      
      if(key_dev.key_event & KEY2_PRES)
      {
        if(KEY2 == GPIO_PIN_RESET)
        {
          key_event |= KEY2_PRES;
        }
        key_dev.key_event ^= KEY2_PRES;
      }
      
      if(key_dev.key_event & KEY1_PRES)
      {
        if(KEY1 == GPIO_PIN_RESET)
        {
          key_event |= KEY1_PRES;
        }
        key_dev.key_event ^= KEY1_PRES;
      }
      
      if(key_dev.key_event & KEY0_PRES)
      {
        if(KEY0 == GPIO_PIN_RESET)
        {
          key_event |= KEY0_PRES;
        }
        key_dev.key_event ^= KEY0_PRES;
      }
    }
  }
  
  //�������а�ť���£���ִ�лص�����
  if(key_event && key_handle->key_cb)
  {
    key_handle->key_cb(key_event, key_handle->p_arg);
  }
}
