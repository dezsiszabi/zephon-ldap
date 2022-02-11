const binding = require("bindings")("zephon_ldap.node");

export interface LDAPSaslBindOptions {
    user: string;
    password: string;
    realm: string;
    proxy_user: string;
}

export interface LdapSearchOptions<TBinaryAttribute> {
    base: string;
    filter: string;
    attributes?: string[];
    binaryAttributes?: TBinaryAttribute[];
    retryAttempts?: number;
}

export type LDAPSearchResult<TBinaryAttribute extends string> = ({ [K in TBinaryAttribute]?: ArrayBuffer[]; } & { [attribute: string]: string[]; });

export class LDAP {
    private lastBind;
    private cnx;

    constructor(private url: string) {
        this.connect();
    }

    public bind(dn: string, password: string): Promise<void> {
        this.lastBind = () => this.cnx.bind(dn, password);
        return this.lastBind();
    }

    public saslbind(mechanism: string, options?: LDAPSaslBindOptions): Promise<void> {
        this.lastBind = () => this.saslbind(mechanism, options);
        return this.lastBind();
    }

    public async search<TBinaryAttribute extends string>(options: LdapSearchOptions<TBinaryAttribute>): Promise<LDAPSearchResult<TBinaryAttribute>[]> {
        const retryAttempts = options.retryAttempts ?? 0;
        try {
            const results = await this.cnx.search(options.base, options.filter, options.attributes ?? ['*'], options.binaryAttributes ?? []);
            return results;
        } catch (error) {
            if (retryAttempts > 0) {;
                this.connect();
                await this.lastBind();
                return this.search({ ...options, retryAttempts: retryAttempts - 1 });
            } else {
                throw error;
            }
        }
    }

    public close(): void {
        this.cnx.close();
    }

    private connect(): void {
        this.cnx = new binding.LDAPCnx(this.url);
    }
}