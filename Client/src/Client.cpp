#include "Client.hpp"
#include "targa_writer.c"
#include "tga_reader.h"
#include <stdio.h>


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
	char arrRecvMsg[ MAX_CHARS ];
	std::string msg;
	int iResult;
	int recvSize;

	sockaddr_in clientService;
    clientService.sin_family		= AF_INET;
    clientService.sin_addr.s_addr	= inet_addr( m_ServerIP );
    clientService.sin_port			= htons( m_ServerPort );

	iResult = connect( *m_ClientSock, ( SOCKADDR* ) & clientService, sizeof( clientService ) );
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
		std::cout << "\n> Waiting for server response...\n";


		// Get confirm from server
		recvSize = recv( *m_ClientSock, arrRecvMsg, MAX_CHARS, 0 );

		msg.clear();
		for ( uInt i = 0; i < ( uInt )recvSize; ++i )
			msg.push_back( arrRecvMsg[ i ] );


		// Send username to server
		if( msg == "All good in the hood" )
		{
			iResult = send( *m_ClientSock, m_UserName.c_str(), strlen( m_UserName.c_str() ), 0);
			if( iResult == SOCKET_ERROR )
				std::cerr << "\nSend failed with error: " << WSAGetLastError() << '\n';
		}


		// Get permishion from servers
		recvSize = recv( *m_ClientSock, arrRecvMsg, MAX_CHARS, 0 );

		msg.clear();
		for ( uInt i = 0; i < ( uInt )recvSize; ++i )
			msg.push_back( arrRecvMsg[ i ] );

		std::cout << "> " << msg << "\n\n";


		// Change state
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
	//char arrSendMsg[ MAX_CHARS ];
	//memset( arrSendMsg, 0, MAX_CHARS );
	//const char* msg;
	std::string msg;

	while( true )
	{
		std::cout << "> ";
		std::getline( std::cin, msg );

		bool canSend = true;


		// works but unfinished
		if( msg[ 0 ] == '@' )
		{
			msg[ 0 ] = eMsgType::WHISPER + 48;
		}
		else if( msg[ 0 ] == '/' )
		{
			if( msg[ 1 ] == '/' )
			{
				msg[ 0 ] = eMsgType::TGA_CHUNK + 48;
				msg.erase( msg.begin() + 1 );
			}
			else
			{
				msg[ 0 ] = eMsgType::TGA_FILE + 48;
			}

			std::size_t found = msg.find(".tga");
			
			if (found!=std::string::npos)
			{
				FILE *file = fopen(msg.substr(1,found+3).c_str(), "r");

				int size = 0;

				int height = 0;
				int width = 0;

				if (file)
				{
					fseek(file, 0, SEEK_END);
					size = ftell(file);
					fseek(file, 0, SEEK_SET);
					unsigned char* buffer = new (unsigned char[size]);
					fread(buffer, 1, size, file);

					width = tgaGetWidth(buffer);
					height = tgaGetHeight(buffer);

					std::cout << msg[0] << std::endl;
					if(msg[0] == eMsgType::TGA_CHUNK + 48)
					{
						buffer = CutChunk(msg, buffer);

						if (buffer == nullptr)
						{
							std::cout << "Could not send file." << std::endl;
							canSend = false;
						}

					}



					if(canSend==true)
					{

						//> send one message with
						//	name	width	height	amount of packages being sent

						std::string infomsg;
						infomsg += msg.substr(0, 1);

						std::size_t rfound = msg.rfind("/",found);

						if (rfound != std::string::npos)
							infomsg+= msg.substr(rfound+1,found-rfound-1);
						else
							infomsg += msg.substr(1, found-1);
						std::cout << infomsg << std::endl;
						infomsg += "*";

						infomsg += std::to_string(width);
						infomsg += "*";
						infomsg += std::to_string(height);
						infomsg += "*";

						if (width > 5000 || height > 5000)
						{	std::cout << "Error: File too large\n";
							canSend=false;	}


						size_t o = 0;
						const char* dataChunk;

						int imageSize=0;
						for (size_t i = 0; i < size; i++)
						{
							if (size > 1024 * i)
								imageSize++;
							else
								break;
						}
						infomsg += std::to_string(imageSize);

						std::cout << infomsg << "\n";

						// Send header file
						int	iResult = send(*m_ClientSock, infomsg.c_str(), strlen(infomsg.c_str()), 0);
						if (iResult == SOCKET_ERROR)
							std::cerr << "Send failed with error: " << WSAGetLastError() << '\n';


						// Send image code chunks (assuming image size is 256x256)
						//static const unsigned int MAX_MTU_SIZE = 1024;
						int orginalWidth = 256;
						int chunkHeight = 256;
						int chunkWidth = 256;
						char* pByteArray = new char[MAX_MTU_SIZE];
						int writeIndex = 0;
						int xOffset = 0;
						int yOffset = 0;
						for (int y = 0; y < chunkHeight; ++y)
						{
							for (int x = 0; x < chunkWidth; ++x)
							{
								pByteArray[writeIndex] = buffer[(y+yOffset) * orginalWidth + xOffset];
								writeIndex += 1;
								if (writeIndex > MAX_MTU_SIZE)
								{
									iResult = send(*m_ClientSock, pByteArray, strlen(pByteArray), 0);
									if (iResult == SOCKET_ERROR)
										std::cerr << "Send failed with error: " << WSAGetLastError() << '\n';

									memset(pByteArray, 0, MAX_MTU_SIZE);
									writeIndex = 0;
								}
							}
						}


						// orginal 128x128
						// 16x16

						// v to get a chunk v
						/*
						static const unsigned int MAX_MTU_SIZE = 1024;
						int orginalWidth = 128;
						int chunkHeight = 16;
						int chunkWidth = 16;
						char* pByteArray = new char[MAX_MTU_SIZE];
						int writeIndex = 0;
						int xOffset = 16;
						for (int y = 0; y < chunkHeight; ++y)
						{
							for (int x = 0; x < chunkWidth; ++x)
							{
								pByteArray[writeIndex] = buffer[y * orginalWidth + xOffset];
								writeIndex += 1;
								if (writeIndex > MAX_MTU_SIZE)
								{
									iResult = send(*m_ClientSock, pByteArray, strlen(pByteArray), 0);
									if (iResult == SOCKET_ERROR)
										std::cerr << "Send failed with error: " << WSAGetLastError() << '\n';
										
									memset(pByteArray, 0, MAX_MTU_SIZE);
									writeIndex = 0;
								}
							}								
						}*/

						//I don't know, it's a crappy failsafe
						canSend = false;
					}

					fclose(file);
					delete[] buffer;

				}
				else
				{
					std::cout << "Error: File not found.\n";
					canSend = false;
				}
			}
			else
			{
				std::cout << "Error: Filename does not end in .tga\n";
				canSend = false;
			}
		}
		
		if (canSend == true)
		{
		
			int	iResult = send(*m_ClientSock, msg.c_str(), strlen(msg.c_str()), 0);
			if (iResult == SOCKET_ERROR)
				std::cerr << "Send failed with error: " << WSAGetLastError() << '\n';
		}
	}
}

