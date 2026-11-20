#ifndef MODULEBUTTON_H_
#define MODULEBUTTON_H_

#include <stddef.h>
#include <stdint.h>
#include "Board.h"

typedef struct ModuleButton ModuleButton_st;

//typedef void (*ModuleButton_ClickCallback)(ModuleButton_st* moduleButton, uint8_t counter);

struct ModuleButton{
	Button_st* button;
	//ModuleButton_ClickCallback clickCallback;

	Board_st* board;
	void* context;
};

#ifdef __cplusplus
extern "C" {
#endif

void ModuleButton_Init(ModuleButton_st* moduleButton, Board_st* board);
void ModuleButton_Run(ModuleButton_st* moduleButton);

//-----Weak-----
void ModuleButton_Click(ModuleButton_st* moduleButton);
//-----Weak-----

#ifdef __cplusplus
}
#endif

#endif
