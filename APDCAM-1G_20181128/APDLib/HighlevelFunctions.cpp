#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#ifdef WINDOWS
 #include "Windows.h"
#endif

#ifdef APD_VERSION
#include "APDLib.h"
#endif

#ifdef ADC_VERSION
#include "ADCLib.h"
#endif


#include "TypeDefs.h"
#include "InterfaceDefs.h"
#include "InternalFunctions.h"
//#include "LowlevelFunctions.h"
//#include "LnxClasses.h"
#include "Helpers.h"
#include "DataEvaluation.h"
//#include "Math.h"
//#include "helper.h"

#define LIBVERSION_MAJOR 1
#define LIBVERSION_MINOR 7

#define SLOTNUMBER 32
#define MAX_PREAMBULE_LENGTH 20
#define DEF_PACKETSIZE 1440
#define STREAM_PORT_BASE_1 57000
#define STREAM_PORT_BASE_2 57001
#define STREAM_PORT_BASE_3 57002
#define STREAM_PORT_BASE_4 57003
#define PAGESIZE 4096

// Forward definitions
// The APDCAM_Sampling & APDCAM_Allocate must be called before use Measure_NonCalibrated.
// It starts the measurement and waits the end of it.
#ifdef APD_VERSION
ADT_RESULT Measure_NonCalibrated(ADT_HANDLE handle, long long sampleCount, int signalFrequency = 100);
#endif

union INTERNAL_HANDLE
{
	struct
	{
		unsigned int index : 8;
		unsigned int magic_number : 24;
	};
	unsigned int handle;

	INTERNAL_HANDLE()
	{
		handle = 0;
	}

	INTERNAL_HANDLE(ADT_HANDLE ah)
	{
		handle = ah;
	}

	INTERNAL_HANDLE(unsigned int i, unsigned int mn)
	{
		index = i;
		magic_number = mn;
	}

	INTERNAL_HANDLE &operator=(ADT_HANDLE ah)
	{
		handle = ah;
		return (*this);
	}

	operator ADT_HANDLE() const
	{ 
		return (ADT_HANDLE)handle;
	};
};

typedef struct tagWORKING_SET
{
	// Preliminary
	// The magic_number is A random number, generated when the slot allocated in the open operation, and cleared in the close operation.
	// When the slot is accessed with a handle, this number is compared with the magic_number part of handle to validate the it.
	INTERNAL_HANDLE handle; 

	//
	ADT_STATE state;

	CAPDClient *client;

	// Primary buffer where the server collects the incommung UDP frames.
	// Temporary buffer is for the data evaluation.
	// User buffer (8 for each channel) where the evaluated data are stored.
	// The size of non-paged memory is primeryBuffer + temporaryBuffer + 8 * userBuffer
	// user_buffer_1_i = user_buffer_1 + i * user_buffer_size_1.
	CAPDServer *stream_1_server;
	CNPMAllocator *np_memory_1;
	CEvent *dataNotification_1; // Set by the server to notify the data processor to start data evaluation;
	CEvent *userNotification_1;	// Set by the data processor to notify user
	CDataEvaluation *eval_1;
	unsigned char* primary_buffer_1;
    unsigned long long primary_buffer_size_1;
	unsigned char* temp_buffer_1;
    unsigned long long temp_buffer_size_1;
	unsigned char* user_buffer_1;
    unsigned long long user_buffer_size_1;

	CAPDServer *stream_2_server;
	CNPMAllocator *np_memory_2;
	CEvent *dataNotification_2; // Set by the server to notify the data processor to start data evaluation;
	CEvent *userNotification_2;	// Set by the data processor to notify user
	CDataEvaluation *eval_2;
	unsigned char* primary_buffer_2;
    unsigned long long primary_buffer_size_2;
	unsigned char* temp_buffer_2;
    unsigned long long temp_buffer_size_2;
	unsigned char* user_buffer_2;
    unsigned long long user_buffer_size_2;

	CAPDServer *stream_3_server;
	CNPMAllocator *np_memory_3;
	CEvent *dataNotification_3; // Set by the server to notify the data processor to start data evaluation;
	CEvent *userNotification_3;	// Set by the data processor to notify user
	CDataEvaluation *eval_3;
	unsigned char* primary_buffer_3;
    unsigned long long primary_buffer_size_3;
	unsigned char* temp_buffer_3;
    unsigned long long temp_buffer_size_3;
	unsigned char* user_buffer_3;
    unsigned long long user_buffer_size_3;

	CAPDServer *stream_4_server;
	CNPMAllocator *np_memory_4;
	CEvent *dataNotification_4; // Set by the server to notify the data processor to start data evaluation;
	CEvent *userNotification_4;	// Set by the data processor to notify user
	CDataEvaluation *eval_4;
	unsigned char* primary_buffer_4;
    unsigned long long primary_buffer_size_4;
	unsigned char* temp_buffer_4;
    unsigned long long temp_buffer_size_4;
	unsigned char* user_buffer_4;
    unsigned long long user_buffer_size_4;

	CWaitForEvents* waitObject; // User notifiaction objects. Objects set by the data evaluators.
	CTriggerManager *triggerManager; // Used to synchronize sw triggers.

	unsigned int ip_h; // ip in host format
	unsigned short command_port_h; // port number for commands
	unsigned short stream_1_port_h; // port number for 1th stream in host format.
	unsigned short stream_2_port_h; // port number for 2nd stream in host format.
	unsigned short stream_3_port_h; // port number for 3rd stream in host format.
	unsigned short stream_4_port_h; // port number for 4th stream in host format.

	// The size of user data in an UDP packet (without CW_FRAME)
	unsigned int packetsize_1; // default = 1440
	unsigned int packetsize_2; // default = 1440
	unsigned int packetsize_3; // default = 1440
	unsigned int packetsize_4; // default = 1440

	unsigned int bufferSizeInSampleNo;	// the size of buffers in samples. The real size of a buffer depends on the data type, stored in that buffer.
	int bits;
	unsigned char channelMask_1;
	unsigned char channelMask_2;
	unsigned char channelMask_3;
	unsigned char channelMask_4;
	// Requested data to arrive, without CW_FARME (Preamble and measurement data) - in one shot mode.
	unsigned int requestedData_1;
	unsigned int requestedData_2;
	unsigned int requestedData_3;
	unsigned int requestedData_4;
	bool setupComplete;

	unsigned int sampleCount;	// Number of samples, required after start or trigger;

	unsigned char adcMult;
	unsigned char adcDiv;
	unsigned char strMult;
	unsigned char strDiv;
	unsigned short sampleDiv;
	unsigned char clkMult;
	unsigned char clkDiv;
	int clkSource;


} WORKING_SET;

WORKING_SET g_WorkingSets[SLOTNUMBER];

CAPDFactory* CAPDFactory::g_pFactory;


int GetIndex(ADT_HANDLE handle)
{
	INTERNAL_HANDLE h = handle;
	int index = h.index;

	if (index >= SLOTNUMBER) return -1;
	if (g_WorkingSets[index].handle != h) return -1;

	return index;
}
#ifdef APD_VERSION
void APDCAM_Init()
#else
void ADC_Init()
#endif
{
	for (int i = 0; i < SLOTNUMBER; i++)
	{
		g_WorkingSets[i].handle = 0;
		g_WorkingSets[i].np_memory_1 = NULL;
		g_WorkingSets[i].np_memory_2 = NULL;
		g_WorkingSets[i].np_memory_3 = NULL;
		g_WorkingSets[i].np_memory_4 = NULL;
	}

	CAPDFactory::SetAPDFactory(new CLnxFactory());
}

#ifdef APD_VERSION
void APDCAM_Done()
#else
void ADC_Done()
#endif
{
	for (int i = 0; i < SLOTNUMBER; i++)
	{
		if (g_WorkingSets[i].handle != 0)
		{
#ifdef APD_VERSION
            APDCAM_Close(g_WorkingSets[i].handle);
#else
            ADC_Close(g_WorkingSets[i].handle);
#endif
		}
	}
	delete CAPDFactory::GetAPDFactory();
}


#ifdef APD_VERSION
void APDCAM_GetSWOptions()
#else
void ADC_GetSWOptions()
#endif
{
    printf("************* Software info *************\n");

#ifdef APD_VERSION
    printf("Library: APDLib\n");
#else
    printf("Library: ADCLib\n");
#endif
    printf("Version: %d.%d\n", LIBVERSION_MAJOR, LIBVERSION_MINOR);

#ifdef CC24
    printf("Continuity counter : 24 bit\n");
#else
    printf("Continuity counter : 8 bit\n");
#endif

    printf("********************************************\n");
}

#ifdef APD_VERSION
void APDCAM_Find(unsigned long from_ip_h, unsigned long to_ip_h, unsigned long *ip_table, int table_size, int *no_of_elements, char *filter_str, int timeout)
#else
void ADC_Find(unsigned long from_ip_h, unsigned long to_ip_h, unsigned long *ip_table, int table_size, int *no_of_elements, char *filter_str, int timeout)
#endif
{
	char serial_no[8];

	CAPDClient *client = CAPDFactory::GetAPDFactory()->GetClient();
	client->Start();

	if (ip_table == NULL) return;
	if (table_size == 0) return;
	if (no_of_elements == NULL) return;

	*no_of_elements = 0;
	for (unsigned long ip_h = from_ip_h; ip_h <= to_ip_h; ip_h++)
	{
		/*
		LARGE_INTEGER performanceCount1, performanceCount2;
		QueryPerformanceCounter(&performanceCount1);
		*/
		unsigned char boardVersion;
		bool res = GetADCBoardVersion(client, &boardVersion, ip_h, 0, timeout);
		if (res)
		{
			boardVersion = (boardVersion >> 5) & 0x07;
			switch (boardVersion)
			{
			case ADC_BOARD: // Must be ADC board
				{
					res = RetrieveADCSerialNo(client, serial_no, 8, ip_h);
					if (!res) break;
					if (Filter_6(filter_str, serial_no))
					{
						if (*no_of_elements < table_size)
						{
							*ip_table = ip_h;
							ip_table++;
							(*no_of_elements)++;
						}
						//						printf("Board version: %d\n", boardVersion);
					}
				}
				break;
			default:
				break;
			}
		}
		/*
		QueryPerformanceCounter(&performanceCount2);
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		double duration = ((double)(performanceCount2.QuadPart - performanceCount1.QuadPart)) / ((double)frequency.QuadPart);
		printf("Duration: %g ms \n", 1000*duration);
		*/
	}

	client->Stop();

}

