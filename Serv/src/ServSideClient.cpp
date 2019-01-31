#include	"Server.hpp"
#include	"ServSideClient.hpp"


//////////////////////////////////////////////////
//	cTor
//////////////////////////////////////////////////
ServSideClient::ServSideClient( SOCKET& acceptSocket, const std::string userName, Server* server )
	: m_UserName		( userName )
	, m_ClientSock		( new SOCKET )
	, m_Server			( server )
	, m_PeerPort		( 0 )
	, m_MsgType			( eMsgType::ALL )
{
	m_ClientSock	= &acceptSocket;
	//m_ThreadReceive	= std::thread( &ServSideClient::ReceiveFromClientSide, this );

	InitClientInfo();
}


//////////////////////////////////////////////////
//	dTor
//////////////////////////////////////////////////
ServSideClient::~ServSideClient( void )
{
	if( m_Server )
		m_Server = nullptr;

	if( m_ThreadReceive.joinable() )
		m_ThreadReceive.join();

	if( m_ClientSock )
	{
		shutdown( *m_ClientSock, SD_BOTH );
		closesocket( *m_ClientSock );
	}

	if( m_ClientSock )
		AUTO_DEL( m_ClientSock );
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
//	GetClientInfo
//////////////////////////////////////////////////
void
ServSideClient::InitClientInfo( void )
{
	struct sockaddr_in	clientPeerInfo;
	//struct sockaddr_in	clientSockInfo;

	int peerLength = sizeof( clientPeerInfo );
	//int sockLength = sizeof( clientSockInfo );

	getpeername( *m_ClientSock, ( struct sockaddr* )&clientPeerInfo, &peerLength );
	//getpeername( *m_ClientSock, ( struct sockaddr* )&clientSockInfo, &sockLength );

	m_PeerIP	= inet_ntoa( clientPeerInfo.sin_addr );
	m_PeerPort	= clientPeerInfo.sin_port;

	//m_SockIP	= inet_ntoa( clientSockInfo.sin_addr );
	//m_SockPort	= clientSockInfo.sin_port;
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


	while( true )
	{
		int recvSize = recv( *m_ClientSock, m_arrRecvMsg, MAX_CHARS, 0 );

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


				// Find where next char that is not space is
				for( uInt i = whisperUserAfterIndex; i < ( uInt )recvSize; ++i )
				{
					if( m_arrRecvMsg[ i ] != ' ' )
						whisperMsgStartIndex = i;
				}


				// Evaluate result
				if( m_arrRecvMsg[ whisperMsgStartIndex ] != '/' )
				{
					HandleTxt( whisperMsgStartIndex, ( uInt )recvSize );
				}
				else if( m_arrRecvMsg[ whisperMsgStartIndex ] == '/' )
				{
					if( m_arrRecvMsg[ whisperMsgStartIndex + 1 ] == '/' )
						HandleTgaChunk();
					else
						HandleTgaFile();
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
			std::cerr << "\nSomething in ServSideClient::ReceiveFromClientSide() borke\n";
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
		m_Server->PushJob( *this );
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
