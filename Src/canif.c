#include "canif.h"
#include "can.h"
#include <stdint.h>
#include <stdbool.h>

CAN_RxHeaderTypeDef rxHeader;  
PduInfoTRx CanIfPduInfo;  
extern volatile int8_t CanIf_Rx;  
Std_ReturnType (*CanTp_Callback)(uint32_t RxPduId, PduInfoTRx* PduInfoPtr) = NULL; 

#define TX_PDU_ID_TO_STD_ID_TABLE_SIZE (sizeof(TxPduIdToStdIdTable) / sizeof(TxPduIdToStdIdTable[0]))
#define RX_PDU_ID_TO_STD_ID_TABLE_SIZE (sizeof(RxPduIdToStdIdTable) / sizeof(RxPduIdToStdIdTable[0]))

typedef enum {
    APPL,    
    NM,     
    DIAG   
} PduType;

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
    uint32_t PDU_ID;
    PduType PDU_TYPE;

    while (1) {
        if (CanIf_Rx) {
            CanIf_Rx = 0;
            CanIfPduInfo.Length = rxHeader.DLC;

            PDU_ID = -1;
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
                        Error_Handler();
                        break;
                }
            }
        }
        vTaskDelay(10);
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan1) {
    if (hcan1->Instance == hcan.Instance) {
        if (HAL_CAN_GetRxMessage(hcan1, CAN_RX_FIFO0, &rxHeader, CanIfPduInfo.Data) != HAL_OK) {
            Error_Handler();
        }
        CanIf_Rx = 1;
    }
}

/* if (CanNm_TxCallback != NULL && TxPduId == 1) {
        CanNm_TxCallback();
    } */
// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);  // 可以用于在发送消息后点亮 LED 等。

void CanIf_setCallback(Std_ReturnType (*IF_Callback)(uint32_t RxPduId, PduInfoTRx* PduInfoPtr)) {
    if (IF_Callback != NULL) {
        CanTp_Callback = IF_Callback;
    }
}
