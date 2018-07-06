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

static uart_dev_t uart_dev;


#if EN_LPUART1_RX   //���ʹ���˽���

//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 NBLOT_RxBuffer[LPUSART_REC_LEN];     //���ջ���,���LPUSART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 NBLOT_USART_RX_STA=0;       //����״̬���	

UART_HandleTypeDef hlpuart1;

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
void lpuart1_init (u32 bound)
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
  
#if EN_LPUART1_RX
		__HAL_UART_ENABLE_IT(&hlpuart1,UART_IT_RXNE);    //���������ж�
		HAL_NVIC_EnableIRQ(LPUART1_IRQn);				//ʹ��USART1�ж�ͨ��
		HAL_NVIC_SetPriority(LPUART1_IRQn,3,3);			//��ռ���ȼ�3�������ȼ�3
#endif	  

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
				else NBLOT_USART_RX_STA|=0x8000;	//��������� 
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


#endif	


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//����Ǵ���1
	{
        
        
	}
}

//����1�жϷ������
void USART1_IRQHandler(void)                	
{ 
    	
	HAL_UART_IRQHandler(&hlpuart1);	//����HAL���жϴ����ú���	
    
    atk_soft_timer_stop(&uart_dev.uart_timer);
	
}




//��ѯ���ʹ�������
int uart_data_tx_poll(UART_HandleTypeDef *huart, uint8_t *pData,uint16_t size, uint32_t Timeout)
{   
    int ret = 0;
    
    if(size == 0 || pData == NULL)
    {
       return -1;
    }
    
    __HAL_UART_DISABLE_IT(&hlpuart1, UART_IT_TXE);    //���ܷ����ж�  
    
    /* �������ó�ʱΪ HAL_MAX_DELAY ms */ 
     ret = HAL_UART_Transmit(huart, pData, size, HAL_MAX_DELAY);
    
    __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_TXE);    //ʹ�ܷ����ж�      
    
     return  ret;
}

//��ѯ���մ�������
int uart_data_rx_poll(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout)
{
    int ret = 0;
    
    if(size == 0 || pData == NULL)
    {
       return -1;
    }
    
    __HAL_UART_DISABLE_IT(&hlpuart1, UART_IT_RXNE);    //��ֹ�����ж�
    
    /* �������ó�ʱΪ HAL_MAX_DELAY ms */
    ret = HAL_UART_Receive(huart, pData, size, Timeout);
    
    __HAL_UART_DISABLE_IT(&hlpuart1, UART_IT_RXNE);    //���������ж�
    
    return ret;
}



//**************************************
// fn : LPUART_RegisterCb
//
// brief : ע�ᰴť�¼��ص�
//
// param : cb -> ����ť�¼�����ָ��
//
// return : none
void LPUART_RegisterCb(uart_cb cb, void *p_arg)
{
  if(cb != 0)
  {
    uart_dev.lpuart_cb = cb;
    uart_dev.p_arg     = p_arg;
  }
}



//�жϽ��մ�������
int uart_data_rx_int(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout)
{
    int ret = 0;
    
    if(size == 0 || pData == NULL)
    {
       return -1;
    }
       
  
    /* �������ó�ʱΪ HAL_MAX_DELAY ms */
    ret = HAL_UART_Receive_IT(huart, pData, size);
    
    /* ��ʼ��ʱ���� */
    atk_soft_timer_init(&uart_dev.uart_timer, uart_dev.lpuart_cb,  uart_dev.p_arg, Timeout, 0); //1s loop
    atk_soft_timer_start(&uart_dev.uart_timer);
    
             
    /* ͬ����ʱ��ʱ�� */
    while (HAL_UART_GetState(&hlpuart1) != HAL_UART_STATE_READY);//�ȴ�����
    
           
    return ret;
}


//�жϷ��ʹ�������
int uart_data_tx_int(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, uint32_t Timeout)
{
    int ret = 0;
    
    if(size == 0 || pData == NULL)
    {
       return -1;
    }
    
    while (HAL_UART_GetState(&hlpuart1) != HAL_UART_STATE_READY);//�ȴ�����

    ret = HAL_UART_Transmit_IT(huart, pData, size);
    
    return ret;
}





