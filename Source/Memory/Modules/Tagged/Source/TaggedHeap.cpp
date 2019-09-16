#include "TaggedHeap.h"


TaggedHeap::TaggedHeap(void* mem, size_t size) {
	_usedSpace = 0;
	_heap = mem;
	_capacity = size;
	_nextFree = std::map<std::string, void*>();
	_tags = std::map<std::string, std::pair<void*, size_t>>();
}

bool TaggedHeap::CreateTag(std::string tag, size_t size) {
	/* If the tag already exists, do nothing */
	if (_tags.count(tag) == 0) {
		return false;
	}

	/*Ensure that there is enough space to add the tag*/
	if (size > _capacity - _usedSpace) {
		return false;
	}

	void* start = static_cast<uint8_t*>(_heap) + _usedSpace; 
	_tags.emplace(tag, std::make_pair(start, size));
	_nextFree.emplace(tag, start);
	return true;
}

void* TaggedHeap::requestBlocks(std::string tag, size_t size, size_t num) {
	std::map<std::string, std::pair<void*, size_t>>::iterator itr = _tags.find(tag);
	
	/* Tag does not exist, so return a nullptr */
	if (itr == _tags.end()) {
		return nullptr;
	}

	/* Ensure that there is enough space in the heap to accomodate a new allocation*/
	uint8_t* next = static_cast<uint8_t*>(_nextFree[tag]);
	uint8_t* end = static_cast<uint8_t*>(itr->second.first) + itr->second.second;
	if (end - next < size * num) {
		return nullptr;
	}

	/* Return the requested blocks */
	void* blocks = _nextFree[tag];
	next = static_cast<uint8_t*>(blocks) + size * num;
	_nextFree.emplace(tag, next);
	return blocks;

}

bool TaggedHeap::clearTag(std::string tag) {
	std::map<std::string, std::pair<void*, size_t>>::iterator itr = _tags.find(tag);
	/* Return false if the tag does not exist */
	if (itr == _tags.end()) {
		return false;
	}

	_usedSpace -= itr->second.second;
	_tags.erase(tag);
	_nextFree.erase(tag);
	return true;
}

void TaggedHeap::clearAll() {
	std::map<std::string, std::pair<void*, size_t>>::iterator tagsItr = _tags.begin();

	while (tagsItr != _tags.end()) {
		TaggedHeap::clearTag(tagsItr->first);
	}
}

size_t TaggedHeap::getCapacity() { return _capacity; }

void* TaggedHeap::getNextFree(const std::string &tag) {
	if (_nextFree.count(tag) == 0) {
		return nullptr;
	}

	return _nextFree[tag];
}

size_t TaggedHeap::getSize(const std::string &tag) {
	if (_nextFree.count(tag) == 0) {
		return 0;
	}
	return _tags[tag].second;
}