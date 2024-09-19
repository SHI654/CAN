/*
 * CanTp.c
 *
 *  Created on: Jun 11, 2024
 *      Author: DELL
 */

#include "CanTp.h"  // 包含 CanTp 模块的头文件，声明了该模块的函数和数据结构。

#define RX_BUFFER_SIZE 100  // 定义接收缓冲区大小为 100 字节。
#define CAN_FRAME_LENGTH 8  // 定义 CAN 帧的长度为 8 字节。
#define CAN_MAX_PAYLOAD_LENGTH 7  // 定义 CAN 帧的最大有效负载长度为 7 字节（考虑到帧的控制字节）。

volatile uint32_t numberOfConsecutiveFramesToSend = 0;  // 定义一个用于存储要发送的连续帧数量的变量。
volatile uint32_t currentConsecutiveFrames = 0;  // 定义一个变量用于记录当前发送的连续帧数量。
volatile uint32_t numberOfConsecutiveFramesToReceive = 0;  // 定义一个用于存储要接收的连续帧数量的变量。
volatile uint32_t numberOfRemainingBytesToSend = 0;  // 定义一个变量用于记录剩余的需要发送的字节数。
volatile uint32_t numberOfRemainingBytesToReceive = 0;  // 定义一个变量用于记录剩余的需要接收的字节数。
volatile uint32_t availableBuffers = 10;  // 定义一个变量表示可用的缓冲区数量，初始值为 10。
void (*App_Callback)(uint32_t RxPduId, PduInfoType* PduInfoPtr) = NULL;  // 定义一个指向回调函数的指针，用于上层应用程序处理接收到的 PDU 数据。

Frame_States expectedFrameState = Any_State;  // 定义一个变量表示预期接收到的帧状态，初始值为任意状态。
PduInfoTRx EncodedPduInfo;  // 定义一个结构体用于存储编码后的 PDU 信息。
PduInfoTRx DecodedPduInfo;  // 定义一个结构体用于存储解码后的 PDU 信息。
PduInfoType CompletePduInfo;  // 定义一个结构体用于存储完整的 PDU 信息。
PduInfoTRx* GlobalRxPduInfoPtr;  // 定义一个指针用于指向全局接收 PDU 信息结构体。
PduInfoType* GlobalTxPduInfoPtr;  // 定义一个指针用于指向全局传输 PDU 信息结构体。

volatile uint8_t ConsecSN;  // 定义一个变量用于存储连续帧的序列号。
volatile uint16_t currentIndex;  // 定义一个变量用于存储当前索引（通常用于缓冲区管理）。
volatile int32_t currentOffset = -1;  // 定义一个变量用于存储当前的偏移量，初始值为 -1。
volatile int32_t startOffset;  // 定义一个变量用于存储起始偏移量。

volatile int8_t CanTp_Rx;  // 定义一个变量用于表示 CAN TP 接收状态。
volatile int8_t CanTp_Tx;  // 定义一个变量用于表示 CAN TP 发送状态。
volatile uint32_t GlobalRxPduId;  // 定义一个变量用于存储全局接收 PDU ID。
volatile uint32_t GlobalTxPduId;  // 定义一个变量用于存储全局传输 PDU ID。

uint8_t RxData[8];  // 定义一个数组用于存储接收到的数据，大小为 8 字节。
// uint8_t TxData[8];  // 定义一个数组用于存储要发送的数据，大小为 8 字节（已注释掉，可能会在后续使用）。

uint8_t rxBuffer[RX_BUFFER_SIZE];  // 定义一个接收缓冲区数组，大小为 RX_BUFFER_SIZE（100 字节）。
volatile int32_t rxBufferIndex = 0;  // 定义一个变量用于记录接收缓冲区的当前索引，初始值为 0。
volatile int32_t rxCurrentMaxIndex = 0;  // 定义一个变量用于记录接收缓冲区的最大索引，初始值为 0。

