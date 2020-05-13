
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Networking.hpp"

sf::TcpSocket Netplay::t_socket{
	
};

sf::UdpSocket Netplay::u_socket{

};

char Netplay::in[128]{

};

char* Netplay::convertToChar(wchar_t a, int size) {
	int i;
	char* s;
	for (i = 0; i < size; i++) {
		s = s + a[&i];
	}
	return s;
}

wchar_t Netplay::convertToWChar(char* a, int size) {
	int i;
	wchar_t s;
	for (i = 0; i < size; i++) {
		s = s + a[i];
	}
	return s;
}