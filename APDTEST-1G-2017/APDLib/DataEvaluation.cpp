#include <stdio.h>

#include "TypeDefs.h"
#include "DataEvaluation.h"
#include "InternalFunctions.h"
#include "Helpers.h"

/* ********** Helpers for data evaluation  ********** */
bool FindPreamble(unsigned char* &pData, int &length)
{

	unsigned char c;
	while (length > 0)
	{
		c = 0x0C;
		while (*pData != c && length > 0)
		{
			pData++; length--;
		}
		while (*pData == c && length > 0)
		{
			pData++; c--; length--;
			if (c == 0) return true;
		}
	}
	return false;
}

unsigned short GetMask(int bitsPerSample)
{
    unsigned short mask = 0xFFFF;
	return mask >> (16 - bitsPerSample);
}

inline unsigned short GetData(unsigned char *pData, int offset, unsigned short mask)
{
	int byteOffset = offset >> 3; // / 8
	int bitOffset = offset & 0x07; // % 8;
	unsigned int data = *((unsigned int*)(pData + byteOffset));
	data = data >> bitOffset;
    return (unsigned short)data & mask;
}

/* ********** Legacy data evaluation methodes  ********** */

int ClearStream(unsigned char *pData, int length)
{
	int l = 0;
	unsigned char *pS = pData;
	unsigned char *pD = pData;

	CW_FRAME_V2 *pCW = (CW_FRAME_V2*)(pS + 1440);
    unsigned short ccp_H = ((pCW->continuityCounter_H & 0x00FF) << 8) + (pCW->continuityCounter_H >> 8);
	unsigned int continuityCounterPrev = (unsigned int)pCW->continuityCounter_L + ((unsigned int)ccp_H << 8);

	continuityCounterPrev--;

	unsigned int cc = 0;
	while (length > 0)
	{
		memcpy(pD, pS, 1440);
		pS += 1440;
		pCW = (CW_FRAME_V2*)pS;
        unsigned short cc_H = ((pCW->continuityCounter_H & 0x00FF) << 8) + (pCW->continuityCounter_H >> 8);
		unsigned int continuityCounter = (unsigned int)pCW->continuityCounter_L + ((unsigned int)cc_H << 8);


		continuityCounterPrev++;
		if (continuityCounterPrev != continuityCounter) 
		{
			printf("Continuity counter error.\n");
			return 0;
		}
		cc = continuityCounter;
	//printf("Cont Count: %d\n", cc);

		pS += sizeof(CW_FRAME_V2);
		length -= 1440 + sizeof(CW_FRAME_V2); 
		pD += 1440;
		l += 1440;
	}
	printf("Cont Count: %d\n", cc);

	return l;
}

void EvaluateData(unsigned char* pData, int length, int noofSample, unsigned char channelMask, int bits, int channelOffset)
{
    unsigned short *ptrs[8];
    ptrs[0] = new unsigned short[noofSample];
    ptrs[1] = new unsigned short[noofSample];
    ptrs[2] = new unsigned short[noofSample];
    ptrs[3] = new unsigned short[noofSample];
    ptrs[4] = new unsigned short[noofSample];
    ptrs[5] = new unsigned short[noofSample];
    ptrs[6] = new unsigned short[noofSample];
    ptrs[7] = new unsigned short[noofSample];

    /*LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER performanceCount1, performanceCount2;
    QueryPerformanceCounter(&performanceCount1);*/

	length = ClearStream(pData, length);
	if (length == 0) return; // There is a continuity error in the stream.
	printf("length: %d, No of sample: %d, Bits: %d, Channel mask: %d\n", length, noofSample, bits, channelMask);

	bool r = FindPreamble(pData, length);

	unsigned char dataType = *pData++; // 0:measurement, 6:test pattern
	length--;
	unsigned char streamNo = (*pData++ & 0x0F); // 1,2,3,4
	length--;

	int channels = GetBitCount(channelMask);
	int paddingBits;
	int sampleBytes = GetBlockSize(channels, bits, &paddingBits);

	if (sampleBytes == 0) return;

    int noofBlocks = std::min(length / sampleBytes, noofSample);
//	printf("noofBlocks: %d\n", noofBlocks);

    unsigned short *ptrs2[8];
	ptrs2[0] = ptrs[0];
	ptrs2[1] = ptrs[1];
	ptrs2[2] = ptrs[2];
	ptrs2[3] = ptrs[3];
	ptrs2[4] = ptrs[4];
	ptrs2[5] = ptrs[5];
	ptrs2[6] = ptrs[6];
	ptrs2[7] = ptrs[7];

    unsigned short mask = GetMask(bits);
	for (int i = 0; i < noofBlocks; i++)
	{
		int offset = paddingBits;
		for (int channel = 0; channel < channels; channel++)
		{
			*ptrs2[channel]++ = GetData(pData, offset, mask);
			offset += bits;
		}
		pData += sampleBytes; 
	}

    /*QueryPerformanceCounter(&performanceCount2);
	double ce = 1000*(double)(performanceCount2.QuadPart - performanceCount1.QuadPart)/(double)frequency.QuadPart;
    printf("Data conversion time: %g ms\n",ce);*/

	char ofName[512];
	for (int channel = 0; channel < channels; channel++)
	{
		int ch = GetBitPosition(channelMask, channel);

		snprintf(ofName, sizeof(ofName), "Channel%d.dat", ch + channelOffset);
		FILE *hFile = fopen(ofName, "w");
		if (hFile == NULL)
			break;
        unsigned short *p = ptrs[channel];
        fwrite(p, sizeof(unsigned short), noofBlocks, hFile);
		fclose(hFile);
	}

	delete ptrs[0];
	delete ptrs[1];
	delete ptrs[2];
	delete ptrs[3];
	delete ptrs[4];
	delete ptrs[5];
	delete ptrs[6];
	delete ptrs[7];

}

