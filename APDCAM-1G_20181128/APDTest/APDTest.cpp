
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

#include <math.h>

#include "../APDLib/APDLib.h"

#define MAX_RW_BYTES 100  // Maximum number of bytes in register read/write 

int ProcessLine(char *buffer);

ADT_HANDLE g_handle = 0;
int g_sampleCount = 0;
int Open(char *param0);
int ForceOpen(char *param0);
int Close();

int ReadLine(FILE * f, char * buffer, int size);
char *GetToken(char *str, char *token);
char *GetString(char *str, char *param);
char *GetInt(char *str, int *param);

int channelMask1;
int channelMask2;
int channelMask3;
int channelMask4;

using namespace std;


string versionNum="V1.5.1";

std::ofstream measLog;


int main(int argc, char* argv[])
{
    cout << "Fusion Instruments Ltd." << endl;
    cout << "APDTest " << versionNum  << endl;
    cout << "=====================================" << endl;

    char batchFileName[512];
    strcpy(batchFileName,"APDMain.txt");

    FILE * batchFile;

    if (argc > 1)
    {
        strcpy(batchFileName, argv[1]);
    }

    if ((batchFile = fopen(batchFileName,"rt")) == NULL)
    {
        printf("%s file not found\n", batchFileName);
        return 1;
    }

    APDCAM_Init();

    while (!feof(batchFile))
    {
        char buffer[1024]={0};
        ReadLine(batchFile,buffer, 1024);
        if (ProcessLine(buffer) < 0)
        {
            break;
        }
    }

	fclose(batchFile);

	APDCAM_Done();
	return 0;
}

char *GetToken(char *str, char *token)
{
	while (*str == ' ') str++;
	char *p = str;
	int c = 0;
	while (*p != '\0' && *p !=' ')
	{
		p++; c++;
	}
	if (c) memcpy(token, str, c);
	token[c] = '\0';
	while (*p == ' ') p++;
	return p;
}
char *GetString(char *str, char *param)
{
	while (*str == ' ') str++;
	char *p = str;
	int c = 0;
	while (*p != 0 && *p !=' ')
	{
		p++; c++;
	}
	if (c) memcpy(param, str, c);
	param[c] = '\0';
	while (*p == ' ') p++;
	return p;
}

char *GetInt(char *str, int *param)
{
	char strParam[32];
	char *p = GetString(str, strParam);
	if (p != str) *param = atoi(strParam);
	return p;
}


void SwOptions()
{
    APDCAM_GetSWOptions();
}

int Open(char *param0)
{
    unsigned long table[32];
	table[0] = 0;
	int nelems = 0;

    APDCAM_Find(ntohl(inet_addr(param0)), ntohl(inet_addr(param0)), table, 1, &nelems, (char *)"*", 50);
	if (nelems == 0) return -1;

	g_handle = APDCAM_Open(table[0]);

    if (g_handle == 0)
        return -1;
    else if (g_handle == 1)
        return -2;
    else if (g_handle == 2)
        return -3;
    else if (g_handle == 3)
        return -4;

    cout << "APDCAM opened." ;

	return 0;
}

int ForceOpen(char *param0)
{


    unsigned long table[32];
    table[0] = 0;
    int nelems = 0;

    APDCAM_Find(ntohl(inet_addr(param0)), ntohl(inet_addr(param0)), table, 1, &nelems, (char *)"*", 50);

    g_handle = APDCAM_Open(table[0], 1);

    if (g_handle == 0)
        return -1;

    if (g_handle == 1)
        return -2;

    cout << "APDCAM opened." ;

    return 0;
}

int Close()
{
	if (g_handle != 0) 
	{
		APDCAM_Close(g_handle);
		g_handle = 0;
	}
	return 0;
}

int SetTiming(int adcMult, int adcDiv, int strMult, int strDiv, int clkSrc, int clkMult, int clkDiv)
{

	ADT_RESULT res = APDCAM_SetTiming(g_handle, adcMult, adcDiv, strMult, strDiv, clkSrc, clkMult, clkDiv);
	if (res != ADT_OK) return -1;
	return 0;
}

int Sampling(int sampleDiv, int sampleSrc)
{

	ADT_RESULT res = APDCAM_Sampling(g_handle, sampleDiv, sampleSrc);
	if (res != ADT_OK) return -1;
	return 0;
}

