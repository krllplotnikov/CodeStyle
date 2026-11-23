#include "Board.h"
#include "main.h"

static Board_st* boardInstance = NULL;

static uint32_t       getTick(void);
static ButtonState_et getButtonState(Button_st* _button);
static void           setLedState(LED_st* _led, LED_State_et _state);

void Board_Init(Board_st* _board){
	boardInstance = _board;

	ButtonDescriptor_st buttonDescriptor = {
		.getState      = getButtonState,
		.getTick       = getTick,
		.eventCallback = NULL,
		.context       = NULL,
	};
	Button_Init(&_board->buttonMain, &buttonDescriptor);

	LED_Descriptor_st ledDescriptor = {
		.setState                 = setLedState,
		.getTick                  = getTick,
		.indicationIsOverCallback = NULL,
		.context                  = NULL,
	};
	LED_Init(&_board->ledMain, &ledDescriptor);
}

static uint32_t getTick(void){
	return HAL_GetTick();
}

static ButtonState_et getButtonState(Button_st* _button){
	if(_button == &boardInstance->buttonMain){
		if(HAL_GPIO_ReadPin(BUTTON_MAIN_GPIO_Port, BUTTON_MAIN_Pin) == GPIO_PIN_SET){
			return BUTTON_STATE_PRESSED;
		}
	}

	return BUTTON_STATE_RELEASED;
}

static void setLedState(LED_st* _led, LED_State_et _state){
	if(_led == &boardInstance->ledMain){
		if(_state == LED_STATE_ON){
			HAL_GPIO_WritePin(LED_MAIN_GPIO_Port, LED_MAIN_Pin, GPIO_PIN_SET);
		}
		else{
			HAL_GPIO_WritePin(LED_MAIN_GPIO_Port, LED_MAIN_Pin, GPIO_PIN_RESET);
		}
	}
}
