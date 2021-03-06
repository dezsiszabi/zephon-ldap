#pragma once

#include <map>
#include <napi.h>
#include <ldap.h>

class SearchAsyncWorker : public Napi::AsyncWorker
{
public:
  SearchAsyncWorker(Napi::Env &env, LDAP *ld, std::string base, std::string filter, std::vector<std::string> attributes, std::vector<std::string> binary_attributes, Napi::Promise::Deferred deferred);
  ~SearchAsyncWorker();
  void Execute();
  void OnOK();
  void OnError(Napi::Error const &error);

private:
  LDAP *ld;
  std::string base;
  std::string filter;
  std::vector<std::string> attributes;
  std::vector<std::string> binary_attributes;
  std::vector<std::map<std::string, std::vector<std::string>>> results;
  Napi::Promise::Deferred deferred;
};