#pragma once

#include <map>
#include <napi.h>
#include <ldap.h>

class SearchAsyncWorker : public Napi::AsyncWorker
{
public:
  SearchAsyncWorker(Napi::Env &env, LDAP *ld, int msgid, Napi::Promise::Deferred deferred);
  ~SearchAsyncWorker();
  void Execute();
  void OnOK();
  void OnError(Napi::Error const &error);

private:
  LDAP *ld;
  int msgid;
  std::vector<std::map<std::string, std::vector<std::string>>> results;
  Napi::Promise::Deferred deferred;
};