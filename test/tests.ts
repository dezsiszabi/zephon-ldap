import { LDAP } from "../main";

const ldap = new LDAP('ldap://localhost:10389');

(async () => {
    await ldap.bind('cn=admin,dc=planetexpress,dc=com', 'GoodNewsEveryone');

    const results = await ldap.search('ou=people,dc=planetexpress,dc=com', '(objectClass=inetOrgPerson)', ['*'], ['jpegPhoto', 'uid']);

    console.log(results);
})();