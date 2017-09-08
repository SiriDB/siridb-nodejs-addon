// sdbcl.cc
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include "sdbcl.h"
#include "v8qpack.h"

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
using v8::Array;
using v8qpack::Pack;
using v8qpack::Unpack;

struct Work
{
    Persistent<Function> cb;
    SiriDBClient* siridb;
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
    siridb_ = siridb_create();
    buf_ = (siridb_) ? suv_buf_create(siridb_) : nullptr;
    if (buf_)
    {
        buf_->data = this;
        buf_->onclose = &OnCloseCb;
        buf_->onerror = &OnErrorCb;
    }
}

SiriDBClient::~SiriDBClient()
{
    if (buf_) suv_buf_destroy(buf_);
    if (siridb_) siridb_destroy(siridb_);
}

void SiriDBClient::Init(Local<Object> exports)
{
    Isolate* isolate = exports->GetIsolate();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "SiriDBClient"));
    tpl->InstanceTemplate()->SetInternalFieldCount(5);

    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "connect", Connect);
    NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
    NODE_SET_PROTOTYPE_METHOD(tpl, "onClose", SetCloseCb);
    NODE_SET_PROTOTYPE_METHOD(tpl, "onError", SetErrorCb);
    NODE_SET_PROTOTYPE_METHOD(tpl, "query", Query);

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
    SiriDBClient * obj = ObjectWrap::Unwrap<SiriDBClient>(args.Holder());
    uv_loop_t * loop = uv_default_loop();
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

    siridb_req_t * req = siridb_req_create(obj->siridb_, ConnectCb, NULL);
    if (!req) throw allocexc;

    suv_connect_t * conn = suv_connect_create(
            req,
            obj->username_.c_str(),
            obj->password_.c_str(),
            obj->dbname_.c_str());
    if (!conn) throw allocexc;

    work = new Work();
    work->cb.Reset(isolate, cb);
    work->siridb = obj;

    conn->data = work;
    req->data = conn;

    suv_connect(loop, conn, obj->buf_, (struct sockaddr *) &addr);

    args.GetReturnValue().Set(Undefined(isolate));
}

void SiriDBClient::SetCloseCb(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    SiriDBClient* obj = ObjectWrap::Unwrap<SiriDBClient>(args.Holder());
    Local<Function> cb = Local<Function>::Cast(args[0]);

    obj->onclosecb_.Reset(isolate, cb);

    args.GetReturnValue().Set(Undefined(isolate));
}

void SiriDBClient::SetErrorCb(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    SiriDBClient* obj = ObjectWrap::Unwrap<SiriDBClient>(args.Holder());
    Local<Function> cb = Local<Function>::Cast(args[0]);

    obj->onerrorcb_.Reset(isolate, cb);

    args.GetReturnValue().Set(Undefined(isolate));
}

void SiriDBClient::Close(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    SiriDBClient* obj = ObjectWrap::Unwrap<SiriDBClient>(args.Holder());

    suv_close(obj->buf_, NULL);

    args.GetReturnValue().Set(Undefined(isolate));
}

void SiriDBClient::Query(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    SiriDBClient* obj = ObjectWrap::Unwrap<SiriDBClient>(args.Holder());
    Local<Function> cb;
    Work* work;
    std::bad_alloc allocexc;

    if (!obj->siridb_ || !obj->buf_)
    {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate, "SiriDB uninitialized")));
        return;
    }

    if (args.Length() < 2)
    {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(
                        isolate, "Wrong number of arguments")));
        return;
    }

    if (!args[0]->IsString() || !args[1]->IsFunction())
    {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(
                        isolate, "Wrong arguments")));
        return;
    }

    String::Utf8Value str(args[0]->ToString());
    if (!*str)
    {
        isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(
                        isolate, "Cannot convert string")));
        return;
    }

    siridb_req_t * req = siridb_req_create(obj->siridb_, QueryCb, NULL);
    if (!req) throw allocexc;

    suv_query_t * suvquery = suv_query_create(req, std::string(*str).c_str());
    if (!suvquery) throw allocexc;

    cb = Local<Function>::Cast(args[1]);

    work = new Work();
    work->cb.Reset(isolate, cb);
    work->siridb = obj;

    suvquery->data = work;
    req->data = suvquery;

    suv_query(suvquery);

    args.GetReturnValue().Set(Undefined(isolate));
}