volatile uint8_t rxData = 0;  // 定义一个变量用于表示当前接收的数据。
volatile uint8_t rxComplete = 0;  // 定义一个变量用于表示接收是否完成。

// 初始化 CanTp 模块
void CanTp_Init(){
	CanIf_setCallback(CanTp_RxIndication);  // 调用 CanIf 模块的函数，设置 CanTp 模块的接收回调函数。
}

void CanTp_MainFunction(){
	while(1){  // 无限循环，不断轮询处理 CAN TP 的接收和发送操作。
		if(CanTp_Rx){  // 如果接收到新的 CAN TP 数据包。
			CanTp_Rx = 0;  // 清除接收标志，表示正在处理接收到的数据。
			
			// // 如果全局接收 PDU ID 不等于 0，则停止程序运行。
			// if(GlobalRxPduId != 0){
			// 	while(1);  // 进入死循环，通常用作错误处理或调试目的。
			// }

			// 获取当前接收帧的数据类型，根据帧的第一个字节确定帧类型。
			Frame_Type frame_type = CanTp_GetFrameType(GlobalRxPduInfoPtr->Data[0]);

			// 根据帧类型调用相应的解码函数，并处理数据。
			switch(frame_type){
			case Single_Frame:  // 如果帧类型是单帧。
				CanTp_decodeSingleFrame(GlobalRxPduId, GlobalRxPduInfoPtr);  // 解码单帧数据。
				break;
			case First_Frame:  // 如果帧类型是首帧。
				CanTp_decodeFirstFrame(GlobalRxPduId, GlobalRxPduInfoPtr);  // 解码首帧数据。
				expectedFrameState = FlowControl_Frame_State;  // 设置期望的下一个帧状态为流控制帧。
				CanTp_Transmit(GlobalRxPduId, (PduInfoType*) GlobalRxPduInfoPtr);  // 发送流控制帧，以便继续接收后续帧。
				break;
			case Consecutive_Frame:  // 如果帧类型是连续帧。
				CanTp_decodeConsecutiveFrame(GlobalRxPduId, GlobalRxPduInfoPtr);  // 解码连续帧数据。
				if(numberOfConsecutiveFramesToReceive == 0 && numberOfRemainingBytesToReceive > 0){
					// 如果接收到的连续帧数量为 0 且还有剩余字节未接收，发送流控制帧。
					expectedFrameState = FlowControl_Frame_State;  // 更新期望的帧状态。
					CanTp_Transmit(GlobalRxPduId, (PduInfoType*) GlobalRxPduInfoPtr);  // 发送流控制帧。
				}
				break;
			case FlowControl_Frame:  // 如果帧类型是流控制帧。
				// 解码流控制帧，并根据对方节点的空闲缓冲区数调整待发送的连续帧数量。
				CanTp_decodeFlowControlFrame(GlobalRxPduId, GlobalRxPduInfoPtr);
				break;
			default:
				break;  // 其他情况不做处理。
			}

			// 如果接收到的帧是流控制帧，进行特定处理（本段代码为空，可能预留扩展）。
			if(frame_type == FlowControl_Frame){

			}
			else if(numberOfRemainingBytesToReceive == 0){  // 如果所有数据都接收完毕。
				if(App_Callback != NULL){  // 如果应用层回调函数不为空。
					currentIndex = 0;  // 重置索引。
					App_Callback(GlobalRxPduId, &CompletePduInfo);  // 调用应用层回调函数，处理完整的数据。
				}
			}
		}
		else if(CanTp_Tx){  // 如果有新的 CAN TP 数据需要发送。
			// // 如果全局传输 PDU ID 不等于 0，则停止程序运行。
			// if(GlobalTxPduId != 0){
			// 	while(1);  // 进入死循环，通常用作错误处理或调试目的。
			// }

			Frame_Type frame_type = None;  // 初始化帧类型为 None。
			if(numberOfRemainingBytesToSend == 0 && expectedFrameState == Any_State){
				// 如果没有剩余字节需要发送且当前期望的帧状态为任意状态。
				numberOfRemainingBytesToSend = GlobalTxPduInfoPtr->Length;  // 更新剩余待发送字节数。
				CompletePduInfo.Length = numberOfRemainingBytesToSend;  // 更新完整 PDU 的长度。
				if(GlobalTxPduInfoPtr->Length < 8){
					frame_type = Single_Frame;  // 如果数据长度小于 8 字节，设置帧类型为单帧。
				}
				else{
					frame_type = First_Frame;  // 否则，设置帧类型为首帧。
				}
			}

			if(numberOfRemainingBytesToSend > 0 || expectedFrameState == FlowControl_Frame_State){
				// 如果有剩余字节需要发送或期望的帧状态为流控制帧状态。
				if(expectedFrameState == Consecutive_Frame_State){
					frame_type = Consecutive_Frame;  // 设置帧类型为连续帧。
				}
				else if(expectedFrameState == FlowControl_Frame_State){
					frame_type = FlowControl_Frame;  // 设置帧类型为流控制帧。
				}
				else{
					// 其他情况不做处理（可能用于扩展）。
				}

				// 根据帧类型调用相应的编码函数，编码数据并发送。
				switch(frame_type){
				case Single_Frame:
					CanTp_encodeSingleFrame(GlobalTxPduId, GlobalTxPduInfoPtr);  // 编码单帧数据并发送。
					break;
				case First_Frame:
					CanTp_encodeFirstFrame(GlobalTxPduId, GlobalTxPduInfoPtr);  // 编码首帧数据并发送。
					frame_type = None;  // 发送完首帧后，将帧类型重置为 None。
					break;
				case Consecutive_Frame:
					if(numberOfConsecutiveFramesToSend > 0){  // 如果仍有待发送的连续帧。
						numberOfConsecutiveFramesToSend--;  // 递减连续帧计数器。
						CanTp_encodeConsecutiveFrame(GlobalTxPduId, GlobalTxPduInfoPtr);  // 编码连续帧数据并发送。
					}
					else{
						frame_type = None;  // 如果没有连续帧待发送，重置帧类型。
						// 等待流控制帧接收中断通知来更新 numberOfConsecutiveFramesToSend 变量。
					}
					break;
				case FlowControl_Frame:
					// 根据 availableBuffers 变量（通常是接收数组的大小）决定如何发送流控制帧。
					CanTp_encodeFlowControlFrame(GlobalTxPduId, GlobalTxPduInfoPtr);
					break;
				default:
					break;  // 其他情况不做处理。
				}
			}

			// 如果所有数据发送完毕。
			if(numberOfRemainingBytesToSend == 0){
				// 重置期望帧状态。
				expectedFrameState = Any_State;
				currentOffset = -1;  // 重置当前偏移量。
				CanTp_Tx = 0;  // 清除发送标志，表示发送操作完成。
			}
		}
		vTaskDelay(100);  // 延迟 100 毫秒，控制主循环的执行频率。
	}
}

