// test.js
const addon = require('./build/Release/addon');
const sleep = require('sleep-async')();
const RETRY = 5;

var connect = function(siridb, cb) {
    console.log("Trying to connect...")     
    siridb.connect((err) => {

        if (err === null) {
            console.log("Successful connected!");
            if (cb) {
                cb();
            }
            return;          
        } 
        console.error(err);
        retry(siridb);
    });
}

var retry = function (siridb) {
    if (!siridb.retry) return;
    console.log(`Retry connect in ${RETRY} seconds...`);
    sleep.sleep(RETRY * 1000, function() {
        if (!siridb.retry) return;
        connect(siridb);
    });            
}

var test = function() {

    var siridb = new addon.SiriDBClient("iris", "siri", "dbtest", "127.0.0.1", 9000);
    siridb.retry = true;
    connect(siridb, () => {
        siridb.query("show", function (resp, err) {
            if (resp !== null) {
                data = JSON.parse(resp);
                console.log(data);
            }
            if (err !== null) {
                console.error("error: ", err);
            }
        });
    });

    siridb.onClose((msg) => {
        console.log("JS On Close: ", msg);
        retry(siridb);
    });


    
    // obj.onerror((errmsg) => {
    //     console.error(errmsg);
    // });
        
    sleep.sleep(20000, function(){
        console.log('Finished...');
        siridb.retry = false;
        siridb.close();
    });
    
    
}
 
test()


if (global.gc !== undefined) {
    global.gc();
}

