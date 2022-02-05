const binding = require("bindings")("zephon_ldap.node");

export interface LDAPSaslBindOptions {
    user: string;
    password: string;
    realm: string;
    proxy_user: string;
}

export type LDAPSearchResult<TBinaryAttribute extends string> = ({ [K in TBinaryAttribute]?: ArrayBuffer[]; } & { [attribute: string]: string[]; });

export class LDAP {
    private readonly cnx;

    constructor(url: string) {
        this.cnx = new binding.LDAPCnx(url);
    }

    public bind(dn: string, password: string): Promise<void> {
        return this.cnx.bind(dn, password);
    }

    public saslbind(mechanism: string, options?: LDAPSaslBindOptions): Promise<void> {
        return this.cnx.saslbind(mechanism, options);
    }

    public search<TBinaryAttribute extends string>(base: string, filter: string, attributes: string[] = ['*'], binaryAttributes: TBinaryAttribute[] = []): Promise<LDAPSearchResult<TBinaryAttribute>[]> {
        return this.cnx.search(base, filter, attributes, binaryAttributes);
    }
}