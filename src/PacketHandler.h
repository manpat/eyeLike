#ifndef PACKETHANDLER_H
#define PACKETHANDLER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

#include <iostream>
#include <vector>
#include <string>

class PacketHandler
{
public:
	PacketHandler(int port); //Constructor calls PrepareSocket()
	~PacketHandler();

	void PrepareSocket(int port);
	std::vector<std::string> ReceivePackets();

	template<typename T>
	void SendPacket(const T& packet)
	{
		int result;
		result = sendto(m_socket, (char*)&packet, sizeof(packet), 0, (SOCKADDR*)&m_prevrecv, sizeof(m_prevrecv));
		if (result == SOCKET_ERROR)
			std::cout << "sendto() failed: Error " << WSAGetLastError() << std::endl;
	}

private:
	sockaddr_in m_prevrecv;
	SOCKET m_socket;
};


#endif // PACKETHANDLER_H