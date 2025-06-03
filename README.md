That service provider suppliles services for all robotkernel modules
with support for key value devices. The service provider will provide
acyclic calls to access the key value entries.

# Configuration

No specific configuration is needed for the service provider. The
robotkernel just needs to know which service provider to load. To load
der *service_provider_key_value* just add it to your config file.

```yaml
service_providers:
- name: kv_sp
  so_file: libservice_provider_key_value.so
```

# Services

**read**
:   Returns a list *values* with the values corresponding to the
    reqeusted *keys*. On error, the *error_message* field will be filled
    with the error cause.

```yaml
request:
- vector/uint32_t: keys
response:
- vector/string: values
- string: error_message
```

**write**
:   Writes key-value-pairs to the device. The lists *keys* and *values*
    must be of equal size. On error, the *error_message* field will be
    filled with the error cause.

```yaml
request:
- vector/uint32_t: keys
- vector/string: values
response:
- string: error_message
```

**list**
:   Returns a list with all available *keys* and their *names*. On
    error, the *error_message* field will be filled with the error
    cause.

```yaml
response:
- vector/uint32_t: keys
- vector/string: names
- string: error_message
```

**list_descriptions**
:   Returns the available description fiels of all keys.

```yaml
response:
- vector/string: description
- vector/string: unit
- vector/string: default_value
- vector/string: format
- vector/uint8_t: read_only
```

All of these services will be available through a robotkernel brigdge
(e.g. [bridge_ln](robotkernel-5/bridge_ln "wikilink"),
[bridge_jsonrpc](robotkernel-5/bridge_jsonrpc "wikilink"),
[bridge_cli](robotkernel-5/bridge_cli "wikilink"), \...)

[Serivice Provider Key Value](Category:Robotkernel-5 "wikilink")
