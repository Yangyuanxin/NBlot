#include "nblot_usart.h"
#include "delay.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////////////////     
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F429������
//����1��ʼ��           
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2015/9/7
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************

//UART�豸�ṹ��
static uart_dev_t uart_dev;


//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���       
atk_ring_buf_t g_uart_ring_buff;  //����һ��uart ringbuff �Ļ����� 


UART_HandleTypeDef hlpuart1;


//ע�ᴮ���¼��ص�����
void lpuart_event_registercb(uart_cb cb, void *p_arg)
{  
    if(cb != 0)
    {
        uart_dev.uart_cb  = cb;
        uart_dev.p_arg    = p_arg;
    }
}

//���ô����¼�
void lpuart_event_set (int uart_event)
{ 
    uart_dev.uart_event |= uart_event;   
}

//��ȡ�����¼�
int lpuart_event_get (int uart_event)
{ 
    return (uart_dev.uart_event & uart_event); 
}

//��������¼�
void lpuart_event_clr (int uart_event)
{ 
    uart_dev.uart_event ^= uart_event;
}

 /**
  * @brief ��ʼ�����λ������������Ϣ
  */
void atk_ring_buff_init (atk_ring_buf_t *p_ring_buff)
{
    p_ring_buff->head   = 0;
    p_ring_buff->tail   = 0;
    p_ring_buff->lenght = 0;
}

 /**
  * @brief �����λ�����д����
  */
u8 atk_ring_buff_write(atk_ring_buf_t *p_ring_buff, uint8_t data)
{
   if(p_ring_buff->lenght >= RING_BUFF_LEN)          //�жϻ������Ƿ�����
    {
        return -1;
    }
    
    //�����ٽ�������
    INTX_DISABLE();
    
    p_ring_buff->ring_buff[p_ring_buff->tail] = data;

    p_ring_buff->tail = (p_ring_buff->tail+1) % RING_BUFF_LEN;//��ֹԽ��Ƿ�����
    p_ring_buff->lenght++;

    INTX_ENABLE();     
    
    return 0;
}

 /**
 * @brief  ��ȡ���λ�����������
 */
u8 atk_read_ringbuff(atk_ring_buf_t *p_ring_buff, uint8_t *data)
{
   if (p_ring_buff->lenght == 0)    //�жϷǿ�
   {
       return -1;
   }
   
   //�����ٽ�������
   INTX_DISABLE();   
   
   *data = p_ring_buff->ring_buff[p_ring_buff->head];        //�Ƚ��ȳ�FIFO���ӻ�����ͷ��
   p_ring_buff->head = (p_ring_buff->head+1) % RING_BUFF_LEN; //��ֹԽ��Ƿ�����
   p_ring_buff->lenght--;
   
   INTX_ENABLE(); 
   
   return TRUE;
}



//UART�ײ��ʼ����ʱ��ʹ�ܣ��������ã��ж�����
//�˺����ᱻHAL_UART_Init()����
//huart:���ھ��
void HAL_LPUART1_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_Initure;
    
    if(huart->Instance==LPUART1)//����Ǵ���1�����д���1 MSP��ʼ��
    {
        __HAL_RCC_LPUART1_CLK_ENABLE();
        
        /* GPIO Ports Clock Enable */
        __HAL_RCC_GPIOB_CLK_ENABLE();
       
        /**LPUART1 GPIO Configuration    
        PB10     ------> LPUART1_RX
        PB11     ------> LPUART1_TX 
        */
        GPIO_Initure.Pin = GPIO_PIN_10|GPIO_PIN_11;
        GPIO_Initure.Mode = GPIO_MODE_AF_PP;
        GPIO_Initure.Pull = GPIO_NOPULL;
        GPIO_Initure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_Initure.Alternate = GPIO_AF8_LPUART1;
        HAL_GPIO_Init(GPIOB, &GPIO_Initure);                 
    }

}


