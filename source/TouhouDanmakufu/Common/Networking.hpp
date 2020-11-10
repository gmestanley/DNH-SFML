
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network.hpp"
#include <iostream>

class Netplay {
public:
	static sf::TcpSocket t_socket;
	static sf::UdpSocket u_socket;
	static char in[128];
	static char* convertToChar(wchar_t a, int size);
	static std::wstring convertToWString(const std::string &s);
};