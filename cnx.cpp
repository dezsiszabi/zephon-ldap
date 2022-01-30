#include <napi.h>
#include <ldap.h>
#include "cnx.h"
#include "async_workers/search.h"
#include "async_workers/bind.h"

using namespace Napi;

Napi::Object LDAPCnx::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "LDAPCnx", {
        InstanceMethod<&LDAPCnx::Search>("search", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::Delete>("delete", static_cast<napi_property_attributes>(napi_default_method)),
        InstanceMethod<&LDAPCnx::Bind>("bind", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::Add>("add", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::Modify>("modify", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::Rename>("rename", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::Abandon>("abandon", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::ErrorString>("errorstring", static_cast<napi_property_attributes>(napi_default_method)),
        InstanceMethod<&LDAPCnx::Close>("close", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::ErrNo>("errno", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::Fd>("fd", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::InstallTls>("installtls", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::StartTls>("starttls", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::CheckTls>("checktls", static_cast<napi_property_attributes>(napi_default_method)),
        // InstanceMethod<&LDAPCnx::SaslBind>("saslbind", static_cast<napi_property_attributes>(napi_default_method)),
    });

    Napi::FunctionReference* constructor = new Napi::FunctionReference();

    // Create a persistent reference to the class constructor. This will allow
    // a function called on a class prototype and a function
    // called on instance of a class to be distinguished from each other.
    *constructor = Napi::Persistent(func);
    exports.Set("LDAPCnx", func);
    env.SetInstanceData<Napi::FunctionReference>(constructor);

    return exports;
}

LDAPCnx::LDAPCnx(const Napi::CallbackInfo& info) : Napi::ObjectWrap<LDAPCnx>(info) {
  int ver = LDAP_VERSION3;
  
  Napi::Env env = info.Env();
  std::string url = info[0].As<Napi::String>();

  if (ldap_initialize(&(this->ld), url.c_str()) != LDAP_SUCCESS) {
    throw Napi::Error::New(env, "Error intializing ldap");
  }

  ldap_set_option(this->ld, LDAP_OPT_PROTOCOL_VERSION, &ver);
}

Napi::Value LDAPCnx::Search(const Napi::CallbackInfo& info) {
  int msgid = 0;
  LDAPControl *page_control[2];
  
  Napi::Env env = info.Env();
  std::string arg_base = info[0].As<Napi::String>();
  std::string arg_filter = info[1].As<Napi::String>();
  Napi::Array arg_attrs = info[2].As<Napi::Array>();

  std::vector<std::string> attributes;

  for(uint i = 0; i < arg_attrs.Length(); i++) {
    std::string attr = ((Napi::Value) arg_attrs[i]).As<Napi::String>();
    attributes.push_back(attr);
  }

  std::vector<const char*> attrs;
  std::transform(std::begin(attributes), std::end(attributes), std::back_inserter(attrs), std::mem_fn(&std::string::c_str));
  attrs.push_back(NULL);

  memset(&page_control, 0, sizeof(page_control));

  if (ldap_search_ext(this->ld, arg_base.c_str(), 2, arg_filter.c_str(), (char**)attrs.data(), 0, page_control, NULL, NULL, 0, &msgid) != LDAP_SUCCESS) {
    throw Napi::Error::New(env, "Error searching ldap");
  }

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  SearchAsyncWorker *searchAsyncWorker = new SearchAsyncWorker(env, this->ld, msgid, deferred);
  searchAsyncWorker->Queue();
  return deferred.Promise();
}

Napi::Value LDAPCnx::Bind(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  std::string dn = info[0].As<Napi::String>();
  std::string password = info[1].As<Napi::String>();

  int msgid = ldap_simple_bind(this->ld, dn.c_str(), password.c_str());

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
  BindAsyncWorker* bindAsyncWorker = new BindAsyncWorker(env, this->ld, msgid, deferred);
  bindAsyncWorker->Queue();
  return deferred.Promise();
}

Napi::Value LDAPCnx::Close(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  ldap_unbind(this->ld);

  return env.Undefined();
}