/* ********** CDataEvaluation class methodes  ********** */

CDataEvaluation::CDataEvaluation() : m_Triggered(false), m_WritePtr(0), m_DataLength(), m_Calibrated(false), m_Bits(0), m_ChannelMask(0), m_PacketSize(0),
    m_PreambSize(0), m_ContinuityCounterOffset(0), m_pDataNotificationSignal(NULL), m_pUserNotificationSignal(NULL)
{
	for (int i = 0; i < sizeof(m_TriggerInfo) / sizeof(m_TriggerInfo[0]); ++i)
	{
		m_TriggerInfo[i].TriggerInfo = 0;
		m_TriggerEnabled[i] = 0;
		m_TriggerLevels[i] = 0;
	}

    //m_ProcessingTime.QuadPart = 0;
	m_ProcessingCount = 0;
    /*m_LastCallingTime.QuadPart = 0;
    m_AvarageCallingTime.QuadPart = 0;*/

    //out.open("out.txt", ios::trunc | ios::binary);
}

CDataEvaluation::~CDataEvaluation()
{
    //out.close();
}

void CDataEvaluation::InternalStop()
{
	m_ExitSignal->Set();
}

unsigned int CDataEvaluation::Handler()
{
	GetMap(m_ChannelMask, m_ChannelMap, &m_ActiveChannelNo);
	m_BlockSize = GetBlockSize(m_ActiveChannelNo, m_Bits, &m_PaddingBits);
	m_Mask = GetMask(m_Bits);

	for (int channel = 0; channel < 8; channel ++)
	{
        m_ChannelData[channel] = (short*)(m_UserBuffer + m_UserBufferSize * channel);
	}

	m_SampleCount = 0;
    m_UserBufferSizeInSample = m_UserBufferSize / sizeof(unsigned short);
	m_PacketNo = 0;
	m_MaxNoofBlocks = 0;
	m_LastContinuityCounter = 0;
	m_Running = true;
    m_PreambSize = 0;

    m_StreamNo = 0;
    //m_LastCallingTime.QuadPart = 0;

	CWaitForEvents *waitObject = CAPDFactory::GetAPDFactory()->GetWaitForEvents();
	if (!waitObject) return 1;
	waitObject->Add(m_ExitSignal);
	waitObject->Add(m_pDataNotificationSignal);

    m_ProcessingCount = 0;
	m_CallingCount = 0;

	bool bQuit = false;
	while (!bQuit)
	{
		int index = -1;
		int retVal = waitObject->WaitAny(-1, &index);
		if (retVal == CWaitForEvents::WR_OK)
		{
			switch (index) 
			{
			case 0:	// exit thread signal
				{ 
                    /*LARGE_INTEGER frequency;
					QueryPerformanceFrequency(&frequency);

                    double ce = 1000*(double)(m_AvarageCallingTime.QuadPart)/(double)frequency.QuadPart;*/
                    /*if (m_CallingCount)
					{
						ce = ce / m_CallingCount;
						printf("Average calling time: %g ms (%d)\n",ce, m_StreamNo);
					}

                    //ce = 1000*(double)(m_ProcessingTime.QuadPart)/(double)frequency.QuadPart;
					if (m_ProcessingCount)
					{
						ce = ce / m_ProcessingCount;
						printf("Average data conversion time: %g ms (%d)\n",ce, m_StreamNo);
                    }*/

					bQuit = true;
				}
				break;
			case 1:	// data arrived signal 
				{
                    m_pDataNotificationSignal->Reset();
                    /*LARGE_INTEGER performanceCount1, performanceCount2;
                    QueryPerformanceCounter(&performanceCount1);*/

                    /*if (m_LastCallingTime.QuadPart != 0)
					{
						m_AvarageCallingTime.QuadPart += performanceCount1.QuadPart - m_LastCallingTime.QuadPart;
						m_CallingCount++;
					}
                    m_LastCallingTime.QuadPart = performanceCount1.QuadPart;*/

					ProcessData();

                    /*QueryPerformanceCounter(&performanceCount2);
                    m_ProcessingTime.QuadPart += performanceCount2.QuadPart - performanceCount1.QuadPart;*/
                    m_ProcessingCount++;
				}
				break;
			default:
				// other unknown error (timeout, etc.)
				break;
			}
		}
	}
	printf("Max noof blocks: %u, %llu (%u)\n", m_MaxNoofBlocks, m_SampleCount, m_StreamNo);

	delete waitObject;

	return 0;
}

