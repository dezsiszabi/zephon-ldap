#include <napi.h>
#include "cnx.h"

using namespace Napi;

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    LDAPCnx::Init(env, exports);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)