//��ʼ��IO LPUART1
//bound:������
uart_dev_t *lpuart1_init (u32 bound)
{   
    hlpuart1.Instance = LPUART1;
    hlpuart1.Init.BaudRate = bound;
    hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
    hlpuart1.Init.StopBits = UART_STOPBITS_1;
    hlpuart1.Init.Parity = UART_PARITY_NONE;
    hlpuart1.Init.Mode = UART_MODE_TX_RX;
    hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    HAL_LPUART1_MspInit(&hlpuart1); 
    
       
    if (HAL_UART_Init(&hlpuart1) != HAL_OK)
    {
       _Error_Handler(__FILE__, __LINE__);
    }
   
    // �����������豸�ṹ��    
    uart_dev.p_huart    = &hlpuart1; 
    uart_dev.uart_event = UART_NONE_EVENT;   
    uart_dev.bound      = bound; 

    __HAL_UART_DISABLE_IT(&hlpuart1, UART_IT_TXE);  //��ֹ�����ж� 
    __HAL_UART_DISABLE_IT(&hlpuart1,UART_IT_TC);    //���ܷ�������ж�       
  
#if EN_LPUART1_RX
    __HAL_UART_ENABLE_IT(&hlpuart1,UART_IT_RXNE);    //���������ж�
    
    HAL_NVIC_EnableIRQ(LPUART1_IRQn);                //ʹ��USART1�ж�ͨ��
    HAL_NVIC_SetPriority(LPUART1_IRQn,3,3);          //��ռ���ȼ�3�������ȼ�3
#endif 

    return  &uart_dev;   
}


//��ѯ���ʹ�������
int uart_data_tx_poll(UART_HandleTypeDef *huart, uint8_t *pData,uint16_t size, uint32_t Timeout)
{   
    int ret = 0;
    
    int tx_int_flag = 0;
    
    if(size == 0 || pData == NULL)
    {
       return -1;
    }
    
    tx_int_flag = __HAL_UART_GET_IT_SOURCE(&hlpuart1, UART_IT_TXE);
    
    
    if (tx_int_flag) {
    
        __HAL_UART_DISABLE_IT(&hlpuart1, UART_IT_TXE);    //���ܷ����ж�  
    }    
        
    /* �������ó�ʱΪ Timeout ms */ 
    ret = HAL_UART_Transmit(huart, pData, size, Timeout);
    
    
    if (ret == HAL_TIMEOUT) {
        //���ô��ڷ��ͳ�ʱ�¼�
        lpuart_event_set(UART_TX_TIMEOUT_EVENT); 
        
    } else if (ret == HAL_OK) {
        
        //���ô��ڷ�������¼�
        lpuart_event_set(UART_TX_EVENT);         
    }   
    
    if (tx_int_flag) {
    
        __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_TXE);    //ʹ�ܷ����ж�     
    }     
            
    return  ret;
}

//��ѯ���մ�������
int uart_data_rx_poll(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout)
{
    int ret = 0;
    
    int rx_int_flag = 0;
    
    if(size == 0 || pData == NULL)
    {
       return -1;
    }
       
    rx_int_flag = __HAL_UART_GET_IT_SOURCE(&hlpuart1, UART_IT_RXNE);
    
    
    if (rx_int_flag) {
    
        __HAL_UART_DISABLE_IT(&hlpuart1, UART_IT_RXNE);    //��ֹ�����ж�
    }
    
    /* �������ó�ʱΪ HAL_MAX_DELAY ms */
    ret = HAL_UART_Receive(huart, pData, size, Timeout);
        
    if (rx_int_flag) {
        __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_RXNE);    //���������ж�
    }
    
    return ret;
}



#if 0

//����1�жϷ������
void LPUART1_IRQHandler(void)                    
{ 
    u8 Res;

    if((__HAL_UART_GET_FLAG(&hlpuart1,UART_FLAG_RXNE)!=RESET))  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
    {
        HAL_UART_Receive(&hlpuart1,&Res,1,1000); 
        if((NBLOT_USART_RX_STA&0x8000)==0)//����δ���
        {
            if(NBLOT_USART_RX_STA&0x4000)//���յ���0x0d
            {
                if(Res!=0x0a)NBLOT_USART_RX_STA=0;//���մ���,���¿�ʼ
                else NBLOT_USART_RX_STA|=0x8000;    //��������� 
            }
            else //��û�յ�0X0D
            {    
                if(Res==0x0d)NBLOT_USART_RX_STA|=0x4000;
                else
                {
                    NBLOT_RxBuffer[NBLOT_USART_RX_STA&0X3FFF]=Res ;
                    NBLOT_USART_RX_STA++;
                    if(NBLOT_USART_RX_STA>(LPUSART_REC_LEN-1))NBLOT_USART_RX_STA=0;//�������ݴ���,���¿�ʼ����      
                }         
            }
        }            
    }
    
    HAL_UART_IRQHandler(&hlpuart1);    
} 

#endif /* if 0 */

   

