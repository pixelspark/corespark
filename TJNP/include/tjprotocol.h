#ifndef _TJ_NP_PROTOCOL_H
#define _TJ_NP_PROTOCOL_H

namespace tj {
	namespace np {
		/* Defines the TJShow network protocol data structures  */
		#pragma pack(push,1)

		// General protocol types
		typedef unsigned short Channel;
		typedef unsigned int PluginHash;
		typedef int InstanceID;
		typedef unsigned int OutletHash;
		typedef unsigned int TransactionIdentifier;

		// For device management & patching
		typedef std::wstring PatchIdentifier;
		typedef std::wstring DeviceIdentifier;
		
		// For input
		typedef int ChannelID;
		typedef int SubChannelID;

		/** TNP3 packet header (needs to be in public protocol header file because Stream/code writers use this **/
		struct PacketHeader {
			PacketHeader();

			char _version[4]; // 'T' 'N' 'P' '3'
			unsigned char _action;
			Channel _channel;
			PluginHash _plugin;
			TransactionIdentifier _transaction;
			InstanceID _from;
			unsigned short _size;
		};

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
			RoleMaster,
			RoleClient,
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

		/** All actions defined in the TNP protocol **/
		enum PacketAction {
			ActionNothing = 0,			// Do nothing
			ActionAnnounce = 1,			// Announce, see above
			ActionLeave = 2,			// When client leaves, see above
			ActionUpdate = 3,			// Update while playing
			ActionUpdatePlugin = 4,		// Update message (may or may not be while playing) meant for the plugin
			ActionSetAddress = 5,		// Set address of client
			ActionPushResource = 6,		// Sent by server to client to inform the client that a specific resource might be needed in the future
			ActionFindResource = 7,		// Sent by client when it detects that a needed resource is missing
			ActionAdvertiseResource = 8,// Sent by a fileserver when it has recieved a ActionFindResource message
			ActionReportError = 9,		// Sent by clients to report an error to the server

			ActionListDevices = 10,		// Sent by server to client to list devices
			ActionListDevicesReply = 11,	// Sent by clients to report a device (in response to ActionListDevices)
		
			ActionListPatches = 12,		// Sent by server to client to list patches
			ActionListPatchesReply = 13,// Sent by clients to server to report a patch (in response to ActionListPatches)
			ActionSetPatch = 14,		// Sent by server to client to set a patch
			ActionResetAll = 15,		// Sent by server to client to clear all patches and addressing (usually when loading a new show)
			ActionInput = 16,			// Sent by client to server when it has received input; [PatchIdentifier patch] [ChannelID cid] [SubChannelID scid] [float value]
			ActionPromoted = 17,		// Sent by a master when it asserts that it is the primary master
			ActionDemoted = 18,			// Sent by a master to notify failover masters that they can take over the network
			ActionAnnounceReply = 19,	// Announce reply
			ActionOutletChange = 20,	// Sent by a client to the master when it wants to change an outlet value (through Talkback) [ChannelID] [wstring outletID] [unsigned int (Any::Type) valueType] [double|bool|int|wstring value]
		};

		class Packet {
			public:
				Packet(const PacketHeader& ph, const char* message, unsigned int size);
				virtual ~Packet();
				unsigned int GetSize();
				PacketHeader GetHeader();
				const char* GetMessage();

				PacketHeader _header;
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