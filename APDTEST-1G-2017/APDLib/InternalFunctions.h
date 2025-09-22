#pragma once
#ifndef __INTERNALFUNCTIONS_H__

#include "LowlevelFunctions.h"
#include "TypeDefs.h"

#define ADC_BOARD 1
#define PC_BOARD 2


// ADC board registers
//=======================================
#define ADC_REG_BOARD_VERSION 0x00
#define ADC_REG_BOARD_VERSION_LEN 0x01

#define ADC_REG_MC_VERSION 0x01
#define ADC_REG_MC_VERSION_LEN 0x02

#define ADC_REG_SERIAL 0x03
#define ADC_REG_SERIAL_LEN 0x02

#define ADC_REG_FPGA_VERSION 0x05
#define ADC_REG_FPGA_VERSION_LEN 0x02

#define ADC_REG_STATUS1 0x08
#define ADC_REG_STATUS1_LEN 0x01

#define ADC_REG_STATUS2 0x09
#define ADC_REG_STATUS2_LEN 0x01

// Control register. See bit definitions
#define ADC_REG_CONTROL 0x0B
#define ADC_REG_CONTROL_LEN 0x01

// ADC_PLL_MULT
#define ADC_REG_ADCCLKMUL 0x0C
#define ADC_REG_ADCCLKMUL_LEN 0x01

// ADC_PLL_DIV
#define ADC_REG_ADCCLKDIV 0x0D
#define ADC_REG_ADCCLKDIV_LEN 0x01

// STREAM_PLL_MULT
#define ADC_REG_STREAMCLKMUL 0x0E
#define ADC_REG_STREAMCLKMUL_LEN 0x01

// STREAM_PLL_DIV
#define ADC_REG_STREAMCLKDIV 0x0F
#define ADC_REG_STREAMCLKDIV_LEN 0x01

// STREAM:CTRL
#define ADC_REG_STREAMCONTROL 0x10
#define ADC_REG_STREAMCONTROL_LEN 0x01

//
#define ADC_REG_SAMPLECNT 0x11
#define ADC_REG_SAMPLECNT_LEN 0x04

// Enables/disables all channel1
#define ADC_REG_CHENABLE1 0x15
#define ADC_REG_CHENABLE1_LEN 0x01

// Enables/disables all channel2
#define ADC_REG_CHENABLE2 0x16
#define ADC_REG_CHENABLE2_LEN 0x01

// Enables/disables all channel3
#define ADC_REG_CHENABLE3 0x17
#define ADC_REG_CHENABLE3_LEN 0x01

// Enables/disables all channel4
#define ADC_REG_CHENABLE4 0x18
#define ADC_REG_CHENABLE4_LEN 0x01

// Enables/disables all channel
#define ADC_REG_CHENABLE 0x15
#define ADC_REG_CHENABLE_LEN 0x04

// Set internal ring buffer size
#define ADC_REG_RINGBUFSIZE 0x19
#define ADC_REG_RINGBUFSIZE_LEN 0x02

// Set/Get bit resolution
#define ADC_REG_RESOLUTION 0x1B
#define ADC_REG_RESOLUTION_LEN 0x01

// SampleDiv_x_7
#define ADC_REG_ADSAMPLEDIV 0x1C
#define ADC_REG_ADSAMPLEDIV_LEN 0x02

// Trigger
#define ADC_REG_TRIGGER 0x1E
#define ADC_REG_TRIGGER_LEN 0x01

// ADC test mode
#define ADC1_REG_TEST_MODE 0x20
#define ADC2_REG_TEST_MODE 0x21
#define ADC3_REG_TEST_MODE 0x22
#define ADC4_REG_TEST_MODE 0x23
#define ADC_REG_TEST_MODE_LEN 0x01

// ADC Errorcode
#define ADC_REG_ERROR_CODE 0x24
#define ADC_REG_ERROR_CODE_LEN 0x01

// Factory reset
#define ADC_REG_FACTORY_RESET 0x25
#define ADC_REG_FACTORY_RESET_LEN 0x01

