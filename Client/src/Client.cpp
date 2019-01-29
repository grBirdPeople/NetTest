#include "Client.hpp"


//////////////////////////////////////////////////
//	cTor
//////////////////////////////////////////////////
Client::Client( void )
	: m_ClientSock			( new SOCKET )
	, m_UserName			( "No name set" )
	, m_ServerIP			( nullptr )
	, m_ServerPort			( 0 )
	, m_CurrentClientState	( eState::INIT )
	, m_ClientIsAlive		( true )
	, m_ClientSockIsAlive	( false )
	, m_InitSendRecvThreads	( false )
{
}


//////////////////////////////////////////////////
//	dTor
//////////////////////////////////////////////////
Client::~Client( void )
{
	if( m_ClientSock )
	{
		closesocket( *m_ClientSock );
		AUTO_DEL( m_ClientSock );
	}
}


//////////////////////////////////////////////////
//	Run
//////////////////////////////////////////////////
void
Client::Run( void )
{
	while( m_ClientIsAlive )
	{
		switch( m_CurrentClientState )
		{
		case eState::INIT:

			Init();

			break;

		case eState::CONNECT:

			ServerConnect();

			break;

		case eState::RUN:

			if( m_InitSendRecvThreads == true )
				CreateThreads();

			if( m_ThreadSend.joinable() )
				m_ThreadSend.join();
			if( m_ThreadReceive.joinable() )
				m_ThreadReceive.join();

			break;

		case eState::END:

			std::string input;
			std::cerr << "Client failed. Retry ( 1 ) or terminate ( 2 ): ";
			std::cin >> input;

			( input == "1" ) ? m_CurrentClientState = eState::INIT : m_ClientIsAlive = false;

			break;

		}
	}
}


//////////////////////////////////////////////////
//	Init
//////////////////////////////////////////////////
void
Client::Init( void )
{
	WSAData wsaData;

	int iResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	if( iResult != NO_ERROR )
	{
		std::cerr << "WSAStartup() failed with error: " << iResult << '\n';
		m_CurrentClientState = eState::END;
	}


	*m_ClientSock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( *m_ClientSock == INVALID_SOCKET )
	{
		std::cerr << "Listening socket failed with error: " << WSAGetLastError() << '\n';
		m_CurrentClientState = eState::END;
	}
	else
		m_CurrentClientState = eState::CONNECT;
}


//////////////////////////////////////////////////
//	ServerConnect
//////////////////////////////////////////////////
void
Client::ServerConnect( void )
{
	sockaddr_in clientService;
    clientService.sin_family		= AF_INET;
    clientService.sin_addr.s_addr	= inet_addr( m_ServerIP );
    clientService.sin_port			= htons( m_ServerPort );

	int iResult = connect( *m_ClientSock, (SOCKADDR *) & clientService, sizeof (clientService) );
	if (iResult == SOCKET_ERROR)
	{
		std::cerr << "Connect function failed with error: " << WSAGetLastError() << '\n';
		iResult = closesocket( *m_ClientSock );

		if (iResult == SOCKET_ERROR)
			std::cerr << "Closesocket function failed with error: " << WSAGetLastError() << '\n';

		m_CurrentClientState = eState::END;
	}
	else
	{
		m_InitSendRecvThreads	= true;
		m_CurrentClientState	= eState::RUN;
	}
}


//////////////////////////////////////////////////
//	Send
//////////////////////////////////////////////////
void
Client::Send( void )
{
	char arrSendMsg[ MAX_CHARS ];
	memset( arrSendMsg, 0, MAX_CHARS );

	while( true )
	{
		const char* msg;
		std::cout << "> ";
		
		std::cin.getline( arrSendMsg, sizeof( arrSendMsg ) );
		msg = arrSendMsg;

		int	iResult = send( *m_ClientSock, msg, strlen( msg ), 0 );
		if ( iResult == SOCKET_ERROR )
			std::cerr << "Send failed with error: " << WSAGetLastError() << '\n';
	}
}


//////////////////////////////////////////////////
//	Receive
//////////////////////////////////////////////////
void
Client::Receive( void )
{
	char		arrRecvMsg[ MAX_CHARS ];
	std::string masg;

	while( true )
	{
		int recvSize = recv( *m_ClientSock, arrRecvMsg, MAX_CHARS, 0 );

		if ( recvSize > 0 )
		{
			masg.clear();
			for ( uInt i = 0; i < ( uInt )recvSize; ++i )
				masg.push_back( arrRecvMsg[i] );

			std::cout << "\nMsg: " << masg << '\n';
		}
	}
}


//////////////////////////////////////////////////
//	Transfer
//////////////////////////////////////////////////
void
Client::CreateThreads( void )
{
	m_ThreadSend			= std::thread( &Client::Send, this );
	m_ThreadReceive			= std::thread( &Client::Receive, this );

	m_InitSendRecvThreads	= false;
}
