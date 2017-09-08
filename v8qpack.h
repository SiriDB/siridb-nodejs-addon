#include <string>
#include <exception>
#include <node.h>

namespace v8qpack
{

std::string Pack(v8::Local<v8::Value>& val);

v8::Local<v8::Value> Unpack(
        v8::Isolate * isolate,
        const unsigned char * ptr,
        size_t len);

class QpackDataException: public std::exception
{
    virtual const char * what() const throw()
    {
        return "Invalid or corrupt QPack data";
    }
};

class QpackEndOfStream: public std::exception
{
    virtual const char * what() const throw()
    {
        return "End of stream";
    }
};

}  // namespace v8qpack
