#include <list>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/capability.h>
#include <sys/resource.h>
#include <sys/mman.h>

#include "LnxClasses.h"
#include "CamClient.h"

using namespace std;

/* ******************* CLnxEvent ******************* */

typedef list<CEvent*> EVENTLIST;

CLnxEvent::CLnxEvent()
{
	m_LastErrorCode = 0;
	pipe(pipefd);

	fcntl(pipefd[0], F_SETFD, FD_CLOEXEC);
	fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);

	fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
	fcntl(pipefd[1], F_SETFL, O_NONBLOCK);
}

CLnxEvent::CLnxEvent(int fd)
{
	m_LastErrorCode = 0;
	pipefd[0] = fd;
	pipefd[1] = -1;
}

CLnxEvent::~CLnxEvent()
{
	if (pipefd[1] != -1)
	{
		close(pipefd[0]);
		close(pipefd[1]);
	}
}

void CLnxEvent::Set() 
{
	m_LastErrorCode = 0;
	if (write(pipefd[1], &m_LastErrorCode, 1) < 0)
		m_LastErrorCode = errno;
}

void CLnxEvent::Reset() 
{
	m_LastErrorCode = 0;
	if (IsSignaled() == false)
		return;

	read(pipefd[0], &m_LastErrorCode, sizeof(m_LastErrorCode));
}

bool CLnxEvent::IsSignaled()
{
	return Wait(0);
}

bool CLnxEvent::Wait(int timeout)
{
	struct pollfd pfd;

	pfd.fd = pipefd[0];
	pfd.events = POLLIN;
	pfd.revents = 0;

	int p = poll(&pfd, 1, timeout);
	if (p == 0)
		return false;
	else if (p == 1)
		return true;

	m_LastErrorCode = errno;
fprintf(stderr, "CLnxEvent::Wait() : %s\n", strerror(m_LastErrorCode));
	return false;
}

/* ******************* CLnxWaitForEvents ******************* */

size_t CLnxWaitForEvents::GetMaxWaitObjects()
{
	return MAXIMUM_WAIT_OBJECTS;
}

CWaitForEvents::WAIT_RESULT CLnxWaitForEvents::Add(CEvent *event) 
{
	if (m_Events.size() >= GetMaxWaitObjects())
	{
		return WR_TO_MANY_OBJECTS;
	}
	m_Events.push_back(event);
	m_Events.unique();
	return WR_OK;
};

void CLnxWaitForEvents::Remove(CEvent *event) 
{
	m_Events.remove(event);
};

void CLnxWaitForEvents::RemoveAll()
{
	m_Events.clear();
}


void CLnxWaitForEvents::createPollFd(struct pollfd *pfd)
{
	int i = 0;

	for (EVENTLIST::const_iterator itr = m_Events.begin(); itr != m_Events.end(); ++itr, ++i)
	{
		pfd[i].fd = ((CLnxEvent*)(*itr))->readFd();

		pfd[i].events = POLLIN;
		pfd[i].revents = 0;
	}
}

CWaitForEvents::WAIT_RESULT CLnxWaitForEvents::WaitAll(int timeout) 
{
	struct pollfd pfd[m_Events.size()];
	int n = m_Events.size();

	createPollFd(pfd);
	while (n)
	{
		int index;
//#warning FIXME:timeout accumulates
		WAIT_RESULT res = WaitAny(pfd, n, timeout, &index);
		if (res != WR_OK)
			return res;

		--n;
		pfd[index].fd = pfd[n].fd;
		pfd[index].revents = 0;
	}

	return WR_OK;
}

CWaitForEvents::WAIT_RESULT CLnxWaitForEvents::WaitAny(pollfd *pfd, int size, int timeout, int *index)
{
	int i = 0;

	i = poll(pfd, size, timeout);
	if (i < 0)
	{
int lerrno = errno;
fprintf(stderr, "CLnxWaitForEvents::WaitAny: %s\n", strerror(lerrno));
		return WR_ERROR;
	}
	if (i == 0)
		return WR_TIMEOUT;

	i = 0;
	for (i = 0; i < size; ++i)
	{
		if (pfd[i].revents == POLLIN)
		{
			*index = i;
			break;
		}
	}

	return WR_OK;
}

