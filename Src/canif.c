#include "canif.h"
#include "can.h"
#include <stdint.h>
#include <stdbool.h>

CAN_RxHeaderTypeDef rxHeader;  // 定义一个用于存储接收 CAN 消息头的变量。
PduInfoTRx CanIfPduInfo;  // 定义一个用于存储接收 CAN 消息数据的结构体变量。
extern volatile int8_t CanIf_Rx;  
Std_ReturnType (*CanTp_Callback)(uint32_t RxPduId, PduInfoTRx* PduInfoPtr) = NULL; 

#define TX_PDU_ID_TO_STD_ID_TABLE_SIZE (sizeof(TxPduIdToStdIdTable) / sizeof(TxPduIdToStdIdTable[0]))
#define RX_PDU_ID_TO_STD_ID_TABLE_SIZE (sizeof(RxPduIdToStdIdTable) / sizeof(RxPduIdToStdIdTable[0]))

// 定义枚举类型表示三种类型
typedef enum {
    APPL,    // 应用程序类型
    NM,     // 网络管理类型
    DIAG   // 诊断类型
} PduType;

// 定义结构体存储ID、值和类型
typedef struct {
    uint32_t pduid;
    uint32_t stdid;
    PduType type;
} PduIdStdId;

const PduIdStdId RxPduIdToStdIdTable[] = {
    {0, 0x232, APPL},
    {1, 0x100, APPL},
    {2, 0x351, APPL},
    {3, 0x245, APPL},
    {4, 0x354, APPL},
    {5, 0x2FE, APPL},
    {6, 0x1AA, APPL},
    {7, 0x523, NM},
    {8, 0x720, DIAG}
};

// 创建结构体数组并初始化数据 (TxPduIdToStdIdTable 从 0 开始)
const PduIdStdId TxPduIdToStdIdTable[] = {
    {0, 0x232, APPL},
    {1, 0x100, APPL},
    {2, 0x351, APPL},
    {3, 0x245, APPL},
    {4, 0x354, APPL},
    {5, 0x2FE, APPL},
    {6, 0x1AA, APPL},
    {7, 0x501, NM},
    {8, 0x728, DIAG}
};


void CanIf_Transmit(uint32_t TxPduId, PduInfoTRx* PduInfoPtr) {
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;
    bool found = false;
    for (int i = 0; i < TX_PDU_ID_TO_STD_ID_TABLE_SIZE; i++) {
        if (TxPduId == TxPduIdToStdIdTable[i].pduid) {
            txHeader.StdId = TxPduIdToStdIdTable[i].stdid;
            found = true;
            break;
        }
    }
    if (!found) {
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



void CanIf_Receive() {
    uint32_t PDU_ID;  // 定义一个变量用于存储接收的 PDU ID
    PduType PDU_TYPE;
    while (1) {
        // 检查是否接收到 CAN 消息
        if (CanIf_Rx) {
           //HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            CanIf_Rx = 0;  // 重置接收标志/
            CanIfPduInfo.Length = rxHeader.DLC;  // 将接收到的消息长度保存到 PDU 结构体中

            // 遍历查找表，根据接收到的标准标识符查找对应的 PDU ID
            PDU_ID = -1;  // 初始化 PDU_ID 为无效值
            for (uint32_t i = 0; i < RX_PDU_ID_TO_STD_ID_TABLE_SIZE; ++i) {
                if (rxHeader.StdId == RxPduIdToStdIdTable[i].stdid) {
                    PDU_ID = RxPduIdToStdIdTable[i].pduid;
                    PDU_TYPE = RxPduIdToStdIdTable[i].type;
                    break;
                }
            }
            
            if (PDU_ID != -1) {
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
                switch (PDU_TYPE) {
                    case NM:  
                        /* if (CanNm_RxCallback != NULL) {
                        CanNm_RxCallback();
                    } */
                        break;

                    case DIAG:  
                          if (CanTp_Callback != NULL) {
                        CanTp_Callback(PDU_ID, &CanIfPduInfo);
                    }  
                        break;

                    case APPL:  
                        CanIf_Transmit(PDU_ID, &CanIfPduInfo);
                        break;

                    default:
                        Error_Handler();  // 未知类型
                        break;
                }
            }
        }
        // 延迟 10 个时间单位，以避免占用过多的 CPU 资源
        vTaskDelay(10);
    }
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan1){
    if (hcan1->Instance == hcan.Instance) // 获得接*收到的数据头和数据
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

/*// 如果发送成功，并且配置了传输完成回调，则调用传输完成回调
	else if(CanNm_TxCallback != NULL && TxPduId == 1){
        
		CanNm_TxCallback();
	}
	l*/
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);  // (注释掉的代码) 可以用于在发送消息后点亮 LED 等。

// 设置 CAN 接收回调函数1
void CanIf_setCallback(Std_ReturnType (*IF_Callback)(uint32_t RxPduId, PduInfoTRx* PduInfoPtr)){
	if(IF_Callback != NULL)
	{
		CanTp_Callback = IF_Callback ;  // 如果传入的回调函数指针不为空，则将其设置为接收回调函数
	}
}
















