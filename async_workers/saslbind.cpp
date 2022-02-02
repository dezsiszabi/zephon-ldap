#include <sasl/sasl.h>
#include "saslbind.h"

using namespace Napi;

static int sasl_callback(LDAP *ld, unsigned flags, void *defaults, void *sasl_interact)
{
  struct sasl_defaults *user_provided_sasl_defaults = (struct sasl_defaults *)defaults;
  sasl_interact_t *current_sasl_interact;

  for (current_sasl_interact = (sasl_interact_t *)sasl_interact; current_sasl_interact->id != SASL_CB_LIST_END; current_sasl_interact++)
  {
    switch (current_sasl_interact->id)
    {
    case SASL_CB_AUTHNAME:
      current_sasl_interact->result = user_provided_sasl_defaults->user;
      break;
    case SASL_CB_PASS:
      current_sasl_interact->result = user_provided_sasl_defaults->password;
      break;
    case SASL_CB_USER:
      current_sasl_interact->result = user_provided_sasl_defaults->proxy_user;
      break;
    case SASL_CB_GETREALM:
      current_sasl_interact->result = user_provided_sasl_defaults->realm;
      break;
    default:
      break;
    }
  }

  return SASL_OK;
}

SaslBindAsyncWorker::SaslBindAsyncWorker(
    Napi::Env &env,
    LDAP *ld,
    std::string mechanism,
    std::string *user,
    std::string *password,
    std::string *realm,
    std::string *proxy_user,
    Napi::Promise::Deferred deferred) : Napi::AsyncWorker(env), ld(ld), mechanism(mechanism), user(user), password(password), realm(realm), proxy_user(proxy_user), deferred(deferred)
{
  this->defaults.user = user == NULL ? NULL : user->c_str();
  this->defaults.password = password == NULL ? NULL : password->c_str();
  this->defaults.realm = realm == NULL ? NULL : realm->c_str();
  this->defaults.proxy_user = proxy_user == NULL ? NULL : proxy_user->c_str();
}

SaslBindAsyncWorker::~SaslBindAsyncWorker() {
  if (this->user) delete this->user;
  if (this->password) delete this->password;
  if (this->realm) delete this->realm;
  if (this->proxy_user) delete this->proxy_user;
}

void SaslBindAsyncWorker::Execute()
{
  LDAPControl **ctrls = NULL;
  char **refs = NULL;
  char *matched = NULL;
  char *info = NULL;
  LDAPControl **sctrlsp = NULL;
  int rc, msgid, err;

  LDAPMessage *result = NULL;
  const char *rmech = NULL;

  do
  {
    rc = ldap_sasl_interactive_bind(this->ld, NULL, this->mechanism.c_str(),
                                    sctrlsp, NULL, LDAP_SASL_QUIET, sasl_callback, &this->defaults,
                                    result, &rmech, &msgid);

    if (rc != LDAP_SASL_BIND_IN_PROGRESS)
    {
      break;
    }

    ldap_msgfree(result);

    if (ldap_result(this->ld, msgid, LDAP_MSG_ALL, NULL, &result) == -1 || !result)
    {
      ldap_get_option(this->ld, LDAP_OPT_RESULT_CODE, (void *)&err);
      ldap_get_option(this->ld, LDAP_OPT_DIAGNOSTIC_MESSAGE, (void *)&info);
      ldap_memfree(info);
      SetError("Error during ldap_sasl_interactive_bind");
      return;
    }
  } while (rc == LDAP_SASL_BIND_IN_PROGRESS);

  if (rc != LDAP_SUCCESS)
  {
    ldap_get_option(this->ld, LDAP_OPT_DIAGNOSTIC_MESSAGE, (void *)&info);
    ldap_memfree(info);
    SetError("Error during ldap_sasl_interactive_bind");
    return;
  }

  if (result)
  {
    rc = ldap_parse_result(this->ld, result, &err, &matched, &info, &refs, &ctrls, 1);
    if (rc != LDAP_SUCCESS)
    {
      SetError("Error during ldap_sasl_interactive_bind");
      return;
    }
  }
}

void SaslBindAsyncWorker::OnOK()
{
  deferred.Resolve(Env().Undefined());
}

void SaslBindAsyncWorker::OnError(Napi::Error const &error)
{
  deferred.Reject(error.Value());
}