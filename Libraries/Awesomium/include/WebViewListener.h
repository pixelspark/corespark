/*
	This file is part of Awesomium, a library that helps facilitate
	the rendering of Web Content in an off-screen context.

	Copyright (C) 2008 Adam J. Simmons

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __WEBVIEWLISTENER_H__
#define __WEBVIEWLISTENER_H__

#include <string>
#include "PlatformUtils.h"
#include "WebCursor.h"

namespace Awesomium {

class _OSMExport WebViewListener
{
public:
	virtual void onBeginNavigation(const std::string& url) = 0;

	virtual void onBeginLoading() = 0;
	virtual void onFinishLoading() = 0;
	
	virtual void onReceiveTitle(const std::wstring& title) = 0;

	virtual void onChangeCursor(Awesomium::WebCursor cursor) = 0;
};

}

#endif