//////////////////////////////////////////////////
//	CutChunk
//////////////////////////////////////////////////
unsigned char*
Client::CutChunk(std::string msg, unsigned char* buffer)
{
	int xpos, ypos, width, height;
	int startPos;

	std::size_t found1 = msg.find("*");
	std::size_t found2 = msg.find("*",found1+1);
	std::size_t found3 = msg.find("*",found2+1);
	std::size_t found4 = msg.find("*",found3+1);

	if (found1 == std::string::npos || found2 == std::string::npos ||
		found3 == std::string::npos || found4 == std::string::npos || found4==msg.size())
	{	std::cout << "Not enough arguments. (Include X*Y*WIDTH*HEIGHT)\n";
		return nullptr;		}

	std::string xString = msg.substr(found1+1, found2-(found1+1));
	std::string yString = msg.substr(found2+1,found3-(found2+1));
	std::string widthString = msg.substr(found3+1, found4-(found3+1));
	std::string heightString = msg.substr(found4+1);

	try
	{
		xpos = stoi(xString);
		ypos = stoi(yString);
		width = stoi(widthString);
		height = stoi(heightString);
	}
	catch (const std::exception& e)
	{	
		std::cout << "Error: One or more arguments were not numbers.\n";
		return nullptr;
	}

	int imageWidth = 50;
	int imageHeight = 50;

	startPos = (ypos*imageWidth) + xpos;

	if (xpos + width > imageWidth || ypos + height > imageHeight)
	{
		std::cout << "Error: Chunk larger than target image.\n";
		return nullptr;
	}

	unsigned char* newBuffer = new (unsigned char[sizeof(buffer)]);
	int bufferPos=0;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; x++)
		{
			newBuffer[bufferPos]   = buffer[startPos + (((imageWidth * y) + x) * 4)];
			newBuffer[bufferPos+1] = buffer[startPos + (((imageWidth * y) + x) * 4)+1];
			newBuffer[bufferPos+2] = buffer[startPos + (((imageWidth * y) + x) * 4)+2];
			newBuffer[bufferPos+3] = buffer[startPos + (((imageWidth * y) + x) * 4)+3];
			bufferPos += 4;
		}
	}
	//buffer = newBuffer;

	//ReceiveImage(reinterpret_cast<char*>(newBuffer));

	return newBuffer;
}


