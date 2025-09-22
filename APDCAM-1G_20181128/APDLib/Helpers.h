#pragma once
#ifndef __HELPERS_H__

#define __HELPERS_H__

#include <stdlib.h>
//#include "TCHAR.h"

//UINT32 hstoul(_TCHAR *hs);
int GetBitPosition(unsigned char uc, int n);
int GetBitCount(unsigned char uc);
void GetMap(unsigned char mask, int *map, int *count);
int GetBlockSize(int channels, int bitsPerSample, int *pPaddingBits = NULL);
bool Filter_6(char *filter_str, char *match_str);
bool IsAphaNum(char c);

#endif  /* __HELPERS_H__ */