// Factory write enable
#define ADC_REG_FACTORY_WRITE_ENABLE 0x26
#define ADC_REG_FACTORY_WRITE_LEN 0x02
#define ADC_REG_FACTORY_WRITE_CODE 0x93B2

// Bytes per sample
#define ADC_REG_BYTES_PER_SAMPLE 0x28
#define ADC_REG_BYTES_PER_SAMPLE_LEN 0x04

//  ADC_REG_EXTCLKMUL
#define ADC_REG_EXTCLKMUL 0x2E
#define ADC_REG_EXTCLKMUL_LEN 0x01

//  ADC_REG_EXTCLKDIV
#define ADC_REG_EXTCLKDIV 0x2F
#define ADC_REG_EXTCLKDIV_LEN 0x01

// DAC Offset ( ADC_REG_DAC1)
#define ADC_REG_OFFSET 0x30
#define ADC_REG_OFFSET_LEN 0x40

// Inernal trigger levels 
#define ADC_REG_INT_TRG_LEVEL 0x70
#define ADC_REG_INT_TRG_LEVEL_LEN 0x40

// Acquired samples per channel
#define ADC_REG_AQRD_SAMPLE 0xB0
#define ADC_REG_AQRD_SAMPLE_LEN 0x10

// Overload level
#define ADC_REG_OVERLOAD_LEVEL 0xC0
#define ADC_REG_OVERLOAD_LEVEL_LEN 0x02

// Overload status
#define ADC_REG_OVERLOAD_STATUS 0xC2
#define ADC_REG_OVERLOAD_STATUS_LEN 0x01

// Overload time
#define ADC_REG_OVERLOAD_TIME 0xC3
#define ADC_REG_OVERLOAD_TIME_LEN 0x02

// Trigger delay
#define ADC_REG_TRIGGER_DELAY 0xC5
#define ADC_REG_TRIGGER_DELAY_LEN 0x04

// Filter coeficients
#define ADC_REG_FILTER_COEFF 0xC9
#define ADC_REG_FILTER_COEFF_LEN 0x10

/* ****** ADC Calibration (factory) table ****** */
#define ADC_REG_CALIBRATION_TABLE 0x100
#define ADC_REG_CALIBRATION_TABLE_LEN 0xBC

// SerialNo
#define ADC_REG_SERIALNO 0x100
#define ADC_REG_SERIALNO_LEN 0x06
// Table version
#define ADC_REG_TABLE_VERSION 0x106
#define ADC_REG_TABLE_VERSION_LEN 0x01
// Table status
#define ADC_REG_TABLE_STATUS 0x107
#define ADC_REG_TABLE_STATUS_LEN 0x01
// short adcLowerLimit
#define ADC_REG_LOWER_LIMIT 0x108
#define ADC_REG_LOWER_LIMIT_LEN 0x02
// short adcUpperLimit
#define ADC_REG_UPPER_LIMIT 0x10A
#define ADC_REG_UPPER_LIMIT_LEN 0x02
// float blockCalibration[4]
#define ADC_REG_ADC_BLOCK_CALIBRATION 0x10C
#define ADC_REG_ADC_BLOCK_CALIBRATION_LEN 0x10
// short adcOffset[32]
#define ADC_REG_ADC_OFFSET 0x11C
#define ADC_REG_ADC_OFFSET_LEN 0x40
// short dacOffset[32]
#define ADC_REG_DAC_OFFSET 0x15C
#define ADC_REG_DAC_OFFSET_LEN 0x40
// float dacConversion[2]
#define ADC_REG_DAC_BLOCK_CALIBRATION 0x19C
#define ADC_REG_DAC_BLOCK_CALIBRATION_LEN 0x08
//float AnalogBW1;
#define ADC_REG_ANALOG_BW1 0x1A4
#define ADC_REG_ANALOG_BW1_LEN 0x04
//float AnalogBW2;
#define ADC_REG_ANALOG_BW2 0x1A8
#define ADC_REG_ANALOG_BW2_LEN 0x04
//float AnalogConversion;
#define ADC_REG_ANALOG_CONVERSION 0x1AC
#define ADC_REG_ANALOG_CONVERSION_LEN 0x04
//unsigned short MaxOffset;
#define ADC_REG_MAX_OFFSET 0x1B0
#define ADC_REG_MAX_OFFSET_LEN 0x02
//unsigned char DetectorId[10];
#define ADC_REG_DETECTOR_ID 0x1B2
#define ADC_REG_DETECTOR_ID_LEN 0x0A


