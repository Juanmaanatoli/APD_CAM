#include "InternalFunctions.h"


//Common functions
//===============================================================
bool GetInfo(CAPDClient *client, APDInfo *apdInfo)
{
    bool retVal=false;

    retVal=GetControlBoardVersion(client,&apdInfo->pcBoardVersion);

    if(!retVal)
        return retVal;

    retVal=GetControlBoardFWVersion(client,&apdInfo->pcFWVersion);

    if(!retVal)
        return retVal;

    retVal=GetADCBoardVersion(client,&apdInfo->adcBoardVersion);

    if(!retVal)
        return retVal;

    retVal=GetMCVersion(client,&apdInfo->adcMCVersion);

    if(!retVal)
        return retVal;

    retVal=GetSerial(client,&apdInfo->adcSerial);

    if(!retVal)
        return retVal;

    retVal=GetFPGAVersion(client,&apdInfo->fpgaVersion);

    if(!retVal)
        return retVal;


    return retVal;

}//GetInfo

bool GetStatus(CAPDClient *client, APDStatus *apdStatus)
{
    bool retVal=false;

    //Control board Status
    //===================================
    unsigned short binValues[4];
    retVal = GetAllHVMonitor(client,binValues);

    if(!retVal)
        return retVal;

    apdStatus->highVoltages[0]=(double)binValues[0]*0.12;
    apdStatus->highVoltages[1]=(double)binValues[1]*0.12;

    /*retVal = GetAllTempSensors(client,apdStatus->temperatures);

    if(!retVal)
        return retVal;*/

    //retVal = GetPeltierOutputVoltage(client,apdStatus->peltierOutputVoltage);

    if(!retVal)
        return retVal;

    retVal = GetAllTempSensors(client,apdStatus->temperatures);

    if(!retVal)
        return retVal;

    retVal = GetFan1Speed(client,&apdStatus->fanSpeed[0]);

    if(!retVal)
        return retVal;

    retVal = GetFan2Speed(client,&apdStatus->fanSpeed[1]);

    if(!retVal)
        return retVal;

    retVal = GetFan3Speed(client,&apdStatus->fanSpeed[2]);

    if(!retVal)
        return retVal;

    int calibLight=0;
    retVal = GetCalibLigth(client,&calibLight);

    if(!retVal)
        return retVal;

    apdStatus->calibrationLight=(unsigned short)calibLight;

    retVal = GetShutterMode(client,(int*)&apdStatus->shutterMode);

    if(!retVal)
        return retVal;

    retVal = GetShutterState(client,(int*)&apdStatus->shutterState);

    if(!retVal)
        return retVal;

    //ADC board status
    //===================================

    unsigned char status1=0;
    retVal = GetStatus1(client,&status1);

    if(!retVal)
        return retVal;

    apdStatus->status1.adcClkPllLocked=((status1 & 1) ? true : false);
    apdStatus->status1.streamClkPllLocked=((status1 & 2) ? true : false);

    unsigned char status2=0;
    retVal = GetStatus2(client,&status2);

    if(!retVal)
        return retVal;

    apdStatus->status2.factoryResetPinState=((status2 & 1) ? true : false);
    apdStatus->status2.overLoad=((status2 & 2) ? true : false);
    apdStatus->status2.externalClkDcmLocked=((status2 & 4) ? true : false);
    apdStatus->status2.ad1SampleEnabled=((status2 & 16) ? true : false);
    apdStatus->status2.ad2SampleEnabled=((status2 & 32) ? true : false);
    apdStatus->status2.ad3SampleEnabled=((status2 & 64) ? true : false);
    apdStatus->status2.ad4SampleEnabled=((status2 & 128) ? true : false);


    unsigned char control=0;
    retVal = GetADCControl(client,&control);

    if(!retVal)
        return retVal;

    apdStatus->control.extClkSelect=((control & 1) ? true : false);
    apdStatus->control.eioClkOutEnable=((control & 2) ? true : false);
    apdStatus->control.sampleClkSelect=((control & 4) ? true : false);
    apdStatus->control.eioSampleOutEnable=((control & 8) ? true : false);
    apdStatus->control.filterOn=((control & 16) ? true : false);
    apdStatus->control.reserveBitOrderInStream=((control & 64) ? true : false);
    apdStatus->control.preamblePatternEnabled=((control & 128) ? true : false);

    return retVal;

}//GetStatus


//ADC board functions
//==================================================================
bool GetADCBoardVersion(CAPDClient *client, unsigned char *boardVersion, unsigned long ipAddress_h, unsigned short ipPort_h, int timeout)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_BOARD_VERSION, (unsigned char*)boardVersion, ADC_REG_BOARD_VERSION_LEN, ipAddress_h, ipPort_h, timeout);
	return retVal;
}

bool GetMCVersion(CAPDClient *client, unsigned short *mcVersion)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_MC_VERSION, (unsigned char*)mcVersion, ADC_REG_MC_VERSION_LEN);
	return retVal;
}

bool GetSerial(CAPDClient *client, unsigned short *serial)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_SERIAL, (unsigned char*)serial, ADC_REG_SERIAL_LEN);
	return retVal;
}

