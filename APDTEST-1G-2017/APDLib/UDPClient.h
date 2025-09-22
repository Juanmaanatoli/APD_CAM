#pragma once
#ifndef __UDPCLIENT_H__

#define __UDPCLIENT_H__

#include <list>

#include "SysLnxClasses.h"

#include "InterfaceDefs.h"

struct DATADESCRIPTOR
{
        /*
        The ipAddress and ipPort are in network byte order.
        */
        DATADESCRIPTOR() : ipAddress_n(0), ipPort_n(0), buffer(NULL), length(0), userData(0) {}
        DATADESCRIPTOR(unsigned char *_buffer, int _length, unsigned int _ipAddress_n = 0, unsigned short _ipPort_n = 0, void *_userData = 0) :ipAddress_n(_ipAddress_n), ipPort_n(_ipPort_n), buffer(_buffer) , length(_length), userData(_userData) {}
        unsigned int ipAddress_n;
        unsigned short ipPort_n;
        unsigned char *buffer;
        int length;
        void *userData;
};

typedef std::list<DATADESCRIPTOR> ITEMLIST;

class CUDPClient : public UDPBase, public Thread
{
public:
	CUDPClient(void);
	~CUDPClient(void);

	// The ipAddress and ipPort must be in host byte order. The methode convert them using the
	// htonl() (host to network long) or htons() (host to network short) respectively.
	bool SetIPAddress(char *ipAddress);
    bool SetIPAddress(unsigned int ipAddress_h);
    void SetPort(unsigned short port_h);
    void BindClient(unsigned short client_port_h);
    bool SendData(unsigned char* buffer, int length, unsigned int ipAddress_h = 0, unsigned short ipPort_h = 0, void *userData = 0);

private:
    unsigned int Handler(void);
    virtual int GetRecvTO() { return 1000;}

	void InternalSend();

	virtual void OnBeforeSend(int clientSocket, unsigned char *buffer, int length, sockaddr_in &sckadddr, void *userData);
	virtual void OnAfterSend(int clientSocket, unsigned char *buffer, int length, void *userData);
    virtual void OnNetworkEvent(int & /*clientSocket*/) {}

	ITEMLIST  m_ItemList;
	CEvent   *m_SendSignal;
	Mutex     m_csSend;
	/* The m_ulIPAddress and m_usPort store address and port number in network byte order (big-endian) */
    unsigned int m_ulIPAddress_n;
    unsigned short m_usPort_n;
	bool m_bBindClient;	// if true, binds the client to the fix m_usClientPort_n port
    unsigned short m_usClientPort_n;
};

#endif  /* __UDPCLIENT_H__ */