// Control board registers
//=======================================
#define PC_REG_BOARD_VERSION 0x00
#define PC_REG_BOARD_VERSION_LEN 0x01

#define PC_REG_FW_VERSION 0x02
#define PC_REG_FW_VERSION_LEN 0x02

#define PC_REG_HV1_MONITOR 0x04
#define PC_REG_HV2_MONITOR 0x06
#define PC_REG_HV3_MONITOR 0x08
#define PC_REG_HV4_MONITOR 0x0A
#define PC_REG_HV_MONITOR_LEN 0x02
#define PC_REG_ALL_HV_MONITORS_LEN 0x08

#define PC_REG_TEMP_SENSOR_1 0x0C
#define PC_REG_TEMP_SENSOR_2 0x0E
#define PC_REG_TEMP_SENSOR_3 0x10
#define PC_REG_TEMP_SENSOR_4 0x12
#define PC_REG_TEMP_SENSOR_5 0x14
#define PC_REG_TEMP_SENSOR_6 0x16
#define PC_REG_TEMP_SENSOR_7 0x18
#define PC_REG_TEMP_SENSOR_8 0x1A
#define PC_REG_TEMP_SENSOR_9 0x1C
#define PC_REG_TEMP_SENSOR_10 0x1E
#define PC_REG_TEMP_SENSOR_11 0x20
#define PC_REG_TEMP_SENSOR_12 0x22
#define PC_REG_TEMP_SENSOR_13 0x24
#define PC_REG_TEMP_SENSOR_14 0x26
#define PC_REG_TEMP_SENSOR_15 0x28
#define PC_REG_TEMP_SENSOR_16 0x2A
#define PC_REG_TEMP_SENSOR_LEN 0x02
#define PC_REG_ALL_TEMP_SENSORS_LEN 0x20

#define PC_REG_PELT_CONTROL_OUT 0x2C
#define PC_REG_PELT_CONTROL_OUT_LEN 0x02

#define PC_REG_PELT_DAC_REG 0x4E
#define PC_REG_PELT_DAC_REG_LEN 0x02

#define PC_REG_P_GAIN 0x50
#define PC_REG_I_GAIN 0x52
#define PC_REG_D_GAIN 0x54
#define PC_REG_GAIN_LEN 0x02

#define PC_REG_HV1_SET 0x56
#define PC_REG_HV2_SET 0x58
#define PC_REG_HV3_SET 0x5A
#define PC_REG_HV4_SET 0x5C
#define PC_REG_HV_SET_LEN 0x02

#define PC_REG_HV_ON 0x5E
#define PC_REG_HV_ON_LEN 0x01

#define PC_REG_HV_ENABLE 0x60
#define PC_REG_HV_ENABLE_LEN 0x01

#define PC_REG_IRQ_ENABLE_HV 0x62
#define PC_REG_IRQ_ENABLE_HV_LEN 0x01

#define PC_REG_IRQ_POWER_PID_ENABLE 0x64
#define PC_REG_IRQ_POWER_PID_ENABLE_LEN 0x01

#define PC_REG_HV_IRQ_LEVEL 0x66
#define PC_REG_HV_IRQ_LEVEL_LEN 0x02

#define PC_REG_HV_FAILURE 0x68
#define PC_REG_HV_FAILURE_LEN 0x01

#define PC_REG_DETECTOR_TEMP_SET 0x6A
#define PC_REG_DETECTOR_TEMP_SET_LEN 0x02

#define PC_REG_FAN1_SPEED 0x6C
#define PC_REG_FAN2_SPEED 0x6E
#define PC_REG_FAN3_SPEED 0x70
#define PC_REG_FAN_SPEED_LEN 0x01