#ifdef APD_VERSION
ADT_HANDLE APDCAM_Open(unsigned long ip_h, unsigned char openMode)
#else
ADT_HANDLE ADC_Open(unsigned long ip_h, unsigned char openMode)
#endif
{
	int slotNumber;
	for (slotNumber = 0; slotNumber < SLOTNUMBER; slotNumber++)
	{
		if (g_WorkingSets[slotNumber].handle == 0) break;
	}
	if (slotNumber >= SLOTNUMBER) return 0;

	WORKING_SET &WorkingSet = g_WorkingSets[slotNumber];

	WorkingSet.handle.index = slotNumber;
	WorkingSet.handle.magic_number = rand() + 1;

	WorkingSet.ip_h = ip_h;

	WorkingSet.state = AS_ERROR;

	WorkingSet.packetsize_1 = DEF_PACKETSIZE;
	WorkingSet.packetsize_2 = DEF_PACKETSIZE;
	WorkingSet.packetsize_3 = DEF_PACKETSIZE;
	WorkingSet.packetsize_4 = DEF_PACKETSIZE;

	WorkingSet.stream_1_port_h = STREAM_PORT_BASE_1 + 4*slotNumber; // port number for 1th stream in host format.
	WorkingSet.stream_2_port_h = STREAM_PORT_BASE_2 + 4*slotNumber; // port number for 2nd stream in host format.
	WorkingSet.stream_3_port_h = STREAM_PORT_BASE_3 + 4*slotNumber; // port number for 3rd stream in host format.
	WorkingSet.stream_4_port_h = STREAM_PORT_BASE_4 + 4*slotNumber; // port number for 4th stream in host format.

	WorkingSet.setupComplete = false;

	WorkingSet.client = CAPDFactory::GetAPDFactory()->GetClient();
	WorkingSet.client->SetIPAddress(WorkingSet.ip_h);
	WorkingSet.client->Start();

	WorkingSet.triggerManager = new CLnxTriggerManager();

	WorkingSet.stream_1_server = CAPDFactory::GetAPDFactory()->GetServer();
	WorkingSet.dataNotification_1 = CAPDFactory::GetAPDFactory()->GetEvent();
	WorkingSet.userNotification_1 = CAPDFactory::GetAPDFactory()->GetEvent();
    WorkingSet.eval_1 = new CLnxDataEvaluation();
	WorkingSet.eval_1->SetServer(WorkingSet.stream_1_server);
	WorkingSet.eval_1->SetDataNotificationSignal(WorkingSet.dataNotification_1);
	WorkingSet.eval_1->SetUserNotificationSignal(WorkingSet.userNotification_1);
	WorkingSet.triggerManager->Add(WorkingSet.eval_1);

	WorkingSet.stream_2_server = CAPDFactory::GetAPDFactory()->GetServer();
	WorkingSet.dataNotification_2 = CAPDFactory::GetAPDFactory()->GetEvent();
	WorkingSet.userNotification_2 = CAPDFactory::GetAPDFactory()->GetEvent();
	WorkingSet.eval_2 = new CLnxDataEvaluation();
	WorkingSet.eval_2->SetServer(WorkingSet.stream_2_server);
	WorkingSet.eval_2->SetDataNotificationSignal(WorkingSet.dataNotification_2);
	WorkingSet.eval_2->SetUserNotificationSignal(WorkingSet.userNotification_2);
	WorkingSet.triggerManager->Add(WorkingSet.eval_2);
	
	WorkingSet.stream_3_server = CAPDFactory::GetAPDFactory()->GetServer();
	WorkingSet.dataNotification_3 = CAPDFactory::GetAPDFactory()->GetEvent();
	WorkingSet.userNotification_3 = CAPDFactory::GetAPDFactory()->GetEvent();
	WorkingSet.eval_3 = new CLnxDataEvaluation();
	WorkingSet.eval_3->SetServer(WorkingSet.stream_3_server);
	WorkingSet.eval_3->SetDataNotificationSignal(WorkingSet.dataNotification_3);
	WorkingSet.eval_3->SetUserNotificationSignal(WorkingSet.userNotification_3);
	WorkingSet.triggerManager->Add(WorkingSet.eval_3);
	
	WorkingSet.stream_4_server = CAPDFactory::GetAPDFactory()->GetServer();
	WorkingSet.dataNotification_4 = CAPDFactory::GetAPDFactory()->GetEvent();
	WorkingSet.userNotification_4 = CAPDFactory::GetAPDFactory()->GetEvent();
	WorkingSet.eval_4 = new CLnxDataEvaluation();
	WorkingSet.eval_4->SetServer(WorkingSet.stream_4_server);
	WorkingSet.eval_4->SetDataNotificationSignal(WorkingSet.dataNotification_4);
	WorkingSet.eval_4->SetUserNotificationSignal(WorkingSet.userNotification_4);
	WorkingSet.triggerManager->Add(WorkingSet.eval_4);

	WorkingSet.waitObject = CAPDFactory::GetAPDFactory()->GetWaitForEvents();

	bool initRes;
	if(openMode==1)
	{
        /*initRes = GetChannels(WorkingSet.client, &WorkingSet.channelMask_1, &WorkingSet.channelMask_2, &WorkingSet.channelMask_3, &WorkingSet.channelMask_4);

		if(!initRes)
            return 0;*/

	}//if
	else if(openMode==0)
	{
		initRes = GetChannels(WorkingSet.client, &WorkingSet.channelMask_1, &WorkingSet.channelMask_2, &WorkingSet.channelMask_3, &WorkingSet.channelMask_4);

		if(!initRes)
		{
			return 0;

		}//if
		else
		{
			unsigned char adcBoardVersion=0;
			bool ret = GetADCBoardVersion(WorkingSet.client, &adcBoardVersion, ip_h, 0);
			if(ret)
			{
				unsigned char boardId = (unsigned char) (adcBoardVersion & 0x20) >> 5;
				if(boardId!=1)
				{
					return 1;	//No ADC Board found

				}//if

			}//if
			else
			{
				return 0;   //Register reading error

			}//else

			unsigned char controlBoardVersion=0;
			ret = GetControlBoardVersion(WorkingSet.client, &controlBoardVersion);
			if(ret)
			{
				unsigned char boardId = (unsigned char) (controlBoardVersion & 0x40) >> 5;
				if(boardId!=2)
				{
					return 2;	//No Control Board found

				}//if

			}//if
			else
			{
				return 0;   //Register reading error

			}//else

            //Checking factory table
            APDFactory apdFactory;

            ret = GetFactoryData(WorkingSet.client, &apdFactory);

            if(!ret)
                return 0;

            if((apdFactory.productCode[0]!='A' && apdFactory.productCode[1]!='P') || (apdFactory.productCode[0]!='M' && apdFactory.productCode[1]!='P'))
            {
                return 3;
            }

            if(apdFactory.adcDataStatus!=1 && apdFactory.adcDataStatus!=2)
            {
                return 3;
            }


            if(apdFactory.pcDataStatus!=1 && apdFactory.pcDataStatus!=2)
            {
                return 3;
            }


		}//else


	}//else if

    initRes = GetResolution(WorkingSet.client, &WorkingSet.bits);
    initRes = GetSampleCount(WorkingSet.client, &WorkingSet.bufferSizeInSampleNo);
	WorkingSet.sampleCount = WorkingSet.bufferSizeInSampleNo;

    initRes = GetADCPLL(WorkingSet.client, &WorkingSet.adcMult, &WorkingSet.adcDiv);
    initRes = GetStreamPLL(WorkingSet.client, &WorkingSet.strMult, &WorkingSet.strDiv);

    unsigned short sampleDiv = 0;

    initRes = GetSampleDiv(WorkingSet.client, &sampleDiv);

    WorkingSet.sampleDiv=sampleDiv/7;

	if (initRes)
	{
		WorkingSet.state = AS_STANDBY;
	}
    else
    {
        WorkingSet.state = AS_ERROR;
        WorkingSet.handle=0;
    }


	return WorkingSet.handle;

}//APDCAM_Open

#ifdef APD_VERSION
ADT_RESULT APDCAM_Close(ADT_HANDLE handle)
#else
ADT_RESULT ADC_Close(ADT_HANDLE handle)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (WorkingSet.client) delete WorkingSet.client;
	WorkingSet.client = NULL;

	if (WorkingSet.waitObject)
	{
		WorkingSet.waitObject->RemoveAll();
		delete WorkingSet.waitObject;
	}
	WorkingSet.waitObject = 0;

	if (WorkingSet.triggerManager)
	{
		WorkingSet.triggerManager->RemoveAll();
		delete WorkingSet.triggerManager;
	}
	WorkingSet.triggerManager = NULL;

	if (WorkingSet.stream_1_server) delete WorkingSet.stream_1_server;
	WorkingSet.stream_1_server = NULL;
	if (WorkingSet.dataNotification_1) delete WorkingSet.dataNotification_1;
	WorkingSet.dataNotification_1 = NULL;
	if (WorkingSet.userNotification_1) delete WorkingSet.userNotification_1;
	WorkingSet.userNotification_1 = NULL;
	if (WorkingSet.eval_1) delete WorkingSet.eval_1;
	WorkingSet.eval_1 = NULL;

	if (WorkingSet.stream_2_server) delete WorkingSet.stream_2_server;
	WorkingSet.stream_2_server = NULL;
	if (WorkingSet.dataNotification_2) delete WorkingSet.dataNotification_2;
	WorkingSet.dataNotification_2 = NULL;
	if (WorkingSet.userNotification_2) delete WorkingSet.userNotification_2;
	WorkingSet.userNotification_2 = NULL;
	if (WorkingSet.eval_2) delete WorkingSet.eval_2;
	WorkingSet.eval_2 = NULL;

	if (WorkingSet.stream_3_server) delete WorkingSet.stream_3_server;
	WorkingSet.stream_3_server = NULL;
	if (WorkingSet.dataNotification_3) delete WorkingSet.dataNotification_3;
	WorkingSet.dataNotification_3 = NULL;
	if (WorkingSet.userNotification_3) delete WorkingSet.userNotification_3;
	WorkingSet.userNotification_3 = NULL;
	if (WorkingSet.eval_3) delete WorkingSet.eval_3;
	WorkingSet.eval_3 = NULL;

	if (WorkingSet.stream_4_server) delete WorkingSet.stream_4_server;
	WorkingSet.stream_4_server = NULL;
	if (WorkingSet.dataNotification_4) delete WorkingSet.dataNotification_4;
	WorkingSet.dataNotification_4 = NULL;
	if (WorkingSet.userNotification_4) delete WorkingSet.userNotification_4;
	WorkingSet.userNotification_4 = NULL;
	if (WorkingSet.eval_4) delete WorkingSet.eval_4;
	WorkingSet.eval_4 = NULL;

	if (WorkingSet.np_memory_1) delete WorkingSet.np_memory_1;
	WorkingSet.np_memory_1 = NULL;
	if (WorkingSet.np_memory_2) delete WorkingSet.np_memory_2;
	WorkingSet.np_memory_2 = NULL;
	if (WorkingSet.np_memory_3) delete WorkingSet.np_memory_3;
	WorkingSet.np_memory_3 = NULL;
	if (WorkingSet.np_memory_4) delete WorkingSet.np_memory_4;
	WorkingSet.np_memory_4 = NULL;

	WorkingSet.handle = 0;

	return ADT_OK;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_SelfTest(ADT_HANDLE handle)
#else
ADT_RESULT ADC_SelfTest(ADT_HANDLE handle)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	ADT_RESULT res = ADT_OK;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	/* Test channel setting routines */
	unsigned char ch1 = 0xFF;
	unsigned char ch2 = 0xFF;
	unsigned char ch3 = 0xFF;
	unsigned char ch4 = 0xFF;


	//Checking that ADC and Control board is present or not
	unsigned char adcBoardVersion=0;
	bool ret = GetADCBoardVersion(WorkingSet.client, &adcBoardVersion, WorkingSet.ip_h, 0);
	if(ret)
	{
		unsigned char boardId = (unsigned char) (adcBoardVersion & 0x20) >> 5;
		if(boardId!=1)
		{
			return ADT_ERROR;	//No ADC Board found

		}//if

	}//if
	else
	{
		return ADT_ERROR;   //Register reading error

	}//else

	unsigned char controlBoardVersion=0;
	ret = GetControlBoardVersion(WorkingSet.client, &controlBoardVersion);
	if(ret)
	{
		unsigned char boardId = (unsigned char) (controlBoardVersion & 0x40) >> 5;
		if(boardId!=2)
		{
			return ADT_ERROR;	//No Control Board found

		}//if

	}//if
	else
	{
		return ADT_ERROR;   //Register reading error

	}//else

    //Checking factory table
    APDFactory apdFactory;

    ret = GetFactoryData(WorkingSet.client, &apdFactory);

    if(!ret)
        return ADT_FACTORY_SETUP_ERROR;

    if(apdFactory.productCode[0]!='A' || apdFactory.productCode[1]!='P')
    {
        return ADT_FACTORY_SETUP_ERROR;
    }

    if(apdFactory.adcDataStatus!=1 || apdFactory.adcDataStatus!=2)
    {
        return ADT_FACTORY_SETUP_ERROR;
    }

    if(apdFactory.pcDataStatus!=1 || apdFactory.pcDataStatus!=2)
    {
        return ADT_FACTORY_SETUP_ERROR;
    }

	//Reading and comparing temperatures
	ADT_SYSTEM_STATUS *systemStatus= {0};

	bool b = GetAllTempSensors(WorkingSet.client, systemStatus->Temperatures);
	if (!b)
	{
		return ADT_ERROR;
	}

	if(systemStatus->Temperatures[0] > 80)
		return ADT_ERROR;
	if(systemStatus->Temperatures[1] > 80)
		return ADT_ERROR;
	if(systemStatus->Temperatures[2] > 80)
		return ADT_ERROR;
	if(systemStatus->Temperatures[3] > 80)
		return ADT_ERROR;
	if(systemStatus->Temperatures[7] > 80)
		return ADT_ERROR;
	if(systemStatus->Temperatures[8] > 80)
		return ADT_ERROR;
	if(systemStatus->Temperatures[14] > 80)
		return ADT_ERROR;

	ADT_STATUS_1 status1;
	if (GetStatus1(WorkingSet.client, &status1.status))
	{
		if (!status1.ADC_PLL_Locked) return ADT_ERROR;
		
	}
	else
	{
		return ADT_ERROR;
	}

	unsigned char error=0;

	if(!GetADCBoardError(WorkingSet.client, &error))
		return ADT_ERROR;

	if(error!=0)
		cout << "ADC board error code: " << error << endl;

	if(!GetControlBoardError(WorkingSet.client, &error))
		return ADT_ERROR;

	if(error!=0)
		cout << "Control board error code: " << error << endl;

	unsigned short hv1;

	if(!GetHV1Max(WorkingSet.client, &hv1))
		return ADT_ERROR;

	if(hv1==0)
		return ADT_FACTORY_SETUP_ERROR;


	unsigned short hv2;

	if(!GetHV2Max(WorkingSet.client, &hv2))
		return ADT_ERROR;

	if(hv2==0)
		return ADT_FACTORY_SETUP_ERROR;

	
	return res;

}//SelfTest


#ifdef APD_VERSION
ADT_RESULT APDCAM_WritePDI(ADT_HANDLE handle, unsigned char address, unsigned long subaddress, unsigned char* buffer, int noofbytes)
#else
ADT_RESULT ADC_WritePDI(ADT_HANDLE handle, unsigned char address, unsigned long subaddress, unsigned char* buffer, int noofbytes)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;
	WORKING_SET &WorkingSet = g_WorkingSets[index];

    bool ret=WritePDISafe(WorkingSet.client, address, subaddress, buffer, noofbytes);

	if(!ret)
		return ADT_ERROR;

	return ADT_OK;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_ReadPDI(ADT_HANDLE handle, unsigned char address, unsigned long subaddress, unsigned char* buffer, int noofbytes)
#else
ADT_RESULT ADC_ReadPDI(ADT_HANDLE handle, unsigned char address, unsigned long subaddress, unsigned char* buffer, int noofbytes)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;
	WORKING_SET &WorkingSet = g_WorkingSets[index];

	bool ret=ReadPDI(WorkingSet.client, address, subaddress, buffer, noofbytes);

	if(!ret)
		return ADT_ERROR;

	return ADT_OK;
}


