#ifndef __GECCOMMANDS_H__

#define __GECCOMMANDS_H__

#include <arpa/inet.h>
#include <string.h>

#include "TypeDefs.h"
#include "helper.h"

#include <stdlib.h>

/*
	History

	Ver: V1.2 Date: 2011.Jan.25 Who: STA
	CW_FRAME_V2 added (Three byte continuity counter)
*/



typedef unsigned short _BEUSHORT;
typedef unsigned int _BEULONG;

/*

4.3.3 DDToIP protocol
*/

#define COMPANY "KFKI-RMKI"

// ************************** Operation Codes
// General instructions:
#define OP_NOP 0x00
#define OP_RESET 0x01
#define OP_WAIT 0x02
#define OP_SENDACK 0x03
#define OP_LASTINSTRUCTION 0x04
#define OP_LOCK 0x05
#define OP_UNLOCK 0x06
#define OP_SETUSERDATA 0x07

//Configuration instructions:
#define OP_SETSERIAL 0x10
#define OP_SETTYPE 0x11
#define OP_SETUSERTEXT 0x12
#define OP_SETCONFIG 0x13

// Network instructions:
#define OP_SETGATEWAY 0x20
#define OP_SETNETMASK 0x21
#define OP_SETPMAC 0x22
#define OP_SETSMAC 0x23
#define OP_SETIP 0x24
#define OP_SETARPADVERTISE 0x25
#define OP_SETTRAP 0x26

// Transport Stream Input instructions:
#define OP_SENDTS 0x30
#define OP_DONTSENDTS 0x31

// Transport Stream Output instructions:
#define OP_SETNCO 0x40
#define OP_DONTRECEIVETS 0x41
#define OP_RECEIVETS 0x42
#define OP_SETTSPORTINTERVAL 0x43
#define OP_SETIGMPREPORTTIME 0x44

// Selector instructions:
#define OP_SETSELECTOR 0x50
#define OP_SETSELECTORBIT 0x51
#define OP_CLEARSELECTORBIT 0x52
#define OP_SAVESELECTOR 0x53

// GPO instructions:
#define OP_SETGPO 0x60
#define OP_SETGPOBIT 0x61
#define OP_CLEARGPOBIT 0x62
#define OP_SAVEGPO 0x63

// Parallel Data Interface instructions:
#define OP_WRITEPDI 0x70
#define OP_READPDI 0x71

// Serial Data Interface instructions:
#define OP_WRITESDI 0x80
#define OP_READSDI 0x81
#define OP_ENABLEON 0x82
#define OP_ENABLEOFF 0x83
#define OP_LOADDATA 0x84
#define OP_SINGLELOADDATA 0x85
#define OP_READDATA 0x86

// IIC instructions:
#define OP_SETIICCLOCK 0x90
#define OP_WRITEIIC 0x91
#define OP_READIIC 0x92

// HTTP instructions (future release):
#define OP_LOADHTTPEEPROM 0xA0
#define OP_READHTTPEEPROM 0xA1

// ************************** Answer Codes
#define AN_ACK 0xF0
#define AN_IRQ 0xF1
#define AN_PDIDATA 0xF2
#define AN_SDIDATA 0xF3
#define AN_IICDATA 0xF4
#define AN_TRAP 0xF5


class CReplyCmdSet
{
public:
	static bool In(int cmd)
	{
		for (int i = 0; i < 4; i++)
		{
			if (cmd == g_CmdSet[i]) return true;
		}
		return false;
	}
private:
	static int g_CmdSet[4];
};

#pragma pack(push)
#pragma pack(1)

struct CW_FRAME
{
	unsigned char PCRLow; // Internal control. Alters 0x00 & 0x10;
    unsigned int PCR;
	unsigned short empty; // Allways 0
	unsigned char continuityCounter;
    unsigned int senderIP; // Big endian
	unsigned short Type;
	unsigned short SerialNumber;
	unsigned char empty2;
	unsigned char Options;
	unsigned char Reserved[8];
	char Signature[6]; // "CW-NET" signature;
};

struct CW_FRAME_V2
{
	unsigned char PCRLow; // Internal control. Alters 0x00 & 0x10;
    unsigned int PCR; // Little endian
	_BEUSHORT continuityCounter_H; // Big endian
	unsigned char continuityCounter_L;
    unsigned int senderIP; // Big endian
	unsigned short Type;
	unsigned short SerialNumber;
	unsigned char empty2;
	unsigned char Options;
	unsigned char Reserved[8];
	char Signature[6]; // "CW-NET" signature;
};

struct DDTOIPHEADER
{
	DDTOIPHEADER();
	unsigned char DDToIp[6];
	char UserText[15];
	unsigned char V;

	void Prepare(char *_usertext);
	bool Validate();
};

struct INSTRUCTIONHEADER
{
	unsigned char opCode;
	_BEUSHORT length;
	unsigned char GetOpCode() { return opCode;};
	inline unsigned short GetDataLength() { return (ntohs(length)); };
	inline unsigned short GetInstructionLength() { return (GetDataLength() + sizeof(INSTRUCTIONHEADER)); };
	inline unsigned short GetCmdLength() { return (GetInstructionLength() + sizeof(DDTOIPHEADER)); };
};

