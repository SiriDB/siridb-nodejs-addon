#include <string.h>
#include "v8qpack.h"

#define ReturnQpRaw(uintx_t__) {                        \
    if ((*pos) + sizeof(uintx_t__) > len)               \
        throw v8qpack::QpackDataException();            \
    int SZ__ = (int) *((uintx_t__ *) (ptr + (*pos)));   \
    (*pos) += sizeof(uintx_t__);                        \
    if ((*pos) + SZ__ > len)                            \
        throw v8qpack::QpackDataException();            \
    Local<Value> val = String::NewFromUtf8(             \
            isolate,                                    \
            (char *) (ptr + (*pos)),                    \
            String::kNormalString, SZ__);               \
    (*pos) += SZ__;                                     \
    return val; }

#define ReturnQpInt(intx_t__) {                         \
    if ((*pos) + sizeof(intx_t__) > len)                \
        throw v8qpack::QpackDataException();            \
    Local<Value> val = Number::New(                     \
            isolate,                                    \
            (double) *((intx_t__ *) (ptr + (*pos))));   \
    (*pos) += sizeof(intx_t__);                        \
    return val; }


namespace v8qpack
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

static void Pack_(std::string& str, Local<Value>& val);
static Local<Value> Unpack_(
        Isolate * isolate,
        const unsigned char * ptr,
        size_t len,
        size_t * pos);

class QpackCloseArray: public std::exception
{
    virtual const char * what() const throw()
    {
        return "Closing array";
    }
};

class QpackCloseMap: public std::exception
{
    virtual const char * what() const throw()
    {
        return "Closing map";
    }
};

std::string Pack(Local<Value>& val)
{
    std::string str = "";
    Pack_(str, val);
    return str;
}

static void Pack_(std::string& str, Local<Value>& val)
{
    if (val->IsStringObject())
    {
        String::Utf8Value tmp(val->ToString());
        if (!*tmp) throw;
        std::string s = std::string(*tmp);
        size_t len = s.length();
        if (len < 100)
        {
            str += (uint8_t) (128 + len);
            str += s;
        }
        return;
    }

    if (val->IsArray())
    {
        Local<Array> arr = Local<Array>::Cast(val);
        uint32_t len = arr->Length();
        Local<Value> val;
        str += (len < 6) ?  (uint8_t) (237 + len) : (uint8_t) 252;
        for(uint32_t i = 0; i < len; i++)
        {
            val = arr->Get(i);
            Pack_(str, val);
        }
        if (len >= 6)
        {
            str += (uint8_t) 254;
        }
        return;
    }

    if (val->IsInt32())
    {
        int8_t i8;
        int16_t i16;
        int32_t i32 = val->Int32Value();
        if (i32 >= 0 && i32 < 64)
        {
            str += (uint8_t) i32;

        }
        else if (i32 >= -60 && i32 < 0)
        {
            str += (uint8_t) (63 - i32);

        }
        else if ((i8 = (int8_t) i32) == i32)
        {
            str += (uint8_t) 232;
            str += i8;
        }
        else if ((i16 = (int16_t) i32) == i32)
        {
            str += (uint8_t) 233;
            char buffer[2];
            memcpy(buffer, &i16, 2);
            str.append(buffer, 2);
        }
        else
        {
            str += (uint8_t) 234;
            char buffer[4];
            memcpy(buffer, &i32, 4);
            str.append(buffer, 4);
        }
        return;
    }

    if (val->IsNumber())
    {
        double d = val->NumberValue();
        if (d == -1.0)
        {
            str += (uint8_t) 125;
        }
        else if (d == 0.0)
        {
            str += (uint8_t) 126;
        }
        else if (d == 1.0)
        {
            str += (uint8_t) 127;
        }
        else
        {
            str += (uint8_t) 136;
            char buffer[8];
            memcpy(buffer, &d, 8);
            str.append(buffer, 8);
        }
        return;
    }

    if (val->IsNull())
    {
        str += (uint8_t) 251;
        return;
    }

    if (val->IsTrue())
    {
        str += (uint8_t) 249;
        return;
    }

    if (val->IsFalse())
    {
        str += (uint8_t) 250;
        return;
    }

    if (val->IsObject())
    {

    }
}


Local<Value> Unpack(
        Isolate * isolate,
        const unsigned char * ptr,
        size_t len)
{
    Local<Value> val;
    size_t p = 0;
    try
    {
        val = Unpack_(isolate, ptr, len, &p);
    }
    catch (QpackCloseMap& e)
    {
        throw QpackDataException();
    }
    catch (QpackCloseArray& e)
    {
        throw QpackDataException();
    }
    return val;
}

