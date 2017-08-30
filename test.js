// test.js
const addon = require('./build/Release/addon');
var sleep = require('sleep-async')();

var test = function() {

    var obj = new addon.SiriDBClient("iris", "siri", "dbtest", "127.0.0.1", 9000);
    obj.connect(function (res) {
        console.log(res);
    });
        
    sleep.sleep(10000, function(){
  
        console.log('Finished...');
        obj.close();
    });
    
    
}
 
test()




if (global.gc !== undefined) {
    global.gc();
}