Std_ReturnType CanTp_Transmit(uint32_t TxPduId, PduInfoType* PduInfoPtr){
	GlobalTxPduInfoPtr = PduInfoPtr;  // 将传入的 PDU 信息指针保存到全局变量中，便于在其他函数中访问。
	GlobalTxPduId = TxPduId;  // 将传入的 PDU ID 保存到全局变量中。
	CanTp_Tx = 1;  // 设置发送标志，表示有数据需要发送。
	return E_OK;  // 返回标准返回值，表示操作成功。
}

Std_ReturnType CanTp_RxIndication (uint32_t RxPduId, PduInfoTRx* PduInfoPtr){
	GlobalRxPduInfoPtr = PduInfoPtr;  // 将接收到的 PDU 信息指针保存到全局变量中，便于在其他函数中访问。
	GlobalRxPduId = RxPduId;  // 将接收到的 PDU ID 保存到全局变量中。
	CanTp_Rx = 1;  // 设置接收标志，表示有数据需要处理。
	return E_OK;  // 返回标准返回值，表示操作成功。
}

Frame_Type CanTp_GetFrameType(uint8_t PCI){
	PCI >>= 4;  // 将 PCI 字节右移 4 位，提取帧类型信息（即 PCI 的高 4 位）。
	if(PCI < 4){  // 如果 PCI 的值小于 4，表示这是一个有效的帧类型。
		return (Frame_Type) PCI;  // 返回相应的帧类型枚举值。
	}
	else{
		return None;  // 否则，返回 `None`，表示无效的帧类型。
	}
}

