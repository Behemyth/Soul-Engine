#pragma once

#include "Core/Object/Object.h"
#include <vector>

/*
 *    Increment time.
 *    @param	parameter1	The first parameter.
 */

void IncrementTime(float);

/* A character. */
class Character : public Object{
public:

	/*
	 *    tag,distance,size.
	 *    @param	parameter1	The first parameter.
	 */

	virtual void Update(double)=0;
	virtual void Load()=0;



	/* Information describing the callback */
	std::vector<bool*> CallbackInfo;
	/* Resets the callback information. */
	void ResetCallbackInfo();
private:
	
};