//////////////////////////////////////////////////
//	Receive
//////////////////////////////////////////////////
void
Client::Receive( void )
{
	char arrRecvMsg[ MAX_CHARS ];
	std::string msg;

	while( true )
	{
		int recvSize = recv( *m_ClientSock, arrRecvMsg, MAX_CHARS, 0 );

		if (arrRecvMsg[0] == '1'|| arrRecvMsg[0] == '2')
		{
			msg.clear();
			for (uInt i = 0; i < (uInt)recvSize; ++i)
				msg.push_back(arrRecvMsg[i]);

			ReceiveImage(arrRecvMsg, msg);
		}
		else
		{
			if (recvSize > 0)
			{
				msg.clear();
				for (uInt i = 0; i < (uInt)recvSize; ++i)
					msg.push_back(arrRecvMsg[i]);

				std::cout << "\nMsg: " << msg << '\n';
			}
		}
	}
}

//////////////////////////////////////////////////
//	ReceiveImage
//////////////////////////////////////////////////
void
Client::ReceiveImage(char arrRecvMsg[], std::string msg)
{
	std::cout << "\nReceiving file ...\n";

	//static const unsigned int MAX_MTU_SIZE = 1024;

	std::cout << msg << std::endl;
	int width = 50;
	int height = 50;
	int ID = 0;
	int amountChunks = 1;

		// Get info from header
	std::size_t found1 = msg.find("*");
	std::size_t found2 = msg.find("*", found1 + 1);
	std::size_t found3 = msg.find("*", found2 + 1);
	std::size_t found4 = msg.find("*", found3 + 1);

	std::string filename = msg.substr(found1 + 1, found2 - (found1 + 1));
	std::string widthString = msg.substr(found2 + 1, found3 - (found2 + 1));
	std::string heightString = msg.substr(found3 + 1, found4 - (found3 + 1));
	std::string chunkString = msg.substr(found4 + 1);

	width = stoi(widthString);
	height = stoi(heightString);
	amountChunks = stoi(chunkString);
	int bpp = 4;

	//char* pOutbuffer = new char[width*height*bpp];
	unsigned char* buffer = new unsigned char[width*height*bpp];

	// bpp = bytes per pixel ( 4 )
	int lastCopiedIndex = 0;
	int bytesLeft = width * height*bpp;


	//while (tcp.poll() == true) <- Make this work? How?
	for(int i=0; i<amountChunks; i++)	// <- Temp solution meanwhile
	{
		int recvSize = recv(*m_ClientSock, arrRecvMsg, MAX_MTU_SIZE, 0);
		char* pReadBuffer = new char[MAX_MTU_SIZE];
		for (uInt i = 0; i < (uInt)recvSize; ++i)
			pReadBuffer[i]=arrRecvMsg[i];

		memcpy(&buffer[lastCopiedIndex], pReadBuffer, MAX_MTU_SIZE);
		lastCopiedIndex += MAX_MTU_SIZE;
		bytesLeft -= MAX_MTU_SIZE;
		if (bytesLeft < MAX_MTU_SIZE)
		{
			recvSize = recv(*m_ClientSock, arrRecvMsg, bytesLeft, 0);
			for (uInt i = 0; i < (uInt)recvSize; ++i)
				pReadBuffer[i] = arrRecvMsg[i];

			memcpy(&buffer[lastCopiedIndex], pReadBuffer, bytesLeft);
			//break;
		}
	}


	unsigned char* pixels;

	if (buffer != nullptr)
	{

		int startY = 0;
		int fakeID = ID;

		int pos = 0;

		pixels = (unsigned char*)tgaRead(buffer, TGA_READER_ABGR);


		targa_header header;       // variable of targa_header type

		header.id_length = 0;
		header.map_type = 0;
		header.image_type = 2;

		header.map_first = 0;
		header.map_length = 0;
		header.map_entry_size = 0;

		header.x = 0;
		header.y = 0;

		header.width = width;
		header.height = height;

		header.bits_per_pixel = 32;

		header.misc = 0x20;       // scan from upper left corner, wut dude


		LPCSTR szDirPath = "downloads";
		CreateDirectory(szDirPath, NULL);
		FILE *tga2;                 // pointer to file that we will write
		std::string fileName = (std::string) "downloads/"+m_UserName+"_file1.tga";
		tga2 = fopen(fileName.c_str(), "wb");


		write_header(header, tga2);

		// To write in entire image
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; x++)
			{
				// B G R order 

				fputc(pixels[(((width * y) + x) * 4) + 2], tga2);
				fputc(pixels[(((width * y) + x) * 4) + 1], tga2);
				fputc(pixels[(((width * y) + x) * 4) + 0], tga2);
				fputc(pixels[(((width * y) + x) * 4) + 3], tga2);

			}
		}

		fclose(tga2);

	}

	// To write a chunk
	/*
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; x++)
		{
			// B G R order 

			fputc(pixels[startPos + (((256 * y) + x) * 4) + 2], tga2);
			fputc(pixels[startPos + (((256 * y) + x) * 4) + 1], tga2);
			fputc(pixels[startPos + (((256 * y) + x) * 4) + 0], tga2);
			fputc(pixels[startPos + (((256 * y) + x) * 4) + 3], tga2);

		}
	}

	fclose(tga2);
	*/

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
