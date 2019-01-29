#include	"Server.hpp"


int main()
{
	//std::string serverIP;
	//std::cout << "Enter server IP: ";
	//std::cin >> serverIP;

	//uInt serverPort;
	//std::cout << "Enter server port: ";
	//std::cin >> serverPort;

	//Server serv;
	//serv.SetServerIP( serverIP.c_str() );
	//serv.SetServerPort( serverPort );
	//serv.Run();


	Server serv;
	serv.SetServerIP( "127.0.0.1" );
	serv.SetServerPort( 147 );
	serv.Run();

}