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
	*m_ClientSock	= acceptSocket;
	m_ThreadReceive	= std::thread( &ServSideClient::ReceiveFromClientSide, this );

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

	closesocket( *m_ClientSock );

	if( m_ClientSock )
		AUTO_DEL( m_ClientSock );
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
	while( true )
	{
		int recvSize = recv( *m_ClientSock, m_arrRecvMsg, MAX_CHARS, 0 );

		m_MsgType =	( m_arrRecvMsg[ 0 ] == '1' )	? eMsgType::WHISPER		:
					( m_arrRecvMsg[ 0 ] == '2' )	? eMsgType::TGA_FILE	:
					( m_arrRecvMsg[ 0 ] == '3' )	? eMsgType::TGA_CHUNK	:
													eMsgType::ALL;


		switch( m_MsgType )
		{
		case eMsgType::WHISPER:


			if( recvSize > 0 )
			{
				uInt		whisperUserStartIndex;
				uInt		whisperMsgStartIndex;


				// Find whisper at username start index
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
					if( m_arrRecvMsg[ i ] == ' ' )
					{
						for( uInt j = i; j < ( uInt )recvSize; ++j )
						{
							if( m_arrRecvMsg[ j ] != ' ' )
							{
								whisperMsgStartIndex = j;
								break;
							}
						}

						break;
					}

					m_whisperAtUserName.push_back( m_arrRecvMsg[ i ] );
				}


				// Fiund actual msg
				m_Msg.clear();

				for( uInt i = whisperMsgStartIndex; i < ( uInt )recvSize; ++i )
					m_Msg.push_back( m_arrRecvMsg[ i ] );


				std::unique_lock< std::mutex > uLock( m_Mutex );
				m_Server->PushJob( *this );
			}

			break;	// Case end //


		case eMsgType::TGA_FILE:



			break;	// Case end //



		case eMsgType::TGA_CHUNK:



			break;	// Case end //


		case eMsgType::ALL:


			if( recvSize > 0 )
			{
				m_Msg.clear();

				for( uInt i = 0; i < ( uInt )recvSize; ++i )
					m_Msg.push_back( m_arrRecvMsg[ i ] );

				std::unique_lock< std::mutex > uLock( m_Mutex );
				m_Server->PushJob( *this );
			}


			break;	// Case end //


		default:
			std::cerr << "\nSomething in ServSideClient::ReceiveFromClientSide() borke\n";
			break;	// Case end //
		}
	}
}
