#include	"Client.hpp"


int main()
{
	//std::string serverIP;
	//std::cout << "Enter server IP: ";
	//std::cin >> serverIP;

	//uInt serverPort;
	//std::cout << "Enter server port: ";
	//std::cin >> serverPort;

	//std::string userName;
	//std::cout << "Enter a username: ";
	//std::cin >> userName;

	//Client client;
	//client.SetServerIP( serverIP.c_str() );
	//client.SetServerPort( serverPort );
	//client.SetUserName( userName );
	//client.Run();


	Client client;

	std::string userName;
	std::cout << "Enter a username: ";
	std::cin >> userName;

	client.SetUserName( userName );
	client.SetServerIP( "127.0.0.1" );
	client.SetServerPort( 147 );
	client.Run();

}