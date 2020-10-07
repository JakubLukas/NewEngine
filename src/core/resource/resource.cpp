#include "core/resource/resource.h"

#include "core/hashes.h"
#include "resource_manager.h"


namespace Veng
{


ResourceType::ResourceType(const char* name) : hash(crc32_string(name)) {}


}