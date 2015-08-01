#include "PacketHandler.h"

PacketHandler::PacketHandler(int port)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cout << "WSAStartupfailed." << std::endl;
		exit(0);
	}

	PrepareSocket(port);
}

PacketHandler::~PacketHandler()
{
}

void PacketHandler::PrepareSocket(int port)
{
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == SOCKET_ERROR)
	{
		std::cout << "Error Opening socket: Error " << WSAGetLastError();
		exit(0);
	}

	sockaddr_in serv_address;
	serv_address.sin_family = AF_INET;
	serv_address.sin_port = htons(port);
	serv_address.sin_addr.s_addr = INADDR_ANY;

	bind(m_socket, (const sockaddr*)(&serv_address), sizeof(serv_address));

	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, "1", 1);
	// DWORD dwBytesReturned = 0;
	// BOOL bNewBehavior = FALSE;
	// WSAIoctl(m_socket, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);
}

std::vector<std::string> PacketHandler::ReceivePackets()
{
	std::vector<std::string> receivedData;
	static char buffer[10000];

	int waiting;
	do
	{
		fd_set checksockets;
		checksockets.fd_count = 1;
		checksockets.fd_array[0] = m_socket;
		struct timeval t;
		t.tv_sec = 0;
		t.tv_usec = 0;
		waiting = select(m_socket, &checksockets, nullptr, nullptr, &t);
		if (waiting > 0)
		{
			int result;
			int length = sizeof(sockaddr_in);
			result = recvfrom(m_socket,	buffer, 10000, 0, (SOCKADDR*)&m_prevrecv, &length);

			if (result == SOCKET_ERROR)
			{
				auto err = WSAGetLastError();
				if(err != WSAECONNRESET)
					std::cout << "recvfrom() failed: Error " << err << std::endl;
			}
			else
			{
				std::string packetData(buffer, result);
				receivedData.push_back(packetData);
			}
		}
	} while (waiting);

	return receivedData;
}