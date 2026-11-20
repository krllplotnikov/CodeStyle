#include "ModuleLED.h"

void ModuleLED_Init(ModuleLED_st* moduleLed){
	//moduleLed->board = board;
	//moduleLed->led = &board->ledMain;
}

void ModuleLED_Run(ModuleLED_st* moduleLed){
	//LED_Run(&moduleLed->board->ledMain);
	LED_Run(moduleLed->led);
}

void ModuleLED_TurnOn(ModuleLED_st* moduleLed){
	//LED_SetIndication(&moduleLed->board->ledMain, LED_INDICATION_ON, LED_INDICATION_INFINITE_REPEAT);
	LED_SetIndication(moduleLed->led, LED_INDICATION_ON, LED_INDICATION_INFINITE_REPEAT);
}

void ModuleLED_TurnOff(ModuleLED_st* moduleLed){
	//LED_SetIndication(&moduleLed->board->ledMain, LED_INDICATION_OFF, LED_INDICATION_INFINITE_REPEAT);
	LED_SetIndication(moduleLed->led, LED_INDICATION_OFF, LED_INDICATION_INFINITE_REPEAT);
}

LED_State_et ModuleLED_GetState(ModuleLED_st* moduleLed){
	return LED_INDICATION_OFF;
}
