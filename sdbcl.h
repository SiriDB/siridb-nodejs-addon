// sdbcl.h
#ifndef SDBCL_H
#define SDBCL_H

#include <node.h>
#include <node_object_wrap.h>
#include <string>
#include <libsiridb/siridb.h>
#include <uv.h>
#include "suv.h"
#include <cstdint>

namespace siridb {

class SiriDBClient : public node::ObjectWrap
{
public:
    static void Init(v8::Local<v8::Object> exports);

private:
    explicit SiriDBClient(
            const std::string username,
            const std::string password,
            const std::string dbname,
            const std::string host,
            const uint16_t port = 9000);
    ~SiriDBClient();

    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Connect(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void ConnectCb(siridb_req_t * req);
    static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
    static v8::Persistent<v8::Function> constructor;
    const std::string username_;
    const std::string password_;
    const std::string dbname_;
    const std::string host_;
    const uint16_t port_;
    siridb_t * siridb_;
    suv_buf_t * buf_;
    uv_tcp_t tcp_;

};

}  // namespace siridb

#endif
