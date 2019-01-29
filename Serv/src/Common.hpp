#pragma once

//////////////////////////////////////////////////
//	Winsock specifics
//////////////////////////////////////////////////
#ifndef		WIN32_LEAN_AND_MEAN
#define		WIN32_LEAN_AND_MEAN
#endif		//WIN32_LEAN_AND_MEAN

#define		_WINSOCK_DEPRECATED_NO_WARNINGS

#include	<windows.h>
#include	<winsock2.h>
#include	<ws2tcpip.h>
#include	<iphlpapi.h>

#pragma comment( lib, "Ws2_32.lib" )


//////////////////////////////////////////////////
//	Other things
//////////////////////////////////////////////////
#include	<mutex>
#include	<iostream>
#include	<string>
#include	<thread>
#include	<vector>
#include	<Windows.h>
#include	<queue>

#define		AUTO_DEL( x ) delete x, x = nullptr

using		uInt = unsigned int;
using		sInt = signed int;