bool GetFPGAVersion(CAPDClient *client, unsigned short *fpgaVersion)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_FPGA_VERSION, (unsigned char*)fpgaVersion, ADC_REG_FPGA_VERSION_LEN);
	return retVal;
}

bool GetStatus1(CAPDClient *client, unsigned char *status1)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_STATUS1, (unsigned char*)status1, ADC_REG_STATUS1_LEN);
	return retVal;
}

bool GetStatus2(CAPDClient *client, unsigned char *status2)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_STATUS2, (unsigned char*)status2, ADC_REG_STATUS2_LEN);
	return retVal;
}

bool SetADCControl(CAPDClient *client, unsigned char adcControl)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_CONTROL, (unsigned char*)&adcControl, ADC_REG_CONTROL_LEN);
	//Sleep(20);
	return retVal;
}

bool GetADCControl(CAPDClient *client, unsigned char *adcControl)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_CONTROL, (unsigned char*)adcControl, ADC_REG_CONTROL_LEN);
	return retVal;
}

bool SetADCPLL(CAPDClient *client, unsigned char mult, unsigned char div)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_ADCCLKMUL, (unsigned char*)&mult, ADC_REG_ADCCLKMUL_LEN);
    retVal &= WritePDISafe(client, ADC_BOARD, ADC_REG_ADCCLKDIV, (unsigned char*)&div, ADC_REG_ADCCLKDIV_LEN);
	//Sleep(20);
	return retVal;
}

bool GetADCPLL(CAPDClient *client, unsigned char *mult, unsigned char *div)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_ADCCLKMUL, (unsigned char*)mult, ADC_REG_ADCCLKMUL_LEN);
	retVal &= ReadPDI(client, ADC_BOARD, ADC_REG_ADCCLKDIV, (unsigned char*)div, ADC_REG_ADCCLKDIV_LEN);
	return retVal;
}

bool SetStreamPLL(CAPDClient *client, unsigned char mult, unsigned char div)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_STREAMCLKMUL, (unsigned char*)&mult, ADC_REG_STREAMCLKMUL_LEN);
    retVal &= WritePDISafe(client, ADC_BOARD, ADC_REG_STREAMCLKDIV, (unsigned char*)&div, ADC_REG_STREAMCLKDIV_LEN);
	//Sleep(20);
	return retVal;
}

bool GetStreamPLL(CAPDClient *client, unsigned char *mult, unsigned char *div)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_STREAMCLKMUL, (unsigned char*)mult, ADC_REG_STREAMCLKMUL_LEN);
	retVal &= ReadPDI(client, ADC_BOARD, ADC_REG_STREAMCLKDIV, (unsigned char*)div, ADC_REG_STREAMCLKDIV_LEN);
	return retVal;
}

bool SetStreamControl(CAPDClient *client, unsigned char streamControl)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_STREAMCONTROL, (unsigned char*)&streamControl, ADC_REG_STREAMCONTROL_LEN);
	//Sleep(20);
	return retVal;
}

bool GetStreamControl(CAPDClient *client, unsigned char *streamControl)
{
    bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_STREAMCONTROL, (unsigned char*)streamControl, ADC_REG_STREAMCONTROL_LEN);
	return retVal;
}

bool SetSampleCount(CAPDClient *client, unsigned int sampleCount)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_SAMPLECNT, (unsigned char*)&sampleCount, ADC_REG_SAMPLECNT_LEN);
	//Sleep(100);
	return retVal;
}

bool GetSampleCount(CAPDClient *client, unsigned int *sampleCount)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_SAMPLECNT, (unsigned char*)sampleCount, ADC_REG_SAMPLECNT_LEN);
	return retVal;
}

/* Channel settings */
// Set channel 1
bool SetChannel_1(CAPDClient *client, unsigned char channelMask_1)
{

    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_CHENABLE1, (unsigned char*)&channelMask_1, ADC_REG_CHENABLE1_LEN);
	//Sleep(20);
	return retVal;
}

// Set channel 2
bool SetChannel_2(CAPDClient *client, unsigned char channelMask_2)
{

    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_CHENABLE2, (unsigned char*)&channelMask_2, ADC_REG_CHENABLE2_LEN);
	//Sleep(20);
	return retVal;
}

// Set channel 3
bool SetChannel_3(CAPDClient *client, unsigned char channelMask_3)
{

    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_CHENABLE3, (unsigned char*)&channelMask_3, ADC_REG_CHENABLE3_LEN);
	//Sleep(20);
	return retVal;
}

// Set channel 4
bool SetChannel_4(CAPDClient *client, unsigned char channelMask_4)
{

    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_CHENABLE4, (unsigned char*)&channelMask_4, ADC_REG_CHENABLE4_LEN);
	//Sleep(20);
	return retVal;
}

// Set all channel
bool SetChannels(CAPDClient *client, unsigned char channelMask_1, unsigned char channelMask_2, unsigned char channelMask_3, unsigned char channelMask_4)
{
	unsigned char channelMaskArray[4];
    channelMaskArray[0] = channelMask_1;
    channelMaskArray[1] = channelMask_2;
    channelMaskArray[2] = channelMask_3;
    channelMaskArray[3] = channelMask_4;

    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_CHENABLE, (unsigned char*)&channelMaskArray[0], ADC_REG_CHENABLE_LEN);
	//Sleep(20);
	return retVal;
}

