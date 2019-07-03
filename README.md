# SiriDB-Node.js-Add-on
Node.js add-on (C++) for [SiriDB](https://github.com/SiriDB/siridb-server#readme)

---------------------------------------
  * [Installation](#installation)
  * [Quick usage](#quick-usage)
  * [SiriDBClient](#siridbclient)
    * [connect](#siridbclientconnect)
    * [query](#siridbclientquery)
    * [insert](#siridbclientinsert)
    * [close](#siridbclientclose)
  * [Events](#events)
    * [onClose](#siridbclientonclose)
    * [onError](#siridbclientonerror)
  * [Status codes](#status-codes)
  * [Version info](#version-info)
  
---------------------------------------

## Installation
```
mkdir my_modules
cd my_modules
git clone https://github.com/SiriDB/siridb-nodejs-addon.git siridb
cd ../
npm install ./my_modules/siridb
```

### About Installation

The addon is built using `cmake` and `node-gyp`.


The dependencies are:

* https://github.com/transceptor-technology/libqpack.git
* https://github.com/SiriDB/libsiridb.git
* https://github.com/SiriDB/libsuv.git

The build command is: `node-gyp configure && node-gyp build`

There is `install.sh` script that will install everything, when the package is installed using npm.
It's possible to do the compilation manually, like in install.sh.

The main module file is specified in package.json:

  `"main": "build/Release/obj.target/siridb.node"`

## Quick usage
```javascript
const sdbaddon = require('siridb');

var siridb = new sdbaddon.SiriDBClient(
    "iris", "siri", "dbtest", "localhost", 9000);

siridb.connect(err => {
    if (err) {
        console.error(`Connection error: ${err}`);
    } else {
        siridb.close();
    }
});
```

## SiriDBClient
Create a new SiriDB Client. This creates a new client but `connect()` must be used to connect.
```javascript
var siridb = new sdbaddon.SiriDBClient(
    "iris",         // database user
    "siri",         // password
    "dbtest",       // database name
    "localhost",    // server address
    9000            // server port
);
```

### SiriDBClient.connect
Connect to SiriDB. A callback function can be used to check if the connect is successful.
```javascript
siridb.connect(err => {
    // success: err is null
    // error:   err is a string with an error message
    if (err) {
        console.error("Connection error: ", err);
    }
});
```

### SiriDBClient.query
Query SiriDB. Requires a string containing the query and a callback function to catch the result.

The callback function will be called with two arguments:  
 - first argument: A response Object
 - second argument: Number indicating the status. The status is 0 when successful or a negative value in case of an error.
   (see [Status codes](#status-codes) for the possible status codes)
   
```javascript
siridb.query("select * from /.*series/", (resp, status) => {
    // success: status is 0 and resp is an Object containing the data
    // error:   status < 0 and resp.error_msg contains a description about the error
    if (status) {
        console.error(`Query error: ${resp.error_msg} (${status})`);
    } else {
        console.log(resp);  // query data
    }
});
```

### SiriDBClient.insert
Insert time series data into SiriDB. Requires an Array with at least one series Object.string containing the query and a callback function to catch the result.

The callback function will be called with two arguments: 
 - first argument: A response Object
 - second argument: Number indicating the status. The status is 0 when successful or a negative value in case of an error.
   (see [Status codes](#status-codes) for the possible status codes)
   
```javascript
var series = [{
    type: 'float',    // float or integer
    name: 'example',  // name
    points: [         // array with points
        [1505118253, 5.4],  // time-stamp, value
        [1505118307, 7.1]   // etc.
    ]
}];

siridb.insert(series, (resp, status) => {
    // success: status is 0 and resp.success_msg contains a description about the successful insert
    // error:   status < 0 and resp.error_msg contains a description about the error
    if (status) {
        console.error(`Insert error: ${resp.error_msg} (${status})`);
    } else {
        console.log(resp.success_msg);  // insert message
    }
});
```

### SiriDBClient.close
Close the connection.
```javascript
siridb.close();
```

## Events
### SiriDBClient.onClose
Will be triggered when the connction is closed or lost. This event will also be triggered when the connection is closed by
the [close()](#siridbclientclose) function.

```javascript
siridb.onClose(msg => {
    console.log(msg);  // msg is a String
});
```

### SiriDBClient.onError
Will be triggered ***only*** when corrupt or broken data is received on the connection. This is very unlikely to happen but in case something is broken you can use this event to do something, for example close and rebuild the connection.

```javascript
siridb.onError(msg => {
    console.error(msg);  // msg is a String
});
```

## Status codes
Sometimes its useful to act on a specific error, for example you might want to retry the request in case of `ERR_SERVER` while a `ERR_INSERT` error indicates something is wrong with the data.

The following status codes can be returned:

- `sdbaddon.ERR_MSG` (-64) *General error*
- `sdbaddon.ERR_QUERY` (-65) *Most likely a syntax error in the query*
- `sdbaddon.ERR_INSERT` (-66) *Most likely the data is invalid or corrupt*
- `sdbaddon.ERR_SERVER` (-67) *The server could not perform the request, you could try another SiriDB server*
- `sdbaddon.ERR_POOL` (-68) *At least one pool has no online SiriDB server*
- `sdbaddon.ERR_ACCESS` (-69) *The database user has not enough privileges to process the request*,
- `sdbaddon.ERR_RUNTIME` (-70) *Unexpected error has occurred, please check the SiriDB log*
- `sdbaddon.ERR_NOT_AUTHENTICATED` (-71) *The connection is not authenticated*
- `sdbaddon.ERR_CREDENTIALS` (-72) *Credentials are invalid*
- `sdbaddon.ERR_UNKNOWN_DB` (-73) *Trying to authenticate to an unknown database*
- `sdbaddon.ERR_LOADING_DB` (-74) *The database is loading*

## Version info
Use `sdbaddon.VERSION` for version information.
