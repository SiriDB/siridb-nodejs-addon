// sdbcl.cc
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "sdbcl.h"
#include <vector>

namespace siridb
{

using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Boolean;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Exception;
using v8::Handle;

struct Work
{
    Persistent<Function> cb;
};

Persistent<Function> SiriDBClient::constructor;

SiriDBClient::SiriDBClient(
        const std::string username,
        const std::string password,
        const std::string dbname,
        const std::string host,
        const uint16_t port) :
        username_(username),
        password_(password),
        dbname_(dbname),
        host_(host),
        port_(port)
{
    std::cout << "Init SiriDB Client" << std::endl;
    siridb_ = siridb_create();
    buf_ = (siridb_) ? suv_buf_create(siridb_) : nullptr;
}

SiriDBClient::~SiriDBClient()
{
    std::cout << "Destroy SiriDB Client" << std::endl;
    if (buf_) suv_buf_destroy(buf_);
    if (siridb_) siridb_destroy(siridb_);
}

void SiriDBClient::Init(Local<Object> exports)
{
    Isolate* isolate = exports->GetIsolate();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "SiriDBClient"));
    tpl->InstanceTemplate()->SetInternalFieldCount(2);

    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "connect", Connect);
    NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "SiriDBClient"),
               tpl->GetFunction());
}

void SiriDBClient::New(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    const int argc = 4;
    std::vector<std::string> v;
    uint16_t port = 9000;

    if (args.Length() < argc)
    {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(
                        isolate, "Wrong number of arguments")));
        return;
    }

    if (args.Length() > argc)
    {
        if (!args[argc]->IsUint32() || args[argc]->Uint32Value() > 0xffff)
        {
            isolate->ThrowException(Exception::TypeError(
                    String::NewFromUtf8(isolate, "Invalid port number")));
            return;
        }
        port = (uint16_t) args[argc]->Uint32Value();
    }

    for (int i = 0; i < argc; i++)
    {
        if (!args[i]->IsString())
        {
            isolate->ThrowException(Exception::TypeError(
                    String::NewFromUtf8(
                            isolate, "Wrong arguments")));
            return;
        }
        String::Utf8Value str(args[i]->ToString());
        if (!*str)
        {
            isolate->ThrowException(Exception::TypeError(
                    String::NewFromUtf8(
                            isolate, "Cannot convert string")));
            return;
        }
        v.push_back(*str);
    }

    if (args.IsConstructCall())
    {
        SiriDBClient* obj = new SiriDBClient(v[0], v[1], v[2], v[3], port);
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    }
    else
    {
        Local<Value> argv[argc] = { args[0], args[1], args[2], args[3] };
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        Local<Object> result =
            cons->NewInstance(context, argc, argv).ToLocalChecked();
        args.GetReturnValue().Set(result);
    }
}

void SiriDBClient::Connect(const FunctionCallbackInfo<Value>& args)
{
    struct sockaddr_in addr;
    Isolate* isolate = args.GetIsolate();
    SiriDBClient* obj = ObjectWrap::Unwrap<SiriDBClient>(args.Holder());
    uv_loop_t* loop = uv_default_loop();
    Local<Function> cb = Local<Function>::Cast(args[0]);
    Work* work;
    std::bad_alloc allocexc;

    int rc;

    if (!obj->siridb_ || !obj->buf_)
    {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "SiriDB uninitialized")));
        return;
    }

    rc = uv_ip4_addr(obj->host_.c_str(), obj->port_, &addr);

    if (rc != 0)
    {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                isolate,
                ("Cannot initialize IPv4 address: " +
                        std::string(uv_strerror(rc))).c_str())));
    }

    siridb_req_t* req = siridb_req_create(obj->siridb_, ConnectCb, NULL);
    if (!req) throw allocexc;
//
    suv_connect_t* conn = suv_connect_create(
            req,
            obj->username_.c_str(),
            obj->password_.c_str(),
            obj->dbname_.c_str());
    if (!conn) throw allocexc;

    work = new Work();
    work->cb.Reset(isolate, cb);

    conn->data = work;
    req->data = conn;

    uv_tcp_init(loop, &obj->tcp_);
    suv_connect(conn, obj->buf_, &obj->tcp_, (struct sockaddr *) &addr);

    args.GetReturnValue().Set(Undefined(isolate));
}

void SiriDBClient::ConnectCb(siridb_req_t * req)
{
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);

    suv_connect_t* conn = (suv_connect_t*) req->data;
    if (conn)
    {
        Work* work = static_cast<Work*>(conn->data);
        Handle<Value> argv[1];

        if (req->status)
        {
            Local<String> val = String::NewFromUtf8(isolate,
                    ("Connect or authentication failed: " +
                            std::string(suv_strerror(req->status)))
                            .c_str());
            argv[0] = val;
        }
        else
        {
            argv[0] = Undefined(isolate);
        }
        Local<Function>::New(isolate, work->cb)->
              Call(isolate->GetCurrentContext()->Global(), 1, argv);

        work->cb.Reset();

        /* cleanup work */
        delete work;

        /* cleanup connection handle */
        suv_connect_destroy(conn);
    }

    /* cleanup connection request */
    siridb_req_destroy(req);
}

void SiriDBClient::Close(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    SiriDBClient* obj = ObjectWrap::Unwrap<SiriDBClient>(args.Holder());

    if (!uv_is_closing((uv_handle_t *) &obj->tcp_))
    {
        uv_close((uv_handle_t *) &obj->tcp_, NULL);
    }
    //    Local<String> val = String::NewFromUtf8(isolate, (obj->username_ + obj->password_).c_str());
    //    Local<Boolean> retval = Boolean::New(isolate, true);

    args.GetReturnValue().Set(Undefined(isolate));
}

}  // namespace siridb
