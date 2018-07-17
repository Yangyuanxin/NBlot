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

//����һ��uart ringbuff �Ļ�����      
static atk_ring_buf_t g_uart_ring_buf;  

//HAL�⴮���豸�ṹ��
static UART_HandleTypeDef hlpuart1;

//������ǰ����
static void __lpuart_rx_timeout_cb (void *p_arg);

//ע�ᴮ���¼��ص�����
void lpuart_event_registercb(uart_handle_t uart_handle, uart_cb cb, void *p_arg)
{  
    if(cb != 0)
    {
        uart_handle->uart_cb  = cb;
        uart_handle->p_arg    = p_arg;
    }
}

//���ô����¼�
void lpuart_event_set (uart_handle_t uart_handle, int uart_event)
{ 
    uart_handle->uart_event |= uart_event;   
}

//��ȡ�����¼�
int lpuart_event_get (uart_handle_t uart_handle, int uart_event)
{ 
    return (uart_handle->uart_event & uart_event); 
}

//��������¼�
void lpuart_event_clr (uart_handle_t uart_handle, int uart_event)
{ 
    uart_handle->uart_event ^= uart_event;
}

/**
 * @brief ��ʼ�����λ������������Ϣ
 */
int atk_ring_buf_init (atk_ring_buf_t *p_ring_buf)
{    
    if (p_ring_buf == NULL) {        
        return -1;       
    }
    
    p_ring_buf->head   = 0;
    p_ring_buf->tail   = 0;
    p_ring_buf->lenght = 0;
    
    return 0;
}

/**
 * @brief �����λ�����д����
 */
int atk_ring_buf_write(atk_ring_buf_t *p_ring_buf, uint8_t data)
{
    if(p_ring_buf->lenght >= RING_BUF_LEN)          //�жϻ������Ƿ�����
    {      
        printf("the ring buf is full\r\n");
        
        return -1;
    } 
    
    //�����ٽ�������
//    INTX_DISABLE();
    
    p_ring_buf->ring_buf[p_ring_buf->tail] = data;

    //��ֹԽ��Ƿ�����
    p_ring_buf->tail = (p_ring_buf->tail+1) % RING_BUF_LEN;
    
    p_ring_buf->lenght++;

//   INTX_ENABLE();     
    
    return 0;
}

/**
 * @brief ���λ���������Ч���ݵĸ���
 */
int atk_ring_buf_avail_len (atk_ring_buf_t *p_ring_buf)
{
     return p_ring_buf->lenght;
}

 /**
 * @brief  ��ȡ���λ�����������
 */
int atk_ring_buf_read(atk_ring_buf_t *p_ring_buf, uint8_t *data)
{
   if (p_ring_buf->lenght == 0)    //�жϷǿ�
   {
       return -1;
   }
   
   //�����ٽ�������
//   INTX_DISABLE();   
   
   //�Ƚ��ȳ�FIFO���ӻ�����ͷ��
   *data = p_ring_buf->ring_buf[p_ring_buf->head];
   
   //��ֹԽ��Ƿ�����
   p_ring_buf->head = (p_ring_buf->head+1) % RING_BUF_LEN; 
   
   p_ring_buf->lenght--;
   
//   INTX_ENABLE(); 
   
   return 0;
}

 /**
 * @brief  �ӽ��ջ������ȡָ�����ȵ����ݣ����ͷ�ռ�õĿռ�
 */
int atk_ring_buf_size_read(atk_ring_buf_t *p_ring_buf, uint8_t *data, int len)
{   
    int i = 0; 
        
    if (len > atk_ring_buf_avail_len(p_ring_buf))
    {
        return -1; 
    }
    
    for (i = 0; i <  len; i++) {
        
         atk_ring_buf_read(p_ring_buf, &data[i]);
    }

    return 0;
}


 /**
 * @brief  д����ջ������ָ�����ȵ����ݣ���ռ�õĿռ�
 */
int atk_ring_buf_size_write(atk_ring_buf_t *p_ring_buf, uint8_t *data, int len)
{
    
    int i = 0; 
    
    //����������С��д��ĳ���    
    if (len > (RING_BUF_LEN - p_ring_buf->lenght))
    {
        return -1; 
    }
    
    for (i = 0; i <  len; i++) {
        
         atk_ring_buf_write(p_ring_buf, data[i]);
    }

    return 0;
}



 /**
 * @brief  ��UART���ջ������ȡָ�����ȵ����ݣ����ͷ�ռ�õĿռ�
 */