void CDataEvaluation::ProcessData()
{
	unsigned int packetNo = m_Server->GetPacketNo();
	unsigned int maxPacketNo = m_Server->GetMaxPacketNo();

	m_MaxNoofBlocks = max(m_MaxNoofBlocks, (packetNo - m_PacketNo)); // Maximum number of block to process

//	m_SampleIndex = m_SampleCount % m_UserBufferSizeInSample;

	while (m_PacketNo < packetNo)
	{
		unsigned char* pSource = m_SrcBuffer + ((m_PacketNo % maxPacketNo) * (m_PacketSize + sizeof(CW_FRAME_V2)));
		if (m_PacketNo == 0)
		{
			CW_FRAME_V2 *pCW = (CW_FRAME_V2*)(pSource + m_PacketSize);
            unsigned short ccp_H = ((pCW->continuityCounter_H & 0x00FF) << 8) + ((pCW->continuityCounter_H >> 8) & 0x00FF);
			m_LastContinuityCounter = ((unsigned int)pCW->continuityCounter_L + ((unsigned int)ccp_H << 8)) & 0x00FFFFFF;
			m_LastContinuityCounter--;
		}

		CW_FRAME_V2 *pCW = (CW_FRAME_V2*)(pSource + m_PacketSize);
        unsigned short cc_H = ((pCW->continuityCounter_H & 0x00FF) << 8) + ((pCW->continuityCounter_H >> 8) & 0x00FF);
		unsigned int continuityCounter = ((unsigned int)pCW->continuityCounter_L + ((unsigned int)cc_H << 8)) & 0x00FFFFFF;
		m_LastContinuityCounter++;
		m_LastContinuityCounter = m_LastContinuityCounter & 0x00FFFFFF;

#ifdef CC24
		if (m_LastContinuityCounter == continuityCounter) 
#else
        if ((m_LastContinuityCounter & 0x000000FF) == (continuityCounter & 0x000000FF))
#endif
		{
            //ProcessFrame(pSource, m_PacketNo);
            m_ContinuityCounterOffset = 0;
		}
		else
		{
            printf("Continuity counter error: %d, %d. (%d) %d, %d\n", m_PacketNo, continuityCounter, m_StreamNo, m_SampleCount, m_SampleIndex);
            //out <<"("<< m_StreamNo <<") " << ((m_LastContinuityCounter) * m_PacketSize - m_PreambSize) % m_BlockSize << endl;
            //out << m_LastContinuityCounter <<" "<< continuityCounter <<" offset: "<< m_LastContinuityCounter*(m_SampleCount * m_BlockSize)+20 << " real offset: " << continuityCounter*(m_SampleCount * m_BlockSize)+20 <<endl;
            m_ContinuityCounterOffset = ((m_LastContinuityCounter) * m_PacketSize - m_PreambSize) % m_BlockSize;
            /*m_pUserNotificationSignal->Set();
            // return 0; // ?
            break;*/
        }
        ProcessFrame(pSource, m_PacketNo);
		m_PacketNo++;
	}

    //m_pUserNotificationSignal->Set();
}