bool GetChannels(CAPDClient *client, unsigned char *channelMask_1, unsigned char *channelMask_2, unsigned char *channelMask_3, unsigned char *channelMask_4)
{
	unsigned char channelMaskArray[4];

    bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_CHENABLE, (unsigned char*)&channelMaskArray[0], ADC_REG_CHENABLE_LEN);

    *channelMask_1 = channelMaskArray[0];
    *channelMask_2 = channelMaskArray[1];
    *channelMask_3 = channelMaskArray[2];
    *channelMask_4 = channelMaskArray[3];

	return retVal;
}

// Sets hardware ringbuffer size
bool SetRingbufferSize(CAPDClient *client, unsigned short bufferSize)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_RINGBUFSIZE, (unsigned char*)&bufferSize, ADC_REG_RINGBUFSIZE_LEN);
	//Sleep(20);
	return retVal;
}

// Returns hardware ringbuffer size
bool GetRingbufferSize(CAPDClient *client, unsigned short *bufferSize)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_RINGBUFSIZE, (unsigned char*)bufferSize, ADC_REG_RINGBUFSIZE_LEN);
	//Sleep(20);
	return retVal;
}

bool SetResolution(CAPDClient *client, int bitNum)
{
	bool retVal = true;
	unsigned char resolution = 0;
	switch (bitNum)
	{
	case 8: resolution = 2; break;
	case 12: resolution = 1; break;
	case 14: resolution = 0; break;
	default: retVal = false;
	}
	if (retVal)
	{
        retVal = WritePDISafe(client, 1, ADC_REG_RESOLUTION, &resolution, ADC_REG_RESOLUTION_LEN);
		//Sleep(100);
	}
	return retVal;
}

bool GetResolution(CAPDClient *client, int *bitNum)
{
	unsigned char resolution = 0;
	bool retVal = ReadPDI(client, 1, ADC_REG_RESOLUTION, &resolution, ADC_REG_RESOLUTION_LEN);
	if (retVal)
	{
		switch (resolution)
		{
		case 0: *bitNum = 14; break;
		case 1: *bitNum = 12; break;
		case 2: *bitNum =  8; break;
		default: retVal = false;
		}
	}

	return retVal;
}
bool SetSampleDiv(CAPDClient *client, unsigned short value)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_ADSAMPLEDIV, (unsigned char*)&value, ADC_REG_ADSAMPLEDIV_LEN);
	//Sleep(20);
	return retVal;
}

bool GetSampleDiv(CAPDClient *client, unsigned short *value)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_ADSAMPLEDIV, (unsigned char*)value, ADC_REG_ADSAMPLEDIV_LEN);
	return retVal;
}

bool SetTrigger(CAPDClient *client, unsigned char value)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_TRIGGER, (unsigned char*)&value, ADC_REG_TRIGGER_LEN);
	//Sleep(20);
	return retVal;
}

bool GetTrigger(CAPDClient *client, unsigned char *value)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_TRIGGER, (unsigned char*)value, ADC_REG_TRIGGER_LEN);
	return retVal;
}

// ADC test mode
bool SetTestMode(CAPDClient *client, unsigned int mode)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC1_REG_TEST_MODE, (unsigned char*)&mode, ADC_REG_TEST_MODE_LEN);
    Sleep(20);
    retVal = WritePDISafe(client, ADC_BOARD, ADC2_REG_TEST_MODE, (unsigned char*)&mode, ADC_REG_TEST_MODE_LEN);
    Sleep(20);
    retVal = WritePDISafe(client, ADC_BOARD, ADC3_REG_TEST_MODE, (unsigned char*)&mode, ADC_REG_TEST_MODE_LEN);
    Sleep(20);
    retVal = WritePDISafe(client, ADC_BOARD, ADC4_REG_TEST_MODE, (unsigned char*)&mode, ADC_REG_TEST_MODE_LEN);

    return retVal;
}

// Returns test mode settings
bool GetTestMode(CAPDClient *client, unsigned int *mode)
{
    bool retVal = ReadPDI(client, ADC_BOARD, ADC1_REG_TEST_MODE, (unsigned char*)mode, ADC_REG_TEST_MODE_LEN);
    return retVal;
}

// Returns ADC board error code
bool GetADCBoardError(CAPDClient *client, unsigned char *error)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_ERROR_CODE, (unsigned char*)error, ADC_REG_ERROR_CODE_LEN);
	return retVal;

}

// 
bool FactoryReset(CAPDClient *client)
{
	bool retVal = false;
	unsigned char dummy;
    retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_FACTORY_RESET, (unsigned char*)&dummy, ADC_REG_FACTORY_RESET_LEN);
	//Sleep(20);
	return retVal;
}

bool GetBytesPerSample(CAPDClient *client, unsigned int *counters)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_BYTES_PER_SAMPLE, (unsigned char*)counters, ADC_REG_BYTES_PER_SAMPLE_LEN);
	return retVal;
}

