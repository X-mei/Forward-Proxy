#ifndef MYEXCEPTION_H
#define MYEXCEPTION_H
#include <string>
#include <exception>

class myException : public std::exception{
private:
  std::string errorMessage;
public:
  myException(std::string message):errorMessage(message){}
  virtual const char * what(){
    return errorMessage.c_str();
  }
};
#endif