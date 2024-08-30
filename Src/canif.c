#include "canif.h"
#include "can.h"

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






/*// 如果发送成功，并且配置了传输完成回调，则调用传输完成回调
	else if(CanNm_TxCallback != NULL && TxPduId == 1){
        
		CanNm_TxCallback();
	}
	*/
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);  // (注释掉的代码) 可以用于在发送消息后点亮 LED 等。