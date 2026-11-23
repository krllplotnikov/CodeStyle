#ifndef MODULEBUTTON_H_
#define MODULEBUTTON_H_

#include <stddef.h>
#include <stdint.h>
#include "Button.h"

#define MODULE_BUTTON_HOLD_COUNT 4

typedef enum ModuleButton_Event ModuleButton_Event_et;

typedef struct ModuleButton_Descriptor ModuleButton_Descriptor_st;
typedef struct ModuleButton            ModuleButton_st;

typedef void (*ModuleButton_EventCallback)(ModuleButton_st* _moduleButton, ModuleButton_Event_et _event);

enum ModuleButton_Event{
	MODULE_BUTTON_EVENT_CLICK,
	MODULE_BUTTON_EVENT_HOLD
};

struct ModuleButton_Descriptor{
	Button_st*                 button;
	ModuleButton_EventCallback eventCallback;
	void*                      context;
};

struct ModuleButton{
	ModuleButton_Descriptor_st descriptor;
};

#ifdef __cplusplus
extern "C" {
#endif

void ModuleButton_Init(ModuleButton_st* _moduleButton, ModuleButton_Descriptor_st* _descriptor);
void ModuleButton_Run(ModuleButton_st* _moduleButton);

#ifdef __cplusplus
}
#endif

#endif