int uart_ring_buf_read(uart_handle_t uart_handle, uint8_t *data, int len)
{
    int ret = 0;
    
    ret = atk_ring_buf_size_read(uart_handle->p_uart_ring_buff, data, len);
     
    return ret;
}

 /**
 * @brief  ��ȡ���ڻ��λ���������Ч���ݵĸ���
 */
int uart_ring_buf_avail_len(uart_handle_t uart_handle)
{
    return atk_ring_buf_avail_len(uart_handle->p_uart_ring_buff);
}

 /**
 * @brief  д��uart���ջ������ָ�����ȵ����ݣ���ռ�õĿռ�
 */
int uart_ring_buf_write(uart_handle_t uart_handle, uint8_t *data, int len)
{
    int ret = 0;
    
    ret = atk_ring_buf_size_write(uart_handle->p_uart_ring_buff, data, len);
     
    return ret;
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
    int err = 0;
    
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
    
    //��ʼ���ճ�ʱ����������ַ�֮��Ľ���ʱ�䳬��10msû�е�����������Ϊ�������
    atk_soft_timer_init(&uart_dev.uart_rx_timer, __lpuart_rx_timeout_cb, &uart_dev, 10, 0);     
    
    //���ջ����ջ��λ�����
    err = atk_ring_buf_init(&g_uart_ring_buf);
   
    //�����������豸�ṹ��    
    uart_dev.p_huart    = &hlpuart1; 
    uart_dev.uart_event = UART_NONE_EVENT;   
    uart_dev.bound      = bound; 
    
    if (err == 0) 
    {
        uart_dev.p_uart_ring_buff  = &g_uart_ring_buf;           
    }
    else
    {
        uart_dev.p_uart_ring_buff  = NULL;   
    }


    //���ܴ���
     __HAL_UART_DISABLE(&hlpuart1);    

    __HAL_UART_DISABLE_IT(&hlpuart1, UART_IT_TXE);  //��ֹ�����ж� 
    __HAL_UART_DISABLE_IT(&hlpuart1, UART_IT_TC);   //���ܷ�������ж�   
    
    //���rx�жϱ��
    __HAL_UART_SEND_REQ(&hlpuart1, UART_RXDATA_FLUSH_REQUEST);
      
    //���ilde�жϱ��
    __HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_IDLEF); 
     
    __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_RXNE);    //���������ж�      
    __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_IDLE);    //���������ж� 
    
     //���ilde�жϱ��
    __HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_IDLEF); 

    HAL_NVIC_EnableIRQ(LPUART1_IRQn);                //ʹ��USART1�ж�ͨ��
    HAL_NVIC_SetPriority(LPUART1_IRQn,3,3);          //��ռ���ȼ�3�������ȼ�3
    
    //ʹ�ܴ���
     __HAL_UART_ENABLE(&hlpuart1);   
       
    return  &uart_dev;   
}

static void __lpuart_rx_timeout_cb (void *p_arg)
{
    
    uart_dev_t *p_lpuart_dev  = (uart_dev_t *)p_arg;
    
    UART_HandleTypeDef *lphuart = p_lpuart_dev->p_huart;

#if !UART_ANY_DATA_LEN_RECV 
 
    /* Disable the UART Parity Error Interrupt and RXNE interrupts */
#if defined(USART_CR1_FIFOEN)
    CLEAR_BIT(lphuart->Instance->CR1, (USART_CR1_RXNEIE_RXFNEIE | USART_CR1_PEIE));
#else
    CLEAR_BIT(lphuart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
#endif //endif  USART_CR1_FIFOEN
    
    /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    CLEAR_BIT(lphuart->Instance->CR3, USART_CR3_EIE);     
    
#endif //endif    UART_ANY_DATA_LEN_RECV
    
    //reset the lphuart->RxState
    lphuart->RxState= HAL_UART_STATE_READY; 

    //����������
    READ_REG(lphuart->Instance->RDR);
             
    //�������жϱ��
    __HAL_UART_CLEAR_IT(&hlpuart1,UART_CLEAR_OREF); 
                               
   
    //���֡�����жϱ��
    __HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_FEF); 
                               
        
    //���У���жϱ��
    __HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_PEF); 
                                   
           
    //��������ж�
    __HAL_UART_SEND_REQ(lphuart, UART_RXDATA_FLUSH_REQUEST);
    
    //���ilde�жϱ��
    __HAL_UART_CLEAR_IT(lphuart, UART_FLAG_IDLE); 

    //���¿��������ж� 
    __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_RXNE);    
       
    //���ô��ڽ��ճ�ʱ�¼�
    lpuart_event_set(p_lpuart_dev, UART_RX_TIMEOUT_EVENT);    
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
    lpuart_event_set(p_lpuart_dev, UART_TX_TIMEOUT_EVENT);    
}

