#ifdef TJ_OS_WIN
	#ifndef _TJDMXFORALLDEVICE_H
	#define _TJDMXFORALLDEVICE_H

	#include "../tjdmxinternal.h"
	#include "../tjdmxdevice.h"

	namespace tj {
		namespace dmx {
			namespace devices {
				using namespace tj::shared;
				using namespace tj::dmx;

				class DMX_EXPORTED DMXForAllDevice: public DMXDevice {
					friend class DMXController;

					public:
						DMXForAllDevice(HMODULE module);
						virtual ~DMXForAllDevice();
						virtual std::wstring GetPort();
						virtual std::wstring GetDeviceID();
						virtual std::wstring GetDeviceInfo();
						virtual std::wstring GetDeviceSerial();
						virtual std::wstring GetDeviceName() const;
						virtual unsigned int GetSupportedUniversesCount();

						#ifdef TJ_DMX_HAS_TJSHAREDUI
							virtual ref<PropertySet> GetProperties();
						#endif
						
						virtual void Save(TiXmlElement* you);
						virtual void Load(TiXmlElement* you);
						
					protected:
						virtual void Connect();
						virtual void OnTransmit(ref<DMXController> controller);

						// Procedures in the DLL
						struct VERSION_INFO {
							char		InterfaceInfo[500];
							DWORD		HW_Version;
							DWORD		NrOfDmxData;
							BOOL		SignalOutput;
							BYTE		MergeMode;
							WORD		MergeStart;
							WORD		MergeStop;
						};

						typedef bool (__stdcall *DMXForAll_FindInterface)(int maxPortNumber);
						typedef bool (__stdcall *DMXForAll_SetDMX)(int firstChannel, int numBytes, const unsigned char* pData);
						typedef bool (__stdcall *DMXForAll_GetInfo)(VERSION_INFO* vi);
						typedef bool (__stdcall *DMXForAll_GetProductID)(int* pid);
						typedef bool (__stdcall *DMXForAll_GetCOMParam)(int*, DWORD*);

						int _universe;
						DMXForAll_FindInterface PFindInterface;
						DMXForAll_SetDMX PSetDMX;
						DMXForAll_GetInfo PGetInfo;
						DMXForAll_GetProductID PGetProductID;
						DMXForAll_GetCOMParam PGetCOMParam;
						bool _connected;
				};

				class DMX_EXPORTED DMXForAllDeviceClass: public DMXDeviceClass {
					public:
						DMXForAllDeviceClass();
						virtual ~DMXForAllDeviceClass();
						virtual ref< std::vector< ref<DMXDevice> > > GetAvailableDevices();

					protected:
						ref<DMXForAllDevice> _device;
						HMODULE _module;
				};
			}
		}
	}

	#endif
#endif