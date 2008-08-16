#ifndef _TJPLATFORM_H
#define _TJPLATFORM_H

#ifdef _WIN32
	/** Playform specific headers not needed by every source unit **/
	#define TJ_DEFAULT_CLASS_NAME (L"TjWndClass")
	#define TJ_DROPSHADOW_CLASS_NAME (L"TjDropWndClass")
#endif

#endif