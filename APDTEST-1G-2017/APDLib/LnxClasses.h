#ifndef __LNXCLASSES_H__

#define __LNXCLASSES_H__

#include "InterfaceDefs.h"
#include "GECClient.h"
#include "CamServer.h"

class CCamClient;
#define MAXIMUM_WAIT_OBJECTS	64

class CLnxEvent : public CEvent
{
	class CLnxEventPrivate;
	friend class CLnxFactory;
	friend class CLnxWaitForEvents;
	friend class CLnxClient;
	friend class CLnxServer;
	friend class CLnxClientContext;
private:
	int readFd() const {return pipefd[0];}

	int pipefd[2];
	unsigned int m_LastErrorCode;
public:
	CLnxEvent(int fd);
	CLnxEvent();
	~CLnxEvent();
	void Set();
	void Reset();
	bool IsSignaled();
	bool Wait(int timeout);
	int GetError() { return m_LastErrorCode; };
};

class CLnxWaitForEvents : public CWaitForEvents
{
	friend class CLnxFactory;
private:
	void createPollFd(struct pollfd* pfd);
	WAIT_RESULT WaitAny(struct pollfd* pfd, int size, int timeout, int *index);
public:
	CLnxWaitForEvents() {};
	size_t GetMaxWaitObjects();
	WAIT_RESULT Add(CEvent *event);
	void Remove(CEvent *event);
	void RemoveAll();
	WAIT_RESULT WaitAll(int timeout);
	WAIT_RESULT WaitAny(int timeout, int *index);
};

class CLnxClientContext : public CClientContext
{
	friend class CLnxFactory;
	friend class CLnxClient;
private:
	CLnxClientContext();
	CLIENTCONTEXT m_ClientContext;
public:
	void CreateCLIENTCONTEXT();
	~CLnxClientContext();
};

class CLnxClient : public CAPDClient
{
	friend class CLnxFactory;
private:
    CLnxClient();
	CCamClient *m_pClient;
    unsigned int m_IPAdress_h;
    unsigned short m_Port_h;
public:

	~CLnxClient();
    void SetUDPPort(unsigned short port_h);
    void SetIPAddress(unsigned int ipAddress_h);
	void SetIPAddress(char *ipAddress);
	void Start();
	void Stop();
    bool SendData(GECCOMMAND* command, CClientContext *clientContext = 0, unsigned int ipAddress_h = 0, unsigned short ipPort_h = 0);
    bool SendData(BULKCMD* commands, CClientContext *clientContext = 0, unsigned int ipAddress_h = 0, unsigned short ipPort_h = 0);
};

class CLnxServer : public CAPDServer
{
	friend class CLnxFactory;
private:
	CLnxServer();
	CCamServer *m_pServer;
public:
	~CLnxServer();
    void SetListeningPort(unsigned short port_h);
    void SetBuffer(unsigned char *buffer, unsigned long long size);
	void SetPacketSize(unsigned int packetsize);
	void SetNotification(unsigned int requested_data, CEvent *event);
	void SetSignalFrequency(unsigned int frequency);
	void Reset();
	bool Start();
	void Stop();
	void SetType(SERVER_TYPE type);
	unsigned int GetReceivedData();
	unsigned int GetMaxPacketNo();
	unsigned int GetPacketNo();
};

class CLnxNPMAllocator : public CNPMAllocator
{
	friend class CLnxFactory;
private:
    CLnxNPMAllocator(unsigned long long requestedSize) throw(CNPMemoryException);

	class CMemDesc
	{
	public:
		CMemDesc() : NumberOfPages(0),
				lpMemReserved(NULL)
                {}
		~CMemDesc();
        unsigned long long NumberOfPages;	// number of pages to request
		void *lpMemReserved;
	};

	CMemDesc m_MemDesc;
    unsigned long long m_BufferSize;

	static void ErrorLog(char *message);

    static unsigned long long m_LockedSoFar;

public:
	unsigned char *GetBuffer() { return (unsigned char*)m_MemDesc.lpMemReserved; };
	unsigned int GetBufferSize() { return (unsigned int)m_BufferSize;};
};

class CLnxFactory : public  CAPDFactory
{
public:
	CAPDServer* GetServer();
	CAPDClient* GetClient();
	CEvent* GetEvent();
	CWaitForEvents* GetWaitForEvents();
	CClientContext* GetClientContext();
    CNPMAllocator* GetNPMemory(unsigned long long requestedSize);
};

#endif  /* __LNXCLASSES_H__ */
