#include "Character.h"

/* Resets the callback information. */
void Character::ResetCallbackInfo(){
	for (int i = 0; i < CallbackInfo.size(); i++){
		(*CallbackInfo[i])=false;
	}
}