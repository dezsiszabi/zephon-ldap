#pragma once

#include <napi.h>
#include <ldap.h>

class BindAsyncWorker : public Napi::AsyncWorker {
public:
  BindAsyncWorker(Napi::Env &env, LDAP *ld, int msgid, Napi::Promise::Deferred deferred);
  ~BindAsyncWorker();
  void Execute();
  void OnOK();
  void OnError(Napi::Error const &error);

private:
  LDAP *ld;
  int msgid;
  Napi::Promise::Deferred deferred;
};