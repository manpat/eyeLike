#include "PacketHandler.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
	m_address.sin_family = AF_INET;
	m_address.sin_port = htons(port);
	m_address.sin_addr.s_addr = INADDR_ANY;

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == SOCKET_ERROR)
	{
		std::cout << "Error Opening socket: Error " << WSAGetLastError();
		exit(0);
	}

	bind(m_socket, (const sockaddr*)(&m_address), sizeof(m_address));

	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, "1", 1);
	// DWORD dwBytesReturned = 0;
	// BOOL bNewBehavior = FALSE;
	// WSAIoctl(m_socket, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);
}

std::vector<std::vector<char>> PacketHandler::ReceivePackets()
{
	std::vector<std::vector<char>> receivedData;
	char buffer[10000];

	int waiting;
	do
	{
		fd_set checksockets;
		checksockets.fd_count = 1;
		checksockets.fd_array[0] = m_socket;
		struct timeval t;
		t.tv_sec = 0;
		t.tv_usec = 0;
		waiting = select(0, &checksockets, nullptr, nullptr, &t);
		if (waiting > 0)
		{
			int result;
			int length = sizeof(m_address);
			result = recvfrom(m_socket,	buffer, 10000, 0, (SOCKADDR*)&m_address, &length);

			if (result == SOCKET_ERROR)
			{
				std::cout << "recvfrom() failed: Error " << WSAGetLastError() << std::endl;
			}
			else
			{
				std::vector<char> packetData;

				int i = 0;
				while (buffer[i] != (char)-52) // This is bad, not sure how to check for empty
				{
					packetData.push_back(buffer[i]);
					i++;
				}

				receivedData.push_back(packetData);
			}
		}
	} while (waiting);

	return receivedData;
}