#include	"Server.hpp"


//////////////////////////////////////////////////
//	cTor
//////////////////////////////////////////////////
Server::Server()
	: m_pServerIP			( nullptr )
	, m_ServerPort			( 0 )
	, m_CurrentServState	( eServState::INIT )
	, m_ServerIsAlive		( true  )
	, m_InitListenThread	( false )
	, m_ListenSock			( nullptr )
{}


//////////////////////////////////////////////////
//	dTor
//////////////////////////////////////////////////
Server::~Server()
{}


//////////////////////////////////////////////////
//	Runs
//////////////////////////////////////////////////
void
Server::Run( void )
{
	while( m_ServerIsAlive )
	{
		switch( m_CurrentServState )
		{
		case eServState::INIT:
			Init();
			break;	// Case end //


		case eServState::DE_INIT:


			DeInit();
			m_ServerIsAlive = false;


			break;	// Case end //


		case eServState::RUN:


			if( m_InitListenThread == true )
				CreateThreads();


			break;	// Case end //


		case eServState::END:


			if( m_ListenSock )
			{
				shutdown( *m_ListenSock, SD_BOTH );
				closesocket( *m_ListenSock );
				AUTO_DEL( m_ListenSock );
			}


			if( m_ThreadAdmin.joinable() )
				m_ThreadAdmin.join();
			if( m_ThreadListen.joinable() )
				m_ThreadListen.join();
			if( m_HandShake.joinable() )
				m_HandShake.join();
			if( m_ThreadDistributeMsg.joinable() )
				m_ThreadDistributeMsg.join();


			m_CurrentServState	= eServState::DE_INIT;


			break;	// Case end //

		}
	}
}


//////////////////////////////////////////////////
//	PushJob
//////////////////////////////////////////////////
void
Server::PushJob( ServSideClient& job )
{
	m_pQueueJob.push( &job );
}


//////////////////////////////////////////////////
//	FindConnectedClient
//////////////////////////////////////////////////
bool
Server::FindConnectedClient( const std::string & name )
{
	for( uInt i = 0; i < m_pVecServSideClient.size(); ++i )
	{
		if( m_pVecServSideClient[ i ]->GetName() == name )
			return true;
	}

	return false;
}


//////////////////////////////////////////////////
//	Init
//////////////////////////////////////////////////
void
Server::Init( void )
{
	WSAData wsaData;

	int iResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
	if( iResult != NO_ERROR )
	{
		std::cerr << "> WSAStartup() failed with error: " << iResult << '\n';
		m_CurrentServState = eServState::END;
	}
	else
	{
		m_InitListenThread	= true;
		m_CurrentServState	= eServState::RUN;
	}
}


//////////////////////////////////////////////////
//	DeInit
//////////////////////////////////////////////////
void
Server::DeInit( void )
{
	for( auto& i : m_pVecServSideClient )
		AUTO_DEL( i );

	WSACleanup();
}


