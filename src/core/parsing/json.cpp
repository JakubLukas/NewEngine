#include "core/iallocator.h"


namespace Veng
{


static void* json_allocate(void* context, size_t size)
{
	IAllocator& allocator = *(IAllocator*)context;
	return allocator.Allocate(size, 16);
}

static void json_deallocate(void* context, void* ptr)
{
	IAllocator& allocator = *(IAllocator*)context;
	allocator.Deallocate(ptr);
}


}


#define JSON_ALLOC(context, size) Veng::json_allocate(context, size);
#define JSON_FREE(context, data) Veng:: json_deallocate(context, data);
#define JSON_ERROR_MESSAGE
#define JSON_INPUT_STRING_IS_STORAGE
#define JSON_IMPLEMENTATION
#include "json.h"
