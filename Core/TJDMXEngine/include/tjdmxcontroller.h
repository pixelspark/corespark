#ifndef _TJDMXCONTROLLER_H
#define _TJDMXCONTROLLER_H

#include "tjdmxinternal.h"
#include "tjdmxdevice.h"

namespace tj {
	namespace dmx {
		using namespace tj::shared;

		class DMX_EXPORTED DMXMacro {
			public:
				enum DMXMacroType {
					DMXMacroTypeNormal = 0,
					DMXMacroTypeMaster,
					DMXMacroTypeSubmix,
					DMXMacroTypeScene,
				};

				virtual ~DMXMacro();
				virtual void Set(float value) = 0;
				virtual float Get() const = 0;
				virtual std::wstring GetAddress() const = 0;
				virtual float GetResult() const = 0;
				virtual DMXMacroType GetType() const = 0;
				virtual float GetResultCached() const;
		};

		enum DMXSource {
			DMXManual = 0,
			DMXSequence,
		};

		// DMXSlot's are dmx channels; the first slot is 1.
		typedef unsigned int DMXSlot;

		class DMX_EXPORTED DMXController: public virtual Object {
			friend class DMXDevice;

			public:
				DMXController();
				virtual ~DMXController();
				
				/* Device management */
				void AddDevice(ref<DMXDevice> d);
				void RemoveDevice(ref<DMXDevice> d);
				bool IsDeviceEnabled(ref<DMXDevice> d) const;
				void ToggleDevice(ref<DMXDevice> d); // add it when it's not here, remove it if it is
				void AddDeviceClass(ref<DMXDeviceClass> dc);
				void RemoveDeviceClass(ref<DMXDeviceClass> dc);
				std::set< ref<DMXDeviceClass> >& GetDeviceClasses();

				/* Macro management */
				ref<DMXMacro> CreateMacro(std::wstring address, DMXSource src); 
				float GetMacroResult(const std::wstring& macro);
				bool IsMacroSubmix(const std::wstring& macro);
				void DestroyMacro(DMXMacro* macro);

				/* Universes */
				void SetUniverseCount(unsigned int c);
				unsigned int GetUniverseCount() const;
				void Reset();
				
				/* Channels */
				int GetTotalChannelCount() const;
				int GetChannelResult(DMXSlot ch);
				int GetChannelResultCached(DMXSlot ch);
				void Set(DMXSlot channel, int value, DMXSource source);
				void Set(const std::wstring& macro, float value, DMXSource source);
				int Get(DMXSlot channel, DMXSource src);
				int Get(const std::wstring& macro, DMXSource src);
				bool IsSwitching(DMXSlot channel) const;
				void SetSwitching(DMXSlot channel, bool s);
				void SetSwitching(const std::set<DMXSlot>& scs);
				void GetSwitching(std::set<DMXSlot>& scs) const;

				/* Grand master & sequence master */
				void SetGrandMaster(int value, DMXSource src);
				void SetSequenceMaster(int value, DMXSource src);
				int GetGrandMaster(DMXSource src);
				int GetSequenceMaster(DMXSource src);
				float GetSequenceMasterValue() const;
				float GetGrandMasterValue() const;

				/* Transmission */
				unsigned int GetModificationID() const;		
				void Process();
				unsigned char const* GetTransmitBuffer() const;

				/* Saving settings */
				virtual void Save(TiXmlElement* you);
				virtual void Load(TiXmlElement* you);

				DMXSlot _highestChannelUsed;
				static DMXSlot ParseChannelNumber(const std::wstring& n);

			protected:
				void SetChannelCount(unsigned int c, bool reset = false);

				struct MacroInfo {
					float _manual;
					float _sequence;
					bool _submix;

					inline MacroInfo(float m = 0.0f, float s = 0.0f, bool is = false) {
						_manual = m;
						_sequence = s;
						_submix = is;
					}
				};

				struct DMXChannel {
					DMXChannel() {
						_sequence = 0;
						_manual = 0;
						_dirty = true;
					}
					unsigned char _sequence;
					unsigned char _manual;
					bool _dirty;
				};
				
				void Transmit();
				
				// Locks
				CriticalSection _lock; // locks modification of values
				CriticalSection _transmitLock; // locks modification of transmit buffer

				// Dirty flags
				volatile unsigned int _modificationID;
				volatile bool _allDirty;
				volatile bool _anyDirty;

				// Values
				DMXChannel _grandMaster;
				DMXChannel _sequenceMaster;
				volatile DMXChannel* _values;
				std::map<std::wstring, MacroInfo > _macro;
				std::set< DMXSlot > _switchingSlots;
				unsigned char* _transmit;
				volatile unsigned int _channelCount;
				
				std::set< ref<DMXDevice> > _devices;
				std::set< ref<DMXDeviceClass> > _classes;
		};

		class DMX_EXPORTED DMXEngine {
			public:
				static strong<DMXController> GetController();

			protected:
				static ref<DMXController> _instance;
		};
	}
}

#endif