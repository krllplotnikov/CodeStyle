#include "ModuleButton.h"

//void ModuleButton_Init(ModuleButton_st* moduleButton, ModuleButton_Descriptor_st* descriptor){
//	moduleButton->descriptor = *descriptor;
//
//	Button_st* button = (Button_st*)moduleButton->descriptor.context;
//
//	Button_Init(button, &button->descriptor);
//}
//
//void ModuleButton_Run(ModuleButton_st* moduleButton){
//	Button_Run(moduleButton->descriptor.button);
//}
//
////-----Callbacks-----
//void EventCallback_Button(Button_st* button, ButtonEvent_et event, uint8_t counter){
//	ModuleButton_st* moduleButton = (ModuleButton_st*)button->descriptor.context;
//
//	if(event == BUTTON_EVENT_CLICK){
//		if(counter == 1){
//			ModuleButton_Click(moduleButton);
//		}
//	}
//
//	if(event == BUTTON_EVENT_HOLD){
//		if(counter == 4){
//			ModuleButton_Hold(moduleButton);
//		}
//	}
//}
////-----Callbacks-----
//
////-----Weak-----
//__attribute__((weak)) void ModuleButton_Click(ModuleButton_st* moduleButton){}
//__attribute__((weak)) void ModuleButton_Hold(ModuleButton_st* moduleButton){}
////-----Weak-----
//
//void EventCallback_Button(Button_st* button, ButtonEvent_et event, uint8_t counter){
//	if(button == &board.buttonMain){
//
//	}
//}
