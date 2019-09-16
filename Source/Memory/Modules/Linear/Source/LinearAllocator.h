#pragma once
#include "Allocator.h"
#include<cassert>

/*This class extends the base allocator class and provides
  the allocation logic for the linear allocator*/
/* A linear allocator. */
class LinearAllocator : public Allocator {
	/*Function declarations*/
	public:

		/*
		 *    Constructor.
		 *    @param 		 	size 	The size.
		 *    @param [in,out]	start	If non-null, the start.
		 */

		LinearAllocator(size_t size, void * start);
		/* Destructor. */
		~LinearAllocator();

		/*
		 *    Allocates.
		 *    @param	size	 	The size.
		 *    @param	alignment	The alignment.
		 *    @return	Null if it fails, else a pointer to a void.
		 */

		void* allocate(size_t size, uint8_t alignment);

		/*
		 *    Deallocates the given block.
		 *    @param [in,out]	block	If non-null, the block.
		 *    @return	Null if it fails, else a pointer to a void.
		 */

		void* deallocate(void* block);
		/* Clears this object to its blank/initial state. */
		void clear();

	private:
		/*The allocator does not need to be copied, so the copy constructor
		  and assignment operator should be private*/

		/*
		 *    Copy constructor.
		 *    @param	parameter1	The first parameter.
		 */

		LinearAllocator(const LinearAllocator&);

		/*
		 *    Assignment operator.
		 *    @param	parameter1	The first parameter.
		 *    @return	A shallow copy of this object.
		 */

		LinearAllocator& operator=(const LinearAllocator&);
		/* keep track of next free space */
		void* _currPos;
};