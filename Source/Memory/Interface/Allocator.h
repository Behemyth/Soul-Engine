#pragma once
#include <cstdint>
#include <cstddef>

/*This class will serve as the base class for the engine's linear allocator.
  It contains public allocate and deallocate functions which will manage the memory.
  These must be implemented by the child linear allocator class itself.*/
class Allocator {
	/*Function declarations*/
	public:
		Allocator(size_t size, void* start);
		virtual ~Allocator();
		virtual void* allocate(size_t size, uint8_t alignment = 4) = 0;
		virtual void* deallocate(void* block) = 0;
		void* getStart() {return _start;}
		size_t getCapacity() {return _capacity;}
		size_t getUsedMem() {return _usedMem;}
		size_t getNumAllocs() {return _numAllocs;}

	/*Member variables*/
	protected:
		void* _start;
		size_t _capacity;
		size_t _usedMem;
		size_t _numAllocs;


};
namespace Memory {
	/*Allocates space for a new object of type T using the specified allocator
	 Arguments: allocator - a reference to the allocator to use
	 Returns: a pointer to the newly allocated memory*/
	template<class T> T* allocateNew(Allocator& allocator) {
		return new (allocator.allocate(sizeof(T), __alignof(T))) T;
	}

	/*Allocates space for object t using the specified allocator
	Arguments: allocator - a reference to the allocator to use
			   t - a reference to the object to store in the newly allocated memory
	Returns: a pointer to the newly allocated memory*/
	template<class T> T* allocateNew(Allocator& allocator, const T& t) {
		return new (allocator.allocate(sizeof(T), __alignof(T))) T(t);
	}

	/*Allocates space for an array of the specified type and length
	 Arguments: allocator - a reference to the allocator to use
				length - the amount of elements the array will hold
	 Returns: a pointer to the newly allocated array*/
	template<class T> T* allocateNewArr(Allocator& allocator, size_t length) {
		/*Determine the size of the header*/
		size_t headerSize = sizeof(size_t)/sizeof(T);
		if (sizeof(size_t) % sizeof(T) > 0) {
			headerSize += 1;
		}
		/*Allocate space for array with header*/
		T* arr = (T*) allocator.allocate(sizeof(T)*(length + headerSize), __alignof(T)) + headerSize;
		*(((size_t*)arr) - 1) = length; //set header size
		for (size_t i = 0; i < length; ++i) {
			new (arr + i) T;
		}
		return arr;
	}

	/*Deallocate the specified item
	 Arguments: allocator - the allocator to use
				object - the object to deallocate
	 Returns: none*/
	template<class T> void deallocateItem(Allocator& allocator, T& object) {
		object.~T();
		allocator.deallocate(&object);
	}

	/*Deallocate the specified array
	 Arguments: allocator - the allocator to use
				arr - the array to deallocate
	 Returns: none 
	*/
	template<class T> void deallocateArr(Allocator& allocator, T* arr) {
		size_t length = *(((size_t*)arr) - 1); //get the number of elements in the array
		for (size_t i = 0; i < length; ++i) {
			*(arr + i).~T();
		}
		/* Size of the header */
		size_t headerSize = sizeof(size_t) / sizeof(T);
		if (sizeof(size_t) % sizeof(T) > 0) {
			headerSize += 1;
		}

		/*
		 *    Constructor.
		 *    @param	headerSize	Size of the header.
		 */

		allocator.deallocate(arr - headerSize);
	}
};
/*Tools to help make aligned allocations*/
/* . */
namespace alloc_tools {
	/*Get next alinged address
	  arguments: address - the base address
				 alignment - the alignment for data to be stored in the address
	  returns: a pointer the next aligned address*/

	/*
	 *    Aligns.
	 *    @param [in,out]	address  	If non-null, the address.
	 *    @param 		 	alignment	The alignment.
	 *    @return	Null if it fails, else a pointer to a void.
	 */

	inline void* align(void* address, uint8_t alignment) {
		return (void*) ((reinterpret_cast<uintptr_t>(address) + alignment - 1) & ~(alignment - 1));
	}

	/*Get the number of bytes required to make the address block aligned
	  Arguments: address - the base address
				 alignment - the alignment for data to be stored
				 headerSize - optional, used when allocating arrays
	  Returns: an unsigned, 8 bit integer indicating the number of bytes needed to align the address*/

	/*
	 *    Gets an adjust.
	 *    @param [in,out]	address   	If non-null, the address.
	 *    @param 		 	alignment 	The alignment.
	 *    @param 		 	headerSize	(Optional) Size of the header.
	 *    @return	The adjust.
	 */

	inline uint8_t getAdjust(void* address, uint8_t alignment, uint8_t headerSize = 0) {
		/*Get the adjustment by masking the address*/
		uint8_t adjustment = alignment - (reinterpret_cast<uintptr_t>(address) & (alignment - 1));
		if (adjustment == alignment) {
			adjustment = 0; //address is already aligned
		}
		/*Determine adjustment if array header is being used*/
		if (adjustment < headerSize) {
			headerSize -= adjustment;
			adjustment += alignment * (headerSize / alignment);
			if (headerSize % alignment > 0) {
				adjustment += alignment;
			}
		}
		return adjustment;
	}
}
