# SiriDB-Node.js-Add-on
Node.js add-on (C++) for SiriDB

>WARN: This project is under contruction and not ready for usage.


### Error Codes
The following error codes can be returned:

- `sdbaddon.ERR_MSG` (-64) *General error code*
- `sdbaddon.ERR_QUERY` (-65) *Most likely a syntax error in the query*
- `sdbaddon.ERR_INSERT` (-66) *Most likely the send data is corrupt*
- `sdbaddon.ERR_SERVER` (-67) *The server could not perform the request, you could try another SiriDB server if available*
- `sdbaddon.ERR_POOL` (-68) *At least one pool has no accesible SiriDB server*
- `sdbaddon.ERR_ACCESS` (-69) *The database user has not enough privileges to process the request*,
- `sdbaddon.ERR_RUNTIME` (-70) *Unexpected error has occurred, please check the SiriDB log*
- `sdbaddon.ERR_NOT_AUTHENTICATED` (-71) *The connection is not authenticated*
- `sdbaddon.ERR_CREDENTIALS` (-72) *Supplied credentials are invalid*
- `sdbaddon.ERR_UNKNOWN_DB` (-73) *Trying to authenticate to an unknown database*
- `sdbaddon.ERR_LOADING_DB` (-74) *The database is loading*