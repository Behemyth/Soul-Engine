#pragma once
#include <map>
#include <list>
#include <string>

/* This class allows for used memory to be tagged on a per-module basis
   Note: Currently, freeing memory for a given module can leave the heap fragmented.
   There is currently no defragmentation procedure in place, but one can be added later */

class TaggedHeap {
	private:
		/*Private variables*/
		void* _heap;
		std::map<std::string, std::pair<void*, size_t>> _tags; // associate module tag with its share of memory
		std::map<std::string, void*> _nextFree; // address of the next free blocks in a tag's part of the heap
		size_t _usedSpace;
		size_t _capacity;
	
	public:
		TaggedHeap(void*, size_t); // Constructor for creating TaggedHeap
		bool CreateTag(std::string tag, size_t size);
		void* requestBlocks(std::string tag, size_t size, size_t num); // Request blocks from a specific tag
		bool clearTag(std::string tag); // Clear memory from a given tag
		void clearAll(); // Clear the entire heap
		size_t getCapacity(); // Get the capacity of the TaggedHeap
		void* getNextFree(const std::string &tag);
		size_t getSize(const std::string &tag);
};