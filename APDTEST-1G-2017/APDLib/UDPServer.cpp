#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#include "UDPServer.h"
#include "LnxClasses.h"


CUDPServer::CUDPServer(void)
{
}

CUDPServer::~CUDPServer(void)
{
}

unsigned int CUDPServer::Handler(void)
{
	sockaddr_in sckdrTCPAddr;

	try
	{
		if (CreateSocket() == false)
		{
			return -1;
		}

		socketRAII sockraii(m_Socket);

		int rvBufferSize = GetRvBufferSize();
		if (rvBufferSize != 0)
		{
			int res = setsockopt(m_Socket,  SOL_SOCKET, SO_RCVBUF, &rvBufferSize, sizeof(rvBufferSize));
			if (res != 0)
			{
				m_ErrorCode = errno;
				fprintf(stderr, "Cannot set SO_RCVBUF: %s\n", strerror(m_ErrorCode));
				return -1;
			}
		}

		sckdrTCPAddr.sin_family = AF_INET;
		sckdrTCPAddr.sin_port = m_port_n;
		sckdrTCPAddr.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr("127.0.0.1");

		// Binds listening socket to the addres and port.
		if (bind(m_Socket, reinterpret_cast<const sockaddr*>(&sckdrTCPAddr), sizeof(sckdrTCPAddr)))
		{
			m_ErrorCode = errno;
			fprintf(stderr, "Cannot bind to INADDR_ANY:%d : %s", m_port_n, strerror(m_ErrorCode));
			return -1;
		}

		// hEvent is a manual reset event, and it is created in nonsignaled state.
		CLnxEvent networkEvent(m_Socket);

		CLnxWaitForEvents waitObjects;
		waitObjects.Add(m_ExitSignal);
		waitObjects.Add(&networkEvent);

		m_ErrorCode = 0;
		InitDone();

		bool quit = false;
		while (!quit)
		{
			int index = -1;
			if (waitObjects.WaitAny(-1, &index) != CWaitForEvents::WR_OK)
			{
				fprintf(stderr, "WaitAny is not WR_OK!\n");
			}

			switch (index)
			{
			case 0:
				// Close request
				quit = true;
				break;
			case 1:
				// Read data
				OnRead();

				break;
			default:
				break;
			}
		}
	}
	catch(...)
	{
	}

	return 0;
}

void CUDPServer::OnRead()
{
	unsigned char buffer[2048];
	sockaddr client;
	socklen_t client_length = sizeof(client); 

	int bytes_received = ReadData(buffer, sizeof(buffer), &client, &client_length);
	if (bytes_received != -1)
	{
	}
}

int CUDPServer::GetRvBufferSize()
{
	return 0;
}


int CUDPServer::ReadData(unsigned char *buffer, int length, sockaddr *from, socklen_t *fromlen)
{
	
	int bytes_received = recvfrom(m_Socket, (char*)buffer, length, 0, (struct sockaddr *)from, fromlen);
	if (bytes_received == -1)
	{
		m_ErrorCode = errno;
	}

	return bytes_received;
}
