#ifndef RESPONSE_H  
#define RESPONSE_H

#include "Http.h"

class Response: public Http{
private:
  string status_code;
  string status_phrase;
  string protocol;
public:
  Response();
  Response(const Response & rhs);
  Response &operator=(const Response & rhs);
  virtual void parseFirstLine();
  ~Response();
};
#endif
