#include	"Server.hpp"
#include	"ServSideClient.hpp"


//////////////////////////////////////////////////
//	cTor
//////////////////////////////////////////////////
ServSideClient::ServSideClient( SOCKET& acceptSocket, const std::string userName, Server* server )
	: m_UserName		( userName )
	, m_pClientSock		( new SOCKET )
	, m_pServer			( server )
	, m_PeerPort		( 0 )
	, m_MsgType			( eMsgType::ALL )
	, m_ClientIsAlive	( true )
{
	m_pClientSock	= &acceptSocket;
	//m_ThreadReceive	= std::thread( &ServSideClient::ReceiveFromClientSide, this );

	InitClientInfo();
}


//////////////////////////////////////////////////
//	dTor
//////////////////////////////////////////////////
ServSideClient::~ServSideClient( void )
{
	if( m_ThreadReceive.joinable() )
		m_ThreadReceive.join();

	if( m_pServer )
		m_pServer = nullptr;

	if( m_pClientSock )
	{
		shutdown( *m_pClientSock, SD_BOTH );
		closesocket( *m_pClientSock );
	}

	if( m_pClientSock )
		AUTO_DEL( m_pClientSock );
}


//////////////////////////////////////////////////
//	StartRecievingThread
//////////////////////////////////////////////////
void
ServSideClient::StartRecievingThread( void )
{
	m_ThreadReceive	= std::thread( &ServSideClient::ReceiveFromClientSide, this );
}


//////////////////////////////////////////////////
//	DeInit
//////////////////////////////////////////////////
void
ServSideClient::Kill( void )
{
	m_ClientIsAlive = false;
}


//////////////////////////////////////////////////
//	GetClientInfo
//////////////////////////////////////////////////
void
ServSideClient::InitClientInfo( void )
{
	struct sockaddr_in	clientPeerInfo;

	int peerLength = sizeof( clientPeerInfo );

	getpeername( *m_pClientSock, ( struct sockaddr* )&clientPeerInfo, &peerLength );

	m_PeerIP	= inet_ntoa( clientPeerInfo.sin_addr );
	m_PeerPort	= clientPeerInfo.sin_port;
}


//////////////////////////////////////////////////
//	Receive
//////////////////////////////////////////////////
void
ServSideClient::ReceiveFromClientSide( void )
{
	uInt whisperUserStartIndex	= 0;
	uInt whisperUserAfterIndex	= 0;
	uInt whisperMsgStartIndex	= 0;


	while( m_ClientIsAlive )
	{
		memset( m_arrRecvMsg, '\0', MAX_CHARS );
		int recvSize = recv( *m_pClientSock, m_arrRecvMsg, MAX_CHARS, 0 );

		if( m_ClientIsAlive == false )
			continue;

		m_MsgType =	( m_arrRecvMsg[ 0 ] == '1' ) ? eMsgType::WHISPER : eMsgType::ALL;


		switch( m_MsgType )
		{
		case eMsgType::WHISPER:


			if( recvSize > 0 )
			{
				// Check until first non ' ' char
				for( uInt i = 1; i < ( uInt )recvSize; ++i )
				{
					if( m_arrRecvMsg[ i ] != ' ' )
					{
						whisperUserStartIndex = i;
						break;
					}
				}


				// Find whisper at username
				m_whisperAtUserName.clear();
				for( uInt i = whisperUserStartIndex; i < ( uInt )recvSize; ++i )
				{
					if( m_arrRecvMsg[ i ] != ' ' )
						m_whisperAtUserName.push_back( m_arrRecvMsg[ i ] );
					else
					{
						whisperUserAfterIndex = i;
						break;
					}
				}


				// Check if username is valid
				if( m_pServer->FindConnectedClient( m_whisperAtUserName ) != true )
				{
					// Send error msg to to user or something
					continue;
				}


				// Find where next char that is not ' ' is
				for( uInt i = whisperUserAfterIndex; i < ( uInt )recvSize; ++i )
				{
					if( m_arrRecvMsg[ i ] != ' ' )
					{
						whisperMsgStartIndex = i;
						break;
					}
				}


				// Evaluate
				if( m_arrRecvMsg[ whisperMsgStartIndex ] != '/' )
				{
					HandleTxt( whisperMsgStartIndex, ( uInt )recvSize );
				}
				else if( m_arrRecvMsg[ whisperMsgStartIndex ] == '/' )
				{
					( m_arrRecvMsg[ whisperMsgStartIndex + 1 ] == '/' ) ? HandleTgaChunk() : HandleTgaFile();
				}
			}


			break;	// Case end //


		case eMsgType::ALL:


			if( recvSize > 0 )
			{
				( m_arrRecvMsg[ 0 ] == '/' ) ?	HandleTgaFile() :
				( m_arrRecvMsg[ 1 ] == '/' ) ?	HandleTgaChunk() :
												HandleTxt( 0, ( uInt )recvSize );
			}


			break;	// Case end //


		default:
			std::cerr << "\n> Something in ServSideClient::ReceiveFromClientSide() borke\n";
			break;	// Case end //
		}
	}
}


//////////////////////////////////////////////////
//	HandleTxt
//////////////////////////////////////////////////
void
ServSideClient::HandleTxt( const uInt startIndex, const uInt recvSize )
{
	m_Msg.clear();
	for( uInt i = startIndex; i < ( uInt )recvSize; ++i )
		m_Msg.push_back( m_arrRecvMsg[ i ] );

	{
		std::lock_guard< std::mutex > lg( m_Mutex );
		m_pServer->PushJob( *this );
	}
}


//////////////////////////////////////////////////
//	HandleTgaFile
//////////////////////////////////////////////////
void
ServSideClient::HandleTgaFile( void )
{
	m_MsgType = eMsgType::TGA_FILE;
}


//////////////////////////////////////////////////
//	HandleTgaChunk
//////////////////////////////////////////////////
void
ServSideClient::HandleTgaChunk( void )
{
	m_MsgType = eMsgType::TGA_CHUNK;
}
