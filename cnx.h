#pragma once

#include <napi.h>
#include <ldap.h>
#include <uv.h>

class LDAPCnx : public Napi::ObjectWrap<LDAPCnx>
{
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  LDAPCnx(const Napi::CallbackInfo &info);

private:
  LDAP *ld;
  Napi::Value Search(const Napi::CallbackInfo &info);
  Napi::Value Bind(const Napi::CallbackInfo &info);
  Napi::Value Close(const Napi::CallbackInfo &info);
  Napi::Value SaslBind(const Napi::CallbackInfo &info);

  std::vector<std::string> GetStringVectorFromNapiArray(Napi::Array array);
};