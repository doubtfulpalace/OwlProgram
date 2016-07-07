#include "Resource.h"
#include "ServiceCall.h"
#include "ProgramVector.h"
#include "ResourceHeader.h"
#include "message.h"

Resource Resource::getResource(const char* name){
  ResourceHeader* header;
  uint8_t* data = nullptr;
  uint32_t size = 0;
  void* args[] = {(void*)name, (void*)&header};
  int ret = getProgramVector()->serviceCall(OWL_SERVICE_GET_RESOURCE, args, 2);
  if(ret == OWL_SERVICE_OK && header != nullptr){
    data = (uint8_t*)header;
    data += sizeof(header);
    size = header->size;
    debugMessage("Found resource", (int)size);
  }else{
    debugMessage("Couldn't find resource");
    // debug << "Couldn't find resource " << name;
  }
  return Resource(data, size);
}

