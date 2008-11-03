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

#ifndef __WEBCORE_H__
#define __WEBCORE_H__

#include "WebView.h"
#include <queue>
#include <vector>

namespace base { class Thread; class AtExitManager; }
class WebCoreProxy;
class WebViewEvent;
class Lock;

namespace Awesomium {

enum LogLevel
{
	LOG_NONE,		// No log is created
	LOG_NORMAL,		// Logs only errors
	LOG_VERBOSE		// Logs everything
};

/**
* The WebCore singleton manages the creation of WebViews, the internal worker thread,
* and various other global states that are required to embed Chromium.
*/
class _OSMExport WebCore
{
public:
	/**
	* Instantiates the WebCore singleton (you can access it later with
	* WebCore::Get or GetPointer).
	*
	* @param	level	The logging level to use (default is LOG_NORMAL).
	*/
	WebCore(LogLevel level = LOG_NORMAL);

	/**
	* Destroys the WebCore singleton.
	*/
	~WebCore();

	/**
	* Retrieves the WebCore singleton.
	*
	* @note	This will assert if the singleton is not instantiated.
	*
	* @return	Returns a reference to the instance.
	*/
	static WebCore& Get();

	/**
	* Retrieves the WebCore singleton.
	*
	* @return	Returns a pointer to the instance.
	*/
    static WebCore* GetPointer();

	/**
	* Sets the base directory.
	*
	* @param	baseDirectory	The absolute path to your base directory. The base directory is a 
	*							location that holds all of your local assets. It will be used for 
	*							WebView::loadFile and WebView::loadHTML (to resolve relative URL's).
	*/
	void setBaseDirectory(const std::string& baseDirectory);

	/**
	* Retrieves the base directory.
	*
	* @return	Returns the current base directory.
	*/
	std::string getBaseDirectory() const;

	/**
	* Creates a new WebView.
	*
	* @param	width	The width of the WebView in pixels.
	* @param	height	The height of the WebView in pixels.
	*
	* @return	Returns a pointer to the created WebView.
	*/
	WebView* createWebView(int width, int height);

	/**
	* Updates the WebCore and allows it to conduct various operations such as the propagation
	* of bound JS callbacks and the invocation of any queued listener events.
	*/
	void update();

protected:
	static WebCore* instance;
	base::Thread* coreThread;
	WebCoreProxy* coreProxy;
	std::queue<WebViewEvent*> eventQueue;
	Lock* eventQueueLock;
	Lock* baseDirLock;
	std::vector<WebView*> views;
	std::string baseDirectory;
	base::AtExitManager* atExitMgr;
	bool logOpen;

	void queueEvent(WebViewEvent* event);
	void removeWebView(WebView* view);

	friend class WebCoreProxy;
	friend class WebView;
	friend class WebViewProxy;
	friend class NamedCallback;
	friend std::string GetDataResource(int id);
};

}

#endif