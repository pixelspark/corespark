#ifndef _TJSOUNDLIGHT_H
#define _TJSOUNDLIGHT_H

namespace tj {
	namespace dmx {
		namespace soundlight {
			/** Defines for the dashard.dll SoundLight driver. This defines:
			- Command values (DHC_*) 
			- Parameter names (DHP_)
			- Errors (DHE_*) 
			**/

			class SoundLightConstants {
				public:
					const static int DeviceUsbDMX2 = 11;
					const static int DeviceUsbDMX1 = 3;

					const static int DHC_SIUDI0	= 0;
					const static int DHC_SIUDI1 = 100;		
					const static int DHC_SIUDI2 = 200;		
					const static int DHC_SIUDI3 = 300;		
					const static int DHC_SIUDI4 = 400;		
					const static int DHC_SIUDI5 = 500;		
					const static int DHC_SIUDI6 = 600;		
					const static int DHC_SIUDI7 = 700;		
					const static int DHC_SIUDI8 = 800;		
					const static int DHC_SIUDI9 = 900;		
					 
					const static int DHC_OPEN = 1;		
					const static int DHC_CLOSE = 2;		
					const static int DHC_DMXOUTOFF = 3;		
					const static int DHC_DMXOUT = 4;		
					const static int DHC_PORTREAD = 5;		
					const static int DHC_PORTCONFIG = 6;		
					const static int DHC_VERSION = 7;		
					const static int DHC_DMXIN = 8;		

					const static int DHC_RESET = 11;		
					const static int DHC_DEBUG_OPEN = 12;		
					const static int DHC_DEBUG_CLOSE = 13;		

					const static int DHC_WRITEMEMORY = 21;		
					const static int DHC_READMEMORY = 22;		
					const static int DHC_SIZEMEMORY = 23;		

					const static int DHC_3DWRITE = 25;		
					const static int DHC_3DREAD = 26;		
					const static int DHC_MMWRITE = 27;		
					const static int DHC_MMREAD = 28;		

					const static int DHC_TRANSPORT = 30;		
					const static int DHP_TRANSPORT_MODEALW = 1;		
					const static int DHP_TRANSPORT_MODEALW32 = 2;		
					const static int DHP_TRANSPORT_MODEOPT = 3;
					const static int DHP_TRANSPORT_MODEOPT32 = 4;

					const static int DHC_SERIALNREAD = 47;
					const static int DHC_SERIALNWRITE = 48;

					const static int DHE_OK = 1;
					const static int DHE_NOTHINGTODO = 2;		

					const static int DHE_ERROR_COMMAND = -1;
					const static int DHE_ERROR_NOTOPEN = -2;
					const static int DHE_DMXOUT_PACKWRITE = -1000;
					const static int DHE_DMXOUT_PACKREAD = -1100;
			};
		}
	}
}

#endif