#define PC_REG_FAN1_TEMP_SET 0x72
#define PC_REG_FAN1_TEMP_SET_LEN 0x02
#define PC_REG_FAN1_TEMP_DIF 0x74
#define PC_REG_FAN1_TEMP_DIF_LEN 0x02
#define PC_REG_FAN2_TEMP_LIMIT 0x76
#define PC_REG_FAN3_TEMP_LIMIT 0x78
#define PC_REG_FAN_TEMP_LIMIT_LEN 0x02

// CALIB_LIGHT 12 bit
#define PC_REG_CALIB_LIGHT 0x7A
#define PC_REG_CALIB_LIGHT_LEN 0x02

#define PC_REG_CALIB_LIGHT_UPDATE_TIME 0x7C
#define PC_REG_CALIB_LIGHT_UPDATE_TIME_LEN 0x02

#define PC_REG_IRQ_STATUS 0x7E
#define PC_REG_IRQ_STATUS_LEN 0x01

#define PC_REG_SHMODE 0x80
#define PC_REG_SHMODE_LEN 0x01

#define PC_REG_SHSTATE 0x82
#define PC_REG_SHSTATE_LEN 0x01

//#define PC_REG_RESET_FACTORY 0x84
//#define PC_REG_RESET_FACTORY_LEN 0x01

#define PC_REG_ERROR_CODE 0x86
#define PC_REG_ERROR_CODE_LEN 0x01

#define PC_REG_HV1MAX 0x102
#define PC_REG_HV1MAX_LEN 0x02

#define PC_REG_HV2MAX 0x104
#define PC_REG_HV2MAX_LEN 0x02

// Factory write enable
#define PC_REG_FACTORY_WRITE_ENABLE 0x88
#define PC_REG_FACTORY_WRITE_LEN 0x02
#define PC_REG_FACTORY_WRITE_CODE 0x00CD


/* ****** PC Calibration (factory) table ****** */
#define PC_REG_CALIBRATION_TABLE 0x200
#define PC_REG_CALIBRATION_TABLE_LEN 0xB2

// Table version
#define PC_REG_TABLE_VERSION 0x200
#define PC_REG_TABLE_VERSION_LEN 0x01
// Table status
#define PC_REG_TABLE_STATUS 0x201
#define PC_REG_TABLE_STATUS_LEN 0x01
// float InputHVCalib[2];
#define PC_REG_INPUT_HV_CALIB 0x202
#define PC_REG_INPUT_HV_CALIB_LEN 0x08
// float OutputHVCalib[2];
#define PC_REG_OUTPUT_HV_CALIB 0x20A
#define PC_REG_OUTPUT_HV_CALIB_LEN 0x08
// short int TempCalib[16];
#define PC_REG_TEMP_CALIB 0x212
#define PC_REG_TEMP_CALIB_LEN 0x20
// unsigned short MinHV;
#define PC_REG_MIN_HV 0x232
#define PC_REG_MIN_HV_LEN 0x02
// unsigned char GainTable[256]; // Must be replaced with a meaningfull structure.
#define PC_REG_GAIN_TABLE 0x234
#define PC_REG_GAIN_TABLE_LEN 0x64
// unsigned short GainVolts[8];
#define PC_REG_GAIN_VOLTS 0x298
#define PC_REG_GAIN_VOLTS_LEN 0x10
// unsigned short GainTemps[5];
#define PC_REG_GAIN_TEMPS 0x2A8
#define PC_REG_GAIN_TEMPS_LEN 0x0A


//Common functions
//===============================================================
bool GetInfo(CAPDClient *client, APDInfo *apdInfo);
bool GetStatus(CAPDClient *client, APDStatus *apdStatus);

bool GetNetworkConfiguration(CAPDClient *client, NetConfStr *netConf);