//////////////////////////////////////////////////
//	Admin
//////////////////////////////////////////////////
void
Server::Admin( void )
{
	system( "CLS" );
	std::cout << "> This is server\n";
	std::cout << "> List commands: cmd\n\n";


	std::string adminInput;
	std::string command;
	uInt		firstSpaceInString;
	uInt		currentAdminState;


	while( m_ServerIsAlive )
	{
		std::cout << "> ";
		std::getline( std::cin, adminInput );


		for( uInt i = 0; i < adminInput.size(); ++i )
		{
			if( adminInput[ i ] == ' ' )
			{
				firstSpaceInString = i;
				break;
			}

			command += adminInput.at( i );
		}


		currentAdminState	=	( command == "cls" )		? eAdminCommands::CLEAR_CONSOLE	:
								( command == "cmd" )		? eAdminCommands::CMD			:
								( command == "ls" )			? eAdminCommands::LIST_USERS	:
								( command == "kick" )		? eAdminCommands::KICK_USER		:
								( command == "terminate" )	? eAdminCommands::TERMINATE		:
															eAdminCommands::SIZE;


		switch( currentAdminState )
		{
		case eAdminCommands::CLEAR_CONSOLE:


			system( "CLS" );
			std::cout << "> This is server\n";
			std::cout << "> List commands: cmd\n\n";


			break;	// Case end //


		case eAdminCommands::CMD:


			std::cout << '\n';

			std::cout << ">\tClear screen:\t\t\tcls\n";
			std::cout << ">\tKick client:\t\t\tkick\n";
			std::cout << ">\tList connected clients:\t\tls\n";
			std::cout << ">\tTerminate server:\t\tterminate\n";

			std::cout << '\n';


			break;	// Case end //


		case eAdminCommands::LIST_USERS:


			std::cout << '\n';
			std::cout << "> Number of clients connected: " << m_pVecServSideClient.size() << "\n\n";

			for( auto& i : m_pVecServSideClient )
			{
				std::cout << ">\tUser name:\t" << i->GetName() << '\n';
				std::cout << ">\tIP:\t\t" << i->GetPeerIP() << '\n';
				std::cout << ">\tPort:\t\t" << i->GetPeerPort() << "\n\n";
			}

			break;	// Case end //


		case eAdminCommands::KICK_USER:


			if( m_pVecServSideClient.size() < 0 )
			{
				std::cout << '\n';
				std::cout << "> No clients connected\n\n";
			}
			else
			{
				std::string	user;
				uInt		kickIndex = 0;
				bool		userFound = false;


				for( uInt i = ( firstSpaceInString + 1 ); i < adminInput.size(); ++i )
					user += adminInput.at( i );


				for( uInt i = 0; i < m_pVecServSideClient.size(); ++i )
				{
					if( m_pVecServSideClient[ i ]->GetName() == user )
					{
						kickIndex = i;
						userFound = true;
						break;
					}
				}


				if( userFound )
				{
					std::cout << '\n';
					std::cout << "> User '" << user << "' was kicked :)\n\n";

					std::string kickMsg = "You where kicked from the server :)";

					int	iResult = send( m_pVecServSideClient[ kickIndex ]->GetSockRef(), kickMsg.c_str(), ( size_t )strlen( kickMsg.c_str() ), 0 );
					if( iResult == SOCKET_ERROR )
						std::cerr << "> Kick send failed with error: " << WSAGetLastError() << '\n';

					//ServSideClient* client = m_VecServSideClient[ kickIndex ];
					m_pVecServSideClient.erase( m_pVecServSideClient.begin() + kickIndex );
					//client->Kill();
					//Sleep( 10 );
					//AUTO_DEL( client );
				}
				else
				{
					std::cout << '\n';
					std::cout << "> No user by name '" << user << "' is connected to the server\n\n";
				}
			}


			break;	// Case end //



		case eAdminCommands::TERMINATE:


			m_CurrentServState	= eServState::END;


			break;	// Case end //


		case eAdminCommands::SIZE:


			std::cout << '\n';
			std::cout << "> Failed to recognize command: " << command << "\n\n";


			break;	// Case end //


		default:
			std::cerr << "\n> Something in ServSideClient::ReceiveFromClientSide() borke\n";
			break;	// Case end //
		}


		adminInput.clear();
		command.clear();
		firstSpaceInString = 0;
	}
}


//////////////////////////////////////////////////
//	Listening
//////////////////////////////////////////////////
void Server::Listen( void )
{
	m_ListenSock			= new SOCKET;
	bool serverSockIsAlive	= true;


    *m_ListenSock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( *m_ListenSock == INVALID_SOCKET )
	{
		std::cerr << "\n> Listening socket failed with error: " << WSAGetLastError() << '\n';
		serverSockIsAlive	= false;
	}


	 sockaddr_in serverInfo;
	 serverInfo.sin_family		= AF_INET;
	 serverInfo.sin_addr.s_addr	= inet_addr( m_pServerIP );
	 serverInfo.sin_port		= htons( m_ServerPort );


	int iResult = bind( *m_ListenSock, ( SOCKADDR* )& serverInfo, sizeof( serverInfo ) );
	if( iResult == SOCKET_ERROR )
	{
		std::cerr << "\n> Bind function failed with error. " << WSAGetLastError() << '\n';
		iResult = closesocket( *m_ListenSock );
		if ( iResult == SOCKET_ERROR )
			std::cerr << "\n> Closesocket function failed with error: " << WSAGetLastError() << '\n';

		serverSockIsAlive	= false;
	}


	while( serverSockIsAlive )
	{
		if( listen( *m_ListenSock, SOMAXCONN ) == SOCKET_ERROR )
		{
			std::cerr << "\n> Listen function failed with error: " << WSAGetLastError() << '\n';
			serverSockIsAlive = false;
		}


		SOCKET* clientSockTCP = new SOCKET;

		*clientSockTCP = accept( *m_ListenSock, NULL, NULL );
		if ( *clientSockTCP == INVALID_SOCKET)
		{
			std::cerr << "\n> Accept failed with error: " << WSAGetLastError() << '\n';
			closesocket( *m_ListenSock );
			serverSockIsAlive = false;
		}
		else
		{
			{
				std::lock_guard< std::mutex > lg( m_Mutex );
				m_pQueueShake.push( new ServSideClient( *clientSockTCP, "NoName", this ) );
			}

			clientSockTCP = nullptr;
		}
	}
}