void CanTp_setCallback(void (*PTF)(uint32_t TxPduId, PduInfoType* PduInfoPtr)){
	if(PTF != NULL){  // 如果传入的函数指针非空。
		App_Callback = PTF;  // 将其保存到全局变量 `App_Callback` 中，便于在接收数据时调用。
	}
}

void CanTp_encodeSingleFrame(uint32_t TxPduId, PduInfoType* PduInfoPtr){
    // 获取需要发送的数据的长度
    uint32_t dataLength = PduInfoPtr->Length;
    // 将要发送的数据长度保存在全局的编码信息结构体中
    EncodedPduInfo.Length = PduInfoPtr->Length;
    // 更新剩余要发送的字节数
    numberOfRemainingBytesToSend -= dataLength;

    // 设置 CAN 帧的第一个字节为 PCI（协议控制信息），其低 4 位为数据长度
    EncodedPduInfo.Data[0] = 0x00 | (dataLength & 0x0F);

    // 初始化一个循环计数变量 i
    uint32_t i;
    // 将数据从 PduInfoPtr 复制到 CAN 帧，从第二个字节（索引为 1）开始
    for (i = 0; i < dataLength; i++) {
        EncodedPduInfo.Data[i + 1] = PduInfoPtr->Data[i];
    }

    // 如果数据未填满整个 CAN 帧，将剩余的字节填充为零
    for (i = dataLength + 1; i < CAN_FRAME_LENGTH; i++) {
        EncodedPduInfo.Data[i] = 0;
    }

    // 调用 CanIf_Transmit 函数发送编码后的 CAN 帧
    CanIf_Transmit(TxPduId, &EncodedPduInfo);
}

void CanTp_encodeFirstFrame(uint32_t TxPduId, PduInfoType* PduInfoPtr){
    // 定义一个循环计数变量 Counter
    uint8_t Counter = 0;
    // 定义一个局部变量 EncodedPduInfo 来存储编码后的 PDU 信息
    PduInfoTRx EncodedPduInfo;

    // 设置 CAN 帧的第一个字节为帧类型（首帧），并且将数据长度的高 4 位编码进去
    EncodedPduInfo.Data[0] = (0x01 << 4) | ((PduInfoPtr->Length) >> 8 & 0x0F);
    // 设置 CAN 帧的第二个字节为数据长度的低 8 位
    EncodedPduInfo.Data[1] = (PduInfoPtr->Length) & 0xFF;

    // 将数据从 PduInfoPtr 复制到 CAN 帧，从第三个字节（索引为 2）开始
    for (Counter = 2; Counter < 8; Counter++) {
        EncodedPduInfo.Data[Counter] = PduInfoPtr->Data[Counter - 2];
    }

    // 更新剩余需要发送的字节数（总长度减去首帧中已发送的 6 个字节）
    numberOfRemainingBytesToSend = (PduInfoPtr->Length - 6);
    // 调用 CanIf_Transmit 函数发送编码后的 CAN 帧
    CanIf_Transmit(TxPduId, &EncodedPduInfo);
}