//ADC board functions
//==================================================================
bool GetADCBoardVersion(CAPDClient *client, unsigned char *boardVersion, unsigned long ipAddress_h = 0, unsigned short ipPort_h = 0, int timeout = 100);
bool GetMCVersion(CAPDClient *client, unsigned short *mcVersion);
bool GetSerial(CAPDClient *client, unsigned short *serial);
bool GetFPGAVersion(CAPDClient *client, unsigned short *fpgaVersion);
bool GetStatus1(CAPDClient *client, unsigned char *status1);
bool GetStatus2(CAPDClient *client, unsigned char *status2);
bool SetADCControl(CAPDClient *client, unsigned char adcControl);
bool GetADCControl(CAPDClient *client, unsigned char *adcControl);
bool SetADCPLL(CAPDClient *client, unsigned char mult, unsigned char div);
bool GetADCPLL(CAPDClient *client, unsigned char *mult, unsigned char *div);
bool SetStreamPLL(CAPDClient *client, unsigned char mult, unsigned char div);
bool GetStreamPLL(CAPDClient *client, unsigned char *mult, unsigned char *div);
bool SetStreamControl(CAPDClient *client, unsigned char streamControl);
bool GetStreamControl(CAPDClient *client, unsigned char *streamControl);
bool SetSampleCount(CAPDClient *client, unsigned int sampleCount);
bool GetSampleCount(CAPDClient *client, unsigned int *sampleCount);
bool SetChannel_1(CAPDClient *client, unsigned char channelMask_1);
bool SetChannel_2(CAPDClient *client, unsigned char channelMask_2);
bool SetChannel_3(CAPDClient *client, unsigned char channelMask_3);
bool SetChannel_4(CAPDClient *client, unsigned char channelMask_4);
bool SetChannels(CAPDClient *client, unsigned char channelMask_1, unsigned char channelMask_2, unsigned char channelMask_3, unsigned char channelMask_4);
bool GetChannels(CAPDClient *client, unsigned char *channelMask_1, unsigned char *channelMask_2, unsigned char *channelMask_3, unsigned char *channelMask_4);
bool SetRingbufferSize(CAPDClient *client, unsigned short bufferSize);
bool GetRingbufferSize(CAPDClient *client, unsigned short *bufferSize);
bool SetResolution(CAPDClient *client, int bitNum);
bool GetResolution(CAPDClient *client, int *bitNum);
bool SetSampleDiv(CAPDClient *client, unsigned short value);
bool GetSampleDiv(CAPDClient *client, unsigned short *value);
bool SetTrigger(CAPDClient *client, unsigned char value);
bool GetTrigger(CAPDClient *client, unsigned char *value);
bool SetTestMode(CAPDClient *client, unsigned int mode);
bool GetTestMode(CAPDClient *client, unsigned int *mode);
bool GetADCBoardError(CAPDClient *client, unsigned char *error);
bool FactoryReset(CAPDClient *client);
bool GetBytesPerSample(CAPDClient *client, unsigned int *counters);
bool SetExtClkPLL(CAPDClient *client, unsigned char mult, unsigned char div);
bool GetExtClkPLL(CAPDClient *client, unsigned char *mult, unsigned char *div);
bool SetADCOffset(CAPDClient *client, short *offsets, int first, int no);
bool GetADCOffset(CAPDClient *client, short *offsets, int first, int no);
bool SetInternalTriggerLevels(CAPDClient *client, unsigned short *levels);
bool GetInternalTriggerLevels(CAPDClient *client, unsigned short *levels);
bool GetAquiredSampleCount(CAPDClient *client, unsigned int *counters);
bool SetOverloadLevel(CAPDClient *client, unsigned short level);
bool GetOverloadLevel(CAPDClient *client, unsigned short *level);
bool SetOverloadStatus(CAPDClient *client, unsigned char status);
bool GetOverloadStatus(CAPDClient *client, unsigned char *status);
bool SetOverloadTime(CAPDClient *client, unsigned short time);
bool GetOverloadTime(CAPDClient *client, unsigned short *time);
bool SetTriggerDelay(CAPDClient *client, unsigned int delay);
bool GetTriggerDelay(CAPDClient *client, unsigned int *delay);
bool SetFilterCoefficients(CAPDClient *client, unsigned short *coefficints);
bool GetSetFilterCoefficients(CAPDClient *client, unsigned short *coefficints);