#ifdef APD_VERSION
ADT_RESULT APDCAM_SetupAllTS(ADT_HANDLE handle)
#else
ADT_RESULT ADC_SetupAllTS(ADT_HANDLE handle)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;
	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (SetupAllTS(WorkingSet.client, WorkingSet.packetsize_1, WorkingSet.packetsize_2, WorkingSet.packetsize_3, WorkingSet.packetsize_4))
	{
		return ADT_OK;
	}

	return ADT_ERROR;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_ShutupAllTS(ADT_HANDLE handle)
#else
ADT_RESULT ADC_ShutupAllTS(ADT_HANDLE handle)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;
	WORKING_SET &WorkingSet = g_WorkingSets[index];

	ShutupAllTS(WorkingSet.client);

	return ADT_OK;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_Allocate(ADT_HANDLE handle, long long sampleCount, int bits, int channelMask_1, int channelMask_2, int channelMask_3, int channelMask_4, int primary_buffer_size)
#else
ADT_RESULT ADC_Allocate(ADT_HANDLE handle, long long sampleCount, int bits, int channelMask_1, int channelMask_2, int channelMask_3, int channelMask_4, int primary_buffer_size)
#endif
{

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;
	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (bits != 8 && bits != 12 && bits != 14 && bits != -1)
	{
		return ADT_PARAMETER_ERROR;
	}

	if (channelMask_1 > 255 || channelMask_2 > 255 || channelMask_3 > 255 || channelMask_4 > 255)
	{
		return ADT_PARAMETER_ERROR;
	}

	/*primary_buffer_size = max(primary_buffer_size, 10);
	primary_buffer_size = min(primary_buffer_size, 100);*/

	if(primary_buffer_size < 10 || primary_buffer_size > 100)
		return ADT_PARAMETER_ERROR;

    if (sampleCount > (long long)0xFFFFFFFF || sampleCount == 0)
	{
		return ADT_PARAMETER_ERROR;
	}

	// After switching on, the sampleCount read back from the ADC board is invalid.
	if (sampleCount < 0 && WorkingSet.bufferSizeInSampleNo == 0)
	{
		return ADT_PARAMETER_ERROR;
	}

	// Delete old setup
	WorkingSet.setupComplete = false;

	if (WorkingSet.np_memory_1) delete WorkingSet.np_memory_1;
	WorkingSet.np_memory_1 = NULL;
	if (WorkingSet.np_memory_2) delete WorkingSet.np_memory_2;
	WorkingSet.np_memory_2 = NULL;
	if (WorkingSet.np_memory_3) delete WorkingSet.np_memory_3;
	WorkingSet.np_memory_3 = NULL;
	if (WorkingSet.np_memory_4) delete WorkingSet.np_memory_4;
	WorkingSet.np_memory_4 = NULL;
	WorkingSet.setupComplete = false;

	// Save new parameters
	if (sampleCount > 0) WorkingSet.bufferSizeInSampleNo = (unsigned int)sampleCount;
	if (bits > 0) WorkingSet.bits = bits;
	if (channelMask_1 >= 0) WorkingSet.channelMask_1 = channelMask_1;
	if (channelMask_2 >= 0)WorkingSet.channelMask_2 = channelMask_2;
	if (channelMask_3 >= 0)WorkingSet.channelMask_3 = channelMask_3;
	if (channelMask_4 >= 0)WorkingSet.channelMask_4 = channelMask_4;

	int channels = GetBitCount(WorkingSet.channelMask_1);
	int blockSize = GetBlockSize(channels, WorkingSet.bits);
	unsigned int framesize = WorkingSet.packetsize_1 + sizeof(CW_FRAME_V2);
	unsigned int userData = blockSize * WorkingSet.bufferSizeInSampleNo + MAX_PREAMBULE_LENGTH;
	unsigned int networkData = (unsigned int)((double)userData * (double)framesize / (double)WorkingSet.packetsize_1);
	networkData = (networkData * primary_buffer_size) / 100;
	unsigned int buffersize = (networkData / framesize + 1) * framesize;
	WorkingSet.primary_buffer_size_1 = (buffersize / PAGESIZE + 1) * PAGESIZE;
	WorkingSet.temp_buffer_size_1 = PAGESIZE;
	WorkingSet.user_buffer_size_1 = ((WorkingSet.bufferSizeInSampleNo * sizeof(unsigned short)) / PAGESIZE + 1) * PAGESIZE;
    unsigned long long size = WorkingSet.primary_buffer_size_1 + WorkingSet.temp_buffer_size_1 + 8 * WorkingSet.user_buffer_size_1;
	WorkingSet.np_memory_1 = CAPDFactory::GetAPDFactory()->GetNPMemory(size);
	WorkingSet.primary_buffer_1 = WorkingSet.np_memory_1->GetBuffer();
	WorkingSet.temp_buffer_1 = WorkingSet.primary_buffer_1 + WorkingSet.primary_buffer_size_1;
	WorkingSet.user_buffer_1 = WorkingSet.temp_buffer_1 + WorkingSet.temp_buffer_size_1;
	WorkingSet.requestedData_1 = userData;
	WorkingSet.eval_1->SetBuffers(WorkingSet.primary_buffer_1, WorkingSet.temp_buffer_1, WorkingSet.user_buffer_1, WorkingSet.user_buffer_size_1);
	WorkingSet.eval_1->SetParams(WorkingSet.bits, WorkingSet.channelMask_1, WorkingSet.packetsize_1);

	channels = GetBitCount(WorkingSet.channelMask_2);
	blockSize = GetBlockSize(channels, WorkingSet.bits);
	framesize = WorkingSet.packetsize_2 + sizeof(CW_FRAME_V2);
	userData = blockSize * WorkingSet.bufferSizeInSampleNo + MAX_PREAMBULE_LENGTH;
	networkData = (unsigned int)((double)userData * (double)framesize / (double)WorkingSet.packetsize_2);
	networkData = (networkData * primary_buffer_size) / 100;
	buffersize = (networkData / framesize + 1) * framesize;
	WorkingSet.primary_buffer_size_2 = (buffersize / PAGESIZE + 1) * PAGESIZE;
	WorkingSet.temp_buffer_size_2 = PAGESIZE;
	WorkingSet.user_buffer_size_2 = ((WorkingSet.bufferSizeInSampleNo * sizeof(unsigned short)) / PAGESIZE + 1) * PAGESIZE;
	size = WorkingSet.primary_buffer_size_2 + WorkingSet.temp_buffer_size_2 + 8 * WorkingSet.user_buffer_size_2;
	WorkingSet.np_memory_2 = CAPDFactory::GetAPDFactory()->GetNPMemory(size);
	WorkingSet.primary_buffer_2 = WorkingSet.np_memory_2->GetBuffer();
	WorkingSet.temp_buffer_2 = WorkingSet.primary_buffer_2 + WorkingSet.primary_buffer_size_2;
	WorkingSet.user_buffer_2 = WorkingSet.temp_buffer_2 + WorkingSet.temp_buffer_size_2;
	WorkingSet.requestedData_2 = userData;
	WorkingSet.eval_2->SetBuffers(WorkingSet.primary_buffer_2, WorkingSet.temp_buffer_2, WorkingSet.user_buffer_2, WorkingSet.user_buffer_size_2);
	WorkingSet.eval_2->SetParams(WorkingSet.bits, WorkingSet.channelMask_2, WorkingSet.packetsize_2);

	channels = GetBitCount(WorkingSet.channelMask_3);
	blockSize = GetBlockSize(channels, WorkingSet.bits);
	framesize = WorkingSet.packetsize_3 + sizeof(CW_FRAME_V2);
	userData = blockSize * WorkingSet.bufferSizeInSampleNo + MAX_PREAMBULE_LENGTH;
	networkData = (unsigned int)((double)userData * (double)framesize / (double)WorkingSet.packetsize_3);
	networkData = (networkData * primary_buffer_size) / 100;
	buffersize = (networkData / framesize + 1) * framesize;
	WorkingSet.primary_buffer_size_3 = (buffersize / PAGESIZE + 1) * PAGESIZE;
	WorkingSet.temp_buffer_size_3 = PAGESIZE;
	WorkingSet.user_buffer_size_3 = ((WorkingSet.bufferSizeInSampleNo * sizeof(unsigned short)) / PAGESIZE + 1) * PAGESIZE;
	size = WorkingSet.primary_buffer_size_3 + WorkingSet.temp_buffer_size_3 + 8 * WorkingSet.user_buffer_size_3;
	WorkingSet.np_memory_3 = CAPDFactory::GetAPDFactory()->GetNPMemory(size);
	WorkingSet.primary_buffer_3 = WorkingSet.np_memory_3->GetBuffer();
	WorkingSet.temp_buffer_3 = WorkingSet.primary_buffer_3 + WorkingSet.primary_buffer_size_3;
	WorkingSet.user_buffer_3 = WorkingSet.temp_buffer_3 + WorkingSet.temp_buffer_size_3;
	WorkingSet.requestedData_3 = userData;
	WorkingSet.eval_3->SetBuffers(WorkingSet.primary_buffer_3, WorkingSet.temp_buffer_3, WorkingSet.user_buffer_3, WorkingSet.user_buffer_size_3);
	WorkingSet.eval_3->SetParams(WorkingSet.bits, WorkingSet.channelMask_3, WorkingSet.packetsize_3);

	channels = GetBitCount(WorkingSet.channelMask_4);
	blockSize = GetBlockSize(channels, WorkingSet.bits);
	framesize = WorkingSet.packetsize_4 + sizeof(CW_FRAME_V2);
	userData = blockSize * WorkingSet.bufferSizeInSampleNo + MAX_PREAMBULE_LENGTH;
	networkData = (unsigned int)((double)userData * (double)framesize / (double)WorkingSet.packetsize_4);
	networkData = (networkData * primary_buffer_size) / 100;
	buffersize = (networkData / framesize + 1) * framesize; // Upp-round to full frame
	WorkingSet.primary_buffer_size_4 = (buffersize / PAGESIZE + 1) * PAGESIZE;
	WorkingSet.temp_buffer_size_4 = PAGESIZE;
	WorkingSet.user_buffer_size_4 = ((WorkingSet.bufferSizeInSampleNo * sizeof(unsigned short)) / PAGESIZE + 1) * PAGESIZE;
	size = WorkingSet.primary_buffer_size_4 + WorkingSet.temp_buffer_size_4 + 8 * WorkingSet.user_buffer_size_4;
	WorkingSet.np_memory_4 = CAPDFactory::GetAPDFactory()->GetNPMemory(size);
	WorkingSet.primary_buffer_4 = WorkingSet.np_memory_4->GetBuffer();
	WorkingSet.temp_buffer_4 = WorkingSet.primary_buffer_4 + WorkingSet.primary_buffer_size_4;
	WorkingSet.user_buffer_4 = WorkingSet.temp_buffer_4 + WorkingSet.temp_buffer_size_4;
	WorkingSet.requestedData_4 = userData;
	WorkingSet.eval_4->SetBuffers(WorkingSet.primary_buffer_4, WorkingSet.temp_buffer_4, WorkingSet.user_buffer_4, WorkingSet.user_buffer_size_4);
	WorkingSet.eval_4->SetParams(WorkingSet.bits, WorkingSet.channelMask_4, WorkingSet.packetsize_4);

	WorkingSet.setupComplete = true;

	return ADT_OK;

}

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetBuffers(ADT_HANDLE handle, short **buffers)
#else
ADT_RESULT ADC_GetBuffers(ADT_HANDLE handle, short **buffers)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (GetBitCount(WorkingSet.channelMask_1) != 0)
	{
		memcpy(buffers, WorkingSet.eval_1->m_ChannelData, 8 * sizeof(short*));
	}
	else
	{
		memset(buffers, 0, 8 * sizeof(short*));
	}

	if (GetBitCount(WorkingSet.channelMask_2) != 0)
	{
		memcpy(buffers+8, WorkingSet.eval_2->m_ChannelData, 8 * sizeof(short*));
	}
	else
	{
		memset(buffers+8, 0, 8 * sizeof(short*));
	}

	if (GetBitCount(WorkingSet.channelMask_3) != 0)
	{
		memcpy(buffers+16, WorkingSet.eval_3->m_ChannelData, 8 * sizeof(short*));
	}
	else
	{
		memset(buffers+16, 0, 8 * sizeof(short*));
	}

	if (GetBitCount(WorkingSet.channelMask_4) != 0)
	{
		memcpy(buffers+24, WorkingSet.eval_4->m_ChannelData, 8 * sizeof(short*));
	}
	else
	{
		memset(buffers+24, 0, 8 * sizeof(short*));
	}

	return ADT_OK;

}

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetSampleInfo(ADT_HANDLE handle, unsigned long long *sampleCounts, unsigned long long *sampleIndices)
#else
ADT_RESULT ADC_GetSampleInfo(ADT_HANDLE handle, unsigned long long *sampleCounts, unsigned long long *sampleIndices)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;
	if (sampleCounts == NULL) return ADT_PARAMETER_ERROR;
	if (sampleIndices == NULL) return ADT_PARAMETER_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	*(sampleCounts + 0) = WorkingSet.eval_1->GetSampleCount();
	*(sampleCounts + 1) = WorkingSet.eval_2->GetSampleCount();
	*(sampleCounts + 2) = WorkingSet.eval_3->GetSampleCount();
	*(sampleCounts + 3) = WorkingSet.eval_4->GetSampleCount();

	*(sampleIndices + 0) = WorkingSet.eval_1->GetSampleIndex();
	*(sampleIndices + 1) = WorkingSet.eval_2->GetSampleIndex();
	*(sampleIndices + 2) = WorkingSet.eval_3->GetSampleIndex();
	*(sampleIndices + 3) = WorkingSet.eval_4->GetSampleIndex();

	return ADT_OK;
}



