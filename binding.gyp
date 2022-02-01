{
  'variables': {
    'SASL': '<!(test -f /usr/include/sasl/sasl.h && echo y || echo n)'
  },
  'targets': [
    {
      'target_name': 'zephon_ldap',
      'sources': [
        './main.cpp',
        './cnx.cpp',
        './async_workers/search.cpp',
        './async_workers/bind.cpp',
        './async_workers/saslbind.cpp'
      ],
      'dependencies': [
        '<!(node -p "require(\'node-addon-api\').gyp")'
      ],
      'include_dirs': [
        '/usr/local/include',
        '<!@(node -p "require(\'node-addon-api\').include")'
      ],
      'defines': [
        'NAPI_CPP_EXCEPTIONS',
        'LDAP_DEPRECATED'
      ],
      'ldflags': [
        '-L/usr/local/lib'
      ],
      'cflags_cc': [
        '-Wall',
        '-Wextra',
        '-fexceptions'
      ],
      'libraries': [
        '-lldap'
      ],
      'conditions': [
        [
          'SASL==\"y\"',
          {
            'defines': [
              'HAVE_CYRUS_SASL'
            ]
          }
        ]
      ]
    }
  ]
}