void CDataEvaluation::ProcessFrame(unsigned char *pFrame, unsigned int packetNo)
{
	if (!m_Running) return;

    if(m_ContinuityCounterOffset!=0)
    {
        memcpy(m_WorkBuffer, &pFrame[m_ContinuityCounterOffset], m_PacketSize-m_ContinuityCounterOffset);
        m_DataLength = m_PacketSize-m_ContinuityCounterOffset;
    }
    else
    {
        memcpy(m_WorkBuffer + m_WritePtr, pFrame, m_PacketSize);
        m_DataLength += m_PacketSize;
    }

    /*memcpy(m_WorkBuffer + m_WritePtr, pFrame, m_PacketSize);
    m_DataLength += m_PacketSize;*/

    //out.write(reinterpret_cast<char*>(m_WorkBuffer), m_PacketSize);

	unsigned char* pData = m_WorkBuffer;
	if (packetNo == 0)
	{
		if (FindPreamble(pData, m_DataLength))
		{
			unsigned char dataType = *pData++; // 0:measurement, 6:test pattern
			m_DataLength--;
			m_StreamNo = (unsigned int )(*pData++ & 0x0F); // 1,2,3,4
			m_DataLength--;
			printf("Preamble found. Stream No: %d\n", m_StreamNo);
            m_PreambSize = m_PacketSize - m_DataLength;
		}
        else
        {
            printf("Could not find Preamble\n");

        }
	}

    /*if(m_StreamNo == 0)
    {
        if (FindPreamble(pData, m_DataLength))
        {
            unsigned char dataType = *pData++; // 0:measurement, 6:test pattern
            m_DataLength--;
            m_StreamNo = (unsigned int )(*pData++ & 0x0F); // 1,2,3,4
            m_DataLength--;
            printf("Preamble found. Stream No: %d\n", m_StreamNo);
        }
        else
        {
            printf("Could not find Preamble\n");

        }

    }//if*/

	m_SampleIndex = m_SampleCount % m_UserBufferSizeInSample;

	while (m_DataLength > m_BlockSize)
	{
		ProcessBlock(pData); // ProcessBlock(m_TempBuffer + m_ReadPtr);
		pData += m_BlockSize;
		m_DataLength -= m_BlockSize;
	}
	memcpy(m_WorkBuffer, pData, m_DataLength); // memcpy(m_WorkBuffer, m_WorkBuffer + m_ReadPtr, m_DataLength);
	m_WritePtr = m_DataLength;
}



#ifdef APD_VERSION
// APD version
void CDataEvaluation::ProcessBlock(unsigned char *pData)
{
	if (m_Running || m_StopAt == 0)
	{
		int offset = m_PaddingBits;
		int *channelMap = m_ChannelMap;
		for (int channel = 0; channel < m_ActiveChannelNo; channel++)
		{
			int luv = *channelMap++; // Gets the real channelindex
            short us = m_ChannelOffsets[luv] - (short)GetData(pData, offset, m_Mask);
			Trigger(luv, us);
			*(m_ChannelData[luv] + m_SampleIndex) = us;
			offset += m_Bits;
		}

		m_SampleCount++;
		m_SampleIndex++;
		if (m_SampleIndex >= m_UserBufferSizeInSample) m_SampleIndex = 0;

		if (m_StopAt != 0 && m_SampleCount == m_StopAt && m_pUserNotificationSignal) 
		{
			m_Running = false;
			m_pUserNotificationSignal->Set();
		}
	}
}
#else
// ADC version
void CDataEvaluation::ProcessBlock(unsigned char *pData)
{
	if (m_Running || m_StopAt == 0)
	{
		int offset = m_PaddingBits;
		int *channelMap = m_ChannelMap;
		for (int channel = 0; channel < m_ActiveChannelNo; channel++)
		{
			int luv = *channelMap++; // Gets the real channelindex
            short us = (short)GetData(pData, offset, m_Mask);
			Trigger(luv, us);
			*(m_ChannelData[luv] + m_SampleIndex) = us;
			offset += m_Bits;
		}
		m_SampleCount++;
		m_SampleIndex++;
		if (m_SampleIndex >= m_UserBufferSizeInSample) m_SampleIndex = 0;

		if (m_SampleCount == m_StopAt && m_StopAt != 0 && m_pUserNotificationSignal) 
		{
			m_Running = false;
			m_pUserNotificationSignal->Set();
		}
	}
}
#endif

void CDataEvaluation::Trigger(int channel, short data)
{
	if (m_Triggered)
	{
		int info = m_TriggerEnabled[channel];
		if (info)
		{
			if (info == 1)
			{//Sensitivity == 0
				if (m_TriggerLevels[channel] < data)
				{
					DoTrigger();
				}
			}
			else
			{//Sensitivity == 1
				if (m_TriggerLevels[channel] > data)
				{
					DoTrigger();
				}
			}
		}
	}
}

bool CDataEvaluation::SetParams(unsigned int bits, unsigned char channelMask, unsigned int packetSize) 
{ 
	if (bits != 8 && bits != 12 && bits != 14) return false;
	m_Bits = bits; 
	m_ChannelMask = channelMask; 
	m_PacketSize = packetSize; 

	return true; //?
};

