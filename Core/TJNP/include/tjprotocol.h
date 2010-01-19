#ifndef _TJ_NP_PROTOCOL_H
#define _TJ_NP_PROTOCOL_H

#include "tjnpinternal.h"

namespace tj {
	namespace np {
		/* Defines the TJShow network protocol data structures  */
		#pragma pack(push,1)

		// General protocol types
		typedef unsigned int ReliablePacketID;
		typedef unsigned int ShowID;
		typedef unsigned short GroupID;
		typedef unsigned short Channel;
		typedef unsigned int PluginHash;
		typedef int InstanceID;
		typedef unsigned int OutletHash;
		typedef unsigned int TransactionIdentifier;

		// For device management & patching
		typedef std::wstring PatchIdentifier;
		typedef std::wstring DeviceIdentifier;
		
		/* For input; InputID is a 'path' that the device can use to identify a particular input channel
		For example, MIDI input plug-ins can use paths like '/cc/123' or '/note/12' */
		typedef std::wstring InputID;

		/** A RunMode specifies in which way a track is played **/
		enum RunMode {
			RunModeDont = 0, /** Don't play this track **/
			RunModeBoth = 3, /** Track is played on master and client at the same time **/
			RunModeClient = 1, /** Track is played on client only **/
			RunModeMaster = 2, /** Track is played on master only **/
		};

		enum PlaybackState {
			PlaybackStop = 0,
			PlaybackPlay,
			PlaybackPause, // is PlaybackPlay, without ticking
			PlaybackWaiting,
			PlaybackAny, // Use in Controller::RecursiveSetState()
		};

		/* Defines the TJShow network protocol (private part) */
		enum Role {
			RoleNone = 0,
			RoleMaster = 1,
			RoleClient = 2,
		};

		/* Feature codes are used to tell the server which features are on a specific client. They
		are also used in error reporting, to tell the server in which component the error occurred */
		enum Feature {
			FeaturesUnknown = 1,
			FeatureFileServer = 2,
			FeatureBatteryPower = 4,
			FeatureACPower = 8,
			FeatureBackupPower = 16,
			FeaturePlugin = 32,
			FeatureEventLogger = 64,
			FeaturePrimaryMaster = 128,
			FeatureAutomaticFailover = 256,
			FeatureTouchScreen = 512,
		};

		typedef unsigned int Features;

		/** Packet flags **/
		typedef unsigned char PacketFlags;
		const static PacketFlags PacketFlagReliable =			0x01;
		const static PacketFlags PacketFlagRequestRedelivery =	0x02;
		const static PacketFlags PacketFlagCannotRedeliver =	0x04;
		const static PacketFlags PacketFlagRedelivery =			0x08;
		

		/** All actions defined in the TNP protocol **/
		typedef unsigned char PacketAction;
		const static PacketAction ActionNothing = 0;		// Do nothing
		const static PacketAction ActionAnnounce = 1;		// Announce, see above
		const static PacketAction ActionLeave = 2;			// When client leaves, see above
		const static PacketAction ActionUpdate = 3;			// Update while playing
		const static PacketAction ActionUpdatePlugin = 4;	// Update message (may or may not be while playing) meant for the plugin
		const static PacketAction ActionSetAddress = 5;		// Set address of client
		const static PacketAction ActionPushResource = 6;	// Sent by server to client to inform the client that a specific resource might be needed in the future
		const static PacketAction ActionFindResource = 7;	// Sent by client when it detects that a needed resource is missing
		const static PacketAction ActionAdvertiseResource = 8;// Sent by a fileserver when it has recieved a ActionFindResource message
		const static PacketAction ActionReportError = 9;	// Sent by clients to report an error to the server

		const static PacketAction ActionListDevices = 10;	// Sent by server to client to list devices
		const static PacketAction ActionListDevicesReply = 11;	// Sent by clients to report a device (in response to ActionListDevices)
	
		const static PacketAction ActionListPatches = 12;	// Sent by server to client to list patches
		const static PacketAction ActionListPatchesReply = 13;// Sent by clients to server to report a patch (in response to ActionListPatches)
		const static PacketAction ActionSetPatch = 14;		// Sent by server to client to set a patch
		const static PacketAction ActionResetAll = 15;		// Sent by server to client to clear all patches and addressing (usually when loading a new show)
		const static PacketAction ActionInput = 16;		// Sent by client to server when it has received input; [PatchIdentifier patch] [ChannelID cid] [SubChannelID scid] [float value]
		const static PacketAction ActionPromoted = 17;		// Sent by a master when it asserts that it is the primary master
		const static PacketAction ActionDemoted = 18;		// Sent by a master to notify failover masters that they can take over the network
		const static PacketAction ActionAnnounceReply = 19;// Announce reply
		const static PacketAction ActionOutletChange = 20;	// Sent by a client to the master when it wants to change an outlet value (through Talkback) [ChannelID] [wstring outletID] [unsigned int (Any::Type) valueType] [double|bool|int|wstring value]
		const static PacketAction ActionResetChannel = 21;	// Sent by server to client to reset a channel

		/** T4 packet header (needs to be in public protocol header file because Stream/code writers use this **/
		struct PacketHeader {
			PacketHeader();

			char _version[2]; // 'T' '4'
			PacketFlags _flags;
			PacketAction _action;
			ReliablePacketID _rpid;
			GroupID _group;
			Channel _channel;
			PluginHash _plugin;
			TransactionIdentifier _transaction;
			InstanceID _from;
			unsigned int _size;
		};

		class Packet {
			public:
				Packet(const PacketHeader& ph, const char* message, unsigned int size);
				Packet(char* buf, unsigned int size);
				virtual ~Packet();
				unsigned int GetSize();
				PacketHeader GetHeader();
				const char* GetMessage();

				PacketHeader* _header;
				char* _message;
				unsigned int _size;

				const static int maximumSize = 2048;
		};

		struct Update {
			Channel _channel;
			unsigned int _size;
		};

		#pragma pack(pop)	
	}
}

#endif