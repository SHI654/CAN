#include "canif.h"
#include "can.h"

CAN_RxHeaderTypeDef rxHeader;  // 定义一个用于存储接收 CAN 消息头的变量。
PduInfoTRx CanIfPduInfo;  // 定义一个用于存储接收 CAN 消息数据的结构体变量。
extern volatile int8_t CanIf_Rx;  

const uint32_t TxPduIdToStdIdTable[] = {
    0x100,
    0x1C0,
    0x280,
    0x340,
    0x400,
    0x4C0,
    0x580,
    0x640,
    0x700,
    0x7C0,
    0x780,
    0x740,
    0x6C0,
    0x600,
    0x5A0,
    0x4E0,
    0x420,
    0x360,
    0x2A0,
    0x1E0
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

void CanIf_Receive(){
	uint32_t PDU_ID;  // 定义一个变量用于存储接收的 PDU ID
	while(1){
        // 检查是否接收到 CAN 消息
		if(CanIf_Rx){
            HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
			CanIf_Rx = 0;  // 重置接收标志
			CanIfPduInfo.Length = rxHeader.DLC;  // 将接收到的消息长度保存到 PDU 结构体中
            // 根据接收到的标准标识符，设置对应的 PDU ID
			switch(rxHeader.StdId)
			{
			case 0x100 :PDU_ID = 0;  // 标识符为 0x100 时，设置 PDU ID 为 0
			break;
			case 0x200 :PDU_ID = 1;  // 标识符为 0x200 时，设置 PDU ID 为 1
			break;
			case 0x300 :PDU_ID = 2;  // 标识符为 0x300 时，设置 PDU ID 为 2
			break;
			case 0x400 :PDU_ID = 3;  // 标识符为 0x400 时，设置 PDU ID 为 3
			break;
			case 0x500 :PDU_ID = 4;  // 标识符为 0x500 时，设置 PDU ID 为 4
			break;
			case 0x600 :PDU_ID = 5;  // 标识符为 0x600 时，设置 PDU ID 为 5
			break;
			case 0x700 :PDU_ID = 6;  // 标识符为 0x700 时，设置 PDU ID 为 6
			break;
			case 0x800 :PDU_ID = 7;  // 标识符为 0x800 时，设置 PDU ID 为 7
			break;
			}
        
            // 如果配置了接收回调函数，并且 PDU ID 为 0，则调用接收回调函数
			/*if(CanTp_Callback != NULL && PDU_ID == 0)
			{
				CanTp_Callback(PDU_ID, &CanIfPduInfo);
			}
			else if(CanNm_RxCallback != NULL && PDU_ID == 1){
                // 如果配置了网络管理接收回调函数，并且 PDU ID 为 1，则调用该回调函数
				CanNm_RxCallback();
			}*/
		}
        // 延迟 10 个时间单位，以避免占用过多的 CPU 资源
		vTaskDelay(10);
	}
}




/*// 如果发送成功，并且配置了传输完成回调，则调用传输完成回调
	else if(CanNm_TxCallback != NULL && TxPduId == 1){
        
		CanNm_TxCallback();
	}
	l*/
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);  // (注释掉的代码) 可以用于在发送消息后点亮 LED 等。