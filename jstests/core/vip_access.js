// test limitation when accessd from vip



function debug( x ) {
    printjson( x );
}


// 1. local.startup_log, local.me, local.replset.minvalid, local.system.replset are forbidden
db = db.getSiblingDB('local');
var invalidCollections = new Array('startup_log', 'me', 'replset.minvalid', 'system.replset');
for (var i = 0; i < invalidCollections.length; i++) {
    var c = db.getCollection(invalidCollections[i]);
    assert.throws(function() { c.findOne(); }, [], 'unauthorized')
}

var colls = db.getCollectionNames();
for (var i = 0; i < invalidCollections.length; i++) {
    assert.eq(colls.indexOf(invalidCollections[i]), -1);
}

// 2. cannot drop admin database
db = db.getSiblingDB('admin');
assert.commandFailed(
        db.runCommand({dropDatabase: 1})
        );

// 3. some commands are forbidden, like replSetGetConfig, replsetSetGetStatus
db = db.getSiblingDB('admin');
assert.commandFailed(
        db.runCommand({replSetGetConfig: 1})
        );
assert.commandFailed(
        db.runCommand({replSetGetStatus: 1})
        );



