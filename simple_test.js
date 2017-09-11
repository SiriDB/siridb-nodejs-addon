// simple_test.js
const sdbaddon = require('./build/Release/siridb');

var siridb = new sdbaddon.SiriDBClient("iris", "siri", "dbtest", "127.0.0.1", 9000);

siridb.onClose((msg) => {
    console.log(msg);
});

siridb.connect((err) => {
    if (err) {
        console.error("Connection error: ", err);
    } else {
        siridb.query("select * from 'aggr'", (resp, status) => {
            console.log('Query Status: ', status);
            console.log(resp);

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
                console.log('Insert Status: ', status);
                console.log(resp);

                /* close connection */
                siridb.close();
            });
        });


    }
});

console.log(sdbaddon);