//////////////////////////////////////////////////
//	HandShake
//////////////////////////////////////////////////
void
Server::HandShake( void )
{
	//char			m_arrRecvOkMsg[ MAX_CHARS ];

	//ServSideClient*	pClient = nullptr;	// No ownage // Only ptr are copied and used to fecth client info

	//int				iResult;

	//std::string		clientUserName;
	//std::string		msg;
	//std::string		whisperAtUserName;


	//while( m_ServerIsAlive )
	//{
	//	while( m_pQueueShake.empty() )
	//		Sleep( 1 );

	//	if( m_pQueueShake.empty() )
	//		continue;


	//	{
	//		std::lock_guard< std::mutex > lg( m_Mutex );
	//		pClient = m_pQueueShake.front();
	//		m_pQueueShake.pop();
	//	}


	//	// Send init connect confirm to client
	//	msg = "All good in the hood";

	//	iResult = send( pClient->GetSockRef(), msg.c_str(), ( size_t )strlen( msg.c_str() ), 0 );
	//	if( iResult == SOCKET_ERROR )
	//		std::cerr << "\n> Send failed with error: " << WSAGetLastError() << '\n';

	//	
	//	// Recieve client username
	//	int recvSize = recv( pClient->GetSockRef(), m_arrRecvOkMsg, MAX_CHARS, 0 );

	//	msg.clear();
	//	for( uInt i = 0; i < ( uInt )recvSize; ++i )
	//		msg.push_back( m_arrRecvOkMsg[ i ] );

	//	{
	//		std::lock_guard< std::mutex > lg( m_Mutex );
	//		pClient->SetUserName( msg );
	//	}

	//	
	//	// Send established connect to client
	//	msg = "Server connection established";

	//	iResult = send( pClient->GetSockRef(), msg.c_str(), ( size_t )strlen( msg.c_str() ), 0 );
	//	if( iResult == SOCKET_ERROR )
	//		std::cerr << "\n> Send failed with error: " << WSAGetLastError() << '\n';


	//	// Add client to connected client pool
	//	{
	//		std::lock_guard< std::mutex > lg( m_Mutex );
	//		m_pVecServSideClient.push_back( pClient );
	//		pClient->StartRecvThreadTCP();
	//	}

	//	pClient = nullptr;


		



	char			m_arrRecvOkMsg[ MAX_CHARS ];

	ServSideClient*	pClient = nullptr;	// No ownage // Only ptr are copied and used to fecth client info

	int				iResult;

	std::string		clientUserName;
	std::string		msg;
	std::string		whisperAtUserName;


	while( m_ServerIsAlive )
	{
		while( m_pQueueShake.empty() )
			Sleep( 1 );

		if( m_pQueueShake.empty() )
			continue;


		{
			std::lock_guard< std::mutex > lg( m_Mutex );
			pClient = m_pQueueShake.front();
			m_pQueueShake.pop();
		}




		sockaddr_in clientInfo;
		clientInfo.sin_family		= AF_INET;
		clientInfo.sin_addr.s_addr	= inet_addr( pClient->GetPeerIP().c_str() );
		clientInfo.sin_port			= htons( pClient->GetPeerPort() );


		SOCKET* clientSockUDP		= new SOCKET;
		*clientSockUDP				= socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
		

		int iResult = bind( *clientSockUDP, ( struct sockaddr* )&clientInfo, sizeof( clientInfo ) );
		if( iResult == SOCKET_ERROR )
		{
			std::cerr << "\n> Bind function failed with error. " << WSAGetLastError() << '\n';
			iResult = closesocket( *m_ListenSock );
			if ( iResult == SOCKET_ERROR )
				std::cerr << "\n> Closesocket function failed with error: " << WSAGetLastError() << '\n';
		}




		// Send init connect confirm to client
		msg = "All good in the hood";

		iResult = send( pClient->GetSockRef(), msg.c_str(), ( size_t )strlen( msg.c_str() ), 0 );
		if( iResult == SOCKET_ERROR )
			std::cerr << "\n> Send failed with error: " << WSAGetLastError() << '\n';

		
		// Recieve client username
		int recvSize = recv( pClient->GetSockRef(), m_arrRecvOkMsg, MAX_CHARS, 0 );

		msg.clear();
		for( uInt i = 0; i < ( uInt )recvSize; ++i )
			msg.push_back( m_arrRecvOkMsg[ i ] );

		{
			std::lock_guard< std::mutex > lg( m_Mutex );
			pClient->SetUserName( msg );
		}

		
		// Send established connect to client
		msg = "Server connection established";

		iResult = send( pClient->GetSockRef(), msg.c_str(), ( size_t )strlen( msg.c_str() ), 0 );
		if( iResult == SOCKET_ERROR )
			std::cerr << "\n> Send failed with error: " << WSAGetLastError() << '\n';


		// Add client to connected client pool
		{
			std::lock_guard< std::mutex > lg( m_Mutex );
			m_pVecServSideClient.push_back( pClient );
			pClient->StartRecvThreadTCP();
		}

		pClient = nullptr;
	}
}


