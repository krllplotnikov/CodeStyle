#include "ModuleLED.h"

void ModuleLED_Init(ModuleLED_st* _moduleLed, ModuleLED_Descriptor_st* _descriptor){
	_moduleLed->descriptor = *_descriptor;
}

void ModuleLED_Run(ModuleLED_st* _moduleLed){
	LED_Run(_moduleLed->descriptor.led);
}

void ModuleLED_TurnOn(ModuleLED_st* _moduleLed){
	LED_SetIndication(_moduleLed->descriptor.led, LED_INDICATION_ON, LED_INDICATION_INFINITE_REPEAT);
}

void ModuleLED_TurnOff(ModuleLED_st* _moduleLed){
	LED_SetIndication(_moduleLed->descriptor.led, LED_INDICATION_OFF, LED_INDICATION_INFINITE_REPEAT);
}

uint8_t ModuleLED_IsOn(ModuleLED_st* _moduleLed){
	return LED_GetIndication(_moduleLed->descriptor.led) == LED_INDICATION_ON;
}
