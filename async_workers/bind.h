#pragma once

#include <napi.h>
#include <ldap.h>

class BindAsyncWorker : public Napi::AsyncWorker {
public:
  BindAsyncWorker(Napi::Env &env, LDAP *ld, std::string dn, std::string password, Napi::Promise::Deferred deferred);
  ~BindAsyncWorker();
  void Execute();
  void OnOK();
  void OnError(Napi::Error const &error);

private:
  LDAP *ld;
  std::string dn;
  std::string password;
  Napi::Promise::Deferred deferred;
};