CWaitForEvents::WAIT_RESULT CLnxWaitForEvents::WaitAny(int timeout, int *index)
{
	struct pollfd pfd[m_Events.size()];
	createPollFd(pfd);
	return WaitAny(pfd, m_Events.size(), timeout, index);
}


/* ******************* CLnxClientContext ******************* */

CLnxClientContext::CLnxClientContext()
{
	m_ClientContext.pBuffer = NULL;
	m_ClientContext.bufferLength = 0;
	m_ClientContext.dataLength = 0;
	m_ClientContext.hEvent = NULL;
}

CLnxClientContext::~CLnxClientContext()
{
}

void CLnxClientContext::CreateCLIENTCONTEXT()
{
	m_ClientContext.hEvent = pEvent;
	m_ClientContext.pBuffer = pBuffer;
	m_ClientContext.bufferLength = bufferLength;
	m_ClientContext.dataLength = 0;
}

/* ******************* CLnxClient ******************* */

CLnxClient::CLnxClient() 
{
	m_pClient = new CCamClient();
    unsigned int ip_n = inet_addr("10.123.13.101");
	m_IPAdress_h = ntohl(ip_n);
	m_Port_h = 56666;
}

CLnxClient::~CLnxClient()
{
	if (m_pClient)
	{
		delete m_pClient;
		m_pClient = NULL;
	}
}

void CLnxClient::SetUDPPort(unsigned short port_h)
{
	m_Port_h = port_h;
	if (m_pClient) m_pClient->SetPort(m_Port_h);
}

void CLnxClient::SetIPAddress(unsigned int ipAddress_h)
{
	m_IPAdress_h = ipAddress_h;
	if (m_pClient) m_pClient->SetIPAddress(m_IPAdress_h);
}

void CLnxClient::SetIPAddress(char *ipAddress)
{
    unsigned int ip_n = inet_addr(ipAddress);
	m_IPAdress_h = ntohl(ip_n);
}

void CLnxClient::Start()
{
	if (m_pClient)
	{
		m_pClient->SetPort(m_Port_h);
		m_pClient->SetIPAddress(m_IPAdress_h);
		bool res = m_pClient->Start(true);
		int i = 0;
		// error handling !
	}
}

void CLnxClient::Stop()
{
	if (m_pClient)
	{
		m_pClient->Stop();
	}
}

bool CLnxClient::SendData(GECCOMMAND* command, CClientContext *clientContext, unsigned int ipAddress_h, unsigned short ipPort_h)
{
	if (m_pClient)
	{
		((CLnxClientContext*)clientContext)->CreateCLIENTCONTEXT();
		return m_pClient->SendData((unsigned char*)command, ((GENERAL_MESSAGE*)command)->GetCommandLength(), ipAddress_h, ipPort_h, (void*)&((CLnxClientContext*)clientContext)->m_ClientContext);
	}
	return false;
}

bool CLnxClient::SendData(BULKCMD* commands, CClientContext *clientContext, unsigned int ipAddress_h, unsigned short ipPort_h)
{
	if (m_pClient)
	{
		((CLnxClientContext*)clientContext)->CreateCLIENTCONTEXT();
		return m_pClient->SendData((unsigned char*)commands, commands->GetCommandLength(), ipAddress_h, ipPort_h, (void*)&((CLnxClientContext*)clientContext)->m_ClientContext);
	}
	return false;
}

/* ******************* CLnxServer ******************* */

CLnxServer::CLnxServer()
{
	m_pServer = new CCamServer();
}

CLnxServer::~CLnxServer()
{
	if (m_pServer)
	{
		delete m_pServer;
		m_pServer = NULL;
	}
}

void CLnxServer::SetListeningPort(unsigned short port_h)
{
	if (!m_pServer) return; // throw ?
	m_pServer->SetListeningPort(port_h);
}

void CLnxServer::SetBuffer(unsigned char *buffer, unsigned long long size)
{
	if (!m_pServer) return; // throw ?
    m_pServer->SetBuffer((void*)buffer, size);
}

