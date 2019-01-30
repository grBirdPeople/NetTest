#include	"Server.hpp"


//////////////////////////////////////////////////
//	cTor
//////////////////////////////////////////////////
Server::Server()
	: m_ServerIP			( nullptr )
	, m_ServerPort			( 0 )
	, m_CurrentServState	( eServState::INIT )
	, m_ServerIsAlive		( true  )
	, m_ServerSockIsAlive	( false )
	, m_InitListenThread	( false )
{}


//////////////////////////////////////////////////
//	dTor
//////////////////////////////////////////////////
Server::~Server()
{}


//////////////////////////////////////////////////
//	Run
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

			break;

		case eServState::DE_INIT:


			DeInit();

			break;


		case eServState::RUN:

			if( m_InitListenThread == true )
				CreateThreads();

			if( m_ThreadAdmin.joinable() )
				m_ThreadAdmin.join();
			if( m_ThreadListen.joinable() )
				m_ThreadListen.join();
			if( m_ThreadDistributeMsg.joinable() )
				m_ThreadDistributeMsg.join();

			break;

		case eServState::END:

			std::string input;
			std::cerr << "Server failed. Restart ( 1 ) or terminate ( 2 ): ";
			std::cin >> input;

			( input == "1" ) ? m_CurrentServState = eServState::INIT : m_ServerIsAlive = false;

			break;

		}
	}
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
		std::cerr << "WSAStartup() failed with error: " << iResult << '\n';
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
	for( auto& i : m_VecServSideClient )
		AUTO_DEL( i );
}


//////////////////////////////////////////////////
//	Admin
//////////////////////////////////////////////////
void
Server::Admin( void )
{
	std::cout << "> This is server\n";
	std::cout << "> List commands: cmd\n\n";

	std::string adminInput;
	std::string command;
	uInt firstSpaceInString	= 0;
	uInt currentCase		= 99;

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


		currentCase	=	( command == "cls" )	? eAdminCommands::CLEAR_CONSOLE	:
						( command == "cmd" )	? eAdminCommands::CMD			:
						( command == "ls" )		? eAdminCommands::LIST_USERS	:
						( command == "kick" )	? eAdminCommands::KICK_USER		:
												eAdminCommands::SIZE;


		switch( currentCase )
		{
		case eAdminCommands::CLEAR_CONSOLE:


			system( "CLS" );
			std::cout << "> List commands: command\n\n";


			break;	// Case end


		case eAdminCommands::CMD:


			std::cout << '\n';

			std::cout << ">\tClear screen:\t\t\t\cls\n";
			std::cout << ">\tList connected clients:\tls\n";
			std::cout << ">\tKick client:\t\t\tkick\n";

			std::cout << '\n';


			break;	// Case end


		case eAdminCommands::LIST_USERS:


			std::cout << '\n';
			std::cout << "> Number of clients connected: " << m_VecServSideClient.size() << "\n\n";

			for( auto& i : m_VecServSideClient )
			{
				std::cout << ">\tUser name:\t" << i->GetName() << '\n';
				std::cout << ">\tIP:\t\t" << i->GetPeerIP() << '\n';
				std::cout << ">\tPort:\t\t" << i->GetPeerPort() << "\n\n";
			}

			break;	// Case end


		case eAdminCommands::KICK_USER:


			if( m_VecServSideClient.size() < 0 )
			{
				std::cout << '\n';
				std::cout << "No clients connected\n\n";
			}
			else
			{
				std::string	user;
				uInt		kickIndex = 0;
				bool		userFound = false;


				for( uInt i = ( firstSpaceInString + 1 ); i < adminInput.size(); ++i )
					user += adminInput.at( i );


				for( uInt i = 0; i < m_VecServSideClient.size(); ++i )
				{
					if( m_VecServSideClient[ i ]->GetName() == user )
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

					int	iResult = send( m_VecServSideClient[ kickIndex ]->GetSockRef(), kickMsg.c_str(), ( size_t )strlen( kickMsg.c_str() ), 0 );
					if( iResult == SOCKET_ERROR )
						std::cerr << "Kick send failed with error: " << WSAGetLastError() << '\n';

					//delete m_VecServSideClient[ kickIndex ];
					m_VecServSideClient.erase( m_VecServSideClient.begin() + ( kickIndex + 1 ) );
				}
				else
				{
					std::cout << '\n';
					std::cout << "> No user by name '" << user << "' is connected to the server\n\n";
				}
			}


			break;	// Case end


		case  eAdminCommands::SIZE:


			std::cout << '\n';
			std::cout << "> Failed to recognize command: " << command << "\n\n";


			break;	// Case end


		default:
			std::cerr << "\nSomething in ServSideClient::ReceiveFromClientSide() borke\n";
			break;	// Case end
		}


		adminInput.clear();
		command.clear();
		firstSpaceInString = 0;
	}
}


