#include "../include/tjsettings.h"
#include "../include/tjfile.h"
using namespace tj::shared;

#ifdef TJ_OS_MAC
#include <Foundation/NSPathUtilities.h>
#include <Foundation/Foundation.h>

String SettingsStorage::GetSettingsPath(const String& vendor, const String& app, const String& file) {
	NSAutoreleasePool* ap = [[NSAutoreleasePool alloc] init];
	NSArray* arr = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	std::wostringstream path;
	bool ok = false;
	if(arr!=nil && [arr count]>0) {
		NSString* firstPath = [arr objectAtIndex:0];
		if(firstPath!=nil) {
			path << Util::MacStringToString((CFStringRef)firstPath) << L"/";
			ok = true;
		}
	}
	[ap release];
	wchar_t sep = File::GetPathSeparator();
	path << vendor << sep << app << sep << file << L".xml";
	return path.str();
}

String SettingsStorage::GetSystemSettingsPath(const String& vendor, const String& app, const String& file) {
	NSAutoreleasePool* ap = [[NSAutoreleasePool alloc] init];
	NSArray* arr = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSLocalDomainMask, YES);
	std::wostringstream path;
	bool ok = false;
	if(arr!=nil && [arr count]>0) {
		NSString* firstPath = [arr objectAtIndex:0];
		if(firstPath!=nil) {
			path << Util::MacStringToString((CFStringRef)firstPath) << L"/";
			ok = true;
		}
	}
	[ap release];
	wchar_t sep = File::GetPathSeparator();
	path << vendor << sep << app << sep << file << L".xml";
	return path.str();
}

#endif
