import { LDAP } from "../main";

function sleep(ms) {
    return new Promise<void>(resolve => setTimeout(() => resolve(), ms));
}

const ldap = new LDAP('ldap://localhost:10389');

(async () => {
    await ldap.bind('cn=admin,dc=planetexpress,dc=com', 'GoodNewsEveryone');

    await ldap.search({
        base: 'ou=people,dc=planetexpress,dc=com',
        filter: '(objectClass=inetOrgPerson)',
        attributes: ['uid'],
        binaryAttributes: ['jpegPhoto'],
        retryAttempts: 5
    });

    await sleep(45000);

    await ldap.search({
        base: 'ou=people,dc=planetexpress,dc=com',
        filter: '(objectClass=inetOrgPerson)',
        attributes: ['uid'],
        retryAttempts: 5
    });

    ldap.close();
})();