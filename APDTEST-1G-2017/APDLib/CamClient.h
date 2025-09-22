#pragma once
#ifndef __CAMCLIENT_H__

#define __CAMCLIENT_H__

#include "GECClient.h"

class CCamClient : public CGECClient
{
public:
	CCamClient() : m_pdiIrqCount(0), m_sdiIrqCount(0) {};
	int GetPdiIrqCount() { int t = m_pdiIrqCount; m_pdiIrqCount = 0; return t;};
	int GetSdiIrqCount() { int t = m_sdiIrqCount; m_sdiIrqCount = 0; return t;};
private:
	virtual void OnAck(unsigned char *buffer, int length, void *userData);
	virtual void OnIrq(unsigned char *buffer, int length);
	virtual void OnPdiData(unsigned char *buffer, int length, void *userData);
	virtual void OnSocketClosed(void *userData);
	virtual void OnReceiveError(void *userData);

	int m_pdiIrqCount;
	int m_sdiIrqCount;
};

#endif  /* __CAMCLIENT_H__ */