void CanTp_encodeConsecutiveFrame(uint32_t TxPduId, PduInfoType* PduInfoPtr){
    // 定义一个循环计数变量 i
    uint8_t i = 0;
    // 根据剩余字节数设置当前帧的长度，如果剩余字节数大于 7，则本帧传输 7 字节
    EncodedPduInfo.Length = numberOfRemainingBytesToSend > 7 ? 7 : numberOfRemainingBytesToSend;
    // 设置 CAN 帧的第一个字节为帧类型（连续帧），并将帧序号编码进去
    EncodedPduInfo.Data[0] = (0x02 << 4) | ConsecSN;

    // 计算当前数据的偏移量，即从哪里开始读取数据
    currentOffset = startOffset + ConsecSN * 7;

    // 将数据从 PduInfoPtr 复制到 CAN 帧，从第二个字节（索引为 1）开始
    for (i = 0; i < EncodedPduInfo.Length; i++) {
        EncodedPduInfo.Data[i + 1] = PduInfoPtr->Data[i + currentOffset];
    }

    // 增加帧序号，如果序号超过 0xF（即 15），则重置为 0 并更新起始偏移量
    ConsecSN++;
    if (ConsecSN > 0xF) {
        startOffset = currentOffset;
        ConsecSN = 0;
    }

    // 更新剩余需要发送的字节数
    numberOfRemainingBytesToSend -= EncodedPduInfo.Length;
    // 调用 CanIf_Transmit 函数发送编码后的 CAN 帧
    CanIf_Transmit(TxPduId, &EncodedPduInfo);
}

void CanTp_encodeFlowControlFrame(uint32_t TxPduId, PduInfoType* PduInfoPtr){
    // 设置流控制帧的第一个字节，流控状态为继续发送（CTS）
    EncodedPduInfo.Data[0] = 0x30;
    // 设置流控制帧的第二个字节为块大小，即可用缓冲区的数量
    EncodedPduInfo.Data[1] = availableBuffers;
    // 设置流控制帧的第三个字节为分离时间（ST），这里设为 0 毫秒
    EncodedPduInfo.Data[2] = 0x00;

    // 更新接收端可以接收的连续帧的数量
    numberOfConsecutiveFramesToReceive = availableBuffers;

    // 将流控帧中剩余的字节填充为 0
    for (uint8_t i = 3; i < 8; i++) {
        EncodedPduInfo.Data[i] = 0x00;
    }

    // 初始化帧序号为 1
    ConsecSN = 1;
    // 调用 CanIf_Transmit 函数发送编码后的流控帧
    CanIf_Transmit(TxPduId, &EncodedPduInfo);
}

void CanTp_decodeSingleFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr){
    // 从 CAN 帧的第一个字节中提取数据长度，数据长度在 PCI 的低 4 位
    uint32_t dataLength = PduInfoPtr->Data[0] & 0x0F;
    // 更新全局变量，记录剩余要接收的字节数
    numberOfRemainingBytesToReceive = dataLength;
    // 设置 CompletePduInfo 结构体中的长度为接收到的字节数
    CompletePduInfo.Length = numberOfRemainingBytesToReceive;

    // 初始化一个循环计数变量 i
    int i;
    // 将数据从接收到的 CAN 帧中复制到解码后的数据结构中
    for (i = 0; i < dataLength; i++) {
        DecodedPduInfo.Data[i] = PduInfoPtr->Data[i+1];
    }

    // 将解码后的数据长度设置为 dataLength
    DecodedPduInfo.Length = dataLength;

    // 调用 CanTp_ConnectData 函数，将解码后的数据与完整的 PDU 信息连接起来
    CanTp_ConnectData(&DecodedPduInfo);
}

void CanTp_decodeFirstFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr){
    // 从首帧的第一个字节的低 4 位和第二个字节提取数据长度
    numberOfRemainingBytesToReceive = ((PduInfoPtr->Data[0] & 0x0F) << 8) | PduInfoPtr->Data[1];
    // 设置 CompletePduInfo 结构体中的长度为接收到的总字节数
    CompletePduInfo.Length = numberOfRemainingBytesToReceive;
    // 首帧中实际有效负载数据长度为 6 个字节
    DecodedPduInfo.Length = 6;

    // 定义一个循环计数变量 Counter
    uint8_t Counter = 0;

    // 将首帧中的数据复制到解码后的数据结构中，从 CAN 帧的第三个字节（索引为 2）开始
    for (Counter = 0; Counter < 8; Counter++) {
        DecodedPduInfo.Data[Counter] = PduInfoPtr->Data[Counter + 2];
    }

    // 调用 CanTp_ConnectData 函数，将解码后的数据与完整的 PDU 信息连接起来
    CanTp_ConnectData(&DecodedPduInfo);
}

