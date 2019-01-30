#pragma once

#include	"Common.hpp"

#define		MAX_CHARS 128


//////////////////////////////////////////////////
//	Client
//////////////////////////////////////////////////
class Client
{
public:

	enum eState
	{
		INIT = 1,
		CONNECT,
		RUN,
		END
	};

	//////////////////////////////////////////////////

	Client( void );
	~Client( void );

	//////////////////////////////////////////////////

	void	SetUserName		( const std::string& userName )	{ m_UserName = userName; }
	void	SetServerIP		( const char* serverIP )		{ m_ServerIP = serverIP; }
	void	SetServerPort	( const uInt serverPort )		{ m_ServerPort = serverPort; }

	void	Run				( void );

	//////////////////////////////////////////////////

private:

	void			Init			( void );

	void			ServerConnect	( void );

	void			Send			( void );
	void			Receive			( void );
	void			ReceiveImage	( char arrRecvMsg[]);

	unsigned char*	CutChunk		( std::string msg, unsigned char* buffer );

	void			CreateThreads	( void );

	//////////////////////////////////////////////////


private:

	SOCKET*		m_ClientSock;

	std::string	m_UserName;

	std::thread m_ThreadSend;
	std::thread m_ThreadReceive;

	const char*	m_ServerIP;
	uInt		m_ServerPort;

	uInt		m_CurrentClientState;

	bool		m_ClientIsAlive;
	bool		m_ClientSockIsAlive;

	bool		m_InitSendRecvThreads;

};