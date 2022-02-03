const fs = require('fs');

const binding = require("bindings")("zephon_ldap.node");

const ldap = new binding.LDAPCnx('ldap://localhost:10389');

(async () => {
    await ldap.bind('cn=admin,dc=planetexpress,dc=com', 'GoodNewsEveryone');

    const results = await ldap.search('ou=people,dc=planetexpress,dc=com', '(objectClass=inetOrgPerson)', ['*'], ['jpegPhoto']);

    console.log(results);
})();