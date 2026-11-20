#ifndef MODULELED_H_
#define MODULELED_H_

#include <stddef.h>
#include <stdint.h>
#include "LED.h"

typedef struct ModuleLED ModuleLED_st;

struct ModuleLED{
	LED_st* led;
};

#ifdef __cplusplus
extern "C" {
#endif

void ModuleLED_Init(ModuleLED_st* moduleLed);
void ModuleLED_Run(ModuleLED_st* moduleLed);

void ModuleLED_TurnOn(ModuleLED_st* moduleLed);
void ModuleLED_TurnOff(ModuleLED_st* moduleLed);

LED_State_et ModuleLED_GetState(ModuleLED_st* moduleLed);

#ifdef __cplusplus
}
#endif

#endif
