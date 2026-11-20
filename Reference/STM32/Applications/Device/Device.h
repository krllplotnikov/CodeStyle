#ifndef DEVICE_H_
#define DEVICE_H_

#include "Board.h"

typedef struct Device Device_st;

struct Device{
	Board_st* board;
};

#ifdef __cplusplus
extern "C" {
#endif

void Device_Init(Device_st* device, Board_st* board);
void Device_Run(Device_st* device);

#ifdef __cplusplus
}
#endif

#endif