#if 0
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance==LPUART1)//����Ǵ���1
    {
       /* �����ڳ�ʱʱ����������ɣ�ֹͣ��ʱ */  
       atk_soft_timer_stop(&uart_dev.uart_rx_timer);
       
       //���ô��ڽ�������¼�
       lpuart_event_set(UART_RX_EVENT);
              
    }
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == LPUART1)//����Ǵ���1
    {
       /* �����ڳ�ʱʱ����������ɣ�ֹͣ��ʱ */  
       atk_soft_timer_stop(&uart_dev.uart_rx_timer);
       
       //���ô��ڷ�������¼�
       lpuart_event_set(UART_TX_EVENT);
       //�ص�ע������Ĵ����¼������� 
//       uart_dev.uart_cb(uart_dev.p_arg); 
              
    }
}

//����1�жϷ������
void LPUART1_IRQHandler(void)                    
{         
    HAL_UART_IRQHandler(&hlpuart1);    //����HAL���жϴ����ú���            
}
#endif


static void __lpuart_rx_timeout_cb (void *p_arg)
{
    
    uart_dev_t *p_lpuart_dev  = (uart_dev_t *)p_arg;
    
    UART_HandleTypeDef *lphuart = p_lpuart_dev->p_huart;
          
      /* Disable the UART Parity Error Interrupt and RXNE interrupts */
#if defined(USART_CR1_FIFOEN)
    CLEAR_BIT(lphuart->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE));
#else
    CLEAR_BIT(lphuart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
#endif 
       
    /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    CLEAR_BIT(lphuart->Instance->CR3, USART_CR3_EIE); 

    //reset the lphuart->RxState
    lphuart->RxState= HAL_UART_STATE_READY;  
       
    //�ٶ�һ�Σ������������ȡ��
    READ_REG(lphuart->Instance->RDR);
       
    //���ô��ڽ��ճ�ʱ�¼�
    lpuart_event_set(UART_RX_TIMEOUT_EVENT);    
}


static void __lpuart_tx_timeout_cb (void *p_arg)
{
    
    uart_dev_t *p_lpuart_dev  = (uart_dev_t *)p_arg;
    
    UART_HandleTypeDef *lphuart = p_lpuart_dev->p_huart;
    
      /* Disable the UART Transmit Data Register Empty Interrupt */
#if defined(USART_CR1_FIFOEN)
      CLEAR_BIT(lphuart->Instance->CR1, USART_CR1_TXEIE_TXFNFIE);
#else
      CLEAR_BIT(lphuart->Instance->CR1, USART_CR1_TXEIE);
#endif
    
    /* Disable the UART Transmit Complete Interrupt */
    CLEAR_BIT(lphuart->Instance->CR1, USART_CR1_TCIE);          

    //reset the lphuart->RxState
    lphuart->gState= HAL_UART_STATE_READY;  
        
    //���ô��ڷ��ͳ�ʱ�¼�
    lpuart_event_set(UART_TX_TIMEOUT_EVENT);    
} 


//�жϽ��մ�������
int uart_data_rx_int(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout)
{
    int ret = 0;
    
    if(size == 0 || pData == NULL)
    {
       return - 1;
    }
         
    /* �������ó�ʱΪ HAL_MAX_DELAY ms */
    ret = HAL_UART_Receive_IT(huart, pData, size);
    
    /* ��ʼ��ʱ���� */
    atk_soft_timer_init(&uart_dev.uart_rx_timer, __lpuart_rx_timeout_cb, &uart_dev, Timeout, 0); 
    atk_soft_timer_start(&uart_dev.uart_rx_timer);
             
    /* ͬ����ʱ��ʱ�� */
    while (HAL_UART_GetState(&hlpuart1) != HAL_UART_STATE_READY);//�ȴ�����
               
    return ret;
}


//�жϷ��ʹ������ݣ� ����ʹ��
int uart_data_tx_int(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout)
{
    int ret = 0;
    
    if(size == 0 || pData == NULL)
    {
       return -1;
    }
    
    ret = HAL_UART_Transmit_IT(huart, pData, size);
    
     /* ��ʼ��ʱ���� */
    atk_soft_timer_init(&uart_dev.uart_tx_timer, __lpuart_tx_timeout_cb, &uart_dev, Timeout, 0); 
    atk_soft_timer_start(&uart_dev.uart_tx_timer);
           
    /* ͬ����ʱ��ʱ�� */    
    while (HAL_UART_GetState(&hlpuart1) != HAL_UART_STATE_READY);//�ȴ�����
    
    return ret;
}

//��ѯ�����¼�
void uart_event_poll(uart_handle_t uart_handle)
{ 
    //�ص�ע������Ĵ����¼������� 
    uart_dev.uart_cb(uart_handle->p_arg);
    
}






