#ifndef _TJPCH_H
#define _TJPCH_H

#undef _WIN32_IE
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0600
#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1

#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <set>
#include <list>
#include <iomanip>
#include <fstream>
#include <deque>
#include <math.h>

#include <windows.h>
#include <Gdiplus.h>

interface IGraphBuilder;
interface IPin;
interface IBaseFilter;

// TODO: move tinyxml to other folder
#include "../../Libraries/tinyxml.h"

#endif