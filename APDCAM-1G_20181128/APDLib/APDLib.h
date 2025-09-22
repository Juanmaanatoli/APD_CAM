#pragma once

#include "TypeDefs.h"

#ifdef __cplusplus
extern "C"
{
#endif


void APDCAM_Init();
void APDCAM_Done();
void APDCAM_GetSWOptions();

void APDCAM_Find(unsigned long from_ip_h, unsigned long to_ip_h, unsigned long *ip_table, int table_size, int *no_of_elements, char *filter_str = NULL, int timeout = 50);

ADT_HANDLE APDCAM_Open(unsigned long ip_h, unsigned char openMode=0);
ADT_RESULT APDCAM_Close(ADT_HANDLE handle);

ADT_RESULT APDCAM_SelfTest(ADT_HANDLE handle);

ADT_RESULT APDCAM_Allocate(ADT_HANDLE handle, long long sampleCount, int bits, int channelMask_1, int channelMask_2, int channelMask_3, int channelMask_4, int primary_buffer_size = 100);
ADT_RESULT APDCAM_GetBuffers(ADT_HANDLE handle, short **buffers);
ADT_RESULT APDCAM_GetSampleInfo(ADT_HANDLE handle, unsigned long long *sampleCounts, unsigned long long *sampleIndices);

ADT_RESULT APDCAM_SetTiming(ADT_HANDLE handle, int adcMult, int adcDiv, int strMult, int strDiv, int clkSource, int clkMult, int clkDiv);
ADT_RESULT APDCAM_Sampling(ADT_HANDLE handle, int sampleDiv, int sampleSrc);

ADT_RESULT APDCAM_Arm(ADT_HANDLE handle, ADT_MEASUREMENT_MODE mode, long long sampleCount, ADT_CALIB_MODE calibMode, int signalFrequency = 100);
ADT_RESULT APDCAM_Trigger(ADT_HANDLE handle, ADT_TRIGGER trigger, ADT_TRIGGER_MODE mode, ADT_TRIGGER_EDGE edge, int triggerDelay, ADT_TRIGGERINFO* triggerInfo, unsigned short preTriggerSampleCount);

ADT_RESULT APDCAM_Start(ADT_HANDLE handle);
ADT_RESULT APDCAM_Wait(ADT_HANDLE handle, int timeout);
ADT_RESULT APDCAM_Stop(ADT_HANDLE handle);

ADT_RESULT APDCAM_SetIP(ADT_HANDLE handle, unsigned long ip_h);

ADT_RESULT APDCAM_DataMode(ADT_HANDLE handle, int modeCode);
ADT_RESULT APDCAM_SetFilter(ADT_HANDLE handle, FILTER_COEFFICIENTS filterCoefficients);
ADT_RESULT APDCAM_CalculateFilterParams(ADT_HANDLE handle, double f_fir, double f_rec,  FILTER_COEFFICIENTS &filterCoefficients);

ADT_RESULT APDCAM_Shutter(ADT_HANDLE handle, int open);
ADT_RESULT APDCAM_SetShutterMode(ADT_HANDLE handle, int mode);
ADT_RESULT APDCAM_GetShutterMode(ADT_HANDLE handle, int *mode);

ADT_RESULT APDCAM_SetCalibLight(ADT_HANDLE handle, int value);
ADT_RESULT APDCAM_GetCalibLight(ADT_HANDLE handle, int *value);
ADT_RESULT APDCAM_Calibrate(ADT_HANDLE handle);
ADT_RESULT APDCAM_SetHV(ADT_HANDLE handle, double highVoltage1, double highVoltage2, int state);
ADT_RESULT APDCAM_GetHV(ADT_HANDLE handle, double *highVoltage1, double *highVoltage2, int *state);

ADT_RESULT APDCAM_GetStatus(ADT_HANDLE handle, APDStatus *apdStatus);
ADT_RESULT APDCAM_GetInfo(ADT_HANDLE handle, APDInfo* apdInfo);

ADT_RESULT APDCAM_SetPeltierControl(ADT_HANDLE handle, APDPeltierControl apdPeltierControl);
ADT_RESULT APDCAM_GetPeltierControl(ADT_HANDLE handle, APDPeltierControl *apdPeltierControl);

ADT_RESULT APDCAM_SetDetectorReferenceTemp(ADT_HANDLE handle, short referenceTemp);
ADT_RESULT APDCAM_GetDetectorReferenceTemp(ADT_HANDLE handle, short *referenceTemp);

ADT_RESULT APDCAM_SetOverload(ADT_HANDLE handle, ADT_OVERLOADINFO overloadInfo, unsigned short overloadTime);
ADT_RESULT APDCAM_GetOverload(ADT_HANDLE handle, ADT_OVERLOADINFO &overloadInfo, unsigned short &overloadTime, unsigned char &status);

ADT_RESULT APDCAM_SetAnalogPower(ADT_HANDLE handle, bool enable);
ADT_RESULT APDCAM_GetAnalogPower(ADT_HANDLE handle, bool *enable);

ADT_RESULT APDCAM_SetADCOffsets(ADT_HANDLE handle, short *values);
ADT_RESULT APDCAM_GetADCOffsets(ADT_HANDLE handle, short *values);

ADT_RESULT APDCAM_GetConfig(ADT_HANDLE handle, APDStr *apdStr);
ADT_RESULT APDCAM_GetFactoryConfig(ADT_HANDLE handle, APDFactory* apdFactory);

ADT_RESULT APDCAM_SetFansSpeed(ADT_HANDLE handle, unsigned char fan1Speed, unsigned char fan2Speed, unsigned char fan3Speed);

ADT_RESULT APDCAM_GetNetworkConfig(ADT_HANDLE handle, NetConfStr* netConfStr);

//--------------------------
ADT_RESULT APDCAM_WritePDI(ADT_HANDLE handle, unsigned char address, unsigned long subaddress, unsigned char* buffer, int noofbytes);
ADT_RESULT APDCAM_ReadPDI(ADT_HANDLE handle, unsigned char address, unsigned long subaddress, unsigned char* buffer, int noofbytes);
ADT_RESULT APDCAM_SetupAllTS(ADT_HANDLE handle);
ADT_RESULT APDCAM_ShutupAllTS(ADT_HANDLE handle);
//--------------------------



#ifdef __cplusplus
}
#endif