//Control board functions
//==================================================================
bool GetControlBoardVersion(CAPDClient *client, unsigned char *boardVersion);
bool GetControlBoardFWVersion(CAPDClient *client, unsigned short *fwVersion);
bool GetAllHVMonitor(CAPDClient *client, unsigned short *binValues);
bool GetAllTempSensors(CAPDClient *client, double *values);
bool GetPeltierOutputVoltage(CAPDClient *client, unsigned short *voltage);
bool SetPeltierIndirectControl(CAPDClient *client, short value);
bool GetPeltierIndirectControl(CAPDClient *client, short *value);
bool SetDetectorReferenceTemp(CAPDClient *client, short value);
bool GetDetectorReferenceTemp(CAPDClient *client, short *value);
bool SetPGain(CAPDClient *client, unsigned short p);
bool GetPGain(CAPDClient *client, unsigned short *p);
bool SetIGain(CAPDClient *client, unsigned short i);
bool GetIGain(CAPDClient *client, unsigned short *i);
bool SetDGain(CAPDClient *client, unsigned short d);
bool GetDGain(CAPDClient *client, unsigned short *d);
bool SetHV1(CAPDClient *client, int binValue);
bool GetHV1(CAPDClient *client, int *binValue);
bool SetHV2(CAPDClient *client, int binValue);
bool GetHV2(CAPDClient *client, int *binValue);
bool SetHVState(CAPDClient *client, int state);
bool GetHVState(CAPDClient *client, int *state);
bool EnableHV(CAPDClient *client, bool enable);
bool GetEnableHV(CAPDClient *client, bool *enable);
bool SetTempInterruptEnable(CAPDClient *client, bool enable);
bool GetTempInterruptEnable(CAPDClient *client, bool *enable);
bool SetAnalogPower(CAPDClient *client, bool enable);
bool GetAnalogPower(CAPDClient *client, bool *enable);
bool SetDisablePIDControl(CAPDClient *client, bool enable);
bool GetDisablePIDControl(CAPDClient *client, bool *enable);
bool SetFan1Speed(CAPDClient *client, unsigned char speed);
bool GetFan1Speed(CAPDClient *client, unsigned char *speed);
bool SetFan2Speed(CAPDClient *client, unsigned char speed);
bool GetFan2Speed(CAPDClient *client, unsigned char *speed);
bool SetFan3Speed(CAPDClient *client, unsigned char speed);
bool GetFan3Speed(CAPDClient *client, unsigned char *speed);
bool SetCalibLigth(CAPDClient *client, int current);
bool GetCalibLigth(CAPDClient *client, int *current);
bool SetShutterMode(CAPDClient *client, int mode);
bool GetShutterMode(CAPDClient *client, int *mode);
bool SetShutterState(CAPDClient *client, int state);
bool GetShutterState(CAPDClient *client, int *state);
bool GetControlBoardError(CAPDClient *client, unsigned char *error);
bool GetHV1Max(CAPDClient *client, unsigned short *hv);
bool GetHV2Max(CAPDClient *client, unsigned short *hv);



// Calibration table handling routines
// Values - stored in the calibration table - not efect the hardware. That table is a non-volative storage place only.
bool RetrieveADCSerialNo(CAPDClient *client, char *serial_no, int len, unsigned long ip_h = 0, unsigned short port_h = 0);
bool StoreADCOffsets(CAPDClient *client, short *adcOffsets, int first, int no);
bool RetrieveADCOffsets(CAPDClient *client, short *adcOffsets, int first, int no);
bool StoreDACOffsets(CAPDClient *client, short *dacOffsets, int first, int no);
bool RetrieveDACOffsets(CAPDClient *client, short *dacOffsets, int first, int no);

//bool EnableFactoryWrite(CAPDClient *client, int boardId, bool enable);
//bool SetFactoryData(CAPDClient *client, ADT_FACTORY_DATA dataType, FACTORY_DATA* pFactoryData);
bool GetFactoryData(CAPDClient *client, APDFactory *factoryData);

#endif
