#include <stdio.h>

#include "CamClient.h"


void CCamClient::OnAck(unsigned char *buffer, int length, void *userData) 
{
	if (userData == 0) return;

	CLIENTCONTEXT *pCilentContext = (CLIENTCONTEXT*)userData;

	pCilentContext->dataLength = min(pCilentContext->bufferLength, (unsigned int)length);
	if (pCilentContext->pBuffer != NULL)
	{
		memcpy(pCilentContext->pBuffer, buffer, pCilentContext->dataLength);
	}

	if (pCilentContext->hEvent != NULL) 
	{
		pCilentContext->hEvent->Set();
	}
}

void CCamClient::OnIrq(unsigned char *buffer, int /*length*/)
{
	unsigned char irqSource = *buffer;
	if (irqSource == 0)
	{
		m_pdiIrqCount++;
	}
	else if (irqSource == 1)
	{
		m_sdiIrqCount++;
	}
	else
	{
	}
}

void CCamClient::OnPdiData(unsigned char *buffer, int length, void *userData) 
{
	if (userData == 0)
		return;

	CLIENTCONTEXT *pCilentContext = (CLIENTCONTEXT*)userData;

	pCilentContext->dataLength = min(pCilentContext->bufferLength, (unsigned int)length);
	if (pCilentContext->pBuffer != NULL)
	{
		memcpy(pCilentContext->pBuffer, buffer, pCilentContext->dataLength);
	}

	if (pCilentContext->hEvent != NULL) 
	{
		pCilentContext->hEvent->Set();
	}
}

void CCamClient::OnSocketClosed(void *userData)
{
	CGECClient::OnSocketClosed(userData);
}

void CCamClient::OnReceiveError(void *userData)
{
	CGECClient::OnReceiveError(userData);
	if (userData == 0) return;

	CLIENTCONTEXT *pCilentContext = (CLIENTCONTEXT*)userData;
	if (pCilentContext->hEvent != NULL) 
	{
		pCilentContext->hEvent->Set();
	}
}