int Allocate(long long sampleCount, int bits, int channelMask_1, int channelMask_2, int channelMask_3, int channelMask_4, int primaryBufferSize)
{

    channelMask1 = channelMask_1;
    channelMask2 = channelMask_2;
    channelMask3 = channelMask_3;
    channelMask4 = channelMask_4;

    if(primaryBufferSize>100)
        primaryBufferSize=100;

	ADT_RESULT res = APDCAM_Allocate(g_handle, sampleCount, bits, channelMask_1, channelMask_2, channelMask_3, channelMask_4, primaryBufferSize);
	if (res != ADT_OK) return -1;
	return 0;
}

int ARM(int measurementMode, int sampleCount, int calibrationMode, int signalFrequency)
{
	ADT_MEASUREMENT_MODE amm;
	switch(measurementMode)
	{
	case 0:
		amm = MM_ONE_SHOT;
		break;
	case 1:
		amm = MM_CYCLIC;
		break;
	default:
		return -1;
		break;
	}

	ADT_CALIB_MODE acm;
	switch (calibrationMode)
	{
	case 0:
		acm = CM_NONCALIBRATED;
		break;
	case 1:
		acm = CM_CALIBRATED;
		break;
	default:
		return -1;
		break;
	}

	g_sampleCount = sampleCount;

    ADT_RESULT res = APDCAM_Arm(g_handle, amm, sampleCount, acm, signalFrequency);
	if (res != ADT_OK) return -1;
	return 0;
}
// triggerSource 0:software, 1:hardware
// triggerMode 0:external (hardware), 1:internal (hardware)
// triggerEdge 0:rising, 1:falling
int Trigger(int triggerSource, int triggerMode, int triggerEdge, int delay, ADT_TRIGGERINFO *trigger, int preTriggerSampleCount)
{
	ADT_TRIGGER ts;
	switch (triggerSource)
	{
	case 0:
		ts = TR_SOFTWARE;
		break;
	case 1:
		ts = TR_HARDWARE;
		break;
	default:
		return -1;
		break;
	}

	ADT_TRIGGER_MODE tm;
	switch (triggerMode)
	{
	case 0:
		tm = TRM_EXTERNAL;
		break;
	case 1:
		tm = TRM_INTERNAL;
		break;
	default:
		return -1;
		break;
	}

	ADT_TRIGGER_EDGE te;
	switch (triggerEdge)
	{
	case 0:
		te = TRE_RISING;
		break;
	case 1:
		te = TRE_FALLING;
		break;
	default:
		return -1;
		break;
	}

    ADT_RESULT res = APDCAM_Trigger(g_handle, ts, tm, te, delay, trigger, (unsigned short)preTriggerSampleCount);
	if (res != ADT_OK) return -1;
	return 0;
}


int Start()
{
	ADT_RESULT res = APDCAM_Start(g_handle);
	if (res != ADT_OK) return -1;
	return 0;
}

int Wait(int timeout)
{
	ADT_RESULT res = APDCAM_Wait(g_handle, timeout);
	if (res != ADT_OK) return -1;
	return 0;
}

int Stop()
{
	ADT_RESULT res = APDCAM_Stop(g_handle);
	if (res != ADT_OK) return -1;
	return 0;
}

int Save()
{
    unsigned long long sampleCounts[4];
    unsigned long long sampleIndices[4];
    ADT_RESULT res = APDCAM_GetSampleInfo(g_handle, sampleCounts, sampleIndices);
    if (res != ADT_OK) return -1;

    bool saveChannel = false;

    short* buffers[32];
    res = APDCAM_GetBuffers(g_handle, buffers);
    if (res != ADT_OK) return -1;

    for (int i = 0; i < 32; i++)
    {
        if(i<8)
            if(channelMask1 & (int)pow(2,i))
                saveChannel = true;
            else
                saveChannel = false;
        else if(i<16)
            if(channelMask2 & (int)pow(2,i-8))
                saveChannel = true;
            else
                saveChannel = false;
        else if(i<24)
            if(channelMask3 & (int)pow(2,i-16))
                saveChannel = true;
            else
                saveChannel = false;
        else if(i<32)
            if(channelMask4 & (int)pow(2,i-24))
                saveChannel = true;
            else
                saveChannel = false;

        if(saveChannel)
        {
            char fileName[512];
            sprintf(fileName, "Channel%02d.dat",i);

            ofstream dataFile(fileName, ios::out | ios::binary);

            dataFile.write(reinterpret_cast<char*>(buffers[i]), g_sampleCount*sizeof(short));

            dataFile.close();

        }//if

    }//for


	return 0;
}