#ifdef UART_ANY_DATA_LEN_RECV  

//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���  
void LPUART1_IRQHandler(void)                    
{ 

     
    if ((__HAL_UART_GET_FLAG(&hlpuart1,UART_FLAG_RXNE)!=RESET))  
    {        
        //������д�뻷�λ�����
        atk_ring_buf_write(&g_uart_ring_buf, hlpuart1.Instance->RDR);  
             
        //�յ�һ�����ݣ����ó�ʱ 
        atk_soft_timer_timeout_change(&uart_dev.uart_rx_timer, 10);
      
          
        /* �յ����ݷ����ڳ�ʱʱ����������ɣ�ֹͣ��ʱ */  
        atk_soft_timer_stop(&uart_dev.uart_tx_timer);
                                   
    }
    
    //ilde�жϲ���������ǰ����֡���ս���
    if ((__HAL_UART_GET_FLAG(&hlpuart1, UART_FLAG_IDLE)!=RESET))  
    {
     
    
        //����ǿ����ж�һ��ʼ��ʹ�ܵ�
       if (__HAL_UART_GET_IT_SOURCE(&hlpuart1, UART_IT_IDLE)!=RESET) {
           
           //�����ڳ�ʱʱ����������ɣ�ֹͣ��ʱ  
           atk_soft_timer_stop(&uart_dev.uart_rx_timer);
           
           //���ô��ڽ�������¼�
           lpuart_event_set(&uart_dev, UART_RX_EVENT); 
       } 

        //���ilde�жϱ��
        __HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_IDLEF);        
                            
    }    
    
    //����HAL�жϴ������
    HAL_UART_IRQHandler(&hlpuart1);    
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == LPUART1)//����Ǵ���1
    {
       /* �����ڳ�ʱʱ����������ɣ�ֹͣ��ʱ */  
       atk_soft_timer_stop(&uart_dev.uart_tx_timer);
       
       //���ô��ڷ�������¼�
       lpuart_event_set(&uart_dev, UART_TX_EVENT);              
    }
}

#else 
//�ú��������ݽ������ʱ������
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance==LPUART1)//����Ǵ���1
    {
       //�����ڳ�ʱʱ����������ɣ�ֹͣ��ʱ  
       atk_soft_timer_stop(&uart_dev.uart_rx_timer);
       
       //���ô��ڽ�������¼�
       lpuart_event_set(&uart_dev,UART_RX_EVENT);
              
    }
}

//����1�жϷ������
void LPUART1_IRQHandler(void)                    
{         
    HAL_UART_IRQHandler(&hlpuart1);    //����HAL���жϴ����ú��� 

    //ilde�жϲ���������ǰ����֡���ս���
    if ((__HAL_UART_GET_FLAG(&hlpuart1, UART_FLAG_IDLE)!=RESET))  
    {
     
        //���ilde�жϱ��
        __HAL_UART_CLEAR_IT(&hlpuart1, UART_FLAG_IDLE); 
                                         
    }    
}
#endif   //end if UART_ANY_DATA_LEN_RECV 
 