#ifdef APD_VERSION
ADT_RESULT APDCAM_Arm(ADT_HANDLE handle, ADT_MEASUREMENT_MODE mode, long long sampleCount, ADT_CALIB_MODE calibMode, int signalFrequency)
#else
ADT_RESULT ADC_Arm(ADT_HANDLE handle, ADT_MEASUREMENT_MODE mode, long long sampleCount, ADT_CALIB_MODE calibMode, int signalFrequency)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (!WorkingSet.setupComplete)
	{
		return ADT_SETUP_ERROR;
	}

	if (sampleCount >= 0)
	{
		WorkingSet.sampleCount = (unsigned int)sampleCount;
	}

	if (mode == MM_ONE_SHOT && WorkingSet.sampleCount == 0)
	{
		return ADT_SETUP_ERROR;
	}

	if (WorkingSet.bufferSizeInSampleNo < sampleCount)
	{
		return ADT_SETUP_ERROR;
	}

	ADT_RESULT res = ADT_OK;

	// Disables HW trigger
	ADT_TRIGGER_CONTROL tc = {0};
	bool b = SetTrigger(WorkingSet.client, tc.trigger_control);

	/*
	short dacValues[32];
	// Set dac values (adc offset) to factory calibration values. (Read from the calibration table) Those are the initial values.
	for (int i = 0; i < 32; i++)
	{
	dacValues[i] = 0;
	}
	// Set adc offsets to values, stored in the dacValues array)
	SetDACOffset(WorkingSet.client, dacValues, 0, 32);
	*/

	// Server for stream 1
	WorkingSet.stream_1_server->SetListeningPort(WorkingSet.stream_1_port_h);
	WorkingSet.stream_1_server->SetBuffer(WorkingSet.primary_buffer_1, WorkingSet.primary_buffer_size_1);
    //WorkingSet.eval_1->Reset();

	int channels = GetBitCount(WorkingSet.channelMask_1);
	if (channels)
	{
		int blockSize = GetBlockSize(channels, WorkingSet.bits);
		WorkingSet.requestedData_1 = blockSize * WorkingSet.sampleCount + MAX_PREAMBULE_LENGTH;
		WorkingSet.dataNotification_1->Reset();
		WorkingSet.stream_1_server->SetNotification(WorkingSet.requestedData_1, WorkingSet.dataNotification_1);
		WorkingSet.stream_1_server->SetPacketSize(WorkingSet.packetsize_1 + sizeof(CW_FRAME_V2));

		WorkingSet.userNotification_1->Reset();
		WorkingSet.eval_1->SetUserNotificationSignal(WorkingSet.userNotification_1);
		WorkingSet.waitObject->Add(WorkingSet.userNotification_1);

		unsigned int framesize = WorkingSet.packetsize_1 + sizeof(CW_FRAME_V2);
		int primary_buuffer_size_in_frames = (int)(WorkingSet.primary_buffer_size_1 / framesize);
		int signal_frequency = min(signalFrequency, primary_buuffer_size_in_frames/4);
		signal_frequency = max(signal_frequency, 100);
		WorkingSet.stream_1_server->SetSignalFrequency(signal_frequency);

		WorkingSet.eval_1->SetCalibratedMode(calibMode == CM_CALIBRATED);
		WorkingSet.eval_1->DisableTrigger();

		if (mode == MM_ONE_SHOT)
		{
			WorkingSet.stream_1_server->SetType(CAPDServer::ST_ONE_SHOT);
			WorkingSet.stream_1_server->Reset();
			if (WorkingSet.requestedData_1 != 0)
			{
				if (WorkingSet.stream_1_server->Start())
				{
                    printf("TS1 started\n");
				}
				else
				{
                    printf("Could not start TS1\n");
					res = ADT_ERROR;
				}
				WorkingSet.eval_1->SetStopAt(WorkingSet.sampleCount);
				WorkingSet.eval_1->Start();
			}
		}
		else
		{
			WorkingSet.stream_1_server->SetType(CAPDServer::ST_CYCLIC);
			WorkingSet.stream_1_server->Reset();
			if (WorkingSet.stream_1_server->Start())
			{
				printf("TS1 started\n");
			}
			else
			{
				printf("Could not start TS1\n");
				res = ADT_ERROR;
			}
			WorkingSet.eval_1->SetStopAt(0);
			WorkingSet.eval_1->Start();
		}
	}

	// Server for stream 2
	WorkingSet.stream_2_server->SetListeningPort(WorkingSet.stream_2_port_h);
	WorkingSet.stream_2_server->SetBuffer(WorkingSet.primary_buffer_2, WorkingSet.primary_buffer_size_2);
    //WorkingSet.eval_2->Reset();

	channels = GetBitCount(WorkingSet.channelMask_2);
	if (channels)
	{
		int blockSize = GetBlockSize(channels, WorkingSet.bits);
		WorkingSet.requestedData_2 = blockSize * WorkingSet.sampleCount + MAX_PREAMBULE_LENGTH;
		WorkingSet.dataNotification_2->Reset();
		WorkingSet.stream_2_server->SetNotification(WorkingSet.requestedData_2, WorkingSet.dataNotification_2);
		WorkingSet.stream_2_server->SetPacketSize(WorkingSet.packetsize_2 + sizeof(CW_FRAME_V2));

		WorkingSet.userNotification_2->Reset();
		WorkingSet.eval_2->SetUserNotificationSignal(WorkingSet.userNotification_2);
		WorkingSet.waitObject->Add(WorkingSet.userNotification_2);

		unsigned int framesize = WorkingSet.packetsize_2 + sizeof(CW_FRAME_V2);
		int primary_buuffer_size_in_frames = (int)(WorkingSet.primary_buffer_size_2 / framesize);
		int signal_frequency = min(signalFrequency, primary_buuffer_size_in_frames/4);
		signal_frequency = max(signal_frequency, 100);
		WorkingSet.stream_2_server->SetSignalFrequency(signal_frequency);

		WorkingSet.eval_2->SetCalibratedMode(calibMode == CM_CALIBRATED);
		WorkingSet.eval_2->DisableTrigger();

		if (mode == MM_ONE_SHOT)
		{
			WorkingSet.stream_2_server->SetType(CAPDServer::ST_ONE_SHOT);
			WorkingSet.stream_2_server->Reset();
			if (WorkingSet.requestedData_2 != 0)
			{
				if (WorkingSet.stream_2_server->Start())
				{
                    printf("TS2 started\n");
				}
				else
				{
                    printf("Could not start TS2\n");
					res = ADT_ERROR;
				}
				WorkingSet.eval_2->SetStopAt(WorkingSet.sampleCount);
				WorkingSet.eval_2->Start();
			}
		}
		else
		{
			WorkingSet.stream_2_server->SetType(CAPDServer::ST_CYCLIC);
			WorkingSet.stream_2_server->Reset();
			if (WorkingSet.stream_2_server->Start())
			{
				printf("TS2 started\n");
			}
			else
			{
				printf("Could not start TS2\n");
				res = ADT_ERROR;
			}
			WorkingSet.eval_2->SetStopAt(0);
			WorkingSet.eval_2->Start();
		}
	}

	// Server for stream 3
	WorkingSet.stream_3_server->SetListeningPort(WorkingSet.stream_3_port_h);
	WorkingSet.stream_3_server->SetBuffer(WorkingSet.primary_buffer_3, WorkingSet.primary_buffer_size_3);
    //WorkingSet.eval_3->Reset();

	channels = GetBitCount(WorkingSet.channelMask_3);
	if (channels)
	{
		int blockSize = GetBlockSize(channels, WorkingSet.bits);
		WorkingSet.requestedData_3 = blockSize * WorkingSet.sampleCount + MAX_PREAMBULE_LENGTH;
		WorkingSet.dataNotification_3->Reset();
		WorkingSet.stream_3_server->SetNotification(WorkingSet.requestedData_3, WorkingSet.dataNotification_3);
		WorkingSet.stream_3_server->SetPacketSize(WorkingSet.packetsize_3 + sizeof(CW_FRAME_V2));

		WorkingSet.userNotification_3->Reset();
		WorkingSet.eval_3->SetUserNotificationSignal(WorkingSet.userNotification_3);
		WorkingSet.waitObject->Add(WorkingSet.userNotification_3);

		unsigned int framesize = WorkingSet.packetsize_3 + sizeof(CW_FRAME_V2);
		int primary_buuffer_size_in_frames = (int)(WorkingSet.primary_buffer_size_3 / framesize);
		int signal_frequency = min(signalFrequency, primary_buuffer_size_in_frames/4);
		signal_frequency = max(signal_frequency, 100);
		WorkingSet.stream_3_server->SetSignalFrequency(signal_frequency);

		WorkingSet.eval_3->SetCalibratedMode(calibMode == CM_CALIBRATED);
		WorkingSet.eval_3->DisableTrigger();

		if (mode == MM_ONE_SHOT)
		{
			WorkingSet.stream_3_server->SetType(CAPDServer::ST_ONE_SHOT);
			WorkingSet.stream_3_server->Reset();
			if (WorkingSet.requestedData_3 != 0)
			{
				if (WorkingSet.stream_3_server->Start())
				{
                    printf("TS3 started\n");
				}
				else
				{
                    printf("Could not start TS3\n");
					res = ADT_ERROR;
				}
				WorkingSet.eval_3->SetStopAt(WorkingSet.sampleCount);
				WorkingSet.eval_3->Start();
			}
		}
		else
		{
			WorkingSet.stream_3_server->SetType(CAPDServer::ST_CYCLIC);
			WorkingSet.stream_3_server->Reset();
			if (WorkingSet.stream_3_server->Start())
			{
				printf("TS3 started\n");
			}
			else
			{
				printf("Could not start TS3\n");
				res = ADT_ERROR;
			}
			WorkingSet.eval_3->SetStopAt(0);
			WorkingSet.eval_3->Start();
		}
	}

	// Server for stream 4
	WorkingSet.stream_4_server->SetListeningPort(WorkingSet.stream_4_port_h);
	WorkingSet.stream_4_server->SetBuffer(WorkingSet.primary_buffer_4, WorkingSet.primary_buffer_size_4);
    //WorkingSet.eval_4->Reset();

	channels = GetBitCount(WorkingSet.channelMask_4);
	if (channels)
	{
		int blockSize = GetBlockSize(channels, WorkingSet.bits);
        WorkingSet.requestedData_4 = blockSize * WorkingSet.sampleCount + MAX_PREAMBULE_LENGTH;
		WorkingSet.dataNotification_4->Reset();
		WorkingSet.stream_4_server->SetNotification(WorkingSet.requestedData_4, WorkingSet.dataNotification_4);
		WorkingSet.stream_4_server->SetPacketSize(WorkingSet.packetsize_4 + sizeof(CW_FRAME_V2));

		WorkingSet.userNotification_4->Reset();
		WorkingSet.eval_4->SetUserNotificationSignal(WorkingSet.userNotification_4);
		WorkingSet.waitObject->Add(WorkingSet.userNotification_4);

		unsigned int framesize = WorkingSet.packetsize_4 + sizeof(CW_FRAME_V2);
		int primary_buuffer_size_in_frames = (int)(WorkingSet.primary_buffer_size_4 / framesize);
		int signal_frequency = min(signalFrequency, primary_buuffer_size_in_frames/4);
		signal_frequency = max(signal_frequency, 100);
		WorkingSet.stream_4_server->SetSignalFrequency(signal_frequency);

		WorkingSet.eval_4->SetCalibratedMode(calibMode == CM_CALIBRATED);
		WorkingSet.eval_4->DisableTrigger();

		if (mode == MM_ONE_SHOT)
		{
			WorkingSet.stream_4_server->SetType(CAPDServer::ST_ONE_SHOT);
			WorkingSet.stream_4_server->Reset();
			if (WorkingSet.requestedData_4 != 0)
			{
				if (WorkingSet.stream_4_server->Start())
				{
                    printf("TS4 started\n");
				}
				else
				{
                    printf("Could not start TS4\n");
					res = ADT_ERROR;
				}
				WorkingSet.eval_4->SetStopAt(WorkingSet.sampleCount);
				WorkingSet.eval_4->Start();
			}
		}
		else
		{
			WorkingSet.stream_4_server->SetType(CAPDServer::ST_CYCLIC);
			WorkingSet.stream_4_server->Reset();
			if (WorkingSet.stream_4_server->Start())
			{
				printf("TS4 started\n");
			}
			else
			{
				printf("Could not start TS4\n");
				res = ADT_ERROR;
			}
			WorkingSet.eval_4->SetStopAt(0);
			WorkingSet.eval_4->Start();
		}
	}

	b = SetupAllTS(WorkingSet.client, WorkingSet.packetsize_1, WorkingSet.packetsize_2, WorkingSet.packetsize_3, WorkingSet.packetsize_4,
		WorkingSet.stream_1_port_h, WorkingSet.stream_2_port_h, WorkingSet.stream_3_port_h, WorkingSet.stream_4_port_h);
	if (!b)
	{
		// Error handling
		res = ADT_ERROR;
	}
    //Sleep(100);

	if (!SetChannels(WorkingSet.client, WorkingSet.channelMask_1, WorkingSet.channelMask_2, WorkingSet.channelMask_3, WorkingSet.channelMask_4))
	{
		// Error handling
		res = ADT_ERROR;
	}

    Sleep(10);

	if (!SetResolution(WorkingSet.client, WorkingSet.bits))
	{
		// Error handling
		res = ADT_ERROR;
	}

    Sleep(10);

	if (mode == MM_ONE_SHOT)
	{
		unsigned int sc = WorkingSet.sampleCount + 1500;
		if (!SetSampleCount(WorkingSet.client, sc))
		{
			// Error handling
			res = ADT_ERROR;
		}
	}
	else
	{
		unsigned int sc = 0;
		if (!SetSampleCount(WorkingSet.client, sc))
		{
			// Error handling
			res = ADT_ERROR;
		}
	}
    //Sleep(100);

	ADT_STATUS_1 status1;
	if (GetStatus1(WorkingSet.client, &status1.status))
	{
		if (!status1.ADC_PLL_Locked) res = ADT_ERROR;
		if (!status1.Stream_PLL_Locked) res = ADT_ERROR;
	}
	else
	{
		res = ADT_ERROR;
	}

	ADT_STATUS_2 status2;
	if (GetStatus2(WorkingSet.client, &status2.status))
	{
	}
	else
	{
		res = ADT_ERROR;
	}

	if (res == ADT_OK)
	{
		WorkingSet.state = AS_ARMED;
	}

	return res;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_Start(ADT_HANDLE handle) 
