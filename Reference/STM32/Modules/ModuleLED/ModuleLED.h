#ifndef MODULELED_H_
#define MODULELED_H_

#include <stddef.h>
#include <stdint.h>
#include "Board.h"

typedef struct ModuleLED ModuleLED_st;

struct ModuleLED{
	Board_st* board;
};

#ifdef __cplusplus
extern "C" {
#endif

void ModuleLED_Init(ModuleLED_st* moduleLed, Board_st* board);
void ModuleLED_Run(ModuleLED_st* moduleLed);

void ModuleLED_TurnOn(ModuleLED_st* moduleLed);
void ModuleLED_TurnOff(ModuleLED_st* moduleLed);

LED_State_et ModuleLED_GetState(ModuleLED_st* moduleLed);

#ifdef __cplusplus
}
#endif

#endif
