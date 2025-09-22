#pragma once
#ifndef __LOWLEVELFUNCTIONS_H__

#define __LOWLEVELFUNCTIONS_H__

#include "CamClient.h"


bool WritePDI(CAPDClient *client, unsigned char address, unsigned long subaddress, unsigned char* buffer, int noofbytes, unsigned long ip_address_h = 0, unsigned short ip_port_h = 0, int timeout = 50);
bool WritePDISafe(CAPDClient *client, unsigned char address, unsigned long subaddress, unsigned char* buffer, int noofbytes, unsigned long ip_address_h = 0, unsigned short ip_port_h = 0, int timeout = 50);
bool ReadPDI(CAPDClient *client, unsigned char address, unsigned long subaddress, unsigned char* buffer, int noofbytes, unsigned long ip_address_h = 0, unsigned short ip_port_h = 0, int timeout = 50);
bool SetupTS(CAPDClient *client, unsigned char channel, int packetSize, unsigned short port_h);
bool SetupAllTS(CAPDClient *client, int packetsize_1, int packetsize_2, int packetsize_3, int packetsize_4, 
				 unsigned short port_h_1 = 57000, unsigned short port_h_2 = 57001, unsigned short port_h_3 = 57002, unsigned short port_h_4 = 57003);
bool ShutupTS(CAPDClient *client, unsigned char channel);
bool ShutupAllTS(CAPDClient *client);
bool SetIP(CAPDClient *client, unsigned long ip_h);
bool SendAck(CAPDClient *client, unsigned char *buffer, unsigned char ackType=1);


#endif //__LOWLEVELFUNCTIONS_H__
