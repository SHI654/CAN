#ifndef _CANIF_H_
#define _CANIF_H_

#include "stdint.h"
#include "can.h"
//#include "cmsis_os.h"


typedef struct
{
	uint8_t Data[8];
	uint32_t Length;
}PduInfoTRx;

typedef enum
{
	E_OK=0,
	E_NOK
}Std_ReturnType;

void CanIf_Transmit(uint32_t TxPduId, PduInfoTRx* PduInfoPtr);
void CanIf_setCallback(Std_ReturnType (*IF_Callback)(uint32_t RxPduId, PduInfoTRx* PduInfoPtr));
#endif /* _CANIF_H_ */