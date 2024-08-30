/**
 ******************************************************************************
 * File Name          : CAN.c
 * Description        : This file provides code for the configuration
 *                      of the CAN instances.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 9;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (canHandle->Instance == CAN1)
  {
    /* USER CODE BEGIN CAN1_MspInit 0 */

    /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
    /* USER CODE BEGIN CAN1_MspInit 1 */

    /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle)
{

  if (canHandle->Instance == CAN1)
  {
    /* USER CODE BEGIN CAN1_MspDeInit 0 */

    /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
    /* USER CODE BEGIN CAN1_MspDeInit 1 */

    /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void CAN_Config(void)
{
  CAN_FilterTypeDef sFilterConfig;

  /* Configure the CAN Filter */
  sFilterConfig.FilterBank = 0;                      // 过滤器编号，使用一个CAN，则可选0-13；使用两个CAN可选0-27
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;  // 过滤器模式，掩码模式或列表模式
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT; // 过滤器位宽.
  sFilterConfig.FilterIdHigh = 0x0000;               // 过滤器验证码ID高16位，0-0xFFFF
  sFilterConfig.FilterIdLow = 0x0000;                // 过滤器验证码ID低16位，0-0xFFFF
  sFilterConfig.FilterMaskIdHigh = 0x0000;           // 过滤器掩码ID高16位，0-0xFFFF
  sFilterConfig.FilterMaskIdLow = 0x0000;            // 过滤器掩码ID低16位，0-0xFFFF
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0; // FIFOx，0或1
  sFilterConfig.FilterActivation = ENABLE;           // 使能过滤器
  sFilterConfig.SlaveStartFilterBank = 14;           // 从过滤器编号，0-27，对于单CAN实例该参数没有意义

  if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
  {
    /* Filter configuration Error */
  }

  /* Start the CAN peripheral */
  if (HAL_CAN_Start(&hcan) != HAL_OK)
  {
    /* Start Error */
  }

  /* Activate CAN RX notification */
  if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  {
    /* Notification Error */
  }
}



void CAN_Filter_Config(void)
{
  CAN_FilterTypeDef sFilterConfig;
  CAN_FilterRegTypeDef IDH = {0};
  CAN_FilterRegTypeDef IDL = {0};

#if CAN_ID_TYPE_STD_ENABLE
  IDH.Sub.STID = (CAN_BASE_ID >> 16) & 0xFFFF; // 标准ID高16位
  IDL.Sub.STID = (CAN_BASE_ID & 0xFFFF);       // 标准ID低16位
#else
  IDH.Sub.EXID = (CAN_BASE_ID >> 16) & 0xFFFF; // 扩展ID高16位
  IDL.Sub.EXID = (CAN_BASE_ID & 0xFFFF);       // 扩展ID低16位
  IDL.Sub.IDE = 1;                             // 扩展帧标志位置位
#endif
  sFilterConfig.FilterBank = 0; // 设置过滤器组编号
#if CAN_FILTER_MODE_MASK_ENABLE
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK; // 屏蔽位模式
#else
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST; // 列表模式
#endif
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT; // 32位宽
  sFilterConfig.FilterIdHigh = IDH.value;            // 标识符寄存器一ID高十六位，放入扩展帧位
  sFilterConfig.FilterIdLow = IDL.value;             // 标识符寄存器一ID低十六位，放入扩展帧位
  sFilterConfig.FilterMaskIdHigh = IDH.value;        // 标识符寄存器二ID高十六位，放入扩展帧位
  sFilterConfig.FilterMaskIdLow = IDL.value;         // 标识符寄存器二ID低十六位，放入扩展帧位
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0; // 过滤器组关联到FIFO0
  sFilterConfig.FilterActivation = ENABLE;           // 激活过滤器
  sFilterConfig.SlaveStartFilterBank = 14;           // 设置从CAN的起始过滤器编号，本单片机只有一个CAN，顾此参数无效
  if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

uint8_t CAN_Transmit(CAN_TxPacketTypeDef *packet)
{
  if (HAL_CAN_AddTxMessage(&hcan, &packet->hdr, packet->payload, &packet->mailbox) != HAL_OK)
    return 1;
  return 0;
}

void CAN_Init(void)
{
  MX_CAN_Init();
  CAN_Filter_Config();
  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING); // 使能CAN接收中断
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *canHandle)
{
  static CAN_RxPacketTypeDef packet;

  // CAN数据接收
  if (canHandle->Instance == hcan.Instance)
  {
    if (HAL_CAN_GetRxMessage(canHandle, CAN_RX_FIFO0, &packet.hdr, packet.payload) == HAL_OK) // 获得接收到的数据头和数据
    {
      for (int i = 0; i < packet.hdr.DLC; i++)
      {
      }
      HAL_CAN_ActivateNotification(canHandle, CAN_IT_RX_FIFO0_MSG_PENDING); // 再次使能FIFO0接收中断
    }
  }
}

CAN_TxPacketTypeDef g_CanTxPacket;

void CAN_SetTxPacket(void)
{
  g_CanTxPacket.hdr.StdId = 0x321;      // 标准ID
                                        //  g_CanTxPacket.hdr.ExtId = 0x10F01234;       // 扩展ID
  g_CanTxPacket.hdr.IDE = CAN_ID_STD;   // 标准ID类型
                                        //  g_CanTxPacket.hdr.IDE = CAN_ID_EXT;         // 扩展ID类型
  g_CanTxPacket.hdr.DLC = 8;            // 数据长度
  g_CanTxPacket.hdr.RTR = CAN_RTR_DATA; // 数据帧
                                        //  g_CanTxPacket.hdr.RTR = CAN_RTR_REMOTE;     // 远程帧
  g_CanTxPacket.hdr.TransmitGlobalTime = DISABLE;

  for (int i = 0; i < 8; i++)
  {
    g_CanTxPacket.payload[i] = i;
  }
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