//////////////////////////////////////////////////
//	DistributeMsgs
//////////////////////////////////////////////////
void
Server::Distribute( void )
{
	ServSideClient* pClient = nullptr;	// No ownage // Only ptr are copied and used to fecth client info

	uInt clientMsgType;
	uInt clientPort;

	int	iResult;

	std::string	clientUserName;
	std::string clientMsg;
	std::string	whisperAtUserName;


	while( m_ServerIsAlive )
	{
		while( m_pQueueJob.empty() )
			Sleep( 1 );

		if( m_pQueueJob.empty() )
			continue;


		{
			std::lock_guard< std::mutex > lg( m_Mutex );
			pClient	= m_pQueueJob.front();
			m_pQueueJob.pop();
		}


		clientPort		= pClient->GetPeerPort();
		clientUserName	= pClient->GetName();
		clientMsgType	= pClient->GetMsgType();
		clientMsg		= pClient->GetMsg();


		switch( clientMsgType )
		{
		case eMsgType::WHISPER:


			whisperAtUserName.clear();
			whisperAtUserName = pClient->GetWhisperName();

			for( uInt i = 0; i < m_pVecServSideClient.size(); ++i )
			{
				if( m_pVecServSideClient[ i ]->GetName() == whisperAtUserName )
				{
					iResult = send( m_pVecServSideClient[ i ]->GetSockRef(), clientMsg.c_str(), ( size_t )strlen( clientMsg.c_str() ), 0 );
					if( iResult == SOCKET_ERROR )
						std::cerr << "\n> Send failed with error: " << WSAGetLastError() << '\n';

					break;
				}
			}


			break;	// Case end //


		case eMsgType::TGA_FILE:



			break;	// Case end //


		case eMsgType::TGA_CHUNK:



			break;	// Case end //


		case eMsgType::ALL:


			for( uInt i = 0; i < m_pVecServSideClient.size(); ++i )
			{
				if( m_pVecServSideClient[ i ]->GetPeerPort() == clientPort )
					continue;

				iResult = send( m_pVecServSideClient[ i ]->GetSockRef(), clientMsg.c_str(), ( size_t )strlen( clientMsg.c_str() ), 0 );
				if( iResult == SOCKET_ERROR )
					std::cerr << "\n> Send failed with error: " << WSAGetLastError() << '\n';
			}


			break;	// Case end //


		default:
			std::cerr << "\n> Something in Server::Distribute() borke\n";
			break;	// Case end
		}
	}

	pClient = nullptr;
}


//////////////////////////////////////////////////
//	CreateThread
//////////////////////////////////////////////////
void
Server::CreateThreads( void )
{
	m_ThreadAdmin			= std::thread( &Server::Admin, this );
	m_ThreadListen			= std::thread( &Server::Listen, this );
	m_HandShake				= std::thread( &Server::HandShake, this );
	m_ThreadDistributeMsg	= std::thread( &Server::Distribute, this );

	m_InitListenThread		= false;
}
