// simple_test.js
const addon = require('./build/Release/addon');

var siridb = new addon.SiriDBClient("iris", "siri", "dbtest", "127.0.0.1", 9000);

siridb.onClose(function (msg) {
    console.log(msg);
});

siridb.connect((err) => {
    if (err) {
        console.error("Connection error: ", err);
    } else {
        siridb.query("list servers", function (resp, status) {
            data = JSON.parse(resp);
            if (status < 0) {
                console.error(`Query error (${status}): `, data);
            } else {
                console.log(data);
            }            
            siridb.close();
        });
    }
});
