#pragma once

#include	"Common.hpp"
#include	"ServSideClient.hpp"


//////////////////////////////////////////////////
//	Server
//////////////////////////////////////////////////
class Server
{
public:

	friend class ServSideClient;

	//////////////////////////////////////////////////

	enum eServState
	{
		INIT = 1,
		RUN,
		END
	};

	//////////////////////////////////////////////////

	Server();
	~Server();

	//////////////////////////////////////////////////

	void	SetServerIP		( const char* serverIP )	{ m_ServerIP = serverIP; }
	void	SetServerPort	( const uInt serverPort )	{ m_ServerPort = serverPort; }

	void	Run				( void );

	//////////////////////////////////////////////////

private:

	void	Init			( void );

	void	Listening		( void );
	void	DistributeMsg	( void );

	void	CreateThreads	( void );

	//////////////////////////////////////////////////

private:

	std::vector< ServSideClient* >	m_VecServSideClient;

	std::queue< ServSideClient* >	m_QueueMsg;		// No ownage // Don't delete when pop, only nullptr

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