int DataMode(int mode)
{
	ADT_RESULT res = APDCAM_DataMode(g_handle, mode);
	if (res != ADT_OK) return -1;
	return 0;
}

int Filter(int *coeffs)
{
	FILTER_COEFFICIENTS filterCoefficients;
	filterCoefficients.FIR[0] = coeffs[0];
	filterCoefficients.FIR[1] = coeffs[1];
	filterCoefficients.FIR[2] = coeffs[2];
	filterCoefficients.FIR[3] = coeffs[3];
	filterCoefficients.FIR[4] = coeffs[4];
	filterCoefficients.RecursiveFilter = coeffs[5];
	filterCoefficients.Reserved = 0;
	filterCoefficients.FilterDevideFactor = 9;
    ADT_RESULT res = APDCAM_SetFilter(g_handle, filterCoefficients);
	if (res != ADT_OK) return -1;
	return 0;
}

int Calibrate()
{
	ADT_RESULT res = APDCAM_Calibrate(g_handle);
	if (res != ADT_OK) return -1;
	return 0;
}


int LoadTriggerInfo(char *fileName, ADT_TRIGGERINFO *trigger)
{
	FILE * triggerFile;
	if ((triggerFile = fopen(fileName,"rt")) == NULL)
	{
		for (int i=0; i<32; i++) {
			trigger[i].TriggerLevel = 0;
			trigger[i].Sensitivity = 0;
			trigger[i].Enable = 0;
		}
		printf("%s trigger file not found\n", fileName);
		return -1;
	}

	int index = 0;
	while (!feof(triggerFile) && index < 32)
	{
		char buffer[1024];
		char *p;
		ReadLine(triggerFile,buffer, 1024);
		int level = 0;
		int sensitivty = 0;
		int enable = 0;
		if (strlen(buffer) == 0) return -1;
		p = GetInt(buffer, &level);
		if (p == NULL)  return -1;
		p = GetInt(p, &sensitivty);
		if (p == NULL)  return -1;
		p = GetInt(p, &enable);

		trigger[index].TriggerLevel = level;
		trigger[index].Sensitivity = sensitivty;
		trigger[index].Enable = enable;

		index++;
	}
	fclose(triggerFile);
	if (index < 32) return -1;
	return 0;
}

int Read(int address, int subaddress, int numbytes)
{
	unsigned char data[MAX_RW_BYTES];
	numbytes = (numbytes > MAX_RW_BYTES) ? MAX_RW_BYTES : numbytes;
	if (g_handle != 0) 
	{
		ADT_RESULT res = APDCAM_ReadPDI(g_handle, (unsigned char)address, (unsigned long)subaddress, (unsigned char *)&data, numbytes);
		if (res == ADT_OK) 
		{
			printf("READ result, Address: %d, subaddress: %d, data: ", address, subaddress);
			for (int i=0; i<numbytes; i++)
			  printf("%d ",(int)data[i]);
			printf("\n");
		} else 
		{
			printf("Error in READ operation.\n");
		}
	}		
	return 0;
}

int Write(int address, int subaddress, int numbytes, int * data)
{
    unsigned char data_c[MAX_RW_BYTES];
    ADT_RESULT res;
	
	for (int i=0; i<numbytes; i++)
		data_c[i] = (unsigned char)data[i];
	if (g_handle != 0) 
	{
		res = APDCAM_WritePDI(g_handle, (unsigned char)address, (unsigned long)subaddress, data_c, numbytes);
	}
	if (res != ADT_OK) 
	{	
		printf("Error writing register(s).\n");
	} else
	{
		printf("Write success.\n");
	}
	return 0;
}

int SelfTest()
{
     ADT_RESULT res;

    res = APDCAM_SelfTest(g_handle);

    if (res != ADT_OK)
    {
        printf("SelfTest failed\n");
    }
    else
    {
        printf("SelfTest success.\n");
    }
    return res;
}

