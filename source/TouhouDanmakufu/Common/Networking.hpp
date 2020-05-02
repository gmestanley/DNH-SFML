
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "gstd/Logger.hpp"
#include "gstd/Script.hpp"
#include "SFML/Network.hpp"
#include <iostream>

class Netplay {
public:
	static sf::TcpSocket t_socket;
	static sf::UdpSocket u_socket;
	static char* convertToChar(wchar_t a, int size);
	static wchar_t convertToWChar(char* a, int size);
};