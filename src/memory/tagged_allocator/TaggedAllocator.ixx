export module synodic.soul.memory:tagged_allocator;

import :allocator;
import :tagged_heap;
import std;

/*Tagged Allocator is an allocator class that utilizes the tagged heap for per-module memory management.
  It uses a LinearAllocator under the hood to make the actual allocations, but obtains memory from the TaggedHeap.
  Memory allocation happens on a per-thread bases to make this allocator thread-safe*/
export class TaggedAllocator : public Allocator {
	mutable std::mutex _mutex;

	private:
		// Private variables
		TaggedHeap* _heap;
		std::string _tag;
		size_t _blockSize;
		size_t _numBlocks;


	public:
		TaggedAllocator(TaggedHeap* heap, const std::string &tag, uint8_t align=4);
		~TaggedAllocator();
		void* allocate(size_t size, uint8_t align=4);
		void* deallocate(void* block);
		void clear();

};