void CDataEvaluation::SetCalibratedMode(bool calibrated) 
{ 
	m_Calibrated = calibrated;
	if (!m_Calibrated)
	{
        short max_value = 16383;
		switch (m_Bits)
		{
		case 8:
			max_value = 255;
			break;
		case 12:
			max_value = 4095;
			break;
		}
		for (int i = 0; i < 8; i++)
		{
			m_ChannelOffsets[i] = max_value;
		}
	}
};

void CDataEvaluation::SetTrigger(ADT_TRIGGERINFO *triggerInfo)
{
	m_Triggered = true;
	m_StopAt = 0;
	memcpy(m_TriggerInfo, triggerInfo, 8*sizeof(ADT_TRIGGERINFO));
	for (int i = 0; i < 8; i++)
	{
		if (!m_TriggerInfo[i].Enable)
		{
			m_TriggerEnabled[i] = 0;
		}
		else if (m_TriggerInfo[i].Sensitivity == 0)
		{
			m_TriggerEnabled[i] = 1;
			m_TriggerLevels[i] = m_TriggerInfo[i].TriggerLevel;
		}
		else
		{
			m_TriggerEnabled[i] = 2;
			m_TriggerLevels[i] = m_TriggerInfo[i].TriggerLevel;
		}
	}
}

bool CDataEvaluation::SetupCalibrationData(CAPDClient *client, int streamNo)
{
	if (m_Bits != 8 && m_Bits != 12 && m_Bits != 14) return false;

    short dacOffsets[32];
	int channelIndex = (streamNo -1) * 8;
	// Retrieves the dac offsets from the calibration table.
    bool retVal = RetrieveDACOffsets(client, dacOffsets, channelIndex, 8);
	if (!retVal) return retVal; // Error reading calibration data
	// Set dac offsets in the hw, using SetDACOffset()
    retVal = SetADCOffset(client, dacOffsets, channelIndex, 8);
	if (!retVal) return retVal; // Error while set DAC data

	// Retrieve ADC offsets from the non-volative store, normalize them, according to the actual bitnumber.
    short adcOffsets[32];
	retVal = RetrieveADCOffsets(client, adcOffsets, channelIndex, 8);
	if (!retVal) return retVal; // Error while retrieving adc offsets
	
	if (m_Bits < 14)
	{
		for (int i = 0; i < 8; i++)
		{
			adcOffsets[i] = adcOffsets[i] >> (14 - m_Bits);
		}
	}
	// Pack normalized adc offsets, according to the channel mask.
	GetMap(m_ChannelMask, m_ChannelMap, &m_ActiveChannelNo);
	for (int i = 0; i < 8; i++)
	{
		m_ChannelOffsets[i] = adcOffsets[i];
	}

	return retVal;
}

void CDataEvaluation::DoTrigger()
{
	if (m_TriggerManager && (m_StopAt == 0))
	{
        unsigned long long l = GetSampleCount();
//		printf("Trigger %d\n", (int)l);
		m_TriggerManager->Trigger(l);
	}
}


/* CTriggerManager */

CTriggerManager::CTriggerManager() : m_Delay(0)
{
}

CTriggerManager::~CTriggerManager()
{
}

void CTriggerManager::Add(CDataEvaluation* dataEvaluator)
{
	lock();
	dataEvaluator->m_TriggerManager = this;
	m_DataEvaluators.push_front(dataEvaluator);
	unlock();
}

void CTriggerManager::Remove(CDataEvaluation* dataEvaluator)
{
	lock();
	dataEvaluator->m_TriggerManager = NULL;
	m_DataEvaluators.remove(dataEvaluator);
	unlock();
}

void CTriggerManager::RemoveAll()
{
	lock();
	for (list<CDataEvaluation*>::iterator itr = m_DataEvaluators.begin(); itr != m_DataEvaluators.end(); itr++)
	{
		(*itr)->m_TriggerManager = NULL;
	}
	m_DataEvaluators.clear();
	unlock();
}

void CTriggerManager::Trigger(long long count)
{
	lock();
	for (list<CDataEvaluation*>::iterator itr = m_DataEvaluators.begin(); itr != m_DataEvaluators.end(); itr++)
	{
		(*itr)->SetStopAt(count + m_Delay);
	}
	unlock();
}

CLnxTriggerManager::CLnxTriggerManager()
{
}

CLnxTriggerManager::~CLnxTriggerManager()
{
}

void CLnxTriggerManager::lock()
{
}

void CLnxTriggerManager::unlock()
{
}
