#pragma once

#include <napi.h>
#include <ldap.h>
#include <uv.h>

struct ldap_cnx
{
  LDAP *ld;
  ldap_conncb *ldap_callback;
  const char *sasl_mechanism;
  uv_poll_t *handle;
  napi_async_context async_context;
  napi_ref reconnect_callback_ref, disconnect_callback_ref, callback_ref;
  napi_ref this_ref;
  napi_env env;
};

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
};