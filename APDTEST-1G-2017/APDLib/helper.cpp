#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "helper.h"
#include "APDLib.h"


int memcpy_s(void *dest, size_t nelem, const void *src, size_t count)
{
	if (dest && src && nelem >= count)
	{
		memcpy(dest, src, count);
		return 0;
	}

	return EINVAL;
}

/*bool QueryPerformanceCounter(LARGE_INTEGER *li)
{
	li->QuadPart = 1;
	return true;
}

bool QueryPerformanceFrequency(LARGE_INTEGER *li)
{
	li->QuadPart = 1;
	return true;
}*/

void Sleep(int64_t msec)
{
	struct timespec ts = { msec / 1000, (msec % 1000) * 1000000};

	nanosleep(&ts, NULL);
}

int Save(ADT_HANDLE handle, int sampleCount)
{
	ADT_RESULT res;

    unsigned long long sampleCounts[4];
    unsigned long long sampleIndices[4];
	res = APDCAM_GetSampleInfo(handle, sampleCounts, sampleIndices);
	if (res != ADT_OK)
	{
		fprintf(stderr, "===Cannot get sampleinfo: %d===\n", res);
		return res;
	}

    short *buffers[32];
	res = APDCAM_GetBuffers(handle, buffers);
	if (res != ADT_OK)
	{
		fprintf(stderr, "===Cannot get buffers: %d===\n", res);
		return res;
	}

	for (int i = 0; i < 32; ++i)
	{
		char filename[32];
		snprintf(filename, sizeof(filename), "Channel%02d.dat", i);
		FILE *file = fopen(filename, "w");
		if (file == NULL)
		{
			int lerrno = errno;
			fprintf(stderr, "===Cannot create file '%s': %s===\n", filename, strerror(lerrno));
			continue;
		}

        fwrite(buffers[i], sizeof(short), sampleCount, file);

		fclose(file);
	}

	return res;
}
