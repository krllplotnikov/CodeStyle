#ifndef BOARD_H_
#define BOARD_H_

#include "LED.h"
#include "Button.h"

typedef struct{
	LED_st ledMain;
	Button_st buttonMain;

	LED_Descriptor_st ledDescriptor;
	ButtonDescriptor_st buttonDescriptor;
}Board_st;

#ifdef __cplusplus
extern "C" {
#endif

void Board_Init();

void EventCallback_Button(Button_st* button, ButtonEvent_et event, uint8_t counter);
void IndicationIsOverCallback_Led(LED_st* led, LED_Indications_et indication, uint8_t repeatCounter);

#ifdef __cplusplus
}
#endif

#endif
