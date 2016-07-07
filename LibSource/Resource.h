#ifndef __Resource_h__
#define __Resource_h__

#include <stdint.h>

class Resource {
private:
  uint8_t* data;
  uint32_t size;
public:
  Resource(uint8_t* dt, uint32_t sz) : data(dt), size(sz){}
  void* getData(){
    return data;
  }
  uint32_t getSize(){
    return size;
  }
  operator bool(){
    return data != nullptr && size > 0;
  }
  static Resource getResource(const char* name);  
};

#endif /* __Resource_h__ */
