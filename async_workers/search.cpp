#include "search.h"

using namespace Napi;

SearchAsyncWorker::SearchAsyncWorker(Napi::Env &env, LDAP *ld, std::string base, std::string filter, std::vector<std::string> attributes, std::vector<std::string> binary_attributes, Napi::Promise::Deferred deferred)
    : Napi::AsyncWorker(env), ld(ld), base(base), filter(filter), attributes(attributes), binary_attributes(binary_attributes), deferred(deferred) {}

SearchAsyncWorker::~SearchAsyncWorker() {}

void SearchAsyncWorker::Execute()
{
  LDAPControl *page_control[2];
  LDAPMessage *res;
  BerElement *ber;
  LDAPMessage *entry;
  char *dn, *attrname;
  struct berval **vals;
  int num_vals;
  int msgid;

  int i, j, rc;
  struct timeval zerotime = {-1, 0};

  std::vector<const char *> attrs;
  std::transform(std::begin(this->attributes), std::end(this->attributes), std::back_inserter(attrs), std::mem_fn(&std::string::c_str));
  attrs.push_back(NULL);

  memset(&page_control, 0, sizeof(page_control));

  if (ldap_search_ext(this->ld, this->base.c_str(), 2, this->filter.c_str(), (char **)attrs.data(), 0, page_control, NULL, NULL, 0, &msgid) != LDAP_SUCCESS)
  {
    SetError("Error searching ldap");
    return;
  }

  rc = ldap_result(this->ld, msgid, LDAP_MSG_ALL, &zerotime, &res);
  switch (rc)
  {
  case -1:
    SetError("Error searching ldap");
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
          results[i][std::string(attrname)].push_back(std::string(vals[j]->bv_val, vals[j]->bv_len));
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
        if (std::find(this->binary_attributes.begin(), this->binary_attributes.end(), attr.first) != this->binary_attributes.end())
        {
          auto buffer = Napi::ArrayBuffer::New(Env(), attr.second[j].length());
          memcpy(buffer.Data(), attr.second[j].c_str(), attr.second[j].length());
          attr_array[j] = buffer;
        }
        else
        {
          attr_array[j] = Napi::String::New(Env(), attr.second[j]);
        }
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