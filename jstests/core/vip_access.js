// test limitation when accessd from vip



function debug( x ) {
    printjson( x );
}


// 1. local.startup_log, local.me, local.replset.minvalid, local.system.replset, local.replset.election are forbidden
db = db.getSiblingDB('local');
var invalidCollections = new Array('startup_log', 'me', 'replset.election', 'replset.minvalid', 'system.replset');
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
var c = db.runCommand({listCommands:1});
var invalidCommands = new Array('replSetInitiate', 'replSetRequestVotes', 'authSchemaUpgrade', 'driverOIDTest', 'copydb');
for (var i = 0; i < invalidCommands.length; i++) {
    assert.eq(false, c.commands.hasOwnProperty(invalidCommands[i]), 'unauthorized')
}
db = db.getSiblingDB('admin');
assert.commandFailed(
        db.runCommand({replSetGetConfig: 1})
        );
assert.commandFailed(
        db.runCommand({replSetGetStatus: 1})
        );
assert.commandWorked(
        db.runCommand({getLog: "startupWarnings"})
        );
assert.commandFailed(
        db.runCommand({getLog: "rs"})
        );

// 4. forbid rename admin.system.users 
assert.commandFailed(
        db.runCommand({renameCollection: "admin.system.users", to:"admin.test"})
        );
assert.commandFailed(
        db.runCommand({renameCollection: "admin.test", to:"admin.system.users"})
        );




