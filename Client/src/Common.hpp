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

#include	<mutex>
#include	<iostream>
#include	<string>
#include	<thread>
#include	<vector>
#include	<Windows.h>

#define		AUTO_DEL( x ) delete x, x = nullptr

using		uInt = unsigned int;
using		sInt = signed int;

//////////////////////////////////////////////////

enum eMsgType
{
	ALL = 0,
	WHISPER,
	TGA_FILE,
	TGA_CHUNK
};


// WHISPER		= @
// TGA_FILE		= / followed by dir & filename (including filetype)					// example: /dir/filename.tga
// TGA_CHUNK	= / followed by dir & filename (including filetype) & dimensions	// example: /dir/filename.tga/16*16*32*32