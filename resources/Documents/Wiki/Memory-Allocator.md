# Description

Soul Engine uses a custom allocator for dynamically allocated objects in order to improve allocation and access times. There is a base Allocator class that contains the basic implementation and helper functions to facilitate allocations. The actual logic of the allocator is defined in the LinearAllocator class, which defines how memory is allocated/deallocated. This design provides an easy way to modify the behavior of the manager if additional functionality is needed. The manager uses a linear allocation algorithm that keeps a pointer to the next available address in memory for the next allocation. Allocations are also block aligned to improve access times. Since there is only a single pointer for the next available space in memory, individual allocations are not supported; the entire buffer must be cleared to free space. Therefore, it is recommended it keep track of each buffer managed by the allocator and clear it once all objects residing in that space are not longer used.

# Usage
## Basic Usage
The allocator is initialized using the LinearAllocator constructor.

```c++
LinearAllocator la(512, buffer);
```
In the above example, 512 is the size in bytes of `buffer`, which is a void pointer to a space of memory allocated by `malloc()`.

To allocate memory, one would use the `allocate` function. When invoked, the function will advance the internal pointer to point to the next aligned block based in the size and alignment parameters. Here's an example:
```c++
std::int32_t* allocatedInt = la.allocate(sizeof(std::int32_t), alignof(std::int32_t));
```
In this example, we are allocating space for an integer. Once the internal representation of the allocator is updated, the `allocate()` function will return a pointer to the newly allocated space in memory.

To clear the memory, the `clear()` function must be used. This function resets the current position pointer to the start of the buffer, resets the amount of used memory to 0, and resets the number of allocations to 0.
```c++
la.clear();
```

There are a few different accessor functions that can be used to check the status of the allocator.

To check the amount of memory currently in use, use `getUsedMem()`:
```c++
size_t used = la.getUsedMem();
```
To check the capacity, use `getCapacity()`:
```c++
size_t cap = la.getCapacity();
```
The start address of the buffer used by the memory manager can checked using `getStart()`:
```c++
void* begin = la.getStart();
```
Lastly, to obtain the number of allocations, use `getNumAllocs()`;
```c++
size_t numAllocs = la.getNumAllocs();
```

## Useful Helper Functions
Allocator.h contains a number of useful template functions located within the `allocator` namespace to facilitate allocations for a variety of types. `allocator::allocateNew<T>()` has two different parameter options. The first accepts only one parameter, which is a reference to the allocator. It returns a pointer to the newly allocated object of the specified type. The function also accepts a second parameter, which is a constant reference to the object to put into the newly allocated space. It will return a pointer to the object in the newly allocated space. Here is an example:
```c++
//allocate space for a new std::int32_t using the LinearAllocator la
std::int32_t* num1 = allocator::allocateNew<std::int32_t>(&la);

//allocate space for the integer 5 using la
std::int32_t* num2 = allocator::allocateNew<std::int32_t>(&la, 5);
```
The `allocator` namespace also contains a helper function for allocating arrays. Using a reference to the allocator and the size of the array, the function will allocate an appropriate amount of space for an array with the specified type. It will also allocate a small amount of space at the beginning of the array to contain a header with the array length; although it is not used for the linear allocation algorithm, it will be useful for other algorithm where individual allocation is permitted. Here is an example:
```c++
std::int32_t* arr = allocator::allocateNewArr<std::int32_t>(&la, 5) //allocate space for an array of five ints
```