int GetNetworkConfig()
{
    ADT_RESULT res;

    NetConfStr netConf;

    res = APDCAM_GetNetworkConfig(g_handle, &netConf);

    if (res != ADT_OK)
    {
        return res;
    }

    if(netConf.linkSpeed!=0)
        cout << "Network speed(Mbps): " << netConf.linkSpeed << endl;
    else
        cout << "Network speed unknown " << endl;

    return res;
}

int GetFactoryConfig()
{
    ADT_RESULT res;

    APDFactory apdFactory;

    res = APDCAM_GetFactoryConfig(g_handle, &apdFactory);

    if (res != ADT_OK)
    {
        return res;
    }


    return res;
}

int SetHV(int voltage, int state)
{
    ADT_RESULT res;

    APDInfo apdInfo;

    res = APDCAM_GetInfo(g_handle, &apdInfo);

    if (res != ADT_OK)
    {
        return res;
    }

    APDFactory apdFactory;

    res = APDCAM_GetFactoryConfig(g_handle,&apdFactory);

    if(apdFactory.productCode[0]=='A' && apdFactory.productCode[1]=='P')
    {
        if(voltage > apdInfo.hv1Max)
        {
            voltage=apdInfo.hv1Max;
            cout << "Voltage higher than max. Settings max voltage: " << voltage << endl;
        }

    }
    else if(apdFactory.productCode[0]=='M' && apdFactory.productCode[1]=='P')
    {
        if(voltage > apdInfo.hv2Max)
        {
            voltage=apdInfo.hv2Max;
            cout << "Voltage higher than max. Settings max voltage: " << voltage << endl;
        }
    }


    res = APDCAM_SetHV(g_handle, (double)voltage, (double)0, state);

    if (res != ADT_OK)
    {
        cout << "SetHV error:  " << res << endl;
        return res;
    }

    return res;

}//SetHV

int GetInfo()
{
    ADT_RESULT res;

    APDInfo apdInfo;

    res = APDCAM_GetInfo(g_handle, &apdInfo);

    if (res != ADT_OK)
    {
        return res;
    }

    cout << "APDCAM infos:\n" << endl;
    cout << "ADC Board version " << (int)apdInfo.adcBoardVersion << endl;
    cout << "ADC MC version " << (int)apdInfo.adcMCVersion << endl;
    cout << "ADC Serial " << (int)apdInfo.adcSerial << endl;
    cout << "ADC FPGA version " << (int)apdInfo.fpgaVersion << endl;

    cout << "\nPC Board version " << (int)apdInfo.pcBoardVersion << endl;
    cout << "\nPC FW version " << (int)apdInfo.pcFWVersion << endl;
    cout << "\nHV1 Max " << (int)apdInfo.hv1Max << endl;
    cout << "\nHV2 Max " << (int)apdInfo.hv2Max << endl;


    return res;

}//GetInfo