#else
ADT_RESULT ADC_Start(ADT_HANDLE handle) 
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];


	if (WorkingSet.state != AS_ARMED)
	{
		return ADT_SETUP_ERROR;
	}

	ADT_RESULT res = ADT_OK;


	WorkingSet.state = AS_MEASURE;

	unsigned char command = 0x0F; // Enables streams on all channel
	if (!SetStreamControl(WorkingSet.client, command))
	{
		// Error handling
		res = ADT_ERROR;
	}

	//printf("Program started\n");
	return res;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_Wait(ADT_HANDLE handle, int timeout)
#else
ADT_RESULT ADC_Wait(ADT_HANDLE handle, int timeout)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (WorkingSet.state != AS_MEASURE) return ADT_ERROR;

	ADT_RESULT res = ADT_OK;

	CWaitForEvents::WAIT_RESULT wres = WorkingSet.waitObject->WaitAll(timeout);
	if (wres != CWaitForEvents::WR_OK)
	{
		printf("TimeOut\n");
		res = ADT_TIMEOUT;
	}

	unsigned char command = 0x00; // Disables streams on all channel
	if (!SetStreamControl(WorkingSet.client, command))
	{
		// Error handling
		res = ADT_ERROR;
	}

	WorkingSet.stream_1_server->Stop();
	WorkingSet.stream_2_server->Stop();
	WorkingSet.stream_3_server->Stop();
	WorkingSet.stream_4_server->Stop();

	ShutupAllTS(WorkingSet.client);
    Sleep(100);

	WorkingSet.waitObject->RemoveAll();

	WorkingSet.state = AS_STANDBY;

	WorkingSet.eval_1->Stop();
	WorkingSet.eval_2->Stop();
	WorkingSet.eval_3->Stop();
	WorkingSet.eval_4->Stop();

	return res;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_Stop(ADT_HANDLE handle)
#else
ADT_RESULT ADC_Stop(ADT_HANDLE handle)
#endif
{
	ADT_RESULT res;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (WorkingSet.state != AS_MEASURE) return ADT_ERROR;

	unsigned char command = 0x00; // Disables streams on all channel
	if (!SetStreamControl(WorkingSet.client, command))
	{
		// Error handling
		res = ADT_ERROR;
	}

	WorkingSet.stream_1_server->Stop();
	WorkingSet.stream_2_server->Stop();
	WorkingSet.stream_3_server->Stop();
	WorkingSet.stream_4_server->Stop();

	ShutupAllTS(WorkingSet.client);
	Sleep(100);

	WorkingSet.waitObject->RemoveAll();

	WorkingSet.state = AS_STANDBY;

	WorkingSet.eval_1->Stop();
	WorkingSet.eval_2->Stop();
	WorkingSet.eval_3->Stop();
	WorkingSet.eval_4->Stop();

	return ADT_OK;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_Trigger(ADT_HANDLE handle, ADT_TRIGGER trigger, ADT_TRIGGER_MODE mode, ADT_TRIGGER_EDGE edge, int triggerDelay, ADT_TRIGGERINFO* triggerInfo, unsigned short preTriggerSampleCount)
#else
ADT_RESULT ADC_Trigger(ADT_HANDLE handle, ADT_TRIGGER trigger, ADT_TRIGGER_MODE mode, ADT_TRIGGER_EDGE edge, int triggerDelay, ADT_TRIGGERINFO* triggerInfo, unsigned short preTriggerSampleCount)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	//??
	if (WorkingSet.state != AS_ARMED)
	{
		return ADT_SETUP_ERROR;
	}

	ADT_RESULT res = ADT_OK;

	if (trigger == TR_SOFTWARE)
	{ // Setup software trigger
		WorkingSet.triggerManager->SetDelay(triggerDelay);

		WorkingSet.eval_1->SetTrigger(triggerInfo);
		WorkingSet.eval_2->SetTrigger(triggerInfo + 8);
		WorkingSet.eval_3->SetTrigger(triggerInfo + 16);
		WorkingSet.eval_4->SetTrigger(triggerInfo + 24);

    }//if
    else if(trigger == TR_HARDWARE)
	{  // Setup hardware trigger
		ADT_TRIGGER_CONTROL tc = {0};
		if (!SetTriggerDelay(WorkingSet.client, triggerDelay))
		{
			// Error handling
			res = ADT_ERROR;
		}

		if(preTriggerSampleCount < 0 || preTriggerSampleCount > 1023)
			return ADT_PARAMETER_ERROR;

		bool ret = SetRingbufferSize(WorkingSet.client, preTriggerSampleCount);

		if(!ret)
			res = ADT_ERROR;

		if (mode == TRM_EXTERNAL)
		{
			if (edge == TRE_RISING)
            {
				tc.Enable_Rising_Edge = 1;

            }//if
            else if(edge == TRE_FALLING)
            {
				tc.Enable_Falling_Edge = 1;

            }//else if
            else
            {
               return ADT_PARAMETER_ERROR;
            }
			bool b = SetTrigger(WorkingSet.client, tc.trigger_control);
			if (!b) res = ADT_SETUP_ERROR;

        }//if
        else if(mode == TRM_INTERNAL)
		{
			int bits = WorkingSet.bits;
			int shift = 0;
			short maxValue = 16383;
			switch (bits)
			{
			case 8: maxValue = 255; shift = 6; break;
			case 12: maxValue = 4095; shift = 2; break;
			case 14: maxValue = 16383; shift = 0; break;
			}
#ifdef APD_VERSION
			for (int i = 0; i < 32; i++)
			{
				triggerInfo[i].TriggerLevel = maxValue - (triggerInfo[i].TriggerLevel << shift);
				triggerInfo[i].Sensitivity = 1 - triggerInfo[i].Sensitivity;
			}
#else
			for (int i = 0; i < 32; i++)
			{
				triggerInfo[i].TriggerLevel = (triggerInfo[i].TriggerLevel << shift);
			}
#endif
			tc.Enable_Internal_Trigger = 1;
			bool b = SetTrigger(WorkingSet.client, tc.trigger_control);
			if (b) b = SetInternalTriggerLevels(WorkingSet.client, (unsigned short*)triggerInfo);
			if (!b) res = ADT_SETUP_ERROR;

        }//else if
        else
        {
           res = ADT_PARAMETER_ERROR;
        }

    }//else if
    else
    {
        res = ADT_PARAMETER_ERROR;

    }//else

	return res;
}


/*#ifdef APD_VERSION
ADT_RESULT APDCAM_SetHWTriggerDelay(ADT_HANDLE handle, int delay)
#else
ADT_RESULT ADC_SetHWTriggerDelay(ADT_HANDLE handle, int delay)
#endif
{
int index = GetIndex(handle);
if (index < 0) return ADT_INVALID_HANDLE_ERROR;

WORKING_SET &WorkingSet = g_WorkingSets[index];

if (WorkingSet.state != AS_ARMED)
{
return ADT_SETUP_ERROR;
}

ADT_RESULT res = ADT_OK;

if (!SetTriggerDelay(WorkingSet.client, delay))
{
// Error handling
res = ADT_ERROR;
}

return res;
}*/


/*#ifdef APD_VERSION
ADT_RESULT APDCAM_SWTrigger(ADT_HANDLE handle)
#else
ADT_RESULT ADC_SWTrigger(ADT_HANDLE handle)
#endif
{
int index = GetIndex(handle);
if (index < 0) return ADT_INVALID_HANDLE_ERROR;

WORKING_SET &WorkingSet = g_WorkingSets[index];

if (WorkingSet.state != AS_MEASURE)
{
return ADT_ERROR;
}

Ulong long sc = WorkingSet.eval_1->GetSampleCount();
sc = max(sc, WorkingSet.eval_2->GetSampleCount());
sc = max(sc, WorkingSet.eval_3->GetSampleCount());
sc = max(sc, WorkingSet.eval_4->GetSampleCount());

Ulong long stopAt = sc + WorkingSet.sampleCount; // stopDelay
WorkingSet.eval_1->SetStopAt(stopAt);
WorkingSet.eval_2->SetStopAt(stopAt);
WorkingSet.eval_3->SetStopAt(stopAt);
WorkingSet.eval_4->SetStopAt(stopAt);

return ADT_OK;
}*/

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetIP(ADT_HANDLE handle, unsigned long ip_h)
#else
ADT_RESULT ADC_SetIP(ADT_HANDLE handle, unsigned long ip_h)
#endif
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;
	WORKING_SET &WorkingSet = g_WorkingSets[index];

	ADT_RESULT retVal = ADT_OK;

	if (SetIP(WorkingSet.client, ip_h))
	{
		WorkingSet.ip_h = ip_h;
		WorkingSet.client->SetIPAddress(WorkingSet.ip_h);
        //Sleep(100);
	}
	else
	{
		retVal = ADT_ERROR;
	}


	return retVal;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetTiming(ADT_HANDLE handle, int adcMult, int adcDiv, int strMult, int strDiv, int clkSource, int clkMult, int clkDiv)
