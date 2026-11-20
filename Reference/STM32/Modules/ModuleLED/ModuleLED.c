#include "ModuleLED.h"

void ModuleLED_Init(ModuleLED_st* moduleLed, Board_st* board){
	moduleLed->board = board;
}

void ModuleLED_Run(ModuleLED_st* moduleLed){
	LED_Run(&moduleLed->board->ledMain);
}

void ModuleLED_TurnOn(ModuleLED_st* moduleLed){
	LED_SetIndication(&moduleLed->board->ledMain, LED_INDICATION_ON, LED_INDICATION_INFINITE_REPEAT);
}

void ModuleLED_TurnOff(ModuleLED_st* moduleLed){
	LED_SetIndication(&moduleLed->board->ledMain, LED_INDICATION_OFF, LED_INDICATION_INFINITE_REPEAT);
}

LED_State_et ModuleLED_GetState(ModuleLED_st* moduleLed){
	return LED_INDICATION_OFF;
}

//-----Callbacks-----
void IndicationIsOverCallback_Led(LED_st* led, LED_Indications_et indication, uint8_t repeatCounter){
	Board_st* board = (Board_st*)led->descriptor.context;

	if(led == &board->ledMain){

	}
}
//-----Callbacks-----