// Clock PLL
bool SetExtClkPLL(CAPDClient *client, unsigned char mult, unsigned char div)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_EXTCLKMUL, (unsigned char*)&mult, ADC_REG_EXTCLKMUL_LEN);
    retVal &= WritePDISafe(client, ADC_BOARD, ADC_REG_EXTCLKDIV, (unsigned char*)&div, ADC_REG_EXTCLKDIV_LEN);
	//Sleep(20);
	return retVal;
}

bool GetExtClkPLL(CAPDClient *client, unsigned char *mult, unsigned char *div)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_EXTCLKMUL, (unsigned char*)mult, ADC_REG_EXTCLKMUL_LEN);
    Sleep(20);
	retVal &= ReadPDI(client, ADC_BOARD, ADC_REG_EXTCLKDIV, (unsigned char*)div, ADC_REG_EXTCLKDIV_LEN);
	return retVal;
}

// Offsets
bool SetADCOffset(CAPDClient *client, short *offsets, int first, int no)
{
	bool retVal = false;
	if (first < 0 || 32 <= first) return retVal;
	if (first + no > 32) return retVal;
    retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_OFFSET + first*sizeof(short), (unsigned char*)offsets, no*sizeof(short));
	//Sleep(20);
	return retVal;
}

bool GetADCOffset(CAPDClient *client, short *offsets, int first, int no)
{
	bool retVal = false;
	if (first < 0 || 32 <= first) return retVal;
	if (first + no > 32) return retVal;
	retVal = ReadPDI(client, ADC_BOARD, ADC_REG_OFFSET + first*sizeof(short), (unsigned char*)offsets, no*sizeof(short));
	return retVal;
}

bool SetInternalTriggerLevels(CAPDClient *client, unsigned short *levels)
{
	bool retVal = true;
	for (int i = 0; i < 8; i++)
	{
        retVal &= WritePDISafe(client, ADC_BOARD, ADC_REG_INT_TRG_LEVEL, (unsigned char*)(levels+4*i), ADC_REG_INT_TRG_LEVEL_LEN/8);
		//Sleep(20);
	}
	return retVal;
}

bool GetInternalTriggerLevels(CAPDClient *client, unsigned short *levels)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_INT_TRG_LEVEL, (unsigned char*)levels, ADC_REG_INT_TRG_LEVEL_LEN);
	return retVal;
}

// Acquired samples per channel
bool GetAquiredSampleCount(CAPDClient *client, unsigned int *counters)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_AQRD_SAMPLE, (unsigned char*)counters, ADC_REG_AQRD_SAMPLE_LEN);
	return retVal;
}

// Overload level
bool SetOverloadLevel(CAPDClient *client, unsigned short level)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_OVERLOAD_LEVEL, (unsigned char*)&level, ADC_REG_OVERLOAD_LEVEL_LEN);
	//Sleep(20);
	return retVal;
}

bool GetOverloadLevel(CAPDClient *client, unsigned short *level)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_OVERLOAD_LEVEL, (unsigned char*)level, ADC_REG_OVERLOAD_LEVEL_LEN);
	return retVal;
}

// Overload status
bool SetOverloadStatus(CAPDClient *client, unsigned char status)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_OVERLOAD_STATUS, (unsigned char*)&status, ADC_REG_OVERLOAD_STATUS_LEN);
	//Sleep(20);
	return retVal;
}

bool GetOverloadStatus(CAPDClient *client, unsigned char *status)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_OVERLOAD_STATUS, (unsigned char*)status, ADC_REG_OVERLOAD_STATUS_LEN);
	return retVal;
}

// Overload time
bool SetOverloadTime(CAPDClient *client, unsigned short time)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_OVERLOAD_TIME, (unsigned char*)&time, ADC_REG_OVERLOAD_TIME_LEN);
	//Sleep(20);
	return retVal;
}

bool GetOverloadTime(CAPDClient *client, unsigned short *time)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_OVERLOAD_TIME, (unsigned char*)time, ADC_REG_OVERLOAD_TIME_LEN);
	return retVal;
}

// Trigger delay
bool SetTriggerDelay(CAPDClient *client, unsigned int delay)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_TRIGGER_DELAY, (unsigned char*)&delay, ADC_REG_TRIGGER_DELAY_LEN);
	//Sleep(20);
	return retVal;
}

bool GetTriggerDelay(CAPDClient *client, unsigned int *delay)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_TRIGGER_DELAY, (unsigned char*)delay, ADC_REG_TRIGGER_DELAY_LEN);
	return retVal;
}

// Filter coefficients
bool SetFilterCoefficients(CAPDClient *client, unsigned short *coefficints)
{
    bool retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_FILTER_COEFF, (unsigned char*)coefficints, ADC_REG_FILTER_COEFF_LEN);
	//Sleep(20);
	return retVal;
}

bool GetSetFilterCoefficients(CAPDClient *client, unsigned short *coefficints)
{
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_FILTER_COEFF, (unsigned char*)coefficints, ADC_REG_FILTER_COEFF_LEN);
	return retVal;
}