#else
ADT_RESULT ADC_SetTiming(ADT_HANDLE handle, int adcMult, int adcDiv, int strMult, int strDiv, int clkSource, int clkMult, int clkDiv)
#endif
{
	ADT_RESULT retVal = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (adcMult < 0) adcMult = WorkingSet.adcMult;
	if (adcDiv < 0) adcDiv = WorkingSet.adcDiv;
	if (strMult < 0) strMult = WorkingSet.strMult;
	if (strDiv < 0) strDiv = WorkingSet.strDiv;
	if (clkMult < 0) clkMult = WorkingSet.clkMult;
	if (clkDiv < 0) clkDiv = WorkingSet.clkDiv;

	if (adcMult < 20 || 50 < adcMult) return ADT_PARAMETER_ERROR;
	if (adcDiv < 8 || 100 < adcDiv) return ADT_PARAMETER_ERROR;

	if (strMult < 20 || 50 < strMult) return ADT_PARAMETER_ERROR;
	if (strDiv < 8 || 100 < strDiv) return ADT_PARAMETER_ERROR;

	if (clkSource==1)
	{
		if (clkMult < 2 || 33 < clkMult) return ADT_PARAMETER_ERROR;
		if (clkDiv < 1 || 32 < clkDiv) return ADT_PARAMETER_ERROR;
	}

	if (adcMult > 0) WorkingSet.adcMult = adcMult;
	if (adcDiv > 0) WorkingSet.adcDiv = adcDiv;
	if (strMult > 0) WorkingSet.strMult = strMult;
	if (strDiv > 0) WorkingSet.strDiv = strDiv;
	if (clkMult > 0) WorkingSet.clkMult = clkMult;
	if (clkDiv > 0) WorkingSet.clkDiv = clkDiv;

	bool initRes = SetADCPLL(WorkingSet.client, WorkingSet.adcMult, WorkingSet.adcDiv);
	initRes &= SetStreamPLL(WorkingSet.client, WorkingSet.strMult, WorkingSet.strDiv);
	if (clkSource==1)
	{
		initRes &= SetExtClkPLL(WorkingSet.client, clkMult, clkDiv);

		unsigned char adcControl=0;
		bool res = GetADCControl(WorkingSet.client, &adcControl);
		if (!res)  retVal = ADT_ERROR;

		adcControl ^= 1 << 0;
        adcControl ^= 1 << 7;

		res = SetADCControl(WorkingSet.client, adcControl);
		if (!res)  retVal = ADT_ERROR;

		unsigned char status2=0;
		res = GetStatus2(WorkingSet.client, &status2);

		if(!(status2 & 4))
			return ADT_ERROR;
	}
	else if(clkSource==0)
	{
		unsigned char adcControl=0;
		bool res = GetADCControl(WorkingSet.client, &adcControl);
		if (!res)  retVal = ADT_ERROR;

		adcControl &= ~(1 << 0);
        adcControl ^= 1 << 7;

		res = SetADCControl(WorkingSet.client, adcControl);
		if (!res)  retVal = ADT_ERROR;

	}

	ADT_STATUS_1 status1;
	if (GetStatus1(WorkingSet.client, &status1.status))
	{
		if (!status1.ADC_PLL_Locked) retVal = ADT_ERROR;
		if (!status1.Stream_PLL_Locked) retVal = ADT_ERROR;
	}
	else
	{
		retVal = ADT_ERROR;
	}

	ADT_STATUS_2 status2;
	if (GetStatus2(WorkingSet.client, &status2.status))
	{
	}
	else
	{
		retVal = ADT_ERROR;
	}

	if (!initRes) retVal = ADT_ERROR;

	return retVal;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_Sampling(ADT_HANDLE handle, int sampleDiv, int sampleSrc)
#else
ADT_RESULT ADC_Sampling(ADT_HANDLE handle, int sampleDiv, int sampleSrc)
#endif
{
	ADT_RESULT retVal = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (sampleDiv <= 0) return ADT_PARAMETER_ERROR;
	if (sampleDiv > 0xFFFF) return ADT_PARAMETER_ERROR;
	if (sampleDiv > 0) WorkingSet.sampleDiv = sampleDiv * 7;

	bool res = SetSampleDiv(WorkingSet.client, WorkingSet.sampleDiv);
	if (!res)  retVal = ADT_ERROR;

	unsigned char adcControl=0;
	res = GetADCControl(WorkingSet.client, &adcControl);
	if (!res)  retVal = ADT_ERROR;

	if(sampleSrc==0)
	{
		adcControl &= ~(1 << 2);
	}
	else if(sampleSrc==1)
	{

		adcControl ^= 1 << 2;
	}

    adcControl ^= 1 << 7;

	res = SetADCControl(WorkingSet.client, adcControl);
	if (!res)  retVal = ADT_ERROR;

	return retVal;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_DataMode(ADT_HANDLE handle, int modeCode)
#else
ADT_RESULT ADC_DataMode(ADT_HANDLE handle, int modeCode)
#endif
{
	ADT_RESULT retVal = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (modeCode < 0 || 7 < modeCode) return ADT_PARAMETER_ERROR;

	bool res =  SetTestMode(WorkingSet.client, modeCode);
	if (!res)  retVal = ADT_ERROR;

	return retVal;;
}

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetFilter(ADT_HANDLE handle, FILTER_COEFFICIENTS filterCoefficients)
#else
ADT_RESULT ADC_SetFilter(ADT_HANDLE handle, FILTER_COEFFICIENTS filterCoefficients)
#endif
{
	ADT_RESULT retVal = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	bool enableFilter=true;

	if(filterCoefficients.FilterDevideFactor==0)
		if(filterCoefficients.FIR[0]==0)
			if(filterCoefficients.FIR[1]==0)
				if(filterCoefficients.FIR[2]==0)
					if(filterCoefficients.FIR[3]==0)
						if(filterCoefficients.FIR[4]==0)
							if(filterCoefficients.RecursiveFilter==0)
								if(filterCoefficients.Reserved==0)
									enableFilter=false;

	bool res = SetFilterCoefficients(WorkingSet.client, (unsigned short*)&filterCoefficients);
	if (!res)  retVal = ADT_ERROR;

	if(enableFilter)
	{
		unsigned char adcControl=0;
		bool res = GetADCControl(WorkingSet.client, &adcControl);
		if (!res)  retVal = ADT_ERROR;

		adcControl ^= 1 << 4;
        adcControl ^= 1 << 7;

		res = SetADCControl(WorkingSet.client, adcControl);
		if (!res)  retVal = ADT_ERROR;

	}
	else
	{
		unsigned char adcControl=0;
		bool res = GetADCControl(WorkingSet.client, &adcControl);
		if (!res)  retVal = ADT_ERROR;

		adcControl &= ~(1 << 4);
        adcControl ^= 1 << 7;

		res = SetADCControl(WorkingSet.client, adcControl);
		if (!res)  retVal = ADT_ERROR;

	}

	return retVal;

}//APDCAM_Filter


#ifdef APD_VERSION
ADT_RESULT APDCAM_CalculateFilterParams(ADT_HANDLE handle, double f_fir, double f_rec,  FILTER_COEFFICIENTS &filterCoefficients)
#else
ADT_RESULT ADC_CalculateFilterParams(ADT_HANDLE handle, double f_fir, double f_rec,  FILTER_COEFFICIENTS &filterCoefficients)
#endif
{
    ADT_RESULT retVal = ADT_NOT_IMPLEMENTED;

	return retVal;

}//APDCAM_CalculateFilterParams


#ifdef APD_VERSION
ADT_RESULT APDCAM_SetCalibLight(ADT_HANDLE handle, int value)
{
	ADT_RESULT retVal = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if(value < 0 || value > 4095)
		return ADT_PARAMETER_ERROR;

	bool res = SetCalibLigth(WorkingSet.client, value);
	if (!res)  retVal = ADT_ERROR;

	return retVal;

}
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetCalibLight(ADT_HANDLE handle, int *value)
{
	ADT_RESULT retVal = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	bool res = GetCalibLigth(WorkingSet.client, value);
	if (!res)  retVal = ADT_ERROR;

	return retVal;
}
#endif



#ifdef APD_VERSION
ADT_RESULT APDCAM_Shutter(ADT_HANDLE handle, int open)
{
	ADT_RESULT retVal = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	int mode;
	bool res = GetShutterMode(WorkingSet.client, &mode);
	if (!res)
	{
		retVal = ADT_ERROR;
		return retVal;
	}


	if (mode != 0)
	{
		retVal = ADT_ERROR;
		return retVal;
	}

	res = SetShutterState(WorkingSet.client, open);
	if (!res)  retVal = ADT_ERROR;

	return retVal;
}
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetShutterMode(ADT_HANDLE handle, int mode)
{
	ADT_RESULT retVal = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	bool res = SetShutterMode(WorkingSet.client, mode);
	if (!res)  retVal = ADT_ERROR;

	return retVal;
}
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetShutterMode(ADT_HANDLE handle, int *mode)
{
	ADT_RESULT retVal = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

	bool res = GetShutterMode(WorkingSet.client, mode);
	if (!res)  retVal = ADT_ERROR;

	return retVal;
}
#endif



#ifdef APD_VERSION
void CalcStatistic(short *buffer, long long sampleCount, double* mean, double* stddev)
{
	short *temp = buffer;
	double _mean = 0;
	for (int sample = 0; sample < sampleCount; sample++)
	{
		_mean += (double)(*temp);
		temp++;
	}
	_mean = _mean / sampleCount;

	temp = buffer;
	double _stddev = 0;
	double d2;
	for (int sample = 0; sample < sampleCount; sample++)
	{
		d2 = (double)(*temp) - _mean;
		_stddev += d2*d2;
		temp++;
	}
	_stddev /= sampleCount;
	_stddev = sqrt(_stddev);

	*mean = _mean;
	*stddev = _stddev;
}



ADT_RESULT APDCAM_Calibrate(ADT_HANDLE handle)
{
    int index = GetIndex(handle);
    if (index < 0) return ADT_INVALID_HANDLE_ERROR;

    WORKING_SET &WorkingSet = g_WorkingSets[index];

    ADT_RESULT res = ADT_OK;

    //Checking factory table
    APDFactory apdFactory;

    bool ret = GetFactoryData(WorkingSet.client, &apdFactory);

    if(!ret)
        return ADT_FACTORY_SETUP_ERROR;

    /*float outputHvCalib = 0;
    float minHv = 0;

    if(apdFactory.pcDataStatus==1 || apdFactory.pcDataStatus==2)
    {
       outputHvCalib = apdFactory.outputHVCalib1;
       minHv = apdFactory.minHV;
    }
    else
    {
       outputHvCalib = 0.12;
       minHv = 250;
    }

    unsigned short ADCHighLimit = 0;

    if(apdFactory.adcDataStatus==1 || apdFactory.adcDataStatus==2)
    {
        ADCHighLimit = apdFactory.adcHighLimit;
    }
    else
    {
        ADCHighLimit = 12000;
    }

    // 16384 is the max value on 14 bit measurement.
    float baseLevel = 16384.0f - (float)ADCHighLimit;

    int adcMult = 20, adcDiv = 80, strMult = 30, strDiv = 10, clkSrc = 0, clkMult = -1, clkDiv = -1;
    res = APDCAM_SetTiming(handle, adcMult, adcDiv, strMult, strDiv, clkSrc, clkMult, clkDiv);

    int sampleDiv = 14; // 2Mhz de 1 khz-en kellene
    int sampleSrc = 0; // internal
    res = APDCAM_Sampling(handle, sampleDiv, sampleSrc);

    // For calibration about 10000 measurements are used
    long long sampleCount = 100000;
    int bits = 14;
    int channelMask_1 = 255, channelMask_2 = 255, channelMask_3 = 255, channelMask_4 = 255;
    res = APDCAM_Allocate(handle, sampleCount, bits, channelMask_1, channelMask_2, channelMask_3, channelMask_4,100);


    CDataEvaluation *dataEvaluate[4];
    dataEvaluate[0] = WorkingSet.eval_1;
    dataEvaluate[1] = WorkingSet.eval_2;
    dataEvaluate[2] = WorkingSet.eval_3;
    dataEvaluate[3] = WorkingSet.eval_4;

    //Close shutter
    APDCAM_Shutter(handle, 0);
    // Switch of calibration light
    APDCAM_SetCalibLight(handle, 0);

    // Set detector bias voltage to 250V, enables and switches on.
    // 0,12V / digit. 250V = 2083
    int hvValue = (int)(minHv / outputHvCalib);
    SetHV1(WorkingSet.client, hvValue);
    EnableHV(WorkingSet.client, true);
    SetHVState(WorkingSet.client, 1);
    Sleep(1000);

    double hv[4];
    int state=0;
    //APDCAM_HVMonitor(handle, hv);
    APDCAM_GetHV(handle, &hv[0], &hv[1], &state);

    double temps[16];
    bool bb = GetAllTempSensors(WorkingSet.client, temps);


    short dacValues[32];
    // Set dac values (adc offset) to factory calibration values. (Read from the calibration table) Those are the initial values.
    for (int i = 0; i < 32; i++)
    {
        dacValues[i] = 512; //2048;
    }

    short step = 256;
    while (step > 0)
    {
        // Set adc offsets to values, stored in the dacValues array)
        if (!SetADCOffset(WorkingSet.client, dacValues, 0, 32))
        {
        }
        short dacValuesReadBack[32];
        if (!GetADCOffset(WorkingSet.client, dacValuesReadBack, 0, 32))
        {
        }
        bool setupError = false;
        for (int i = 0; i < 32; i++)
        {
            if (dacValues[i] != dacValuesReadBack[i]) setupError = true;
        }
        if (setupError) printf("DAC offsett setup error\n");

        Sleep(20);

        res = Measure_NonCalibrated(handle, sampleCount);
        if (res != ADT_OK)
        {
            return res;
        }

        // Calculates mean for each channel
        for (int channel = 0; channel < 32; channel ++)
        {
            int streamIndex = channel / 8;
            int channelIndex = channel % 8;
            CDataEvaluation *eval = dataEvaluate[streamIndex];
            short *buffer = eval->m_ChannelData[channelIndex];
            double mean, stddev;
            CalcStatistic(buffer, sampleCount, &mean, &stddev);
            printf("%d  - %g, %g \n", channel, mean, stddev);

            // When the dac values are calibrated, the mean must be about 2^14 * 0.9 ~ 16386*0.9 = 14745 (BASE_LEVEL)
            // some margin must be left.

            if (mean < baseLevel)
            { // increase dac value
                dacValues[channel] = min(dacValues[channel] - step, 4096);
            }
            else
            { // decrease dac value
                dacValues[channel] = max(dacValues[channel] + step, 0);
            }
        }
        step = step / 2;
        Sleep(100);
    }

    // Takes a meauserement with the final offsett settings.
    if (!SetADCOffset(WorkingSet.client, dacValues, 0, 32))
    {
    }
    short dacValuesReadBack[32];
    if (!GetADCOffset(WorkingSet.client, dacValuesReadBack, 0, 32))
    {
    }
    bool setupError = false;
    for (int i = 0; i < 32; i++)
    {
        if (dacValues[i] != dacValuesReadBack[i]) setupError = true;
    }
    if (setupError) printf("DAC offsett setup error\n");
    Sleep(20);

    res = Measure_NonCalibrated(handle, sampleCount);
    if (res != ADT_OK)
    {
        return res;
    }

    // Calculates mean for each channel
    double adcOffsets[32];
    for (int channel = 0; channel < 32; channel ++)
    {
        int streamIndex = channel / 8;
        int channelIndex = channel % 8;
        CDataEvaluation *eval = dataEvaluate[streamIndex];
        short *buffer = eval->m_ChannelData[channelIndex];
        double mean, stddev;
        CalcStatistic(buffer, sampleCount, &mean, &stddev);
        printf("%d  - %g, %g \n", channel, mean, stddev);

        // When the dac values are calibrated, the mean must be about 2^14 * 0.9 ~ 16386*0.9 = 14745 (BASE_LEVEL)
        // some margin must be left.

        adcOffsets[channel] = mean;
    }

    for (int i = 0; i < 32; i++)
    {
        printf("Offsett[%d] \t:%6d, %g\n", i, dacValues[i], adcOffsets[i]);
    }

    SetHVState(WorkingSet.client, 0);

    // At the end the adc values are measured, as calibration data, and stored in the calibration table

    short shAdcOffsets[32];
    for (int i = 0; i < 32; i++)
    {
        shAdcOffsets[i] = (short)16384 - (short)(floor(adcOffsets[i] + 0.5));
    }

    //b = EnableFactoryWrite(WorkingSet.client, 1, true);
    //b = StoreADCOffsets(WorkingSet.client, shAdcOffsets, 0, 32);
    //b = EnableFactoryWrite(WorkingSet.client, 1, false);

    short adcOffsetsReadBack[32];
    bool b = RetrieveADCOffsets(WorkingSet.client, adcOffsetsReadBack, 0, 32);

    setupError = false;
    for (int i = 0; i < 32; i++)
    {
        if (shAdcOffsets[i] != adcOffsetsReadBack[i]) setupError = true;
    }
    if (setupError) printf("ADC offsett setup error\n");*/

    return res;
}
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetHV(ADT_HANDLE handle, double highVoltage1, double highVoltage2, int state)
{
    int index = GetIndex(handle);
    if (index < 0) return ADT_INVALID_HANDLE_ERROR;

    WORKING_SET &WorkingSet = g_WorkingSets[index];

    ADT_RESULT res = ADT_OK;

    /*if (state != 0)
    {*/

        //Checking factory table
        APDFactory apdFactory;

        float outputHvCalib1 = 0;
        float outputHvCalib2 = 0;

        bool ret = GetFactoryData(WorkingSet.client, &apdFactory);

        if(!ret)
            return ADT_ERROR;

        if(apdFactory.pcDataStatus==1 || apdFactory.pcDataStatus==2)
        {
            outputHvCalib1 = apdFactory.outputHVCalib1;
            outputHvCalib2 = apdFactory.outputHVCalib2;
        }
        else
        {
            outputHvCalib1 = (float)0.12;
            outputHvCalib2 = (float)0.12;
        }

        if (isnan(outputHvCalib1) || outputHvCalib1 < 0 || 1 < outputHvCalib1)
        {
            res = ADT_FACTORY_SETUP_ERROR;
            return res;
        }

        if (isnan(outputHvCalib2) || outputHvCalib2 < 0 || 1 < outputHvCalib2)
        {
            res = ADT_ERROR;
            return res;
        }

        // Set detector bias voltage, enables and switches on.
        // 0,12V / digit.
        int binValue1 = 0;
        int binValue2 = 0;

        if(apdFactory.productCode[0]=='A' && apdFactory.productCode[1]=='P')
        {
            if((highVoltage1 < (double)250) || (highVoltage1 > (double)430))
            {
                return ADT_PARAMETER_ERROR;

            }//if

            binValue1 = (int)(highVoltage1 / outputHvCalib1);
            //binValue2 = (int)(highVoltage2 / outputHvCalib2);

        }//if
        else if(apdFactory.productCode[0]=='M' && apdFactory.productCode[1]=='P')
        {
            if((highVoltage1 < (double)50) || (highVoltage1 > (double)58))
            {
                return ADT_PARAMETER_ERROR;

            }//if

            double A = 0.132002;
            double x = (highVoltage1 - A) / outputHvCalib2;
            //binValue1 = (int)(highVoltage1 / outputHvCalib1);
            binValue2 = (int)x;

        }//Detector is MPPC
        else
        {
            res = ADT_FACTORY_SETUP_ERROR;
            return res;
        }

        unsigned short hv1;

        if(!GetHV1Max(WorkingSet.client, &hv1))
            return ADT_ERROR;

        if(hv1==0)
            return ADT_FACTORY_SETUP_ERROR;

        unsigned short hv2;

        if(!GetHV2Max(WorkingSet.client, &hv2))
            return ADT_ERROR;

        if(hv2==0)
            return ADT_FACTORY_SETUP_ERROR;


        if (!SetHV1(WorkingSet.client, binValue1))
        {
            res = ADT_ERROR;
            return res;
        };

        if (!SetHV2(WorkingSet.client, binValue2))
        {
            res = ADT_ERROR;
            return res;
        };

        if (!EnableHV(WorkingSet.client, (state ? true : false)))
        {
            res = ADT_ERROR;
            return res;
        };

        if (!SetHVState(WorkingSet.client, state))
        {
            res = ADT_ERROR;
        };

    /*}
    else
    {
        if (!SetHVState(WorkingSet.client, 0)) { res = ADT_ERROR; };
        if (!EnableHV(WorkingSet.client, false)) { res = ADT_ERROR; };
    }*/
    return res;
}
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetHV(ADT_HANDLE handle, double *highVoltage1, double *highVoltage2, int *state)
{
    int index = GetIndex(handle);
    if (index < 0) return ADT_INVALID_HANDLE_ERROR;

    WORKING_SET &WorkingSet = g_WorkingSets[index];

    ADT_RESULT res = ADT_OK;

    float inputHvCalib1 = 0;
    float inputHvCalib2 = 0;

    APDFactory apdFactory;

    bool ret = GetFactoryData(WorkingSet.client, &apdFactory);

    if(!ret)
        return ADT_ERROR;

    if(apdFactory.pcDataStatus==1 || apdFactory.pcDataStatus==2)
    {
        inputHvCalib1 = apdFactory.inputHVCalib1;
        inputHvCalib2 = apdFactory.inputHVCalib2;
    }
    else
    {
        inputHvCalib1 = (float) 0.12;
        inputHvCalib2 = (float) 0.12;
    }

    if (isnan(inputHvCalib1) || inputHvCalib1 < 0 || 1 < inputHvCalib1)
    {
        res = ADT_FACTORY_SETUP_ERROR;
        return res;
    }

    if (isnan(inputHvCalib2) || inputHvCalib2 < 0 || 1 < inputHvCalib2)
    {
        res = ADT_FACTORY_SETUP_ERROR;
        return res;
    }

    int binValue1 = 0;
    int binValue2 = 0;
    if (!GetHV1(WorkingSet.client, &binValue1))
    {
        res = ADT_ERROR;
        return res;
    };

    if (!GetHV2(WorkingSet.client, &binValue2))
    {
        res = ADT_ERROR;
        return res;
    };

    *highVoltage1 = 0;
    *highVoltage2 = 0;

    if(apdFactory.productCode[0]=='A' && apdFactory.productCode[1]=='P')
    {
        *highVoltage1 = (double) binValue1 * inputHvCalib1;

    }
    else if(apdFactory.productCode[0]=='M' && apdFactory.productCode[1]=='P')
    {
        double A = 0.132002;
        double x = (double) binValue2 * inputHvCalib2;
        *highVoltage1 = x+A;
    }

    if (!GetHVState(WorkingSet.client, state))
    {
        res = ADT_ERROR;
        return res;
    }
    return res;
}
#endif


#ifdef APD_VERSION
ADT_RESULT APDCAM_HVMonitor(ADT_HANDLE handle, double *value)
{
    ADT_RESULT res = ADT_OK;

    int index = GetIndex(handle);
    if (index < 0) return ADT_INVALID_HANDLE_ERROR;

    WORKING_SET &WorkingSet = g_WorkingSets[index];


    float inputHvCalib1 = 0;
    float inputHvCalib2 = 0;

    APDFactory apdFactory;

    bool ret = GetFactoryData(WorkingSet.client, &apdFactory);

    if(!ret)
        return ADT_ERROR;

    if(apdFactory.pcDataStatus==1 || apdFactory.pcDataStatus==2)
    {
        inputHvCalib1 = apdFactory.inputHVCalib1;
        inputHvCalib2 = apdFactory.inputHVCalib2;
    }
    else
    {
        inputHvCalib1 = (float) 0.12;
        inputHvCalib2 = (float) 0.12;
    }

    unsigned short binValues[4];
    bool b = GetAllHVMonitor(WorkingSet.client, binValues);
    if (!b)
    {
        res = ADT_ERROR;
        return res;
    }

    value[0] = binValues[0] * inputHvCalib1;
    value[1] = binValues[1] * inputHvCalib2;

    return res;
}
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetStatus(ADT_HANDLE handle, APDStatus *apdStatus)
{
    ADT_RESULT res = ADT_ERROR;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];

    if (GetStatus(WorkingSet.client, apdStatus))
        res =  ADT_OK;


	return res;
}
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetInfo(ADT_HANDLE handle, APDInfo* apdInfo)
{
    int index = GetIndex(handle);
    if (index < 0)
        return ADT_INVALID_HANDLE_ERROR;

    ADT_RESULT res = ADT_ERROR;

    WORKING_SET &WorkingSet = g_WorkingSets[index];

    if (GetInfo(WorkingSet.client, apdInfo))
        res =  ADT_OK;

    APDFactory apdFactory;

    if(GetFactoryData(WorkingSet.client, &apdFactory))
    {
        double voltage1 = (double)apdInfo->hv1Max * apdFactory.inputHVCalib1;
        apdInfo->hv1Max=(unsigned short)voltage1;

        double voltage2 = (double)apdInfo->hv2Max * apdFactory.inputHVCalib2;
        apdInfo->hv2Max=(unsigned short)voltage2;

    }//if
    else
    {
        apdInfo->hv1Max=0;
        apdInfo->hv2Max=0;

    }//else


    return res;

}//APDCAM_GetInfo
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetPeltierControl(ADT_HANDLE handle, APDPeltierControl apdPeltierControl)
{
    int index = GetIndex(handle);
    if (index < 0)
        return ADT_INVALID_HANDLE_ERROR;

    ADT_RESULT res = ADT_ERROR;

    WORKING_SET &WorkingSet = g_WorkingSets[index];

    if(SetPeltierIndirectControl(WorkingSet.client,apdPeltierControl.indirectPeltierControl))
        res = ADT_OK;
    else
        res = ADT_ERROR;


    if(SetPGain(WorkingSet.client,apdPeltierControl.pGain))
        res = ADT_OK;
    else
        res = ADT_ERROR;

    if(SetIGain(WorkingSet.client,apdPeltierControl.iGain))
        res = ADT_OK;
    else
        res = ADT_ERROR;

    if(SetDGain(WorkingSet.client,apdPeltierControl.dGain))
        res = ADT_OK;
    else
        res = ADT_ERROR;


    if(SetDisablePIDControl(WorkingSet.client,apdPeltierControl.disablePIDControl))
        res = ADT_OK;
    else
        res = ADT_ERROR;


    return res;

}//APDCAM_SetPeltierControl
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetPeltierControl(ADT_HANDLE handle, APDPeltierControl *apdPeltierControl)
{
    int index = GetIndex(handle);
    if (index < 0)
        return ADT_INVALID_HANDLE_ERROR;

    ADT_RESULT res = ADT_ERROR;

    WORKING_SET &WorkingSet = g_WorkingSets[index];


    if(GetPeltierIndirectControl(WorkingSet.client,&apdPeltierControl->indirectPeltierControl))
        res = ADT_OK;
    else
        res = ADT_ERROR;


    if(GetPGain(WorkingSet.client,&apdPeltierControl->pGain))
        res = ADT_OK;
    else
        res = ADT_ERROR;

    if(GetIGain(WorkingSet.client,&apdPeltierControl->iGain))
        res = ADT_OK;
    else
        res = ADT_ERROR;

    if(GetDGain(WorkingSet.client,&apdPeltierControl->dGain))
        res = ADT_OK;
    else
        res = ADT_ERROR;


    if(GetDisablePIDControl(WorkingSet.client,&apdPeltierControl->disablePIDControl))
        res = ADT_OK;
    else
        res = ADT_ERROR;



    return res;

}//APDCAM_GetPeltierControl
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetDetectorReferenceTemp(ADT_HANDLE handle, short referenceTemp)
{
    int index = GetIndex(handle);
    if (index < 0)
        return ADT_INVALID_HANDLE_ERROR;

    ADT_RESULT res = ADT_ERROR;

    WORKING_SET &WorkingSet = g_WorkingSets[index];


    if(SetDetectorReferenceTemp(WorkingSet.client,referenceTemp*10))
        res = ADT_OK;
    else
        res = ADT_ERROR;



    return res;

}//APDCAM_SetDetectorReferenceTemp
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetDetectorReferenceTemp(ADT_HANDLE handle, short *referenceTemp)
{
    int index = GetIndex(handle);
    if (index < 0)
        return ADT_INVALID_HANDLE_ERROR;

    ADT_RESULT res = ADT_ERROR;

    WORKING_SET &WorkingSet = g_WorkingSets[index];

    *referenceTemp = *referenceTemp /10;

    if(GetDetectorReferenceTemp(WorkingSet.client,referenceTemp))
        res = ADT_OK;
    else
        res = ADT_ERROR;



    return res;

}//APDCAM_GetDetectorReferenceTemp
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetOverload(ADT_HANDLE handle, ADT_OVERLOADINFO overloadInfo, unsigned short overloadTime)
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];
	ADT_RESULT res = ADT_OK;

	bool b = SetOverloadLevel(WorkingSet.client, overloadInfo.OverloadInfo);
	if (!b) { res = ADT_ERROR; return res; }

	b = SetOverloadTime(WorkingSet.client, overloadTime);
	if (!b) { res = ADT_ERROR; return res; }

	return res;
}
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetOverload(ADT_HANDLE handle, ADT_OVERLOADINFO &overloadInfo, unsigned short &overloadTime, unsigned char &status)
{
	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;

	WORKING_SET &WorkingSet = g_WorkingSets[index];
	ADT_RESULT res = ADT_OK;

	bool b = GetOverloadLevel(WorkingSet.client, &overloadInfo.OverloadInfo);
	if (!b) { res = ADT_ERROR; return res; }

	b = GetOverloadTime(WorkingSet.client, &overloadTime);
	if (!b) { res = ADT_ERROR; return res; }

	b = GetOverloadStatus(WorkingSet.client, &status);
	if (!b) { res = ADT_ERROR; return res; }

	return res;
}
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetAnalogPower(ADT_HANDLE handle, bool enable)
{
    int index = GetIndex(handle);
    if (index < 0) return ADT_INVALID_HANDLE_ERROR;

    WORKING_SET &WorkingSet = g_WorkingSets[index];
    ADT_RESULT res = ADT_OK;

    if (!SetAnalogPower(WorkingSet.client, enable))
    {
        res = ADT_ERROR;
    }

    return res;

}//APDCAM_SetAnalogPower
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetAnalogPower(ADT_HANDLE handle, bool *enable)
{
    int index = GetIndex(handle);
    if (index < 0) return ADT_INVALID_HANDLE_ERROR;

    WORKING_SET &WorkingSet = g_WorkingSets[index];
    ADT_RESULT res = ADT_OK;

    if (!GetAnalogPower(WorkingSet.client, enable))
    {
        res = ADT_ERROR;
    }

    return res;

}//APDCAM_GetAnalogPower
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetADCOffsets(ADT_HANDLE handle, short *values)
{
	ADT_RESULT res = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;
	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (!SetADCOffset(WorkingSet.client, values, 0, 32))
	{
		res = ADT_ERROR;
	}
	return res;
}
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetADCOffsets(ADT_HANDLE handle, short *values)
{
	ADT_RESULT res = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;
	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if (!GetADCOffset(WorkingSet.client, values, 0, 32))
	{
		res = ADT_ERROR;
	}
	return res;
}
#endif