void SiriDBClient::ConnectCb(siridb_req_t * req)
{
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);

    suv_connect_t * conn = (suv_connect_t *) req->data;
    if (conn)
    {
        Work* work = static_cast<Work*>(conn->data);
        Handle<Value> argv[1];

        if (req->status)
        {
            argv[0] = String::NewFromUtf8(isolate,
                    ("Connect or authentication failed: " +
                            std::string(suv_strerror(req->status)))
                            .c_str());
        }
        else
        {
            switch (req->pkg->tp)
            {
            case CprotoErrAuthCredentials:
                argv[0] = String::NewFromUtf8(isolate, "Invalid credentials");
                suv_close(work->siridb->buf_, NULL);
                break;
            case CprotoErrAuthUnknownDb:
                argv[0] = String::NewFromUtf8(isolate, "Unknown database");
                suv_close(work->siridb->buf_, NULL);
                break;
            case CprotoResAuthSuccess:
                argv[0] = Null(isolate);
                break;
            default:
                argv[0] = String::NewFromUtf8(isolate, "Unexpected error");
                suv_close(work->siridb->buf_, NULL);
            }
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




/*
 * Return JSON compatible text for a given package type.
 */
const char * suv_errproto(uint8_t tp)
{
    switch (tp)
    {
    case CprotoResAuthSuccess:
        return "{\"success_msg\":\"Successful authentication.\"}";
    case CprotoResAck:
        return "{\"success_msg\":\"Acknowledged.\"}";
    case CprotoAckAdmin:
        return "{\"success_msg\":\"Acknowledged.\"}";
    case CprotoAckAdminData:
        return "{\"success_msg\":\"Acknowledged.\"}";
    case CprotoErr:
        return "{\"error_msg\":\"General error.\"}";
    case CprotoErrNotAuthenticated:
        return "{\"error_msg\":\"Not authenticated.\"}";
    case CprotoErrAuthCredentials:
        return "{\"error_msg\":\"Invalid credentials.\"}";
    case CprotoErrAuthUnknownDb:
        return "{\"error_msg\":\"Unknown database.\"}";
    case CprotoErrLoadingDb:
        return "{\"error_msg\":\"Loading database.\"}";
    case CprotoErrFile:
        return "{\"error_msg\":\"Error returning file.\"}";
    case CprotoErrAdmin:
        return "{\"error_msg\":\"General service request error.\"}";
    case CprotoErrAdminInvalidRequest:
        return "{\"error_msg\":\"Invalid service request.\"}";
    default:
        return "{\"error_msg\":\"Unknown error.\"}";
    }
}

Local<Object> SiriDBClient::BuildErr(Isolate * isolate, std::string err)
{
    Local<Object> obj = Object::New(isolate);
    Local<String> key = String::NewFromUtf8(isolate, "error_msg");
    Local<String> val = String::NewFromUtf8(isolate, err.c_str());
    obj->Set(key, val);
    return obj;
}

std::string SiriDBClient::GetMsg(uint8_t tp)
{
    switch (tp)
    {
    case CprotoResAuthSuccess:
        return "Successful authenticated.";
    case CprotoResAck:
        return "Acknowledge received.";
    case CprotoErr:
        return "General exception in SiriDB occurred.";
    case CprotoErrNotAuthenticated:
        return "Connection is not authenticated.";
    case CprotoErrAuthCredentials:
        return "Invalid credential.";
    case CprotoErrAuthUnknownDb:
        return "Unknown database";
    case CprotoErrLoadingDb:
        return "Database is loading.";
    default:
        return "Unpacking response has failed.";
    }
}

void SiriDBClient::QueryCb(siridb_req_t * req)
{
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);

    suv_query_t * suvquery = (suv_query_t *) req->data;
    Work* work = static_cast<Work*>(suvquery->data);
    Handle<Value> argv[2];

    if (req->status != 0)
    {
        argv[0] = SiriDBClient::BuildErr(
                isolate,
                "Unable to handle request: " +
                std::string(suv_strerror(req->status)));
        argv[1] = Number::New(isolate, -CprotoErrMsg);
    }
    else
    {
        try
        {
            argv[0] = Unpack(isolate, req->pkg->data, req->pkg->len);
            argv[1] = Number::New(isolate,
                    (req->pkg->tp == CprotoReqQuery) ? 0 : -req->pkg->tp);
        }
        catch (...)
        {
            argv[0] = SiriDBClient::BuildErr(
                    isolate,
                    SiriDBClient::GetMsg(req->pkg->tp));
            argv[1] = Number::New(isolate,
                    (req->pkg->tp == CprotoReqQuery) ?
                            -CprotoErrMsg : -req->pkg->tp);
        }

    }

    Local<Function>::New(isolate, work->cb)->
          Call(isolate->GetCurrentContext()->Global(), 2, argv);

    work->cb.Reset();

    /* cleanup work */
    delete work;

    /* cleanup query */
    suv_query_destroy(suvquery);

    /* cleanup connection request */
    siridb_req_destroy(req);
}

void SiriDBClient::OnCloseCb(void * buf_data, const char * msg)
{
    SiriDBClient * obj = static_cast<SiriDBClient *>(buf_data);

    if (!obj->onclosecb_.IsEmpty())
    {
        Isolate * isolate = Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate);
        Handle<Value> argv[] = { String::NewFromUtf8(isolate, msg) };
        Local<Function>::New(isolate, obj->onclosecb_)->
              Call(isolate->GetCurrentContext()->Global(), 1, argv);

    }
}

void SiriDBClient::OnErrorCb(void * buf_data, const char * msg)
{
    SiriDBClient * obj = static_cast<SiriDBClient *>(buf_data);

    if (!obj->onerrorcb_.IsEmpty())
    {
        Isolate * isolate = Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate);
        Handle<Value> argv[] = { String::NewFromUtf8(isolate, msg) };
        Local<Function>::New(isolate, obj->onerrorcb_)->
              Call(isolate->GetCurrentContext()->Global(), 1, argv);

    }
}

}  // namespace siridb
