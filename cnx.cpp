#include <napi.h>
#include <ldap.h>
#include "cnx.h"
#include "async_workers/search.h"
#include "async_workers/bind.h"
#include "async_workers/saslbind.h"

using namespace Napi;

Napi::Object LDAPCnx::Init(Napi::Env env, Napi::Object exports)
{
  Napi::Function func = DefineClass(env, "LDAPCnx", {
                                                        InstanceMethod<&LDAPCnx::Search>("search", static_cast<napi_property_attributes>(napi_default_method)),
                                                        InstanceMethod<&LDAPCnx::Bind>("bind", static_cast<napi_property_attributes>(napi_default_method)),
                                                        InstanceMethod<&LDAPCnx::Close>("close", static_cast<napi_property_attributes>(napi_default_method)),
                                                        InstanceMethod<&LDAPCnx::SaslBind>("saslbind", static_cast<napi_property_attributes>(napi_default_method)),
                                                    });

  Napi::FunctionReference *constructor = new Napi::FunctionReference();

  // Create a persistent reference to the class constructor. This will allow
  // a function called on a class prototype and a function
  // called on instance of a class to be distinguished from each other.
  *constructor = Napi::Persistent(func);
  exports.Set("LDAPCnx", func);
  env.SetInstanceData<Napi::FunctionReference>(constructor);

  return exports;
}

LDAPCnx::LDAPCnx(const Napi::CallbackInfo &info) : Napi::ObjectWrap<LDAPCnx>(info)
{
  int ver = LDAP_VERSION3;

  Napi::Env env = info.Env();
  std::string arg_url = info[0].As<Napi::String>();

  if (ldap_initialize(&(this->ld), arg_url.c_str()) != LDAP_SUCCESS)
  {
    throw Napi::Error::New(env, "Error intializing ldap");
  }

  ldap_set_option(this->ld, LDAP_OPT_PROTOCOL_VERSION, &ver);
}

std::vector<std::string> LDAPCnx::GetStringVectorFromNapiArray(Napi::Array array)
{
  std::vector<std::string> vect;

  for (uint i = 0; i < array.Length(); i++)
  {
    std::string elem = ((Napi::Value)array[i]).As<Napi::String>();
    vect.push_back(elem);
  }

  return vect;
}

Napi::Value LDAPCnx::Search(const Napi::CallbackInfo &info)
{
  LDAPControl *page_control[2];

  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  std::string arg_base = info[0].As<Napi::String>();
  std::string arg_filter = info[1].As<Napi::String>();
  Napi::Array arg_attrs = info[2].As<Napi::Array>();
  Napi::Array arg_binary_attrs = info[3].As<Napi::Array>();

  std::vector<std::string> attributes = this->GetStringVectorFromNapiArray(arg_attrs);
  std::vector<std::string> binary_attributes = this->GetStringVectorFromNapiArray(arg_binary_attrs);

  memset(&page_control, 0, sizeof(page_control));

  SearchAsyncWorker *searchAsyncWorker = new SearchAsyncWorker(env, this->ld, arg_base, arg_filter, attributes, binary_attributes, deferred);
  searchAsyncWorker->Queue();
  return deferred.Promise();
}

Napi::Value LDAPCnx::Bind(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  std::string arg_dn = info[0].As<Napi::String>();
  std::string arg_password = info[1].As<Napi::String>();

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  BindAsyncWorker *bindAsyncWorker = new BindAsyncWorker(env, this->ld, arg_dn, arg_password, deferred);
  bindAsyncWorker->Queue();
  return deferred.Promise();
}

Napi::Value LDAPCnx::Close(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  ldap_unbind(this->ld);

  return env.Undefined();
}

Napi::Value LDAPCnx::SaslBind(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
#ifdef HAVE_CYRUS_SASL
  std::string arg_mechanism = info[0].As<Napi::String>();
  std::string *arg_user = NULL, *arg_password = NULL, *arg_realm = NULL, *arg_proxy_user = NULL;

  if (info.Length() == 2)
  {
    Napi::Object options = info[1].As<Napi::Object>();
    Napi::Value user_value = options.Get("user");
    if (!user_value.IsUndefined()) arg_user = new std::string(user_value.As<Napi::String>());
    Napi::Value password_value = options.Get("password");
    if (!password_value.IsUndefined()) arg_password = new std::string(password_value.As<Napi::String>());
    Napi::Value realm_value = options.Get("realm");
    if (!realm_value.IsUndefined()) arg_realm = new std::string(realm_value.As<Napi::String>());
    Napi::Value proxy_user_value = options.Get("proxy_user");
    if (!proxy_user_value.IsUndefined()) arg_proxy_user = new std::string(proxy_user_value.As<Napi::String>());
  }

  SaslBindAsyncWorker *saslBindAsyncWorker = new SaslBindAsyncWorker(env, this->ld, arg_mechanism, arg_user, arg_password, arg_realm, arg_proxy_user, deferred);
  saslBindAsyncWorker->Queue();
  
#else
  deferred.Reject(Napi::Error::New(env, "Not compiled with SASL support").Value());
#endif
  return deferred.Promise();
}