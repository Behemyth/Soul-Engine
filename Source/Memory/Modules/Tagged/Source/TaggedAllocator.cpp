#include "TaggedAllocator.h"
#include "TaggedHeap.h"
TaggedAllocator::TaggedAllocator(TaggedHeap* heap, const std::string &tag, uint8_t align) : Allocator(heap->getSize(tag), heap->getNextFree(tag)) {
	_heap = heap;
	_tag = tag;
	_blockSize = align;
	_numBlocks = 0;
}

TaggedAllocator::~TaggedAllocator() {
	_numBlocks = 0;
}

void* TaggedAllocator::allocate(size_t size, uint8_t align) {
	std::lock_guard<std::mutex> l(_mutex);
	void* blocks = _heap->requestBlocks(_tag, align, size);
	if (blocks == nullptr) {
		return nullptr;
	}

	_numBlocks += size;

	return blocks;
}

void* TaggedAllocator::deallocate(void* block) {
	return nullptr;
}

void TaggedAllocator::clear() {
	_heap->clearTag(_tag);
}