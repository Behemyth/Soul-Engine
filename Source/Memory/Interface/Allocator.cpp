#include "Allocator.h"

/*The constructor for the allocator
Arguments: size - the amount of memory to use
start - the begining of the allocated block*/

/*
 *    Constructor.
 *    @param 		 	size 	The size.
 *    @param [in,out]	start	If non-null, the start.
 */

Allocator::Allocator(size_t size, void* start) {
	_start = start;
	_capacity = size;
	_usedMem = 0;
	_numAllocs = 0;
}

/*Destructor for the allocator*/
/* Destructor. */
Allocator::~Allocator() {
	_start = nullptr;
	_capacity = 0;
}