void CLnxServer::SetPacketSize(unsigned int packetsize)
{
	if (!m_pServer) return; // throw ?
	m_pServer->SetPacketSize((int) packetsize);
}

void CLnxServer::SetNotification(unsigned int requested_data, CEvent *event)
{
	if (!m_pServer) return; // throw ?
    m_pServer->SetNotification((unsigned long long)requested_data, event);
}

void CLnxServer::SetSignalFrequency(unsigned int frequency)
{
	if (!m_pServer) return; // throw ?
	m_pServer->SetSignalFrequency(frequency);
}

void CLnxServer::Reset()
{
	if (!m_pServer) return; // throw ?
	m_pServer->Reset();
}
bool CLnxServer::Start()
{
	if (!m_pServer) return false; // throw ??
	return m_pServer->Start(true);
}

void CLnxServer::Stop()
{
	if (!m_pServer) return; // throw ?
	m_pServer->Stop();
}

void CLnxServer::SetType(SERVER_TYPE type)
{
	if (!m_pServer) return; // throw ?
	if ( type == ST_ONE_SHOT)
		m_pServer->SetType(0);
	else
		m_pServer->SetType(1);
}

unsigned int CLnxServer::GetReceivedData()
{
	if (!m_pServer) return 0; // throw ??
	return (unsigned int)m_pServer->GetReceivedData();
}

unsigned int CLnxServer::GetMaxPacketNo()
{
	if (!m_pServer) return 0; // throw ??
	return (unsigned int)m_pServer->GetMaxPacketNo();
}

unsigned int CLnxServer::GetPacketNo()
{
	if (!m_pServer) return 0; // throw ??
	return (unsigned int)m_pServer->GetPacketNo();
}

/* ****** CLnxNPMAllocator ******* */
#define MESSAGE_SIZE 1024

unsigned long long CLnxNPMAllocator::m_LockedSoFar = 0;

