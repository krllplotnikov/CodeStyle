#include "ModuleButton.h"

void ModuleButton_Init(ModuleButton_st* moduleButton, Board_st* board){
	moduleButton->board = board;
	moduleButton->button = &board->buttonMain;
}

void ModuleButton_Run(ModuleButton_st* moduleButton){
	Button_Run(&moduleButton->board->buttonMain);
	//Button_Run(moduleButton->button);
}



//-----Weak-----
__attribute__((weak)) void ModuleButton_Click(ModuleButton_st* moduleButton){}
//-----Weak-----

