#ifndef PACKETHANDLER_H
#define PACKETHANDLER_H

#include "winsock2.h"
#include "Ws2tcpip.h"
#include "mswsock.h"

#include <vector>

class PacketHandler
{
public:
	PacketHandler(int sock, PCSTR ip); //Constructor calls PrepareSocket()
	~PacketHandler();

	void PrepareSocket(int socket, PCSTR ip);
	std::vector<std::vector<char>> ReceivePackets();

	template<typename T>
	void SendPacket(T packet)
	{
		int result;
		result = sendto(m_socket, (char*)&packet, sizeof(packet), 0, (SOCKADDR*)&m_address, sizeof(m_address));
		if (result == SOCKET_ERROR)
			std::cout << "sendto() failed: Error " << WSAGetLastError() << std::endl;
	}

private:
	sockaddr_in m_address;
	SOCKET m_socket;
};


#endif // PACKETHANDLER_H