// Calibration table operations
// The output buffer must be 7 byte or longer.
bool RetrieveADCSerialNo(CAPDClient *client, char *serial_no, int len, unsigned long ip_h, unsigned short port_h)
{
	if (len <= (ADC_REG_SERIALNO_LEN + 1)) return false;
	bool retVal = ReadPDI(client, ADC_BOARD, ADC_REG_SERIALNO, (unsigned char*)serial_no, ADC_REG_SERIALNO_LEN, ip_h, port_h);
	serial_no[ADC_REG_SERIALNO_LEN] = 0;
	return retVal;
}

bool StoreADCOffsets(CAPDClient *client, short *adcOffsets, int first, int no)
{
	bool retVal = false;
	if (first < 0 || 32 <= first) return retVal;
	if (first + no > 32) return retVal;

    retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_ADC_OFFSET + first*sizeof(short), (unsigned char*)adcOffsets, no*sizeof(short));
	//Sleep(20);
	return retVal;
}


// Returns 'no' number of calibration data from the index - defined in the 'first' parameter.
// The correct size of output buffer is up to the caller. The best practice is use short[32] for buffer
bool RetrieveADCOffsets(CAPDClient *client, short *adcOffsets, int first, int no)
{
	bool retVal = false;
	if (first < 0 || 32 <= first) return retVal;
	if (first + no > 32) return retVal;

	retVal = ReadPDI(client, ADC_BOARD, ADC_REG_ADC_OFFSET + first*sizeof(short), (unsigned char*)adcOffsets, no*sizeof(short));
	return retVal;
}

bool StoreDACOffsets(CAPDClient *client, short *dacOffsets, int first, int no)
{
	bool retVal = false;
	if (first < 0 || 32 <= first) return retVal;
	if (first + no > 32) return retVal;

    retVal = WritePDISafe(client, ADC_BOARD, ADC_REG_DAC_OFFSET + first*sizeof(short), (unsigned char*)dacOffsets, no*sizeof(short));
	//Sleep(20);
	return retVal;
}


// Returns 'no' number of calibration data from the index - defined in the 'first' parameter.
// The correct size of output buffer is up to the caller. The best practice is use short[32] for buffer
bool RetrieveDACOffsets(CAPDClient *client, short *dacOffsets, int first, int no)
{
	bool retVal = false;
	if (first < 0 || 32 <= first) return retVal;
	if (first + no > 32) return retVal;

	retVal = ReadPDI(client, ADC_BOARD, ADC_REG_DAC_OFFSET + first*sizeof(short), (unsigned char*)dacOffsets, no*sizeof(short));
	return retVal;
}



//Control board functions
//==================================================================
bool GetControlBoardVersion(CAPDClient *client, unsigned char *boardVersion)
{
	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_BOARD_VERSION, (unsigned char*)boardVersion, PC_REG_BOARD_VERSION_LEN);
	//Sleep(20);	
	return retVal;

}//GetControlBoardVersion

bool GetControlBoardFWVersion(CAPDClient *client, unsigned short *fwVersion)
{
    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_FW_VERSION, (unsigned char*)fwVersion, PC_REG_FW_VERSION_LEN);

    return retVal;

}//GetControlBoardFWVersion


bool GetAllHVMonitor(CAPDClient *client, unsigned short *binValues)
{
	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_HV1_MONITOR, (unsigned char*)binValues, PC_REG_ALL_HV_MONITORS_LEN);
	//Sleep(20);
	for (int i = 0; i < 4; i++) binValues[i] &= 0x0FFF;
	return retVal;
}


bool GetAllTempSensors(CAPDClient *client, double *values)
{
	short binValues[16];
	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_TEMP_SENSOR_1, (unsigned char*)binValues, PC_REG_ALL_TEMP_SENSORS_LEN);
	for (int i = 0; i < 16; i++)
	{
		values[i] = binValues[i]/10.0;
	}
	return retVal;
}

bool GetPeltierOutputVoltage(CAPDClient *client, unsigned short *voltage)
{
    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_PELT_CONTROL_OUT, (unsigned char*)voltage, PC_REG_PELT_CONTROL_OUT_LEN);

    return retVal;

}//GetPeltierOutputVoltage

bool SetPeltierIndirectControl(CAPDClient *client, short value)
{
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_PELT_DAC_REG, (unsigned char*)&value, PC_REG_PELT_DAC_REG_LEN);

    return retVal;

}//SetPeltierIndirectControl


bool GetPeltierIndirectControl(CAPDClient *client, short *value)
{
    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_PELT_DAC_REG, (unsigned char*)value, PC_REG_PELT_DAC_REG_LEN);

    return retVal;

}//GetPeltierIndirectControl

bool SetDetectorReferenceTemp(CAPDClient *client, short value)
{
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_DETECTOR_TEMP_SET, (unsigned char*)&value, PC_REG_DETECTOR_TEMP_SET_LEN);

    return retVal;

}//SetDetectorReferenceTemp


bool GetDetectorReferenceTemp(CAPDClient *client, short *value)
{
    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_DETECTOR_TEMP_SET, (unsigned char*)value, PC_REG_DETECTOR_TEMP_SET_LEN);

    return retVal;

}//GetDetectorReferenceTemp

