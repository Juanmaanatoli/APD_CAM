#pragma once
#ifndef __CAMSERVER_H__

#define __CAMSERVER_H__

#include "InterfaceDefs.h"
#include "UDPServer.h"

class CCamServer : public CUDPServer
{
public:
    CCamServer() : m_type(0), m_PacketSize(16384), m_RequestedData(0), m_hEvent(NULL), m_pBuffer(NULL), m_Length(0), m_DataReceived(0), m_UserData(0), m_SignalFrequency(1) {}
    void SetBuffer(void* buffer, unsigned long long length) {m_pBuffer = (unsigned char*)buffer; m_Length = length; m_MaxPacketNo = (m_PacketSize > 0) ? (unsigned int)(m_Length / m_PacketSize): 0; }
    void SetPacketSize(int packetSize) { m_PacketSize = packetSize; m_MaxPacketNo = (m_PacketSize > 0) ? (unsigned int)(m_Length / m_PacketSize): 0; }
    void SetNotification(unsigned long long requestedData, CEvent *hEvent) { m_RequestedData = requestedData; m_hEvent = hEvent; }
    void SetSignalFrequency(unsigned int frequency) { if (frequency > 0) m_SignalFrequency = frequency; }
    void Reset() { m_DataReceived = 0; m_UserData = 0; m_PacketCounter = 0; }
    void SetType(int type) { m_type = type; }

    unsigned long long GetReceivedData() { return m_DataReceived; }
    unsigned long long GetMaxPacketNo() { return m_MaxPacketNo; }
    unsigned long long GetPacketNo() { return m_PacketCounter; }

private:
	virtual void OnRead();
	virtual int GetRvBufferSize();

	int m_type; // 0_ one-shot, 1:cyclic
	int m_PacketSize;
    unsigned long long m_RequestedData;
	CEvent *m_hEvent;

	unsigned char *m_pBuffer; 
    unsigned long long m_Length;
    unsigned long long m_DataReceived;
    unsigned long long m_UserData;
	unsigned int m_MaxPacketNo;  // The size of buffer measured in packets.
	unsigned int m_PacketCounter;
	unsigned int m_SignalFrequency; // In Cyclic mode the event is signaled when m_PacketCounter % m_SignalFrequency == 0;
};

#endif  /* __CAMSERVER_H__ */
