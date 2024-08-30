/**
  ******************************************************************************
  * File Name          : CAN.h
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __can_H
#define __can_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */


extern CAN_HandleTypeDef hcan;

typedef struct
{
  uint32_t mailbox;
  CAN_TxHeaderTypeDef hdr;
  uint8_t payload[8];
} CAN_TxPacketTypeDef;

extern CAN_TxPacketTypeDef g_CanTxPacket;

typedef struct
{
  CAN_RxHeaderTypeDef hdr;
  uint8_t payload[8];
} CAN_RxPacketTypeDef;

/// CAN过滤器寄存器位宽类型定义
typedef union
{
  __IO uint32_t value;
  struct
  {
    uint8_t REV : 1;    ///< [0]    ：未使用
    uint8_t RTR : 1;    ///< [1]    : RTR（数据帧或远程帧标志位）
    uint8_t IDE : 1;    ///< [2]    : IDE（标准帧或扩展帧标志位）
    uint32_t EXID : 18; ///< [21:3] : 存放扩展帧ID
    uint16_t STID : 11; ///< [31:22]: 存放标准帧ID
  } Sub;
} CAN_FilterRegTypeDef;

#define CAN_BASE_ID 0 ///< CAN标准ID，最大11位，也就是0x7FF

#define CAN_FILTER_MODE_MASK_ENABLE 1 ///< CAN过滤器模式选择：=0：列表模式  =1：屏蔽模式

#define CAN_ID_TYPE_STD_ENABLE 1 ///< CAN过滤ID类型选择：=1：标准ID，=0：扩展ID

void CAN_Config(void);
void CAN_Filter_Config(void);
uint8_t CAN_Transmit(CAN_TxPacketTypeDef *packet);
void CAN_Init(void);
void CAN_SetTxPacket(void);

/* USER CODE END Prototypes */






#ifdef __cplusplus
}
#endif
#endif /*__ can_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
