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

	void	SetServerIP			( const char* serverIP )	{ m_pServerIP = serverIP; }
	void	SetServerPort		( const uInt serverPort )	{ m_ServerPortTCP = serverPort; }

	void	Run					( void );

	void	PushJob				( ServSideClient& job );
	bool	FindConnectedClient	( const std::string& name );

	//////////////////////////////////////////////////

private:

	void	Init			( void );
	void	DeInit			( void );

	void	Admin			( void );
	void	Listen			( void );
	void	HandShake		( void );
	void	Distribute		( void );

	void	CreateThreads	( void );

	//////////////////////////////////////////////////
	
private:

	std::vector< ServSideClient* >	m_pVecServSideClient;

	std::queue< ServSideClient* >	m_pQueueShake;
	std::queue< ServSideClient* >	m_pQueueJob;		// No ownage // Don't delete when pop, only nullptr

	std::thread						m_ThreadAdmin;
	std::thread						m_ThreadListen;
	std::thread						m_HandShake;
	std::thread						m_ThreadDistributeMsg;

	std::mutex						m_Mutex;

	SOCKET*							m_ServerSockUDP;

	const char*						m_pServerIP;
	uInt							m_ServerPortTCP;
	uInt							m_ServerPortUDP;

	uInt							m_CurrentServState;

	bool							m_ServerIsAlive;

	bool							m_InitListenThread;

	SOCKET*							m_ListenSock;

};