bool SetPGain(CAPDClient *client, unsigned short p)
{
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_P_GAIN, (unsigned char*)&p, PC_REG_GAIN_LEN);

    return retVal;

}//SetPGain


bool GetPGain(CAPDClient *client, unsigned short *p)
{
    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_P_GAIN, (unsigned char*)p, PC_REG_GAIN_LEN);

    return retVal;

}//GetPGain


bool SetIGain(CAPDClient *client, unsigned short i)
{
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_I_GAIN, (unsigned char*)&i, PC_REG_GAIN_LEN);

    return retVal;

}//SetIGain


bool GetIGain(CAPDClient *client, unsigned short *i)
{
    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_I_GAIN, (unsigned char*)i, PC_REG_GAIN_LEN);

    return retVal;

}//GetIGain


bool SetDGain(CAPDClient *client, unsigned short d)
{
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_D_GAIN, (unsigned char*)&d, PC_REG_GAIN_LEN);

    return retVal;

}//SetDGain


bool GetDGain(CAPDClient *client, unsigned short *d)
{
    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_D_GAIN, (unsigned char*)d, PC_REG_GAIN_LEN);

    return retVal;

}//GetDGain

bool SetHV1(CAPDClient *client, int binValue)
{
	binValue &= 0x0FFF;
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_HV1_SET, (unsigned char*)&binValue, PC_REG_HV_SET_LEN);
	//Sleep(20);
	return retVal;
}

bool GetHV1(CAPDClient *client, int *binValue)
{
	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_HV1_SET, (unsigned char*)binValue, PC_REG_HV_SET_LEN);
	//Sleep(20);
	*binValue &= 0x0FFF;
	return retVal;
}

bool SetHV2(CAPDClient *client, int binValue)
{
	binValue &= 0x0FFF;
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_HV2_SET, (unsigned char*)&binValue, PC_REG_HV_SET_LEN);
	//Sleep(20);
	return retVal;
}

bool GetHV2(CAPDClient *client, int *binValue)
{
	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_HV2_SET, (unsigned char*)binValue, PC_REG_HV_SET_LEN);
	//Sleep(20);
	*binValue &= 0x0FFF;
	return retVal;
}

bool SetHVState(CAPDClient *client, int state)
{
	int internalState = state ? 0x03:0x00;
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_HV_ON, (unsigned char*)&internalState, PC_REG_HV_ON_LEN);
	//Sleep(20);
	return retVal;
}

bool GetHVState(CAPDClient *client, int *state)
{
	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_HV_ON, (unsigned char*)state, PC_REG_HV_ON_LEN);
	//Sleep(20);
	return retVal;
}

bool EnableHV(CAPDClient *client, bool enable)
{
    unsigned char value=0;
	value = enable ? 0xAB : 0x00;
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_HV_ENABLE, (unsigned char*)&value, PC_REG_HV_ENABLE_LEN);
	//Sleep(20);
	return retVal;
}

bool GetEnableHV(CAPDClient *client, bool *enable)
{
    unsigned char value=0;
    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_HV_ENABLE, (unsigned char*)&value, PC_REG_HV_ENABLE_LEN);

    if(value==0xAB)
        *enable = true;
    else if(value==0x00)
        *enable = false;

    //Sleep(20);
    return retVal;

}//GetEnableHV


bool SetTempInterruptEnable(CAPDClient *client, bool enable)
{
    unsigned char state=0;

    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_IRQ_POWER_PID_ENABLE, (unsigned char*)&state, PC_REG_IRQ_POWER_PID_ENABLE_LEN);

    if(retVal)
    {
        if(enable)
        {
            state|= 1 << 0;
        }
        else
        {
            state&= ~(1 << 0);
        }

        retVal = WritePDISafe(client, PC_BOARD, PC_REG_IRQ_POWER_PID_ENABLE, (unsigned char*)&state, PC_REG_IRQ_POWER_PID_ENABLE_LEN);

    }//if

    return retVal;


}//SetTempInterruptEnable


bool GetTempInterruptEnable(CAPDClient *client, bool *enable)
{
    unsigned char state=0;

    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_IRQ_POWER_PID_ENABLE, (unsigned char*)&state, PC_REG_IRQ_POWER_PID_ENABLE_LEN);

    *enable=(((state & 0x01) !=0) ? true : false );

    return retVal;

}//GetTempInterruptEnable

bool SetAnalogPower(CAPDClient *client, bool enable)
{

    unsigned char state=0;

    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_IRQ_POWER_PID_ENABLE, (unsigned char*)&state, PC_REG_IRQ_POWER_PID_ENABLE_LEN);

    if(retVal)
    {
        if(enable)
        {
            state|= 1 << 1;
        }
        else
        {
            state&= ~(1 << 1);
        }

        retVal = WritePDISafe(client, PC_BOARD, PC_REG_IRQ_POWER_PID_ENABLE, (unsigned char*)&state, PC_REG_IRQ_POWER_PID_ENABLE_LEN);

    }//if

    return retVal;

}//SetAnalogPower


