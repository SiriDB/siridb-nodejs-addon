// simple_test.js
console.log('Hallo!!!');

const addon = require('./build/Release/addon');

var siridb = new addon.SiriDBClient("iris", "siri", "dbtest", "127.0.0.1", 9000);

siridb.onClose(function (msg) {
    console.log(msg);
});

siridb.connect((err) => {
    if (err) {
        console.error("Connection error: ", err);
    } else {
        siridb.query("select * from 'aggr'", function (resp, status) {
            console.log('Status: ', status);
            console.log(resp);
                        
            siridb.close();
        });
    }
});
