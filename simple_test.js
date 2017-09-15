// simple_test.js
const sdbaddon = require('./build/Release/siridb');

var ntest = 2;
var siridb = new sdbaddon.SiriDBClient(
    "iris",         // database user
    "siri",         // password
    "dbtest",       // database name
    "localhost",    // server address
    9000            // server port
);

function test_query(siridb) {
    siridb.query("select * from /.*series/", (resp, status) => {
        console.log(`Query Status: ${status}`);
        console.log(resp);
        if (!--ntest) siridb.close();
    });
}

function test_insert(siridb) {    
    siridb.insert([{
        type: 'float',
        name: 'some float series',
        points: [
            [1505118253, 5.4],
            [1505118307, 7.1]
        ]
    }, {
        type: 'integer',
        name: 'some integer series',
        points: [
            [1505118253, 5],
            [1505118307, 7]
        ]
    }], (resp, status) => {
        console.log(`Insert Status: ${status}`);
        console.log(resp);
        if (!--ntest) siridb.close();
    });
}

siridb.onClose((msg) => {
    console.log(msg);
});

siridb.connect((err) => {
    if (err) {
        console.error(`Connection error: ${err}`);
    } else {
        test_query(siridb);
        test_insert(siridb);
    }
});