bool GetAnalogPower(CAPDClient *client, bool *enable)
{
    unsigned char state=0;

    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_IRQ_POWER_PID_ENABLE, (unsigned char*)&state, PC_REG_IRQ_POWER_PID_ENABLE_LEN);

    *enable=(((state & 0x02) !=0) ? true : false );

    return retVal;

}//GetAnalogPower

bool SetDisablePIDControl(CAPDClient *client, bool enable)
{
    unsigned char state=0;

    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_IRQ_POWER_PID_ENABLE, (unsigned char*)&state, PC_REG_IRQ_POWER_PID_ENABLE_LEN);

    if(retVal)
    {
        if(enable)
        {
            state|= 1 << 2;
        }
        else
        {
            state&= ~(1 << 2);
        }

        retVal = WritePDISafe(client, PC_BOARD, PC_REG_IRQ_POWER_PID_ENABLE, (unsigned char*)&state, PC_REG_IRQ_POWER_PID_ENABLE_LEN);

    }//if

    return retVal;

}//SetDisablePIDControl


bool GetDisablePIDControl(CAPDClient *client, bool *enable)
{
    unsigned char state=0;

    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_IRQ_POWER_PID_ENABLE, (unsigned char*)&state, PC_REG_IRQ_POWER_PID_ENABLE_LEN);

    *enable=(((state & 0x04) !=0) ? true : false );

    return retVal;

}//GetDisablePIDControl

bool SetFan1Speed(CAPDClient *client, unsigned char speed)
{
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_FAN1_SPEED, (unsigned char*)&speed, PC_REG_FAN_SPEED_LEN);

    return retVal;

}//SetFan1Speed


bool GetFan1Speed(CAPDClient *client, unsigned char *speed)
{
    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_FAN1_SPEED, (unsigned char*)speed, PC_REG_FAN_SPEED_LEN);

    return retVal;

}//GetFan1Speed


bool SetFan2Speed(CAPDClient *client, unsigned char speed)
{
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_FAN2_SPEED, (unsigned char*)&speed, PC_REG_FAN_SPEED_LEN);

    return retVal;

}//SetFan2Speed


bool GetFan2Speed(CAPDClient *client, unsigned char *speed)
{
    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_FAN2_SPEED, (unsigned char*)speed, PC_REG_FAN_SPEED_LEN);

    return retVal;

}//GetFan2Speed


bool SetFan3Speed(CAPDClient *client, unsigned char speed)
{
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_FAN3_SPEED, (unsigned char*)&speed, PC_REG_FAN_SPEED_LEN);

    return retVal;

}//SetFan3Speed


bool GetFan3Speed(CAPDClient *client, unsigned char *speed)
{
    bool retVal = ReadPDI(client, PC_BOARD, PC_REG_FAN3_SPEED, (unsigned char*)speed, PC_REG_FAN_SPEED_LEN);

    return retVal;

}//GetFan3Speed


bool SetCalibLigth(CAPDClient *client, int current)
{
	current &= 0x0FFF;
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_CALIB_LIGHT, (unsigned char*)&current, PC_REG_CALIB_LIGHT_LEN);
	//Sleep(20);
	return retVal;
}

bool GetCalibLigth(CAPDClient *client, int *current)
{
	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_CALIB_LIGHT, (unsigned char*)current, PC_REG_CALIB_LIGHT_LEN);
	*current &= 0x0FFF;
	return retVal;
}

bool SetShutterMode(CAPDClient *client, int mode)
{
	mode &= 0x0001;
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_SHMODE, (unsigned char*)&mode, PC_REG_SHMODE_LEN);
	//Sleep(20);
	return retVal;
}
bool GetShutterMode(CAPDClient *client, int *mode)
{
	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_SHMODE, (unsigned char*)mode, PC_REG_SHMODE_LEN);
	*mode &= 0x00000001;
	return retVal;
}

bool SetShutterState(CAPDClient *client, int state)
{
	state &= 0x0001;
    bool retVal = WritePDISafe(client, PC_BOARD, PC_REG_SHSTATE, (unsigned char*)&state, PC_REG_SHSTATE_LEN);
	//Sleep(20);
	return retVal;
}

bool GetShutterState(CAPDClient *client, int *state)
{
	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_SHSTATE, (unsigned char*)state, PC_REG_SHSTATE_LEN);
	*state &= 0x00000001;
	return retVal;
}

bool GetControlBoardError(CAPDClient *client, unsigned char *error)
{

	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_ERROR_CODE, (unsigned char*)error, PC_REG_ERROR_CODE_LEN);
	return retVal;
}

bool GetHV1Max(CAPDClient *client, unsigned short *hv)
{

	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_HV1MAX, (unsigned char*)hv, PC_REG_HV1MAX_LEN);
	//Sleep(20);
	*hv &= 0x0FFF;
	return retVal;

}

bool GetHV2Max(CAPDClient *client, unsigned short *hv)
{

	bool retVal = ReadPDI(client, PC_BOARD, PC_REG_HV2MAX, (unsigned char*)hv, PC_REG_HV2MAX_LEN);
	//Sleep(20);
	*hv &= 0x0FFF;
	return retVal;

}

