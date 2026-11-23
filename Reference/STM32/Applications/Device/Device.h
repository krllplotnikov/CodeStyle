#ifndef DEVICE_H_
#define DEVICE_H_

#include <stddef.h>
#include <stdint.h>
#include "ModuleButton.h"
#include "ModuleLED.h"

typedef struct Device_Descriptor Device_Descriptor_st;
typedef struct Device            Device_st;

struct Device_Descriptor{
	Button_st* button;
	LED_st*    led;
};

struct Device{
	Device_Descriptor_st descriptor;

	ModuleButton_st      moduleButton;
	ModuleLED_st         moduleLed;
};

#ifdef __cplusplus
extern "C" {
#endif

void Device_Init(Device_st* _device, Device_Descriptor_st* _descriptor);
void Device_Run(Device_st* _device);

#ifdef __cplusplus
}
#endif

#endif
