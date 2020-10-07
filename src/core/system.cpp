#include "system.h"

#include "hashes.h"


namespace Veng
{


ComponentBase::ComponentBase(const char* name) : name(name), type(crc32_string(name)) {}


}