struct SENDACK_I : INSTRUCTIONHEADER
{
	unsigned char ACKType; // 0 v. 1.
	void Prepare(unsigned char _type)
	{
		opCode = OP_SENDACK;
		length = htons(1);
		ACKType = _type;
	}
};

struct WRITEPDI_I : INSTRUCTIONHEADER
{
	unsigned char Addr;
	_BEULONG SubAddr;
	unsigned char data[1024];

	void Prepare(unsigned char _addr, unsigned int _subaddr, unsigned char *_data, int _length)
	{
        _length = std::min(_length, 1024);
		opCode = OP_WRITEPDI;
		length = htons(_length + 5);
		Addr = _addr;
		SubAddr = htonl(_subaddr);
		memcpy_s(data, 1024, _data, _length); 
	}
};

struct READPDI_I : INSTRUCTIONHEADER
{
	unsigned char Addr;
	_BEULONG SubAddr;
	_BEUSHORT NOB;

	void Prepare(unsigned char _addr, unsigned int _subaddr, unsigned short _nob)
	{
		opCode = OP_READPDI;
		length = htons(7);
		Addr = _addr;
		SubAddr = htonl(_subaddr);
		NOB = htons(_nob);
	}
};

struct SETIP_I : INSTRUCTIONHEADER
{
	char reserved[12];
	_BEULONG IPAddr;

	void Prepare(unsigned long _ip_h)
	{
		memset(this, 0, sizeof(SETIP_I));
		opCode = OP_SETIP;
		length = htons(0x10);
		IPAddr = htonl(_ip_h);
	}
};


struct SENDTS_I : INSTRUCTIONHEADER
{
	unsigned char channel;
	struct
	{
		unsigned char DV : 1;
		unsigned char FRMT : 1;
		unsigned char ALWY : 1;
		unsigned char reserved1 : 5;
	};
	union
	{
		_BEUSHORT CWNetSize;
		_BEUSHORT IPTVMode;
	};
	struct
	{
		unsigned char DESTMODE : 4;
		unsigned char reserved2 : 3;
		unsigned char IPF : 1;
	};
	unsigned char MAC[6];
	union
	{
		unsigned char IP[16];
		_BEULONG IPv4;
	};
	_BEUSHORT Port;

	void Prepare(unsigned char _channel, unsigned int _packetSize, unsigned short _port)
	{
		memset(this, 0, sizeof(SENDTS_I));
		opCode = OP_SENDTS;
		length = htons(0x1D);
		channel = _channel;
		DV = 0; // disable Data valid pin
		FRMT = 0; // CW-Net format
		ALWY = 0; // Sen the TS
		CWNetSize = htons(_packetSize);
		IPF = 0; // IPv4 protocol
		DESTMODE = 0; // Send to me.
		IPv4 = 0;
		Port = htons(_port);
	}
};

struct DONTSENDTS_I : INSTRUCTIONHEADER
{
	unsigned char channel;
	void Prepare(unsigned char _channel)
	{
		opCode = OP_DONTSENDTS;
		length = htons(1);
		channel = _channel;
	}
};

struct GECCOMMANDBASE
{
	DDTOIPHEADER header;
};


struct GECCOMMAND : GECCOMMANDBASE 
{
};


struct GENERAL_MESSAGE  : GECCOMMAND
{
	INSTRUCTIONHEADER instructionheader;
	bool Validate() { return header.Validate(); }
	int GetOpCode() { return (int)instructionheader.GetOpCode(); };
	int GetDataLength() { return (int)instructionheader.GetDataLength(); };
	int GetInstructionLength() { return (int)instructionheader.GetInstructionLength(); };
	int GetCommandLength() { return (int)instructionheader.GetCmdLength(); }
};

struct BULKCMD : GECCOMMANDBASE
{
	BULKCMD() : length(0) {};
	unsigned char instructions[1024];
	void Add(GECCOMMAND &command)
	{
		GENERAL_MESSAGE &cmd = reinterpret_cast<GENERAL_MESSAGE&>(command);
		int len = cmd.GetInstructionLength();
		if (length + len < 1024)
		{
			memcpy(&instructions[length], &cmd.instructionheader, len);
			length += len;
		}
	}
	void Reset() { length = 0;};
	int GetCommandLength() { return length + sizeof(DDTOIPHEADER);};

private:
	int length;
};

struct SENDACK : GECCOMMAND
{
	SENDACK_I instruction;
};

struct WRITEPDI : GECCOMMAND
{
	WRITEPDI_I instruction;
};

struct READPDI : GECCOMMAND
{
	READPDI_I instruction;
};

struct SETIP : GECCOMMAND
{
	SETIP_I instruction;
};

struct SENDTS : GECCOMMAND
{
	SENDTS_I instruction;
};

struct DONTSENDTS : GECCOMMAND
{
	DONTSENDTS_I instruction;
};

#pragma pack(pop)

#endif  /* __GECCOMMANDS_H__ */
