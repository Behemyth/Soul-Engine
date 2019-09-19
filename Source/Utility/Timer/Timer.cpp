#include "Timer.h"

/* Default constructor. */
Timer::Timer() {
	Reset();
}
/* Destructor. */
Timer::~Timer() {

}

/* Resets this object. */
void Timer::Reset() {

	tickHold = Clock::now();

}

/*
 *    Gets the elapsed.
 *    @return	A double.
 */

double Timer::Elapsed() {

	std::chrono::duration<double, std::milli> ms = Clock::now() - tickHold;
	return ms.count();

}
