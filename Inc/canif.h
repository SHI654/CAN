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

void CanIf_Transmit(uint32_t TxPduId, PduInfoTRx* PduInfoPtr);

#endif /* _CANIF_H_ */