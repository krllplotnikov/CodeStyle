#ifndef BOARD_H_
#define BOARD_H_

#include <stddef.h>
#include <stdint.h>
#include "Button.h"
#include "LED.h"

typedef struct Board Board_st;

struct Board{
	Button_st buttonMain;
	LED_st    ledMain;
};

#ifdef __cplusplus
extern "C" {
#endif

void Board_Init(Board_st* _board);

#ifdef __cplusplus
}
#endif

#endif
