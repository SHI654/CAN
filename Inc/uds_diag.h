
#ifndef INC_APP_UDS_DIAG_H_
#define INC_APP_UDS_DIAG_H_


#include "stdint.h"
#include "can.h"
#include "gpio.h"
#include "string.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "CanIf.h"
#include "CanTp.h"
//#define NULL_Value 0
//For Security
//extern uint8_t* seed;
//extern uint8_t* key;

/*******************************************************************/

// select one target for MC

#define Tempreture_Add  	1
#define Pressure_Add    	2
#define Functional_Add  	3

#define	Global_Target 		Tempreture_Add


// select CAN_MODE for MC :
//Client
//Global_Target
#define Client 				0
#define CAN_MODE 			Global_Target

//Can Tp Structs & functions


#include "stdint.h"



typedef enum {
	Main_Menu = 0,
	Control_Session_Menu,
	Read_Data_Menu,
	Write_Data_Menu,
	Security_Access_Menu,
	Response_Permission_Menu,
	Target_Selection_Menu,

} ClientMenu;


//typedef enum {
//	Default_Session = 1,
//	Extended_Session=3,
//
//} SessionType;


typedef enum {
	Control_Service = 0x10,
	Read_Service = 0x22,
	Security_Service = 0x27,
	Write_Service = 0x2E,
	Tester_Representer_Service = 0x3E,

} Services;


typedef enum {
	Not_Available = 0x00,
	Available = 0x01,

} Availability;


typedef enum {
	Control_Service_Availability = Available,
	Read_Service_Availability = Available,
	Security_Service_Availability = Available,
	Write_Service_Availability = Available,
	Tester_Representer_Service_Availability = Available,

} Services_Availability;


typedef enum {
	SW_version = 0xF18C,
	Active_Session = 0xF186,
	VIN_number = 0xEDA0,
	SW_version_First_byte=0xF1,
	SW_version_Second_byte=0x8C,
	Active_Session_First_byte=0xF1,
	Active_Session_Second_byte=0x86,
	VIN_number_First_byte=0xED,
	VIN_number_Second_byte=0xA0,
}DID;




typedef enum {
	Secure = 0x01,
	Un_Secure = 0x00,

}Security_Access_State;


typedef enum {
	DefaultSession = 0x01,
	ExtendedSession = 0x03,
	Seed = 0x01,
	Key = 0x02,

}Sub_Fun;



// Struct For:
//void UDS_Send_Pos_Res(void);
//void UDS_Send_Neg_Res(void);

typedef struct {
	Services SID;
	DID did;
	Sub_Fun sub_fun;
	int8_t* data;

}Frame_Info;


typedef struct
{
	// uint8_t ADD_Source;
	// uint8_t ADD_Target;
	uint8_t SID;
	int8_t SUB_FUNC;
	int8_t DID[2];
	uint8_t DID_Length;
	int8_t Data[512];
	uint8_t Data_Length;
}ServiceInfo;

/*

typedef enum
{
	E_OK = 0x00,
	E_NOK
}Std_ReturnType;
 */

typedef enum
{
	Key_Code = 0x05,
	Seed_Key_Lenght = 4
}Security_Info;


typedef struct
{
	uint8_t* Seed;
	uint8_t* Key;
	Security_Access_State state;//enum
}Security_Key;

/*
typedef enum {
	PCI = 0,
	SID = 1,
	DID_1 = 2,
	DID_2 = 3,
	Sub_F = 2,
	Data_DID = 4 ,
	Data_Sub_Fun = 3 ,

	Neg_Res_INDEX=1,
	SID_NR_INDEX=2,
	NRC_INDEX=3,

} Indecis;

 */


typedef enum {
	// PCI, ADD_Source ,ADD_Target, SID ,(DID1,DID2) / Sub_F , Data.
	// PCI, ADD_Source ,ADD_Target, SID ,(DID1,DID2) / Sub_F , Data.

	Addition_length=2,
	PCI = 0,
	ADD_Source= 1,
	ADD_Target= 2,
	SID = 0,
	DID_1 = 1 ,
	DID_2 = 2 ,
	Sub_F = 1,
	Data_DID = 6 ,
	Data_Sub_Fun = 5 ,

	Neg_Res_INDEX=0,
	SID_NR_INDEX=1,
	NRC_INDEX=2,

}Indices;

typedef enum {
	Client_Address = 0xF1 ,
	Tempreture_Address= 0x3D ,
	Pressure_Address= 0x4D,
	Functional_Address=0xFF
}Addresses;


typedef enum
{
	NRC_WRITE_secuirty = 10,
	NRC_WRITE_defualt	=15,
	NRC_SID = 20,
	NRC_sub_fun = 30,
	NRC_sec_key_seed = 40


}NRC_VAR;


typedef enum
{
	serviceNotSupported	 = 0x11,
	subFunctionNotSupported	 = 0x12,
	serviceNotSupportedInActiveSession = 0x7F,
	invalidKey = 0x35,
	securityAccessDenied = 0x33,

	incorrectMessageLengthOrInvalidFormat = 0x13,
	RequestOutofRange = 0x31,  // when we want to read var didn't exist

}NRC_222;


//Std_ReturnType CanTp_Transmit(uint32_t TxPduId, PduInfoType* PduInfoPtr);
//Std_ReturnType CanTp_RxIndication (uint32_t RxPduId, PduInfoTRx* PduInfoPtr);


void UDS_Init(void);
//void UDS_Read_Data_Server( PduInfoType* PduInfoTypePtr );


//void UDS_Write_Data_Server(PduInfoType* Received_data);

void UDS_Send_Security_Client(Sub_Fun sub_fun);
//void UDS_Send_Security_Server(void);

void UDS_Tester_Presenter_Client(void);
//void UDS_Client_Callback(uint32_t RxPduId,PduInfoType *Ptr);

void sendHexArrayAsASCII(uint8_t* hexArray, uint16_t length);

void UART_ReceiveAndConvert(uint8_t RX_BUFFER_SIZE, PduInfoType* PduInfoType_Ptr);
uint8_t charToHex(uint8_t ascii);

void UDS_Write_Data_Server(uint8_t* received_data, uint16_t received_length);
void UDS_Send_Pos_Res(ServiceInfo* Response);
void UDS_Send_Neg_Res(uint8_t SID, uint8_t NRC);
void Sec_u32GetSeed (void);
uint32_t Sec_u32GetAlgorithm(void);
uint32_t Sec_u32GetKey (void);
uint8_t Sec_uint32SecurityAccess (PduInfoType * Ptr);
void UDS_Read_Data_Server(uint8_t* data);
void UDS_Control_Session_Server(uint8_t *Received);
void reset_timer(void);
void stop_timer(TIM_HandleTypeDef* htim);
void start_timer(void);
void UDS_Tester_Presenter_Server(void);
void UDS_MainFunction();
void server_call_back(uint32_t TxPduId, PduInfoType* ptr);
void UDS_Client_Callback(uint32_t TxPduId,PduInfoType *PduInfoPtr);

#endif /* INC_APP_UDS_DIAG_H_ */