#ifdef APD_VERSION
ADT_RESULT APDCAM_GetConfig(ADT_HANDLE handle, APDStr *apdStr)
{
    int index = GetIndex(handle);
    if (index < 0)
        return ADT_INVALID_HANDLE_ERROR;

    ADT_RESULT res = ADT_OK;

    WORKING_SET &WorkingSet = g_WorkingSets[index];

    unsigned char adcDiv=0;
    unsigned char adcMult=0;

    bool retVal = GetADCPLL(WorkingSet.client, &adcMult, &adcDiv);

    if(!retVal)
        return ADT_ERROR;

    apdStr->adcMult=adcMult;
    apdStr->adcDiv=adcDiv;

    unsigned char strDiv=0;
    unsigned char strMult=0;

    retVal = GetStreamPLL(WorkingSet.client, &strMult, &strDiv);

    if(!retVal)
        return ADT_ERROR;

    apdStr->strMult=strMult;
    apdStr->strDiv=strDiv;

    unsigned char extClkSel=0;

    retVal = GetADCControl(WorkingSet.client, &extClkSel);

    if(!retVal)
        return ADT_ERROR;

    apdStr->clkSource = ((extClkSel & 1) ? 1:0);

    unsigned char clkDiv=0;
    unsigned char clkMult=0;

    retVal = GetExtClkPLL(WorkingSet.client, &clkMult, &clkDiv);

    apdStr->clkMult=clkMult;
    apdStr->clkDiv=clkDiv;

    if(!retVal)
        return ADT_ERROR;

    unsigned short sampleDiv=0;

    retVal = GetSampleDiv(WorkingSet.client, &sampleDiv);

    if(!retVal)
        return ADT_ERROR;

    apdStr->sampleDiv=sampleDiv/7;

    unsigned char sampleSrc=0;

    retVal = GetADCControl(WorkingSet.client, &sampleSrc);

    if(!retVal)
        return ADT_ERROR;

    apdStr->sampleSrc = ((sampleSrc & 4) ? 1:0);

    unsigned int sampleNum=0;

    retVal = GetSampleCount(WorkingSet.client, &sampleNum);

    if(!retVal)
        return ADT_ERROR;

    apdStr->sampleNum=sampleNum;

    unsigned int testMode;

    retVal = GetTestMode(WorkingSet.client, &testMode);

    if(!retVal)
        return ADT_ERROR;

    apdStr->testPattern=testMode;

    retVal = GetResolution(WorkingSet.client, &apdStr->bits);

    if(!retVal)
        return ADT_ERROR;

    unsigned char channels[4];

    retVal = GetChannels(WorkingSet.client, &channels[0], &channels[1],
                                            &channels[2], &channels[3]);

    if(!retVal)
        return ADT_ERROR;

    apdStr->channelMask1=channels[0];
    apdStr->channelMask2=channels[1];
    apdStr->channelMask3=channels[2];
    apdStr->channelMask4=channels[3];

    unsigned char trigger=0;

    retVal = GetTrigger(WorkingSet.client, &trigger);

    if(!retVal)
        return ADT_ERROR;

    unsigned short ringBufferSize=0;

    retVal = GetRingbufferSize(WorkingSet.client, &ringBufferSize);

    if(!retVal)
        return ADT_ERROR;

    apdStr->preTrigSampleCount=ringBufferSize;


    //Reading voltages
    float inputHvCalib1 = 0;
    float inputHvCalib2 = 0;

    APDFactory apdFactory;

    retVal = GetFactoryData(WorkingSet.client, &apdFactory);

    if(!retVal)
        return ADT_ERROR;

    if(apdFactory.pcDataStatus==1 || apdFactory.pcDataStatus==2)
    {
        inputHvCalib1 = apdFactory.inputHVCalib1;
        inputHvCalib2 = apdFactory.inputHVCalib2;
    }
    else
    {
        inputHvCalib1 = (float) 0.12;
        inputHvCalib2 = (float) 0.12;
    }

    if (isnan(inputHvCalib1) || inputHvCalib1 < 0 || 1 < inputHvCalib1)
    {
        return ADT_FACTORY_SETUP_ERROR;
    }

    if (isnan(inputHvCalib2) || inputHvCalib2 < 0 || 1 < inputHvCalib2)
    {
        return ADT_FACTORY_SETUP_ERROR;
    }

    int binValue1 = 0;
    int binValue2 = 0;

    if (!GetHV1(WorkingSet.client, &binValue1))
    {
        return ADT_ERROR;
    };

    if (!GetHV2(WorkingSet.client, &binValue2))
    {
        return ADT_ERROR;
    };

    apdStr->hv2 = 0.0;


    if(apdFactory.productCode[0]=='A' && apdFactory.productCode[1]=='P')
    {
        apdStr->hv1 = (double) binValue1 * inputHvCalib1;

    }
    else if(apdFactory.productCode[0]=='M' && apdFactory.productCode[1]=='P')
    {

        double A = 0.132002;
        double x = (double) binValue2 * inputHvCalib2;
        apdStr->hv1 = x+A;

    }//else if

    unsigned char hvOn=0;
    if (!GetHVState(WorkingSet.client, (int*)&hvOn))
    {
        return ADT_ERROR;
    }

    apdStr->hvOn[0]=((hvOn & 1) ? true : false);
    apdStr->hvOn[1]=((hvOn & 2) ? true : false);


    retVal = GetEnableHV(WorkingSet.client, &apdStr->hvEnabled);

    if(!retVal)
        return ADT_ERROR;

    retVal = GetPeltierIndirectControl(WorkingSet.client, &apdStr->peltierIndirectControl);

    if(!retVal)
        return ADT_ERROR;

    short refTemp = 0;

    retVal = GetDetectorReferenceTemp(WorkingSet.client, &refTemp);

    if(!retVal)
        return ADT_ERROR;

    apdStr->detectorReferenceTemp = refTemp / 10;

    retVal = GetPGain(WorkingSet.client, &apdStr->pGain);

    if(!retVal)
        return ADT_ERROR;

    retVal = GetIGain(WorkingSet.client, &apdStr->iGain);

    if(!retVal)
        return ADT_ERROR;

    retVal = GetDGain(WorkingSet.client, &apdStr->dGain);

    if(!retVal)
        return ADT_ERROR;

    retVal = GetDisablePIDControl(WorkingSet.client, &apdStr->disablePIDControl);

    if(!retVal)
        return ADT_ERROR;

    retVal = GetAnalogPower(WorkingSet.client, &apdStr->analogEnable);

    if(!retVal)
        return ADT_ERROR;


    retVal = GetFan1Speed(WorkingSet.client,&apdStr->fanSpeed[0]);

    if(!retVal)
        return ADT_ERROR;

    retVal = GetFan2Speed(WorkingSet.client,&apdStr->fanSpeed[1]);

    if(!retVal)
        return ADT_ERROR;

    retVal = GetFan3Speed(WorkingSet.client,&apdStr->fanSpeed[2]);

    if(!retVal)
        return ADT_ERROR;

    int calibLight=0;
    retVal = GetCalibLigth(WorkingSet.client,&calibLight);

    if(!retVal)
        return ADT_ERROR;

    apdStr->calibrationLight=(unsigned short)calibLight;

    retVal = GetShutterMode(WorkingSet.client,(int*)&apdStr->shutterMode);

    if(!retVal)
        return ADT_ERROR;

    retVal = GetShutterState(WorkingSet.client,(int*)&apdStr->shutterState);

    if(!retVal)
        return ADT_ERROR;

    if (!GetADCOffset(WorkingSet.client, &apdStr->offsets[0], 0, 32))
    {
        return ADT_ERROR;
    }

    return res;

}//APDCAM_GetConfig
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetFactoryConfig(ADT_HANDLE handle, APDFactory* apdFactory)
{
    ADT_RESULT res = ADT_OK;

    int index = GetIndex(handle);
    if (index < 0) return ADT_INVALID_HANDLE_ERROR;
    WORKING_SET &WorkingSet = g_WorkingSets[index];

    bool ret = GetFactoryData(WorkingSet.client, apdFactory);

    if(!ret)
        return ADT_FACTORY_SETUP_ERROR;

    return res;

}//APDCAM_GetFactoryConfig
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetFansSpeed(ADT_HANDLE handle, unsigned char fan1Speed, unsigned char fan2Speed, unsigned char fan3Speed)
{
    ADT_RESULT res = ADT_OK;

    int index = GetIndex(handle);
    if (index < 0) return ADT_INVALID_HANDLE_ERROR;
    WORKING_SET &WorkingSet = g_WorkingSets[index];

    bool retVal = SetFan1Speed(WorkingSet.client, fan1Speed);

    if(!retVal)
        return ADT_ERROR;

    retVal = SetFan2Speed(WorkingSet.client, fan2Speed);

    if(!retVal)
        return ADT_ERROR;

    retVal = SetFan3Speed(WorkingSet.client, fan3Speed);

    if(!retVal)
        return ADT_ERROR;


    return res;

}//APDCAM_SetFansSpeed
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_GetNetworkConfig(ADT_HANDLE handle, NetConfStr* netConfStr)
{
    ADT_RESULT res = ADT_OK;

    int index = GetIndex(handle);
    if (index < 0) return ADT_INVALID_HANDLE_ERROR;
    WORKING_SET &WorkingSet = g_WorkingSets[index];

    bool ret = GetNetworkConfiguration(WorkingSet.client, netConfStr);

    if(!ret)
        return ADT_FACTORY_SETUP_ERROR;

    return res;

}//APDCAM_GetNetworkConfig
#endif

