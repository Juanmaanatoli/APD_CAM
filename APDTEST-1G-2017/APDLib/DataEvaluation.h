#ifndef __DATAEVALUATION_H__

#define __DATAEVALUATION_H__

#include "CamServer.h"
#include "CamClient.h"
#include "TypeDefs.h"

#include "LnxClasses.h"
#include "SysLnxClasses.h"

#include <list>

//for debugging purposes
#include <iostream>
//#include "fstream"

using namespace std;

void EvaluateData(unsigned char* pData, int length, int noofSample, unsigned char channelMask, int bits, int channelOffset);

// define CC24 for 24 bit, CC8 for 8 bit continuity counter
//#define CC24 
#define CC8

class CTriggerManager;

class CDataEvaluation : public Thread
{
	friend class CTriggerManager;

protected:
	void ProcessData();
	void ProcessFrame(unsigned char *pFrame, unsigned int packetNo);
	void ProcessBlock(unsigned char *pData);
    void Trigger(int channel, short data);

	CTriggerManager *m_TriggerManager;

public:
	CDataEvaluation();
	virtual ~CDataEvaluation();
	unsigned int Handler();

    //For debug
    //ofstream out;

    inline unsigned long long GetSampleCount() { return m_SampleCount; }
    inline unsigned long long GetSampleIndex() { return m_SampleIndex; }

    inline void SetStopAt(unsigned long long stopAt) { m_StopAt = stopAt; }
	bool SetParams(unsigned int bits, unsigned char channelMask, unsigned int packetSize);
    inline void SetBuffers(unsigned char* srcBuffer, unsigned char* workBuffer, unsigned char* userBuffer, unsigned long long userBufferSize)
        {m_SrcBuffer = srcBuffer; m_WorkBuffer = workBuffer; m_UserBuffer = userBuffer; m_UserBufferSize = userBufferSize; }
    inline void SetServer(CAPDServer *server) { m_Server = server; }
    inline void SetDataNotificationSignal(CEvent *dataNotification) { m_pDataNotificationSignal = dataNotification; }
    inline void SetUserNotificationSignal(CEvent *userNotification) { m_pUserNotificationSignal = userNotification; }
	void SetCalibratedMode(bool calibrated);
    inline void DisableTrigger() { m_Triggered = true; }
	void SetTrigger(ADT_TRIGGERINFO *triggerInfo);

	// Trigger
protected:
	bool m_Triggered;
	ADT_TRIGGERINFO m_TriggerInfo[8];
	int m_TriggerEnabled[8];
    short m_TriggerLevels[8];

public:
	void DoTrigger();

	// The SetupCalibrationData must be called after the SetParams, becaues it uses the bits and channelmask.
	// The routin reads the appropiate calibration data from theADC board, normalizes them (according to bit number) and arrenges it, according to channel mask.
	// The streamNo is 1..4
	bool SetupCalibrationData(CAPDClient *client, int streamNo);
	CEvent *m_pDataNotificationSignal;
	CEvent *m_pUserNotificationSignal;
	unsigned int m_PacketNo;

    short *m_ChannelData[8];
	int m_ChannelMap[8];
	int m_ActiveChannelNo;

protected:
	CAPDServer *m_Server;
	unsigned char* m_SrcBuffer;
	unsigned int m_WritePtr;	// indicates the where have to write: (m_TempBuffer+m_WritePtr)
	int m_DataLength;			// The amount of data to be processed.
	bool m_Running;
	bool m_Calibrated;

	unsigned char* m_WorkBuffer;

	unsigned char *m_UserBuffer;	// points to the 0. channel
    unsigned long long m_UserBufferSize;	// per channel

	unsigned int m_Bits;		// bit resolution. Values: 8,12,14
	unsigned char m_ChannelMask;
	unsigned int m_PacketSize;
	int m_BlockSize;			// Size of one sample.
//	int m_Channels;				// Used in data decoding
	int m_PaddingBits;			// Used in data decoding
    unsigned short m_Mask;		// Used in data decoding. Its value can be 0x3FFF (14 bit), 0x0FFF (12 bit) or 0x00FF (8 bit)

    unsigned char m_PreambSize;
    unsigned char m_ContinuityCounterOffset;

	// The channel pffset values in packed format. E.g. If 1st and 3rd channel are used, the m_ChannelOffsets[0] is for 1st, m_ChannelOffsets[1] is for 3rd.
    short m_ChannelOffsets[8];

	unsigned int m_MaxNoofBlocks;
	unsigned int m_LastContinuityCounter;
    unsigned long long m_SampleCount;
    unsigned long long m_SampleIndex;  // Index in the ring buffer, where the data must be placed.
    unsigned long long m_UserBufferSizeInSample;
    unsigned long long m_StopAt;

	unsigned int m_StreamNo;

	void InternalStop();

public:
	// Win specific
/*	LARGE_INTEGER m_ReferenceTime;
    LARGE_INTEGER m_ProcessingTime;*/
	int m_ProcessingCount;
    /*LARGE_INTEGER m_LastCallingTime;
    LARGE_INTEGER m_AvarageCallingTime;*/
	int m_CallingCount;
};

class CLnxDataEvaluation : public CDataEvaluation
{
};

class CTriggerManager
{
	list<CDataEvaluation*> m_DataEvaluators;
    unsigned long long m_Delay;
public:
	CTriggerManager();
	virtual ~CTriggerManager();
    inline void SetDelay(unsigned long long delay) { m_Delay = delay; }
	void Add(CDataEvaluation* dataEvaluator);
	void Remove(CDataEvaluation* dataEvaluator);
	void RemoveAll();
    void Trigger(long long count);
protected:
	virtual void lock() = 0;
	virtual void unlock() = 0;
};

class CLnxTriggerManager : public CTriggerManager
{
protected:

public:
	CLnxTriggerManager();
	~CLnxTriggerManager();

protected:
	virtual void lock();
	virtual void unlock();
};

#endif  /* __DATAEVALUATION_H__ */
