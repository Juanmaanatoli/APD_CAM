#include "CamServer.h"

void CCamServer::OnRead()
{
	unsigned char *pBuffer = m_pBuffer + ((m_PacketCounter % m_MaxPacketNo) * m_PacketSize);
	sockaddr client;
	socklen_t client_length = sizeof(client); 

	int bytes_received = ReadData(pBuffer, m_PacketSize, &client, &client_length);

	if (m_type == 0)
	{ // one-shot mode
		if (bytes_received != -1 && m_UserData < m_RequestedData)
		{
			m_PacketCounter++;
			m_DataReceived += bytes_received;
			m_UserData  += bytes_received - 32;

			if (m_UserData >= m_RequestedData && m_hEvent != NULL) 
			{
				m_hEvent->Set();
			}
			if (((m_PacketCounter % m_SignalFrequency) == 0) && (m_hEvent != NULL)) 
				m_hEvent->Set();

/*
			if (client.sa_family == AF_INET)
			{
				sockaddr_in *p = (sockaddr_in*)&client;
				printf("0x%08X, %d\n ", ntohl(p->sin_addr.S_un.S_addr), p->sin_port);
			}
			for (int i = 0; i < bytes_received; i++)
			{
				printf("0x%02X ", buffer[i]);
			}
			printf("\n");
*/
		}
	}
	else
	{ // cyclic mode
		if (bytes_received != -1)
		{
			m_PacketCounter++;
			m_DataReceived += bytes_received;
			m_UserData  += bytes_received - 32;
/*
			if (m_UserData >= m_RequestedData && m_hEvent != NULL) 
			{
				m_hEvent->Set();
			}
*/
			if (/*(m_PacketCounter > 0) && -- never zero here*/((m_PacketCounter % m_SignalFrequency) == 0) && (m_hEvent != NULL)) 
				m_hEvent->Set();

		}
	}
}

int CCamServer::GetRvBufferSize()
{
	return 1024 * 1024;
}
