#ifndef DEVICE_H_
#define DEVICE_H_

#include "Board.h"
#include "ModuleLED.h"
#include "ModuleButton.h"

typedef struct Device Device_st;

struct Device{
	ModuleLED_st* moduleLed;
	ModuleButton_st* moduleButton;
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