//////////////////////////////////////////////////
//	Listening
//////////////////////////////////////////////////
void Server::Listening( void )
{
	m_ServerSockIsAlive	= true;
	SOCKET* listenSock	= new SOCKET;


    *listenSock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( *listenSock == INVALID_SOCKET )
	{
		std::cerr << "\nListening socket failed with error: " << WSAGetLastError() << '\n';
		m_ServerSockIsAlive	= false;
	}
	//else
	//	std::cerr << "Listening socket creation succeeded\n";


	 sockaddr_in serverInfo;
	 serverInfo.sin_family		= AF_INET;
	 serverInfo.sin_addr.s_addr	= inet_addr( m_ServerIP );
	 serverInfo.sin_port		= htons( m_ServerPort );


	int iResult = bind( *listenSock, ( SOCKADDR* )& serverInfo, sizeof( serverInfo ) );
	if( iResult == SOCKET_ERROR )
	{
		std::cerr << "\nBind function failed with error. " << WSAGetLastError() << '\n';
		iResult = closesocket( *listenSock );
		if ( iResult == SOCKET_ERROR )
			std::cerr << "\nClosesocket function failed with error: " << WSAGetLastError() << '\n';

		m_ServerSockIsAlive	= false;
	}


	while( m_ServerSockIsAlive )
	{
		if( listen( *listenSock, SOMAXCONN ) == SOCKET_ERROR )
		{
			std::cerr << "\nListen function failed with error: " << WSAGetLastError() << '\n';
			m_ServerSockIsAlive = false;
		}

		//std::cerr << "Listening on socket...\n";


		SOCKET* clientSock = new SOCKET;
		//std::cerr << "Waiting for client to connect...\n";


		*clientSock = accept( *listenSock, NULL, NULL );
		if ( *clientSock == INVALID_SOCKET)
		{
			std::cerr << "\nAccept failed with error: " << WSAGetLastError() << '\n';
			closesocket( *listenSock );
			m_ServerSockIsAlive = false;
		}
		else
		{
			//std::cout << "Clients connected: " << m_VecServSideClient.size() + 1 << '\n';

			m_VecServSideClient.push_back( new ServSideClient( *clientSock, "NoName", this ) );
			clientSock = nullptr;
		}
	}


	if( listenSock )
	{
		closesocket( *listenSock );
		AUTO_DEL( listenSock );
	}

	m_CurrentServState = eServState::END;
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


	while( m_ServerSockIsAlive )
	{
		m_Mutex.lock();

		while( m_QueueMsg.empty() )
			Sleep( 1 );

		if( m_QueueMsg.empty() )
			continue;


		pClient	= m_QueueMsg.front();
		m_QueueMsg.pop();

		clientMsgType	= pClient->GetMsgType();
		clientPort		= pClient->GetPeerPort();
		clientUserName	= pClient->GetName();
		clientMsg		= pClient->GetMsg();


		switch( clientMsgType )
		{
		case eMsgType::WHISPER:


			whisperAtUserName.clear();
			whisperAtUserName = pClient->GetWhisperName();

			for( uInt i = 0; i < m_VecServSideClient.size(); ++i )
			{
				if( m_VecServSideClient[ i ]->GetName() == whisperAtUserName )
				{
					iResult = send( m_VecServSideClient[ i ]->GetSockRef(), clientMsg.c_str(), ( size_t )strlen( clientMsg.c_str() ), 0 );
					if( iResult == SOCKET_ERROR )
						std::cerr << "\nSend failed with error: " << WSAGetLastError() << '\n';

					break;
				}
			}


			break;	// Case end


		case eMsgType::TGA_FILE:



			break;	// Case end


		case eMsgType::TGA_CHUNK:



			break;	// Case end


		case eMsgType::ALL:


			for( uInt i = 0; i < m_VecServSideClient.size(); ++i )
			{
				if( m_VecServSideClient[ i ]->GetPeerPort() == clientPort )
					continue;

				iResult = send( m_VecServSideClient[ i ]->GetSockRef(), clientMsg.c_str(), ( size_t )strlen( clientMsg.c_str() ), 0 );
				if( iResult == SOCKET_ERROR )
					std::cerr << "\nSend failed with error: " << WSAGetLastError() << '\n';
			}


			break;	// Case end


		default:
			std::cerr << "\nSomething in Server::Distribute() borke\n";
			break;	// Case end
		}


		m_Mutex.unlock();
	}
}


//////////////////////////////////////////////////
//	CreateThread
//////////////////////////////////////////////////
void
Server::CreateThreads( void )
{
	m_ThreadAdmin			= std::thread( &Server::Admin, this );
	m_ThreadListen			= std::thread( &Server::Listening, this );
	m_ThreadDistributeMsg	= std::thread( &Server::Distribute, this );

	m_InitListenThread		= false;
}

