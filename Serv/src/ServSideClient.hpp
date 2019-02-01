#pragma once

#include	"Common.hpp"

#define		MAX_CHARS 1024

class		Server;


//////////////////////////////////////////////////
//	ServSideClient
//////////////////////////////////////////////////
class ServSideClient
{
public:

	ServSideClient	( SOCKET& acceptSocketTCP, const std::string userName, Server* server );
	~ServSideClient	( void );

	//////////////////////////////////////////////////

	SOCKET			GetSockRef				( void )						{ return *m_pClientSockTCP; }
	uInt			GetPeerPort				( void )						{ return m_PeerPort; }
	uInt			GetMsgType				( void )						{ return m_MsgType; }
	std::string&	GetPeerIP				( void )						{ return m_PeerIP; }
	std::string&	GetMsg					( void )						{ return m_Msg; }
	std::string&	GetName					( void )						{ return m_UserName; }
	std::string&	GetWhisperName			( void )						{ return m_whisperAtUserName; }

	void			SetMsg					( const std::string& msg )		{ m_Msg = msg; }
	void			SetUserName				( const std::string& userName )	{ m_UserName = userName; }

	void			StartRecvThreadTCP		( void );
	void			StartRecvThreadUDP		( void );

	void			Kill					( void );
	
	bool			GetSendingFile			( void )						{ return m_SendingFile; }
	void			SetSendingFile			( bool truefalse )				{ m_SendingFile=truefalse; }

	//////////////////////////////////////////////////

private:

	void	InitClientInfoTCP	( void );
	void	RecvTCP				( void );
	void	RecvUDP				( void );

	void	HandleTxt			( const uInt startIndex, const uInt recvSize );
	void	HandleTgaFile		( const uInt recvSize );
	void	HandleTgaChunk		( void );

	//////////////////////////////////////////////////

private:

	char		m_arrRecvMsg[ MAX_CHARS ];

	std::string	m_Msg;
	std::string	m_UserName;
	std::string	m_PeerIP;

	std::string	m_whisperAtUserName;

	std::thread	m_ThreadReceiveTCP;
	std::thread	m_ThreadReceiveUDP;

	std::mutex	m_Mutex;

	uInt		m_PeerPort;

	uInt		m_MsgType;

	SOCKET*		m_pClientSockTCP;
	SOCKET*		m_pClientSockUDP;

	Server*		m_pServer;	// No ownage // Don't delete, only nullptr

	bool		m_ClientIsAlive;
	bool		m_SendingFile;

};
