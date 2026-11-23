#ifndef MODULELED_H_
#define MODULELED_H_

#include <stddef.h>
#include <stdint.h>
#include "LED.h"

typedef struct ModuleLED_Descriptor ModuleLED_Descriptor_st;
typedef struct ModuleLED            ModuleLED_st;

struct ModuleLED_Descriptor{
	LED_st* led;
};

struct ModuleLED{
	ModuleLED_Descriptor_st descriptor;
};

#ifdef __cplusplus
extern "C" {
#endif

void    ModuleLED_Init(ModuleLED_st* _moduleLed, ModuleLED_Descriptor_st* _descriptor);
void    ModuleLED_Run(ModuleLED_st* _moduleLed);

void    ModuleLED_TurnOn(ModuleLED_st* _moduleLed);
void    ModuleLED_TurnOff(ModuleLED_st* _moduleLed);
uint8_t ModuleLED_IsOn(ModuleLED_st* _moduleLed);

#ifdef __cplusplus
}
#endif

#endif