CLnxNPMAllocator::CLnxNPMAllocator(unsigned long long requestedSize) throw (CNPMemoryException) : CNPMAllocator(requestedSize)
{
	int pageSize = getpagesize();
	// Calculate the number of pages of memory to request.
    m_MemDesc.NumberOfPages = (unsigned long long)((requestedSize + pageSize) - 1) / pageSize;
	if (m_MemDesc.NumberOfPages == 0)
	{
		return;
	}

	int map_locked = 0;

	// Round up requested size to the next page boundery.
	m_BufferSize = m_MemDesc.NumberOfPages * pageSize;

	struct rlimit rlim;
	if (getrlimit(RLIMIT_MEMLOCK, &rlim))
	{
		int lerrno = errno;
		char message[MESSAGE_SIZE];

		snprintf(message, MESSAGE_SIZE, "Cannot get resource limits: %s\n", strerror(lerrno));
		ErrorLog(message);
	}
fprintf(stderr, "Lockable memory limits: %ld:%ld\n", rlim.rlim_cur, rlim.rlim_max);
	if (rlim.rlim_cur < m_BufferSize + m_LockedSoFar)
	{
		fprintf(stderr, "Trying to adjust the size of the lockable memory from %ld to %llu...\n", rlim.rlim_cur, m_LockedSoFar + m_BufferSize);
		fprintf(stderr, "Hardlimit: %ld\n", rlim.rlim_max);
		rlim.rlim_cur = m_LockedSoFar + m_BufferSize;
		if (rlim.rlim_max < rlim.rlim_cur)
		{
			cap_t caps;
			cap_flag_value_t cap_eff_state = CAP_CLEAR;
			cap_flag_value_t cap_perm_state = CAP_CLEAR;
			cap_flag_value_t cap_inh_state = CAP_CLEAR;

			if ((caps = cap_get_proc()) == NULL)
			{
				int lerrno = errno;
				char message[MESSAGE_SIZE];

				snprintf(message, MESSAGE_SIZE, "Cannot get capabilities: %s\n", strerror(lerrno));
				ErrorLog(message);
			}
			cap_get_flag(caps, CAP_SYS_RESOURCE, CAP_EFFECTIVE, &cap_eff_state);
			cap_get_flag(caps, CAP_SYS_RESOURCE, CAP_PERMITTED, &cap_perm_state);
			cap_get_flag(caps, CAP_SYS_RESOURCE, CAP_INHERITABLE, &cap_inh_state);
fprintf(stderr, "CAP_INHERITABLE is %s on CAP_SYS_RESOURCE\n", cap_inh_state == CAP_SET ? "set" : "not set");
			if (cap_eff_state == CAP_CLEAR && cap_perm_state == CAP_CLEAR)
			{
				char message[MESSAGE_SIZE];

				snprintf(message, MESSAGE_SIZE, "Cannot acquire CAP_SYS_RESOURCE\n");
				ErrorLog(message);
			}
			cap_value_t new_caps = CAP_SYS_RESOURCE;
			if (cap_set_flag(caps, CAP_PERMITTED, 1, &new_caps, CAP_SET))
			{
				int lerrno = errno;
				fprintf(stderr, "Cannot CAP_PERMITTED cap_set_flag: %s\n", strerror(lerrno));
			}
			if (cap_set_flag(caps, CAP_EFFECTIVE, 1, &new_caps, CAP_SET))
			{
				int lerrno = errno;
				fprintf(stderr, "Cannot CAP_EFFECTIVE cap_set_flag: %s\n", strerror(lerrno));
			}
			else if (cap_set_proc(caps))
			{
				int lerrno = errno;
				fprintf(stderr, "Cannot set CAP_SYS_RESOURCE: %s\n", strerror(lerrno));
			}

			cap_free(caps);
			rlim.rlim_max = rlim.rlim_cur;
		}
		if (setrlimit(RLIMIT_MEMLOCK, &rlim))
		{
			int lerrno = errno;
			char message[MESSAGE_SIZE];

			snprintf(message, MESSAGE_SIZE, "Cannot set resource limits: %s\n", strerror(lerrno));
			ErrorLog(message);
		}
		else
			map_locked = MAP_LOCKED;
	}
	else
		map_locked = MAP_LOCKED;

	void *addr = mmap(NULL, m_BufferSize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | map_locked /*| MAP_HUGETLB*/, -1, 0);

	if (addr == MAP_FAILED)
	{
		int lerrno = errno;
		char message[MESSAGE_SIZE];

		snprintf(message, MESSAGE_SIZE, "Cannot mmap() %llu bytes of memory: %s\n", m_BufferSize, strerror(lerrno));
		ErrorLog(message);
		throw CNPMemoryException();
	}

	m_LockedSoFar += m_BufferSize;
	m_MemDesc.lpMemReserved = addr;
}

void CLnxNPMAllocator::ErrorLog(char *message)
{
	fprintf(stderr, message);
}

CLnxNPMAllocator::CMemDesc::~CMemDesc()
{
	if (lpMemReserved)
	{
		if (munmap(lpMemReserved, NumberOfPages * getpagesize()))
		{
			int lerrno = errno;
			char message[MESSAGE_SIZE];

			snprintf(message, MESSAGE_SIZE, "Cannot munmap() memory: %s\n", strerror(lerrno));
			ErrorLog(message);
		}
	}
}

/* ****** CLnxFactory ******* */
CAPDServer* CLnxFactory::GetServer()
{
	return new CLnxServer();
}

CAPDClient* CLnxFactory::GetClient() 
{
	return new CLnxClient();
}

CEvent* CLnxFactory::GetEvent()
{
	return new CLnxEvent();
}

CWaitForEvents* CLnxFactory::GetWaitForEvents()
{
	return new CLnxWaitForEvents();
}

CClientContext* CLnxFactory::GetClientContext()
{
	return new CLnxClientContext();
}

CNPMAllocator* CLnxFactory::GetNPMemory(unsigned long long requestedSize)
{
	CLnxNPMAllocator *pAllocator = NULL;
	try
	{
		pAllocator = new CLnxNPMAllocator(requestedSize);
	}
	catch (const CNPMemoryException &e)
	{
	}

	return pAllocator;
}
