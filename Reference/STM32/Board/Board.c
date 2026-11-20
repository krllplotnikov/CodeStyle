#include "main.h"
#include "Board.h"

//-----Function prototypes-----
ButtonState_et GetState_Button(Button_st* button);
uint32_t GetTick_Button();

uint32_t GetTick_Led();
void SetState_Led(LED_st* led, LED_State_et state);
//-----Function prototypes-----



//-----Variables-----
Button_st buttonMain = {0};
LED_st ledMain = {0};
//-----Variables-----



//-----Descriptors-----
ButtonDescriptor_st buttonDescriptor = {
		.getTick = HAL_GetTick,
		.getState = GetState_Button,
		.eventCallback = EventCallback_Button
};

LED_Descriptor_st ledDescriptor = {
		.getTick = HAL_GetTick,
		.setState = SetState_Led,
		.indicationIsOverCallback = IndicationIsOverCallback_Led,
};
//-----Descriptors-----



//-----Function definitions-----
ButtonState_et GetState_Button(Button_st* button){
	Board_st* board = (Board_st*)button->descriptor.context;

	if(button == &board->buttonMain){
		return (ButtonState_et)HAL_GPIO_ReadPin(BUTTON_MAIN_GPIO_Port, BUTTON_MAIN_Pin);
	}

	return BUTTON_STATE_RELEASED;
}

void SetState_Led(LED_st* led, LED_State_et state){
	Board_st* board = (Board_st*)led->descriptor.context;

	if(led == &board->ledMain){
		if(state == LED_STATE_OFF) HAL_GPIO_WritePin(LED_MAIN_GPIO_Port, LED_MAIN_Pin, GPIO_PIN_RESET);
		else                       HAL_GPIO_WritePin(LED_MAIN_GPIO_Port, LED_MAIN_Pin, GPIO_PIN_SET);
	}
}
//-----Function definitions-----

void Board_Init(Board_st* board){
	board->ledMain = ledMain;
	board->buttonMain = buttonMain;

	board->ledDescriptor = ledDescriptor;
	board->buttonDescriptor = buttonDescriptor;

	LED_Init(&board->ledMain, &board->ledDescriptor);
	Button_Init(&board->buttonMain, &board->buttonDescriptor);
}
