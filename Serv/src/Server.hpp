#pragma once

#include	"Common.hpp"
#include	"ServSideClient.hpp"


//////////////////////////////////////////////////
//	Server
//////////////////////////////////////////////////
class Server
{
public:

	enum eServState
	{
		INIT = 1,
		DE_INIT,
		RUN,
		END
	};

	enum eAdminCommands
	{
		CLEAR_CONSOLE = 0,
		CMD,
		KICK_USER,
		LIST_USERS,
		RESTART,
		TERMINATE,
		SIZE
	};

	//////////////////////////////////////////////////

	Server();
	~Server();

	//////////////////////////////////////////////////

	void	SetServerIP		( const char* serverIP )	{ m_ServerIP = serverIP; }
	void	SetServerPort	( const uInt serverPort )	{ m_ServerPort = serverPort; }

	void	Run				( void );

	void	PushJob			( ServSideClient& job );

	//////////////////////////////////////////////////

private:

	void	Init			( void );
	void	DeInit			( void );

	void	Admin			( void );
	void	Listen			( void );
	void	Distribute		( void );

	void	CreateThreads	( void );

	void	PushClient		( ServSideClient& client );

	//////////////////////////////////////////////////

private:

	std::vector< ServSideClient* >	m_VecServSideClient;

	std::queue< ServSideClient* >	m_QueueJob;		// No ownage // Don't delete when pop, only nullptr

	std::thread						m_ThreadAdmin;
	std::thread						m_ThreadListen;
	std::thread						m_ThreadDistributeMsg;

	std::mutex						m_Mutex;

	const char*						m_ServerIP;
	uInt							m_ServerPort;

	uInt							m_CurrentServState;

	bool							m_ServerIsAlive;
	bool							m_ServerSockIsAlive;

	bool							m_InitListenThread;

};