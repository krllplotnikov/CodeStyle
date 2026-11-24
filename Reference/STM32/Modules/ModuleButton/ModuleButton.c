#include "ModuleButton.h"

static void buttonEvent(Button_st* _button, Button_Event_et _event, uint8_t _counter);
static void sendEvent(ModuleButton_st* _moduleButton, ModuleButton_Event_et _event);

void ModuleButton_Init(ModuleButton_st* _moduleButton, ModuleButton_Descriptor_st* _descriptor){
	_moduleButton->descriptor = *_descriptor;

	Button_SetEventCallback(_moduleButton->descriptor.button, buttonEvent, _moduleButton);
}

void ModuleButton_Run(ModuleButton_st* _moduleButton){
	Button_Run(_moduleButton->descriptor.button);
}

static void buttonEvent(Button_st* _button, Button_Event_et _event, uint8_t _counter){
	ModuleButton_st* moduleButton = (ModuleButton_st*)_button->descriptor.context;

	switch(_event){
	case BUTTON_EVENT_CLICK:{
		if(_counter == 1){
			sendEvent(moduleButton, MODULE_BUTTON_EVENT_CLICK);
		}
		break;
	}

	case BUTTON_EVENT_HOLD:{
		if(_counter == MODULE_BUTTON_HOLD_COUNT){
			sendEvent(moduleButton, MODULE_BUTTON_EVENT_HOLD);
		}
		break;
	}

	default: break;
	}
}

static void sendEvent(ModuleButton_st* _moduleButton, ModuleButton_Event_et _event){
	if(_moduleButton->descriptor.eventCallback != NULL){
		_moduleButton->descriptor.eventCallback(_moduleButton, _event);
	}
}
