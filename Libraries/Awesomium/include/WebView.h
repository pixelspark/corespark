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

#ifndef __WEBVIEW_H__
#define __WEBVIEW_H__

#include "WebViewListener.h"
#include "Callback.h"
#include <map>

#if defined(_WIN32)
#include <windows.h>
#pragma warning( disable: 4251 )
#endif

class WebViewWaitState;
class WebViewProxy;
namespace base { class Thread; }
namespace WebViewEvents { class InvokeCallback; }

namespace Awesomium {

enum MouseButton {
	LEFT_MOUSE_BTN,
	MIDDLE_MOUSE_BTN,
	RIGHT_MOUSE_BTN
};

struct _OSMExport Rect {
	int x, y, width, height;

	Rect();
	Rect(int x, int y, int width, int height);
	bool isEmpty() const;
};

/**
* A WebView is is essentially a single instance of a web-browser (created via the WebCore singleton)
* that you can interact with (via input injection, javascript, etc.) and render to an off-screen buffer.
*/
class _OSMExport WebView
{
public:

	/**
	* Explicitly destroys this WebView instance. If you neglect to call this, the WebCore singleton
	* will automatically destroy all lingering WebView instances at shutdown.
	*/
	void destroy();

	/**
	* Registers a WebViewListener to call upon various events (such as load completions, title reception,
	* cursor changes, etc).
	*
	* @param	listener	The WebViewListener to register. Or, you can pass '0' to undo any current registrations.
	*/
	void setListener(WebViewListener* listener);

	/**
	* Retrieves the current WebViewListener.
	*
	* @return	If a WebViewListener is registered, returns a pointer to the instance, otherwise returns 0.
	*/
	WebViewListener* getListener();

	/**
	* Loads a URL into the WebView asynchronously.
	*
	* @param	url	The URL to load.
	*/
	void loadURL(const std::string& url);

	/**
	* Loads a string of HTML into the WebView asynchronously.
	*
	* @param	html	The HTML string to load.
	*
	* @note	The base directory (specified via WebCore::setBaseDirectory) will be used to resolve
	*		relative URL's/resources (such as images, links, etc).
	*/
	void loadHTML(const std::string& html);

	/**
	* Loads a local file into the WebView asynchronously.
	*
	* @param	file	The file to load.
	*
	* @note	The file should exist within the base directory (specified via WebCore::setBaseDirectory).
	*/
	void loadFile(const std::string& file);

	/**
	* Executes a string of Javascript in the context of the current page.
	*
	* @param	javascript	The string of Javascript to execute.
	*/
	void executeJavascript(const std::string& javascript);

	/**
	* Sets a global 'Client' property that can be accessed via Javascript from
	* within all pages loaded into this web-view.
	*
	* @param	name	The name of the property.
	* @param	value	The javascript-value of the property.
	*
	* @note	You can access all properties you set via the 'Client' object using Javascript. For example,
	*		if you set the property with a name of 'color' and a value of 'blue', you could access
	*		this from the page using Javascript: document.write("The color is " + Client.color);
	*/
	void setProperty(const std::string& name, const JSValue& value);

	/**
	* Sets a global 'Client' callback that can be invoked via Javascript from
	* within all pages loaded into this view.
	*
	* @param	name	The name of the callback.
	* @param	callback	The C++ callback to invoke when called via Javascript.
	*
	* @note	All C++ callbacks should follow the general form of:
	*		void myCallback(Awesomium::WebView* webView, const Awesomium::JSArgs& args)
	*		{
	*		}
	*
	*		An example of specifying a function as a callback:
	*			myView->setCallback("itemSelected", &onItemSelected);
	*
	*		An example of specifying a member function as a callback:
	*			myView->setCallback("itemSelected", Awesomium::Callback(this, &MyClass::onItemSelected));
	*
	*		An example of calling a callback from Javascript:
	*			Client.itemSelected();
	*/
	void setCallback(const std::string& name, const Callback& callback);

	/**
	* Returns whether or not the current web-view is dirty and needs to be re-rendered.
	*
	* @return	If the web-view is dirty, returns true, otherwise returns false.
	*/
	bool isDirty();
	
	/**
	* Renders the web-view to an off-screen buffer.
	*
	* @param	destination	The buffer to render to, it's dimensions should match the WebView's.
	* @param	destRowSpan	The row-span of the destination buffer (number of bytes per row).
	* @param	destDepth	The depth (bytes per pixel) of the destination buffer. Valid options
	*						include 3 (BGR) or 4 (BGRA).
	* @param	renderedRect	Optional (pass 0 to ignore); a pointer to a Rect to store the dimensions
	*							of the rendered area, or rather, the dimensions of the area that actually
	*							changed since the last render.
	*/
	void render(unsigned char* destination, int destRowSpan, int destDepth, Awesomium::Rect* renderedRect = 0);

	/**
	* Injects a mouse-move event in local coordinates.
	*
	* @param	x	The absolute x-coordinate of the mouse (localized to the WebView).
	* @param	y	The absolute y-coordinate of the mouse (localized to the WebView).
	*/
	void injectMouseMove(int x, int y);

	/**
	* Injects a mouse-down event.
	*
	* @param	button	The button that was pressed.
	*/
	void injectMouseDown(Awesomium::MouseButton button);

	/**
	* Injects a mouse-up event.
	*
	* @param	button	The button that was released.
	*/
	void injectMouseUp(Awesomium::MouseButton button);

#if defined(_WIN32)
	/**
	* Injects a keyboard event.
	*
	* @note	The native Windows keyboard message should be passed, valid message types include:
	*		- WM_KEYDOWN
	*		- WM_KEYUP
	*		- WM_SYSKEYDOWN
	*		- WM_SYSKEYUP
	*		- WM_CHAR
	*		- WM_IMECHAR
	*		- WM_SYSCHAR
	*/
	void injectKeyboardEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
#endif

	/**
	* Invokes a 'cut' action using the system clipboard.
	*/
	void cut();

	/**
	* Invokes a 'copy' action using the system clipboard.
	*/
	void copy();

	/**
	* Invokes a 'paste' action using the system clipboard.
	*/
	void paste();

	/**
	* Selects all items on the current page.
	*/
	void selectAll();

	/**
	* De-selects all items on the current page.
	*/
	void deselectAll();

protected:
	WebView(int width, int height, base::Thread* coreThread);
	~WebView();

	void startup();
	void setDirtiness(bool dirtiness);
	void setFinishRender();
	void invokeCallback(const std::string& name, const JSArguments& args);

	base::Thread* coreThread;
	WebViewProxy* viewProxy;
	WebViewWaitState* waitState;
	WebViewListener* listener;
	std::map<std::string, Callback> callbackMap;
	bool dirtiness;

	friend class WebCore;
	friend class WebViewProxy;
	friend class WebViewEvents::InvokeCallback;
};

}

#endif