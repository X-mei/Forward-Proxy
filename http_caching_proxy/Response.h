#ifndef RESPONSE_H  
#define RESPONSE_H
#include <string>
#include "Http.h"
class Response: public Http{
private:
public:
  Response();
  Response(const Response & rhs);
  Response &operator=(const Response & rhs);
  virtual void parseFirstLine();
  virtual ~Response();
};
#endif
