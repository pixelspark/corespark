#ifndef ENTTEC_DMX_H
#define ENTTEC_DMX_H

namespace enttec {
	#define GET_WIDGET_PARAMS 3
	#define GET_WIDGET_PARAMS_REPLY 3
	#define SET_WIDGET_PARAMS 4
	#define SET_DMX_RX_MODE 5
	#define SET_DMX_TX_MODE 6
	#define SEND_DMX 6
	#define SEND_DMX_RDM_TX 7
	#define RECEIVE_DMX_ON_CHANGE 8
	#define RECEIVED_DMX_COS_TYPE 9
	#define GET_WIDGET_SERIAL 10
	#define GET_WIDGET_SERIAL_REPLY 10

	#pragma pack(1)
	typedef struct {
			unsigned char FirmwareLSB;
			unsigned char FirmwareMSB;
			unsigned char BreakTime;
			unsigned char MaBTime;
			unsigned char RefreshRate;
	} DMXUSBPROParamsType;

	typedef struct {
			unsigned char UserSizeLSB;
			unsigned char UserSizeMSB;
			unsigned char BreakTime;
			unsigned char MaBTime;
			unsigned char RefreshRate;
	} DMXUSBPROSetParamsType;
	#pragma pack()

	struct ReceivedDmxCosStruct {
		unsigned char start_changed_byte_number;
		unsigned char changed_byte_array[5];
		unsigned char changed_byte_data[40];
	};
}


#endif