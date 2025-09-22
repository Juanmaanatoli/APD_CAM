#include "Helpers.h"

#if 0
bool IsHexDigit(_TCHAR c)
{
	if (_TCHAR('0') <= c && c <= _TCHAR('9')) return true;
	if (_TCHAR('a') <= c && c <= _TCHAR('f')) return true;
	if (_TCHAR('A') <= c && c <= _TCHAR('F')) return true;
	return false;
}

unsigned int GetValue(_TCHAR c)
{
	if (_TCHAR('0') <= c && c <= _TCHAR('9')) return c - _TCHAR('0');
	if (_TCHAR('a') <= c && c <= _TCHAR('f')) return c - _TCHAR('a') + 10;
	if (_TCHAR('A') <= c && c <= _TCHAR('F')) return c - _TCHAR('A') + 10;
	return 0;
}

UINT32 hstoul(_TCHAR *hs)
{
	if (_tcslen(hs) > 8) return 0xFFFFFFFF;

	UINT32 ul = 0;
	while (IsHexDigit(*hs))
	{
		ul = 16*ul + GetValue(*hs);
		hs++;
	}
	return ul;
}
#endif

// Returns the bit position of n-th 1. Eg: 0 for (01101001, 0), 3 for (01101001, 1) & 5 for (01101001, 2) 
int GetBitPosition(unsigned char uc, int n)
{
	int c = -1;
	for (int i = 0; i < 8; i++)
	{
		if ((uc & 0x01) !=0) c++;
		uc = uc >> 1;
		if (c == n) return i;
	}
	return -1;
}

int GetBitCount(unsigned char uc)
{
	int count = 0;
	for (int i = 0; i < 8; i++)
	{
		if ((uc & 0x01) !=0) count++;
		uc = uc >> 1;
	}
	return count;
}

/*
E.g. For 01101001

map[0] = 0
map[1] = 3
map[2] = 5
map[3] = 6
map[4] = n/a
map[5] = n/a 
map[6] = n/a
map[7] = n/a

no = 4;
*/
void GetMap(unsigned char mask, int *map, int *count)
{
	for (int i = 0; i < 8; i++) map[i] = -1;
	*count = 0;
	for (int i = 0; i < 8; i++)
	{
		if ((mask & 0x01) !=0)
		{
			map[*count] = i;
			(*count)++;
		}
		mask = mask >> 1;
	}
}

int GetBlockSize(int channels, int bitsPerSample, int *pPaddingBits)
{
	// Calculates sample block size
	int sampleBits = channels * bitsPerSample;
	int paddingBits = (8 - (sampleBits % 8)) % 8;
	if (pPaddingBits) *pPaddingBits = paddingBits;
	sampleBits = sampleBits + paddingBits;
	int sampleBytes = sampleBits / 8;
	return sampleBytes;
}


bool Filter_6(char *filter_str, char *match_str)
{
	if (filter_str == NULL || match_str == NULL) return true;
	for (int i = 0; i < 6; i++)
	{
		if (*filter_str == '*') return true;
		if (*filter_str != *match_str) return false;
		filter_str++;
		match_str++;
	}
	return true;
}

bool IsAphaNum(char c)
{
	if ('0' <= c && c <= '9') return true;
	if ('A' <= c && c <= 'Z') return true;
	if ('a' <= c && c <= 'z') return true;
	return false;
}

