#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <string>
#include <sstream>

using std::cerr; using std::endl;

int main(){
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0){
		cerr << "Fuck WSA is fucked" << endl;
		return -1;
	}

	cerr << "Init" << endl;

	auto sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, "1", 1);
	ioctlsocket(sock, FIONBIO, (unsigned long*)1);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1337);
	addr.sin_addr.s_addr = INADDR_ANY;

	bind(sock, (const sockaddr*)(&addr), sizeof(addr));

	sockaddr from;
	int fromlen;
	char buf[512] = {0};

	while(true){
		fromlen = sizeof(from);
		auto ret = recvfrom(sock, buf, 512, 0, &from, &fromlen);
		if(ret > 0){
			std::string data(buf, ret);

			cerr << "Received data " << ret << " " << data << endl;

			if(data.find("\x01\x02\x03\x04") != data.npos){
				std::wstringstream thing;

				thing << "Blah " << 3.1415926 << " ";
				thing << (true?"true":"false") << " " << 55;

				auto thingstr = thing.str();
				sendto(sock, (char*)thingstr.data(), thingstr.size()*2, 0, &from, fromlen);
				// cerr << "Sent " << thingstr.data() << endl;
			}else{
				sendto(sock, data.data(), data.size(), 0, &from, fromlen);
			}
		}
	}

	return 0;
}