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

#ifndef __WEBCURSOR_H__
#define __WEBCURSOR_H__

namespace Awesomium {

enum WebCursor {
	CUR_ARROW,
    CUR_IBEAM,
    CUR_WAIT,
    CUR_CROSS,
    CUR_UPARROW,
    CUR_SIZE,
    CUR_ICON,
    CUR_SIZENWSE,
    CUR_SIZENESW,
    CUR_SIZEWE,
    CUR_SIZENS,
    CUR_SIZEALL,
    CUR_NO,
    CUR_HAND,
    CUR_APPSTARTING,
    CUR_HELP,
    CUR_ALIAS,
    CUR_CELL,
    CUR_COLRESIZE,
    CUR_COPYCUR,
    CUR_ROWRESIZE,
    CUR_VERTICALTEXT,
    CUR_ZOOMIN,
    CUR_ZOOMOUT,
    CUR_CUSTOM
};

}

#endif