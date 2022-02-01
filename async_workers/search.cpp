#include "search.h"

using namespace Napi;

SearchAsyncWorker::SearchAsyncWorker(Napi::Env &env, LDAP *ld, int msgid, Napi::Promise::Deferred deferred)
    : Napi::AsyncWorker(env), ld(ld), msgid(msgid), deferred(deferred) {}

SearchAsyncWorker::~SearchAsyncWorker() {}

void SearchAsyncWorker::Execute()
{
  LDAPMessage *res;
  BerElement *ber;
  LDAPMessage *entry;
  char *dn, *attrname;
  struct berval **vals;
  int num_vals;

  int i, j, rc;
  struct timeval zerotime = {-1, 0};

  rc = ldap_result(this->ld, this->msgid, LDAP_MSG_ALL, &zerotime, &res);
  switch (rc)
  {
  case -1:
    SetError("Error during ldap search");
    break;
  case LDAP_RES_SEARCH_ENTRY:
  case LDAP_RES_SEARCH_RESULT:
    for (entry = ldap_first_entry(this->ld, res), i = 0; entry; entry = ldap_next_entry(this->ld, entry), i++)
    {
      dn = ldap_get_dn(this->ld, entry);
      results.push_back({
          {"dn", {std::string(dn)}},
      });

      for (attrname = ldap_first_attribute(this->ld, entry, &ber); attrname; attrname = ldap_next_attribute(this->ld, entry, ber))
      {
        vals = ldap_get_values_len(this->ld, entry, attrname);
        num_vals = ldap_count_values_len(vals);

        for (j = 0; j < num_vals; j++)
        {
          results[i][std::string(attrname)].push_back(std::string(vals[j]->bv_val));
        }

        ldap_value_free_len(vals);
        ldap_memfree(attrname);
      }
    }
  default:
    break;
  }

  ldap_msgfree(res);
}

void SearchAsyncWorker::OnOK()
{
  Napi::Array res_array = Napi::Array::New(Env(), results.size());
  for (uint i = 0; i < results.size(); i++)
  {
    Napi::Object obj = Napi::Object::New(Env());
    for (auto const &attr : results[i])
    {
      Napi::Array attr_array = Napi::Array::New(Env(), attr.second.size());
      for (uint j = 0; j < attr.second.size(); j++)
      {
        attr_array[j] = Napi::String::New(Env(), attr.second[j]);
      }
      obj.Set(attr.first, attr_array);
    }
    res_array[i] = obj;
  }
  deferred.Resolve(res_array);
}

void SearchAsyncWorker::OnError(Napi::Error const &error)
{
  deferred.Reject(error.Value());
}