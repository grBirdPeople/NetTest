#pragma once

#include	"Common.hpp"

#define		MAX_CHARS 128

class		Server;


//////////////////////////////////////////////////
//	ServSideClient
//////////////////////////////////////////////////
class ServSideClient
{
public:

	ServSideClient	( SOCKET& acceptSocket, const std::string userName, Server* server );
	~ServSideClient	( void );

	//////////////////////////////////////////////////

	SOCKET			GetSockRef				( void )						{ return *m_pClientSock; }
	uInt			GetPeerPort				( void )						{ return m_PeerPort; }
	uInt			GetMsgType				( void )						{ return m_MsgType; }
	std::string&	GetPeerIP				( void )						{ return m_PeerIP; }
	std::string&	GetMsg					( void )						{ return m_Msg; }
	std::string&	GetName					( void )						{ return m_UserName; }
	std::string&	GetWhisperName			( void )						{ return m_whisperAtUserName; }

	void			SetMsg					( const std::string& msg )		{ m_Msg = msg; }
	void			SetUserName				( const std::string& userName )	{ m_UserName = userName; }

	void			StartRecievingThread	( void );

	void			Kill					( void );

	//////////////////////////////////////////////////

private:

	void	InitClientInfo			( void );
	void	ReceiveFromClientSide	( void );

	void	HandleTxt				( const uInt startIndex, const uInt recvSize );
	void	HandleTgaFile			( void );
	void	HandleTgaChunk			( void );

	//////////////////////////////////////////////////

private:

	char		m_arrRecvMsg[ MAX_CHARS ];

	std::string	m_Msg;
	std::string	m_UserName;
	std::string	m_PeerIP;

	std::string	m_whisperAtUserName;

	std::thread	m_ThreadReceive;

	std::mutex	m_Mutex;

	uInt		m_PeerPort;

	uInt		m_MsgType;

	SOCKET*		m_pClientSock;

	Server*		m_pServer;	// No ownage // Don't delete, only nullptr

	bool		m_ClientIsAlive;

};