int ProcessLine(char *buffer)
{
    char token[32]={0};
//usleep(1000);
//printf("Line: %s\n",buffer);
printf("\n");
	buffer = GetToken(buffer, token);

	for (int i=0;  token[i] != '\0'; i++) token[i] = toupper(token[i]);
	if (strcmp("PAUSE", token) == 0)
	{
		printf("Press enter to continue...\n");
		getchar();
	}
	else if (strcmp("REM", token) == 0)
	{
	}
	else if (strcmp("MESSAGE", token) == 0)
	{
		printf("%s\n", buffer);
	}
	else if (strcmp("SWOPTIONS", token) == 0)
	{
		SwOptions();
	}
	else if (strcmp("OPEN", token) == 0)
	{
		char param0[32];
		buffer = GetString(buffer, param0);
		if (strlen(param0) == 0) return -1;

		int res = Open(param0);
		if (res == 0)
			printf("Open succes\n");
		else
			printf("Open failed\n");

		return res; 
	}
    else if (strcmp("FORCEOPEN", token) == 0)
    {
        char param0[32];
        buffer = GetString(buffer, param0);
        if (strlen(param0) == 0) return -1;

        int res = ForceOpen(param0);
        if (res == 0)
            printf("Open succes\n");
        else
            printf("Open failed\n");

        return res;
    }
	else if (strcmp("CLOSE", token) == 0)
	{
		int res = Close();
		if (res == 0)
			printf("Close succes\n");
		else
			printf("Close failed\n");

		return res; 
	}
	else if (strcmp("SETTIMING", token) == 0)
	{
		int adcMult;
		int adcDiv;
		int strMult;
		int strDiv;
		int clkSrc = 0;
		int clkMult = -1;
		int clkDiv = -1;

		buffer = GetInt(buffer, &adcMult);
		buffer = GetInt(buffer, &adcDiv);
		buffer = GetInt(buffer, &strMult);
		buffer = GetInt(buffer, &strDiv);
		if (strlen(buffer)) buffer = GetInt(buffer, &clkSrc);
		if (strlen(buffer)) buffer = GetInt(buffer, &clkMult);
		if (strlen(buffer)) buffer = GetInt(buffer, &clkDiv);

		int res = SetTiming(adcMult, adcDiv, strMult, strDiv, clkSrc, clkMult, clkDiv);
		if (res == 0)
			printf("SetTiming succes\n");
		else
			printf("SetTiming failed\n");

		return res;
	}
	else if (strcmp("SAMPLING", token) == 0)
	{
		int sampleDiv;
		int sampleSrc;
		buffer = GetInt(buffer, &sampleDiv);
		buffer = GetInt(buffer, &sampleSrc);
		int res = Sampling(sampleDiv, sampleSrc);
		if (res == 0)
			printf("Sampling succes\n");
		else
			printf("Sampling failed\n");

		return res;
	}
	else if (strcmp("ALLOCATE", token) == 0)
	{
		int sampleCount;
		int bits;
		int channelMask_1;
		int channelMask_2;
		int channelMask_3;
		int channelMask_4;
		int primaryBufferSize = 10;

		buffer = GetInt(buffer, &sampleCount);
		buffer = GetInt(buffer, &bits);
		buffer = GetInt(buffer, &channelMask_1);
		buffer = GetInt(buffer, &channelMask_2);
		buffer = GetInt(buffer, &channelMask_3);
		buffer = GetInt(buffer, &channelMask_4);
		GetInt(buffer, &primaryBufferSize);

		int res = Allocate(sampleCount, bits, channelMask_1, channelMask_2, channelMask_3, channelMask_4, primaryBufferSize);
		if (res == 0)
			printf("Allocate succes\n");
		else
			printf("Allocate failed\n");
	}
	else if (strcmp("ARM", token) == 0)
	{
		int measurementMode;
		int sampleCount;
		int calibrationMode;
		int signalFrequency = 100;

		buffer = GetInt(buffer, &measurementMode);
		buffer = GetInt(buffer, &sampleCount);
		buffer = GetInt(buffer, &calibrationMode);
		GetInt(buffer, &signalFrequency);

		int res = ARM(measurementMode, sampleCount, calibrationMode, signalFrequency);
		if (res == 0)
			printf("Arm succes\n");
		else
			printf("Arm failed\n");
	}
	else if (strcmp("TRIGGER", token) == 0)
	{
		int triggerSource;
		int triggerMode;
		int triggerEdge;
		int delay;
		char triggerFileName[512];
        int preTriggerSampleCount;
		ADT_TRIGGERINFO trigger[32];
		buffer = GetInt(buffer, &triggerSource);
		buffer = GetInt(buffer, &triggerMode);
		buffer = GetInt(buffer, &triggerEdge);
		buffer = GetInt(buffer, &delay);
		buffer = GetString(buffer, triggerFileName);
        buffer = GetInt(buffer, &preTriggerSampleCount);
		
		LoadTriggerInfo(triggerFileName, trigger);

        int res = Trigger(triggerSource, triggerMode, triggerEdge, delay, trigger, preTriggerSampleCount);
		if (res == 0)
			printf("Trigger succes\n");
		else
			printf("Trigger failed\n");
	}
	else if (strcmp("START", token) == 0)
	{
		int res = Start();
		if (res == 0)
			printf("Start succes\n");
		else
			printf("Start failed\n");
	}
    else if (strcmp("START-LOG", token) == 0)
    {
        measLog.open("APDTest.log",ios_base::out | ios_base::trunc);
        int res = Start();
        if (res == 0)
        {
            printf("Start succes\n");
            measLog << "Start success" ;
        }
        else
        {
            printf("Start failed\n");
            measLog << "Start failed " << res;
        }
        measLog.close();
    }
	else if (strcmp("WAIT", token) == 0)
	{
		int timeout = -1;
		if (buffer) buffer = GetInt(buffer, &timeout);

		int res = Wait(timeout);
		if (res == 0)
			printf("Wait succes\n");
		else
			printf("Wait time out\n");
	}
	else if (strcmp("STOP", token) == 0)
	{
		int res = Stop();
		if (res == 0)
			printf("Stop succes\n");
		else
			printf("Stop failed\n");
	}
	else if (strcmp("SAVE", token) == 0)
	{
        int res = Save();
		if (res == 0)
			printf("Save succes\n");
		else
			printf("Save failed\n");
	}
	else if (strcmp("DATAMODE", token) == 0)
	{
		int mode = 0;
		if (strlen(buffer)) buffer = GetInt(buffer, &mode);
		int res = DataMode(mode);
		if (res == 0)
			printf("DataMode succes\n");
		else
			printf("DatMode failed\n");
	}
	else if (strcmp("FILTER", token) == 0)
	{
		int coeffs[6] = {0};
		for (int i = 0; i < 6; i++)
		{
			if (strlen(buffer)) 
				buffer = GetInt(buffer, &coeffs[i]);
			else
			{
				break;
			}
		}
		int res = Filter(coeffs);
		if (res == 0)
			printf("Filter setting succes\n");
		else
			printf("Filter setting failed\n");
	}
	else if (strcmp("CALIBRATE", token) == 0)
	{
		int res = Calibrate();
		if (res == 0)
			printf("Calibration succes\n");
		else
			printf("Calibration failed\n");
	}
	else if (strcmp("READ", token) == 0)
	{
		int address;
		int subaddress;
		int numbytes;
		buffer = GetInt(buffer, &address);
		buffer = GetInt(buffer, &subaddress);
		buffer = GetInt(buffer, &numbytes);
		Read(address, subaddress, numbytes);
	}
	else if (strcmp("WRITE", token) == 0)
	{
		int address;
		int subaddress;
		int data[MAX_RW_BYTES];
		int numbytes;

		buffer = GetInt(buffer, &address);
		buffer = GetInt(buffer, &subaddress);
		buffer = GetInt(buffer, &numbytes);
		for (int iloc=0;(iloc<numbytes) & (iloc<MAX_RW_BYTES); iloc++)
		buffer = GetInt(buffer, &(data[iloc]));
		Write(address, subaddress, numbytes, data);
	}
    else if (strcmp("SELFTEST", token) == 0)
    {
        int res = SelfTest();
        if (res == 0)
            printf("SelfTest succes\n");
        else
            printf("SelfTest failed\n");
    }
    else if (strcmp("GETNETCONF", token) == 0)
    {
        int res = GetNetworkConfig();
        if (res == 0)
            printf("GetNetworkConfig success\n");
        else
            printf("GetNetworkConfig failed\n");
    }
    else if (strcmp("GETFACTCONF", token) == 0)
    {
        int res = GetFactoryConfig();
        if (res == 0)
            printf("GetFactoryConfig success\n");
        else
            printf("GetFactoryConfig failed\n");
    }
    else if (strcmp("GETINFO", token) == 0)
    {
        int res = GetInfo();
        if (res == 0)
            printf("GetInfo success\n");
        else
            printf("GetInfo failed\n");
    }
    else if (strcmp("SETHV", token) == 0)
    {
        int voltage=0;
        int state=0;
        buffer = GetInt(buffer, &voltage);
        buffer = GetInt(buffer, &state);

        int res = SetHV(voltage,state);
        if (res == 0)
            printf("SetHV success\n");
        else
            printf("SetHV failed\n");
    }
    else if (token[0] == -1)
    {
        //cout << "Zero line!\n" << endl;
    }
    else
    {
        cout << "Unknown command!\n" << endl;
    }


	return 0;
}

int ReadLine(FILE * f, char * buffer, int size)
{
	int i;
	for (i=0; (i<size-1) && !feof(f); i++)
	{
		buffer[i] = fgetc(f);
		if ((buffer[i] == '\n') || (buffer[i] == '\r'))
		{
			buffer[i] = '\0';
			 break;
		}
	}
	buffer[i] = '\0';
return 0;
}