#ifdef APD_VERSION
ADT_RESULT APDCAM_SetRingBufferSize(ADT_HANDLE handle, unsigned short bufferSize)
#else
ADT_RESULT ADC_SetRingBufferSize(ADT_HANDLE handle, unsigned short bufferSize)
#endif
{
	ADT_RESULT res = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;
	WORKING_SET &WorkingSet = g_WorkingSets[index];

	if(bufferSize < 0 || bufferSize > 1023)
		return ADT_PARAMETER_ERROR;

	bool ret = SetRingbufferSize(WorkingSet.client, bufferSize);

	if(!ret)
		return ADT_ERROR;


	return res;
}


#ifdef APD_VERSION
ADT_RESULT APDCAM_GetRingBufferSize(ADT_HANDLE handle, unsigned short* bufferSize)
#else
ADT_RESULT ADC_GetRingBufferSize(ADT_HANDLE handle, unsigned short* bufferSize)
#endif
{
	ADT_RESULT res = ADT_OK;

	int index = GetIndex(handle);
	if (index < 0) return ADT_INVALID_HANDLE_ERROR;
	WORKING_SET &WorkingSet = g_WorkingSets[index];

	bool ret = GetRingbufferSize(WorkingSet.client, bufferSize);

	if(!ret)
		return ADT_ERROR;


	return res;
}


#ifdef APD_VERSION
ADT_RESULT Measure_NonCalibrated(ADT_HANDLE handle, long long sampleCount, int signalFrequency)
{
    ADT_RESULT res = APDCAM_Arm(handle, MM_ONE_SHOT, sampleCount, CM_NONCALIBRATED);
	if (res != ADT_OK) 
	{
		printf("Error in ARM\n");
		return res;
	}
	res = APDCAM_Start(handle);
	if (res != ADT_OK) 
	{
		printf("Error in start\n");
		return res;
	}
	res = APDCAM_Wait(handle, 5000);
	if (res != ADT_OK) 
	{
		printf("Error reading data\n");
		return res;
	}

	return res;
}
#endif

