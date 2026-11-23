#include "Device.h"

static void moduleButtonEvent(ModuleButton_st* _moduleButton, ModuleButton_Event_et _event);

void Device_Init(Device_st* _device, Device_Descriptor_st* _descriptor){
	_device->descriptor = *_descriptor;

	ModuleLED_Descriptor_st moduleLedDescriptor = {
		.led = _descriptor->led,
	};
	ModuleLED_Init(&_device->moduleLed, &moduleLedDescriptor);

	ModuleButton_Descriptor_st moduleButtonDescriptor = {
		.button        = _descriptor->button,
		.eventCallback = moduleButtonEvent,
		.context       = _device,
	};
	ModuleButton_Init(&_device->moduleButton, &moduleButtonDescriptor);
}

void Device_Run(Device_st* _device){
	ModuleButton_Run(&_device->moduleButton);
	ModuleLED_Run(&_device->moduleLed);
}

static void moduleButtonEvent(ModuleButton_st* _moduleButton, ModuleButton_Event_et _event){
	Device_st* device = (Device_st*)_moduleButton->descriptor.context;

	switch(_event){
	case MODULE_BUTTON_EVENT_CLICK:{
		ModuleLED_TurnOn(&device->moduleLed);
		break;
	}

	case MODULE_BUTTON_EVENT_HOLD:{
		ModuleLED_TurnOff(&device->moduleLed);
		break;
	}

	default: break;
	}
}
