const binding = require("bindings")("zephon_ldap.node");

const ldap = new binding.LDAPCnx('ldap://ldap.forumsys.com:389');

(async () => {
    await ldap.bind('cn=read-only-admin,dc=example,dc=com', 'password');

    const results = await ldap.search('dc=example,dc=com', 'objectClass=person', ['*']);

    console.log(results);
})();