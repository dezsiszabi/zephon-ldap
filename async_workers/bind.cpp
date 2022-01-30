#include "bind.h"

using namespace Napi;

BindAsyncWorker::BindAsyncWorker(Napi::Env &env, LDAP *ld, std::string dn, std::string password, Napi::Promise::Deferred deferred)
  : Napi::AsyncWorker(env), ld(ld), dn(dn), password(password), deferred(deferred) {}

BindAsyncWorker::~BindAsyncWorker() {}

void BindAsyncWorker::Execute() {
  LDAPMessage *res;
  struct timeval zerotime = { -1, 0 };

  int msgid = ldap_simple_bind(this->ld, this->dn.c_str(), this->password.c_str());

  if (msgid == -1) {
    SetError("Error during ldap_simple_bind");
    return;
  }

  int rc = ldap_result(this->ld, msgid, LDAP_MSG_ALL, &zerotime, &res);

  switch (rc) {
    case -1:
      break; // TODO: handle error
    default:
      if (ldap_result2error(this->ld, res, 1) != LDAP_SUCCESS) {
        // TODO: handle error
      }
  }
}

void BindAsyncWorker::OnOK() {
  deferred.Resolve(Env().Undefined());
}

void BindAsyncWorker::OnError(Napi::Error const &error) {
  deferred.Reject(error.Value());
}