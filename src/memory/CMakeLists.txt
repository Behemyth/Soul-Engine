
##############################################
#Memory

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			memory.ixx
			Allocator.ixx
			LinearAllocator.ixx
			tagged_allocator/TaggedAllocator.ixx
			tagged_allocator/TaggedHeap.ixx
    PRIVATE
			Allocator.cpp
			LinearAllocator.cpp
			tagged_allocator/TaggedAllocator.cpp
			tagged_allocator/TaggedHeap.cpp
)


