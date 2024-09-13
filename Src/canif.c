#include "canif.h"
#include "can.h"

CAN_RxHeaderTypeDef rxHeader;  // 定义一个用于存储接收 CAN 消息头的变量。
PduInfoTRx CanIfPduInfo;  // 定义一个用于存储接收 CAN 消息数据的结构体变量。
extern volatile int8_t CanIf_Rx;  
Std_ReturnType (*CanTp_Callback)(uint32_t RxPduId, PduInfoTRx* PduInfoPtr) = NULL; 

const uint32_t TxPduIdToStdIdTable[] = {
    0x100,
    0x122,
    0x144,
    0x166,
    0x188,
    0x1AA,
    0x1CC,
    0x1EE,
    0x210,
    0x232,
    0x254,
    0x276,
    0x298,
    0x2BA,
    0x2DC,
    0x2FE,
    0x320,
    0x342,
    0x364,
    0x386,
    0x505,
    0x643,
};



void CanIf_Transmit(uint32_t TxPduId, PduInfoTRx* PduInfoPtr) {
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;

    if (TxPduId < (sizeof(TxPduIdToStdIdTable) / sizeof(TxPduIdToStdIdTable[0]))) {
        txHeader.StdId = TxPduIdToStdIdTable[TxPduId];
    } else {
        Error_Handler();
        return;
    }

    txHeader.ExtId = 0x00;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.DLC = 8;

    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0);

    if (HAL_CAN_AddTxMessage(&hcan, &txHeader, PduInfoPtr->Data, &txMailbox) != HAL_OK) {
        Error_Handler();
    }
}

#define TX_PDU_ID_TO_STD_ID_TABLE_SIZE (sizeof(TxPduIdToStdIdTable) / sizeof(TxPduIdToStdIdTable[0]))

void CanIf_Receive() {
    uint32_t PDU_ID;  // 定义一个变量用于存储接收的 PDU ID
    while (1) {
        // 检查是否接收到 CAN 消息
        if (CanIf_Rx) {
           HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            CanIf_Rx = 0;  // 重置接收标志
            CanIfPduInfo.Length = rxHeader.DLC;  // 将接收到的消息长度保存到 PDU 结构体中

            // 遍历查找表，根据接收到的标准标识符查找对应的 PDU ID
            PDU_ID = -1;  // 初始化 PDU_ID 为无效值
            for (uint32_t i = 0; i < TX_PDU_ID_TO_STD_ID_TABLE_SIZE; ++i) {
                if (rxHeader.StdId == TxPduIdToStdIdTable[i]) {
                    PDU_ID = i;
                    break;
                }
            }
            
            if (PDU_ID != -1) {
                // 根据 PDU_ID 执行相应的回调函数逻辑
                if (PDU_ID == TX_PDU_ID_TO_STD_ID_TABLE_SIZE - 2) {  // 倒数第二个元素
                    /* if (CanNm_RxCallback != NULL) {
                        CanNm_RxCallback();
                    } */
                } else if (PDU_ID == TX_PDU_ID_TO_STD_ID_TABLE_SIZE - 1) {  // 最后一个元素
                   /*  if (CanTp_Callback != NULL) {
                        CanTp_Callback(PDU_ID, &CanIfPduInfo);
                    } */ 
                }
                else
                {
                    CanIf_Transmit(PDU_ID, &CanIfPduInfo);
                }
                // 其他元素和未找到的元素，不处理
            }
        }
        // 延迟 10 个时间单位，以避免占用过多的 CPU 资源
        vTaskDelay(10);
    }
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan1){
    if (hcan1->Instance == hcan.Instance) // 获得接收到的数据头和数据
    {
    // 尝试从 FIFO0 中获取一条 CAN 消息
	if (HAL_CAN_GetRxMessage(hcan1, CAN_RX_FIFO0, &rxHeader, CanIfPduInfo.Data) != HAL_OK) {
		// 如果获取消息失败，调用错误处理函数
		Error_Handler();
	}
    // 设置接收标志为 1，表示有消息接收到了
    }
    CanIf_Rx = 1;
}

/*// 如果发送功，并且配置了传输完成回调，则调用传输完成回调
	else if(CanNm_TxCallback != NULL && TxPduId == 1){
        
		CanNm_TxCallback();
	}
	l*/
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);  // (注释掉的代码) 可以用于在发送消息后点亮 LED 等。
