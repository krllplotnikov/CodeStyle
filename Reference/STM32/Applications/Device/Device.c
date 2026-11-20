#include "Device.h"

void Device_Init(Device_st* device, Board_st* board){
	device->board = board;
}

void Device_Run(Device_st* device){
//	ModuleButton_Run(device->descriptor.moduleButton);
	//ModuleLED_Run(device->descriptor.moduleLed);
}

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
