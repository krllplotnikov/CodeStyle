#include "Device.h"

void Device_Init(Device_st* device, Board_st* board){
	device->board = board;

	device->moduleLed->led = &board->ledMain;

	ModuleLED_Init(device->moduleLed);
	ModuleButton_Init(device->moduleButton, board);


}

void Device_Run(Device_st* device){
	ModuleLED_Run(device->moduleLed);
	ModuleButton_Run(device->moduleButton);
}

//-----Callbacks-----
void EventCallback_Button(Button_st* button, ButtonEvent_et event, uint8_t counter){
	Board_st* board = (Board_st*)button->descriptor.context;

	if(button == &board->buttonMain){
		if(event == BUTTON_EVENT_CLICK){
			if(counter == 1){
				//ModuleButton_Click(device->moduleButton);
			}
		}

		if(event == BUTTON_EVENT_HOLD){
			if(counter == 4){
				//ModuleButton_Hold(moduleButton);
			}
		}
	}
}
//-----Callbacks-----

//-----Callbacks-----
void IndicationIsOverCallback_Led(LED_st* led, LED_Indications_et indication, uint8_t repeatCounter){
	Board_st* board = (Board_st*)led->descriptor.context;

	if(led == &board->ledMain){

	}
}
//-----Callbacks-----

//void ModuleButton_Click(ModuleButton_st* moduleButton){
//	Device_st* device = (Device_st*)moduleButton->descriptor.context;
//
//	ModuleLED_TurnOn(device->descriptor.moduleLed);
//}
//
//void ModuleButton_Hold(ModuleButton_st* moduleButton){
//	Device_st* device = (Device_st*)moduleButton->descriptor.context;
//
//	ModuleLED_TurnOff(device->descriptor.moduleLed);
//}
