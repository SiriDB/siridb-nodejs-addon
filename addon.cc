// addon.cc
#include <node.h>
#include <libsiridb/siridb.h>
#include "sdbcl.h"

namespace siridb {

using v8::Local;
using v8::Object;

void InitAll(Local<Object> exports) {
  SiriDBClient::Init(exports);
}

NODE_MODULE(addon, InitAll)

}  // namespace siridb