static Local<Value> Unpack_(
        Isolate * isolate,
        const unsigned char * ptr,
        size_t len,
        size_t * pos)
{
    uint8_t tp;
    if (*pos >= len)
    {
        throw QpackEndOfStream();
    }
    tp = *(ptr + (*pos));
    (*pos)++;
    switch (tp)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
        return Number::New(isolate, (double) tp);

    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
    case 123:
        return Number::New(isolate, (double) (63 - tp));

    case 124:
        return Undefined(isolate);  // QP_HOOK

    case 125:
    case 126:
    case 127:
        return Number::New(isolate, (double) (tp - 126));

    case 128:
    case 129:
    case 130:
    case 131:
    case 132:
    case 133:
    case 134:
    case 135:
    case 136:
    case 137:
    case 138:
    case 139:
    case 140:
    case 141:
    case 142:
    case 143:
    case 144:
    case 145:
    case 146:
    case 147:
    case 148:
    case 149:
    case 150:
    case 151:
    case 152:
    case 153:
    case 154:
    case 155:
    case 156:
    case 157:
    case 158:
    case 159:
    case 160:
    case 161:
    case 162:
    case 163:
    case 164:
    case 165:
    case 166:
    case 167:
    case 168:
    case 169:
    case 170:
    case 171:
    case 172:
    case 173:
    case 174:
    case 175:
    case 176:
    case 177:
    case 178:
    case 179:
    case 180:
    case 181:
    case 182:
    case 183:
    case 184:
    case 185:
    case 186:
    case 187:
    case 188:
    case 189:
    case 190:
    case 191:
    case 192:
    case 193:
    case 194:
    case 195:
    case 196:
    case 197:
    case 198:
    case 199:
    case 200:
    case 201:
    case 202:
    case 203:
    case 204:
    case 205:
    case 206:
    case 207:
    case 208:
    case 209:
    case 210:
    case 211:
    case 212:
    case 213:
    case 214:
    case 215:
    case 216:
    case 217:
    case 218:
    case 219:
    case 220:
    case 221:
    case 222:
    case 223:
    case 224:
    case 225:
    case 226:
    case 227:
        /* unpack fixed sized raw strings */
        {
            int size = tp - 128;
            if ((*pos) + size > len)
            {
                throw v8qpack::QpackDataException();
            }
            Local<Value> val = String::NewFromUtf8(
                    isolate,
                    (char *) (ptr + (*pos)),
                    String::kNormalString, size);
            (*pos) += size;
            return val;
        }
    case 228:
        ReturnQpRaw(uint8_t)
    case 229:
        ReturnQpRaw(uint16_t)
    case 230:
        ReturnQpRaw(uint32_t)
    case 231:
        ReturnQpRaw(uint64_t)
    case 232:
        ReturnQpInt(int8_t)
    case 233:
        ReturnQpInt(int16_t)
    case 234:
        ReturnQpInt(int32_t)
    case 235:
        ReturnQpInt(int64_t)
    case 236:
        if ((*pos) + sizeof(double) > len)
        {
            throw v8qpack::QpackDataException();
        }
        {
            double d;
            memcpy(&d, (char *) (ptr + (*pos)), sizeof(double));
            (*pos) += sizeof(double);
            return Number::New(isolate, d);
        }
    case 237:
    case 238:
    case 239:
    case 240:
    case 241:
    case 242:
        {
            unsigned int count = tp - 237;
            Local<Array> arr = Array::New(isolate);
            Local<Value> val;
            for (unsigned int i = 0; i < count; i++)
            {
                val = Unpack_(isolate, ptr, len, pos);
                arr->Set(i, val);
            }
            return arr;
        }
    case 243:
    case 244:
    case 245:
    case 246:
    case 247:
    case 248:
        {
            unsigned int count = tp - 243;
            Local<Object> obj = Object::New(isolate);
            Local<Value> key;
            Local<Value> val;
            for (unsigned int i = 0; i < count; i++)
            {
                key = Unpack_(isolate, ptr, len, pos);
                val = Unpack_(isolate, ptr, len, pos);
                obj->Set(key, val);
            }
            return obj;
        }
    case 249:
        return True(isolate);
    case 250:
        return False(isolate);
    case 251:
        return Null(isolate);
    case 252:
        {
            Local<Array> arr = Array::New(isolate);
            Local<Value> val;
            for (unsigned int i = 0;;i++)
            {
                try
                {
                    val = Unpack_(isolate, ptr, len, pos);
                }
                catch (QpackEndOfStream& e)
                {
                    break;
                }
                catch (QpackCloseArray& e)
                {
                    break;
                }
                arr->Set(i, val);
            }
            return arr;
        }
    case 253:
        {
            Local<Object> obj = Object::New(isolate);
            Local<Value> key;
            Local<Value> val;
            while (1)
            {
                try
                {
                    key = Unpack_(isolate, ptr, len, pos);
                }
                catch (QpackEndOfStream& e)
                {
                    break;
                }
                catch (QpackCloseMap& e)
                {
                    break;
                }
                val = Unpack_(isolate, ptr, len, pos);
                obj->Set(key, val);
            }
            return obj;
        }
    case 254:
        throw QpackCloseArray();
    case 255:
        throw QpackCloseMap();
    default:
        throw QpackDataException();
    }
}

}  // namespace v8qpack
