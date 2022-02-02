#pragma once

#include <napi.h>
#include <ldap.h>

typedef struct sasl_defaults
{
  const char *user = NULL;
  const char *password = NULL;
  const char *realm = NULL;
  const char *proxy_user = NULL;
} sasl_defaults;

class SaslBindAsyncWorker : public Napi::AsyncWorker
{
public:
  SaslBindAsyncWorker(Napi::Env &env, LDAP *ld, std::string mechanism, std::string *user, std::string *password, std::string *realm, std::string *proxy_user, Napi::Promise::Deferred deferred);
  ~SaslBindAsyncWorker();
  void Execute();
  void OnOK();
  void OnError(Napi::Error const &error);

private:
  LDAP *ld;
  std::string mechanism;
  std::string *user;
  std::string *password;
  std::string *realm;
  std::string *proxy_user;
  sasl_defaults defaults;
  Napi::Promise::Deferred deferred;
};