//��ѯ���ʹ�������
int uart_data_tx_poll(uart_handle_t uart_handle, uint8_t *pData,uint16_t size, uint32_t Timeout)
{   
    int ret = 0;
    
    int tx_int_flag = 0;
    
    if(size == 0 || pData == NULL)
    {
       return -1;
    }
    
    tx_int_flag = __HAL_UART_GET_IT_SOURCE(uart_handle->p_huart, UART_IT_TXE);
    
    
    if (tx_int_flag) {
    
        __HAL_UART_DISABLE_IT(uart_handle->p_huart, UART_IT_TXE);    //���ܷ����ж�  
    }    
        
    /* �������ó�ʱΪ Timeout ms */ 
    ret = HAL_UART_Transmit(uart_handle->p_huart, pData, size, Timeout);
    
    
    if (ret == HAL_TIMEOUT) {
      
        //���ô��ڷ��ͳ�ʱ�¼�
        lpuart_event_set(uart_handle, UART_TX_TIMEOUT_EVENT); 
      
        return ret;
        
    } else if (ret == HAL_OK) {
        
        //���ô��ڷ�������¼�
        lpuart_event_set(uart_handle, UART_TX_EVENT);         
    }   
    
    if (tx_int_flag) {
    
        __HAL_UART_ENABLE_IT(uart_handle->p_huart, UART_IT_TXE);    //ʹ�ܷ����ж�     
    }
    
    
    /* ��ʼ��ʱ���� */
    atk_soft_timer_init(&uart_handle->uart_tx_timer, __lpuart_tx_timeout_cb, uart_handle, Timeout, 0); 
    atk_soft_timer_start(&uart_handle->uart_tx_timer);    
    
    return  ret;
}

//��ѯ���մ�������
int uart_data_rx_poll(uart_handle_t uart_handle, uint8_t *pData, uint16_t size, uint32_t Timeout)
{
    int ret = 0;
    
    int rx_int_flag = 0;
    
    if(size == 0 || pData == NULL)
    {
       return -1;
    }
       
    rx_int_flag = __HAL_UART_GET_IT_SOURCE(uart_handle->p_huart, UART_IT_RXNE);
    
    
    if (rx_int_flag) {
    
        __HAL_UART_DISABLE_IT(uart_handle->p_huart, UART_IT_RXNE);    //��ֹ�����ж�
    }
    
    /* �������ó�ʱΪ Timeout ms */
    ret = HAL_UART_Receive(uart_handle->p_huart, pData, size, Timeout);
    
    if (ret == HAL_TIMEOUT) {
        //���ô��ڽ��ճ�ʱ�¼�
        lpuart_event_set(uart_handle, UART_RX_TIMEOUT_EVENT); 
        
    } else if (ret == HAL_OK) {
        
        //���ô��ڽ�������¼�
        lpuart_event_set(uart_handle, UART_RX_EVENT);         
    }
        
    if (rx_int_flag) {
        __HAL_UART_ENABLE_IT(uart_handle->p_huart, UART_IT_RXNE);    //���������ж�
    }
    
    return ret;
}


//�жϽ��մ�������
int uart_data_rx_int(uart_handle_t uart_handle, uint8_t *pData, uint16_t size, uint32_t Timeout)
{
    int ret = 0;
    
    if(size == 0 || pData == NULL)
    {
       return - 1;
    }
         
    /* �������ó�ʱΪ HAL_MAX_DELAY ms */
    ret = HAL_UART_Receive_IT(uart_handle->p_huart, pData, size);
    
    /* ��ʼ��ʱ���� */
    atk_soft_timer_init(&uart_handle->uart_rx_timer, __lpuart_rx_timeout_cb, uart_handle, Timeout, 0); 
    atk_soft_timer_start(&uart_handle->uart_rx_timer);
             
    /* ͬ����ʱ��ʱ�� */
    while (HAL_UART_GetState(uart_handle->p_huart) != HAL_UART_STATE_READY);//�ȴ�����
               
    return ret;
}


//�жϷ��ʹ������ݣ� ����ʹ��
int uart_data_tx_int(uart_handle_t uart_handle, uint8_t *pData, uint16_t size, uint32_t Timeout)
{
    int ret = 0;
    
    if(size == 0 || pData == NULL)
    {
       return -1;
    }
    
    ret = HAL_UART_Transmit_IT(uart_handle->p_huart, pData, size);
    
    /* ��ʼ��ʱ���� */
    atk_soft_timer_init(&uart_handle->uart_tx_timer, __lpuart_tx_timeout_cb, uart_handle, Timeout, 0); 
    atk_soft_timer_start(&uart_handle->uart_tx_timer);
           
    /* ͬ����ʱ��ʱ�� */    
    while (HAL_UART_GetState(uart_handle->p_huart) != HAL_UART_STATE_READY);//�ȴ�����    
    return ret;
}

//��ѯ�����¼�
void uart_event_poll(uart_handle_t uart_handle)
{ 
    //�ص�ע������Ĵ����¼������� 
    uart_dev.uart_cb(uart_handle->p_arg);
    
}






