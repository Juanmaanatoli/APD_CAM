#pragma once

#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

#define TPRINTF _tcprintf

typedef unsigned int ADT_HANDLE;
enum ADT_RESULT {ADT_OK = 0, ADT_ERROR, ADT_INVALID_HANDLE_ERROR, ADT_PARAMETER_ERROR, ADT_SETUP_ERROR, ADT_NOT_IMPLEMENTED, ADT_TIMEOUT, ADT_FACTORY_SETUP_ERROR};
enum ADT_MEASUREMENT_MODE {MM_ONE_SHOT, MM_CYCLIC};
enum ADT_CALIB_MODE {CM_NONCALIBRATED, CM_CALIBRATED};

enum ADT_TRIGGER {TR_SOFTWARE, TR_HARDWARE};
enum ADT_TRIGGER_MODE {TRM_EXTERNAL, TRM_INTERNAL};
enum ADT_TRIGGER_EDGE {TRE_RISING, TRE_FALLING};

struct TrgInfo
{

    unsigned short TriggerLevel;
    unsigned short Sensitivity;
    unsigned short Enable;

};

//For getting config on startup
struct APDStr
{

    int adcMult;
    int adcDiv;
    int strMult;
    int strDiv;
    int clkSource;
    int clkMult;
    int clkDiv;

    int sampleDiv;
    int sampleSrc;
    unsigned char testPattern;

    long long sampleNum;
    int bits;
    int channelMask1;
    int channelMask2;
    int channelMask3;
    int channelMask4;
    //int primaryBufferSize;
    //int measMode;
    //int calibMode;
    //int signalFreq;

    int trigSource;
    int trigMode;
    int trigEdge;
    int trigDelay;
    TrgInfo trigInfo[32];
    unsigned short preTrigSampleCount;

    double hv1;
    double hv2;
    bool hvOn[2];
    bool hvEnabled;
    bool analogEnable;

    short peltierIndirectControl;
    short detectorReferenceTemp;
    unsigned short pGain;
    unsigned short iGain;
    unsigned short dGain;
    bool disablePIDControl;

    unsigned char fanSpeed[3];

    unsigned short calibrationLight;
    unsigned char shutterMode;
    unsigned char shutterState;

    short offsets[32];

};

struct ADT_SYSTEM_STATUS
{

	unsigned short Firmware; // offs 0x02
	int	HVSate; // The four lower bits are the state of high voltage generators
	double HighVoltages[4];  // offs 0x04 - 0x0A
	double Temperatures[16]; // offs 0x0C - 0x2A
	double PeltierOutputVoltage; // offs 0x0C
	// Fan state?
	unsigned char ErrorCode; // // offs 0x86 - b bit
	unsigned char ShutterState; // offs 0x82
    unsigned short CalibrationLigth; // 12 bits // offs 0x7A - GetCalibLigth
};

struct Status1
{
    bool adcClkPllLocked;
    bool streamClkPllLocked;
};

struct Status2
{
    bool factoryResetPinState;
    bool overLoad;
    bool externalClkDcmLocked;
    bool ad1SampleEnabled;
    bool ad2SampleEnabled;
    bool ad3SampleEnabled;
    bool ad4SampleEnabled;
};

struct Control
{
    bool extClkSelect;
    bool eioClkOutEnable;
    bool sampleClkSelect;
    bool eioSampleOutEnable;
    bool filterOn;
    bool reserveBitOrderInStream;
    bool preamblePatternEnabled;
};

struct APDStatus
{
    //Control card status
    double highVoltages[4];
    double temperatures[16];
    double peltierOutputVoltage;
    unsigned char fanSpeed[3];
    unsigned short calibrationLight;
    unsigned char shutterMode;
    unsigned char shutterState;

    //ADC board status
    Status1 status1;
    Status2 status2;
    Control control;

};//APDStatus

struct APDInfo
{
    //Control card infos
    //unsigned short pcBoardSerial;
    unsigned char pcBoardVersion;
    unsigned short pcFWVersion;
    unsigned short hv1Max;
    unsigned short hv2Max;

    //ADC infos
    unsigned char adcBoardVersion;
    unsigned short adcMCVersion;
    unsigned short adcSerial;
    unsigned short fpgaVersion;


};//APDInfo

struct APDPeltierControl
{
    short indirectPeltierControl;
    unsigned short pGain;
    unsigned short iGain;
    unsigned short dGain;
    bool disablePIDControl;
};

struct APDFactory
{
    //Control board
    unsigned char pcTableVersion;
    unsigned char pcDataStatus;
    float inputHVCalib1;
    float inputHVCalib2;
    float outputHVCalib1;
    float outputHVCalib2;
    unsigned short tempCalib[16];
    std::string tempSensorName[16];
    short minHV;

    //ADC board
    char productCode[2];
    unsigned char serialNo[4];
    unsigned char adcTableVersion;
    unsigned char adcDataStatus;
    unsigned short adcLowLimit;
    unsigned short adcHighLimit;
    float adcBlockCal1;
    float adcBlockCal2;
    float adcBlockCal3;
    float adcBlockCal4;
    short adcOffsets[32];
    short dacOffsets[32];
    float dacBlockCal1;
    float dacBlockCal2;
    int analogBW1;
    int analogBW2;
    float analogConversion;
    short maxOffset;
    short defaultOffset;
    std::string detectorIDList[10];


};//APDFactoryData

struct NetConfStr
{
    unsigned short linkSpeed;
};

union ADT_STATUS_1
{
	unsigned char status;
	struct
	{
		unsigned char ADC_PLL_Locked : 1;
		unsigned char Stream_PLL_Locked : 1;
	};
};

union ADT_STATUS_2
{
	unsigned char status;
	struct
	{
		unsigned char reserved1 : 1;
		unsigned char Overload : 1;
		unsigned char External_clock_PLL_Locked : 1;
		unsigned char reserved2 : 1;
		unsigned char ADC_1_Sample_Enabled;
		unsigned char ADC_2_Sample_Enabled;
		unsigned char ADC_3_Sample_Enabled;
		unsigned char ADC_4_Sample_Enabled;
	};
};

union ADT_CONTROL
{
	unsigned char control;
	struct
	{
		unsigned char External_Clock_Select : 1;
		unsigned char Clock_Out_Enable : 1;
		unsigned char External_Sample_Select : 1;
		unsigned char Sample_Out_Enable : 1;
		unsigned char Digital_Filter_Enable;
		unsigned char Reserved;
		unsigned char Reverse_Bit_Order;
		unsigned char Preamble_Enable;
	};
};

union ADT_TRIGGER_CONTROL
{
	unsigned char trigger_control;
	struct
	{
		unsigned char Enable_Rising_Edge : 1;
		unsigned char Enable_Falling_Edge : 1;
		unsigned char Enable_Internal_Trigger : 1;
		unsigned char Reserved : 5;
	};
};



union ADT_TRIGGERINFO
{
	unsigned short TriggerInfo;
	struct
	{
		unsigned short TriggerLevel : 14;
		unsigned short Sensitivity : 1; // 0: positive, 1: negative
		unsigned short Enable : 1;
	};
};

union ADT_OVERLOADINFO
{
	unsigned short OverloadInfo;
	struct
	{
		unsigned short level : 14;
		unsigned short polarity : 1;
		unsigned short enable : 1;
	};
};

#pragma pack(push)
#pragma pack(2)

struct FILTER_COEFFICIENTS
{
	short FIR[5];
	short RecursiveFilter;
	short Reserved;
	short FilterDevideFactor;
};

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