bool GetFactoryData(CAPDClient *client, APDFactory *factoryData)
{

    //Read PC factory data
    bool res = ReadPDI(client, PC_BOARD, PC_REG_CALIBRATION_TABLE, (unsigned char*)&factoryData->pcTableVersion, 1);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, PC_BOARD, PC_REG_CALIBRATION_TABLE+1, (unsigned char*)&factoryData->pcDataStatus, 1);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, PC_BOARD, PC_REG_CALIBRATION_TABLE+2, (unsigned char*)&factoryData->inputHVCalib1, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, PC_BOARD, PC_REG_CALIBRATION_TABLE+6, (unsigned char*)&factoryData->inputHVCalib2, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, PC_BOARD, PC_REG_CALIBRATION_TABLE+10, (unsigned char*)&factoryData->outputHVCalib1, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, PC_BOARD, PC_REG_CALIBRATION_TABLE+14, (unsigned char*)&factoryData->outputHVCalib2, 4);

    if(!res)
        return res;

    Sleep(10);

    for(int i=0;i<16;i++)
    {

        res = ReadPDI(client, PC_BOARD, ((PC_REG_CALIBRATION_TABLE+18)+(i*2)), (unsigned char*)&(factoryData->tempCalib[i]), 2);

        if(!res)
            return res;

        Sleep(10);

    }

    Sleep(10);

    for(int i=0;i<16;i++)
    {
        unsigned char temp[10] = {0};
        res = ReadPDI(client, PC_BOARD, ((PC_REG_CALIBRATION_TABLE+50)+(i*10)), (unsigned char*)&temp, 10);

        string temp2;
        temp2.assign(reinterpret_cast<const char*>(&temp[0]));

        factoryData->tempSensorName[i] = temp2;

        if(!res)
            return res;


        Sleep(10);

    }

    Sleep(10);

    res = ReadPDI(client, PC_BOARD, PC_REG_CALIBRATION_TABLE+210, (unsigned char*)&factoryData->minHV, 2);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE, (unsigned char*)&(factoryData)->productCode, 2);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+2, (unsigned char*)&(factoryData)->serialNo, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+6, (unsigned char*)&factoryData->adcTableVersion, 1);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+7, (unsigned char*)&factoryData->adcDataStatus, 1);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+8, (unsigned char*)&factoryData->adcLowLimit, 2);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+10, (unsigned char*)&factoryData->adcHighLimit, 2);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+12, (unsigned char*)&factoryData->adcBlockCal1, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+16, (unsigned char*)&factoryData->adcBlockCal2, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+20, (unsigned char*)&factoryData->adcBlockCal3, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+24, (unsigned char*)&factoryData->adcBlockCal4, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+28, (unsigned char*)&(factoryData)->adcOffsets, 64);

    if(!res)
        return res;

    Sleep(100);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+92, (unsigned char*)&(factoryData)->dacOffsets, 64);

    if(!res)
        return res;

    Sleep(100);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+156, (unsigned char*)&factoryData->dacBlockCal1, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+160, (unsigned char*)&factoryData->dacBlockCal2, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+164, (unsigned char*)&factoryData->analogBW1, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+168, (unsigned char*)&factoryData->analogBW2, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+172, (unsigned char*)&factoryData->analogConversion, 4);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+176, (unsigned char*)&factoryData->maxOffset, 2);

    if(!res)
        return res;

    Sleep(10);

    res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+178, (unsigned char*)&factoryData->defaultOffset, 2);

    if(!res)
        return res;

    Sleep(10);

    /*res = ReadPDI(client, ADC_BOARD, ADC_REG_CALIBRATION_TABLE+180, (unsigned char*)&factoryData->detectorIDList, 10);

    if(!res)
        return res;*/

    for(int i=0;i<1;i++)
    {
        unsigned char temp[10] = {0};
        res = ReadPDI(client, ADC_BOARD, ((ADC_REG_CALIBRATION_TABLE+180)+(i*10)), (unsigned char*)&temp, 10);

        string temp2;
        temp2.assign(reinterpret_cast<const char*>(&temp[0]));

        factoryData->detectorIDList[i] = temp2;

        if(!res)
            return res;

        Sleep(10);

    }

    return res;

}//GetFactoryData

bool GetNetworkConfiguration(CAPDClient *client, NetConfStr *netConf)
{
    bool retVal = false;

    unsigned char buffer[509];
    retVal = SendAck(client, buffer);

    /*for(int i=0;i<509;i++)
    {
        cout << (unsigned int)buffer[i] << endl;
    }*/

    if(buffer[15]==64)
    {
        netConf->linkSpeed=10;
    }
    else if(buffer[15]==128)
    {
        netConf->linkSpeed=100;
    }
    else if(buffer[15]==192)
    {
        netConf->linkSpeed=1000;
    }
    else
    {
        netConf->linkSpeed=0;
    }

    //For debug
    /*ofstream out;

    out.open("ackTest.txt", ios::app | ios::binary);
    out.write(reinterpret_cast<char*>(buffer), 509);
    out.close();*/

    Sleep(20);

    return retVal;

}//GetNetworkConfiguration