void CanTp_decodeConsecutiveFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr){
    // 每收到一个连续帧，就减少剩余要接收的帧数
    numberOfConsecutiveFramesToReceive--;
    // 初始化一个循环计数变量 i
    uint8_t i = 0;
    // 设置解码后的帧长度为 7 字节或剩余字节数中的较小者
    DecodedPduInfo.Length = numberOfRemainingBytesToReceive > 7 ? 7 : numberOfRemainingBytesToReceive;

    // 检查连续帧的序号是否与当前期望的序号匹配
    if (ConsecSN == (PduInfoPtr->Data[0] & 0x0F)) {
        // 将数据从接收到的 CAN 帧中复制到解码后的数据结构中
        for (i = 0; i < DecodedPduInfo.Length; i++) {
            DecodedPduInfo.Data[i] = PduInfoPtr->Data[i+1];
        }

        // 更新序号，如果序号超过 0xF（即 15），则重置为 0，否则递增
        ConsecSN = ConsecSN + 1 > 0xF ? 0 : ConsecSN + 1;

        // 调用 CanTp_ConnectData 函数，将解码后的数据与完整的 PDU 信息连接起来
        CanTp_ConnectData(&DecodedPduInfo);
    }
}

void CanTp_decodeFlowControlFrame(uint32_t RxPduId, PduInfoTRx* PduInfoPtr){
    // 从接收到的 PDU 中提取流控制状态、块大小和分离时间（ST）
    uint8_t flowStatus = PduInfoPtr->Data[0];
    uint8_t blockSize = PduInfoPtr->Data[1];

    // 更新可以发送的连续帧的数量
    numberOfConsecutiveFramesToSend = blockSize;

    // 根据不同的流控制状态执行相应操作
    switch (flowStatus) {
        case 0x30:  // 继续发送 (CTS)
            // 更新期望的帧状态为连续帧
            expectedFrameState = Consecutive_Frame_State;
            // 更新当前偏移量为起始偏移量
            startOffset = currentOffset;
            // 初始化帧序号为 1
            ConsecSN = 1;
            break;

        case 0x31:  // 等待 (WT)
            // 流控制状态为等待
            expectedFrameState = FlowControl_Frame_State;
            break;

        case 0x32:  // 溢出/中止 (OVFLW/ABORT)
            // 处理溢出或中止条件
            // 更新期望的帧状态为任意状态（如需要）
            // expectedFrameState = Any_State;
            // 执行额外的中止操作，如通知上层协议
            break;

        default:
            // 处理无效的流控制状态，如设置错误状态
            // expectedFrameState = Any_State;
            break;
    }
}

void CanTp_ConnectData(PduInfoTRx* PduInfoPtr){
    // 使用 CompletePduInfo 结构体，将接收到的数据与完整的 PDU 信息连接起来
    // 定义一个临时变量 tempCurrentIndex，保存当前索引
    uint16_t tempCurrentIndex = currentIndex;
    
    // 将接收到的 PDU 数据逐字节连接到 CompletePduInfo 数据中
    while (currentIndex < PduInfoPtr->Length + tempCurrentIndex) {
        CompletePduInfo.Data[currentIndex] = PduInfoPtr->Data[currentIndex - tempCurrentIndex];
        // 更新当前索引
        currentIndex++;
    }

    // 更新剩余需要接收的字节数
    numberOfRemainingBytesToReceive -= PduInfoPtr->Length;
}

