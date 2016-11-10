// check dbaas mongo interface


function assertIsNumber(value) {
  assert(typeof(value) == 'number');
}

function assertIsString(value) {
  assert(typeof(value) == 'string');
}

function assertIsBool(value) {
  assert(typeof(value) == 'boolean');
}

function assertIsNumberLong(value) {
  assert(typeof(value) == 'object');
  assert(value.toString().startsWith('NumberLong'));
}

function assertIsTimestamp(value) {
  assert(typeof(value) == 'object');
  assert(value.toString().startsWith('Timestamp'));
}

function assertIsISODate(value) {
  assert(typeof(value) == 'object');
  assert(value.toString().indexOf('GMT') != -1);
}

// perf_agent use serverStatus
var serverStatus = db.adminCommand({serverStatus:1});

var opCounters = serverStatus['opcounters'];
assertIsNumber(opCounters.insert);
assertIsNumber(opCounters.query);
assertIsNumber(opCounters.update);
assertIsNumber(opCounters.delete);
assertIsNumber(opCounters.getmore);
assertIsNumber(opCounters.command);

var asserts = serverStatus['asserts'];
assertIsNumber(asserts.regular);
assertIsNumber(asserts.warning);
assertIsNumber(asserts.msg);
assertIsNumber(asserts.user);

var mem = serverStatus['mem'];
assertIsNumber(mem.resident);
assertIsNumber(mem.virtual);
assertIsNumber(mem.mapped);

assertIsNumber(serverStatus.connections.current);

var metrics = serverStatus['metrics'];
var cursor = metrics['cursor'];
assertIsNumberLong(cursor.open.total);
assertIsNumberLong(cursor.timedOut);

var network = serverStatus['network'];
assertIsNumberLong(network.bytesIn);
assertIsNumberLong(network.bytesOut);
assertIsNumberLong(network.numRequests);

var globalLockCurrentQueues = serverStatus['globalLock']['currentQueue']
assertIsNumber(globalLockCurrentQueues.total);
assertIsNumber(globalLockCurrentQueues.readers);
assertIsNumber(globalLockCurrentQueues.writers);

assertIsNumber(serverStatus['extra_info']['page_faults'])  

var wiredTiger = serverStatus['wiredTiger']
var wtCache = wiredTiger['cache']
assertIsNumberLong(wtCache['bytes read into cache'])
assertIsNumber(wtCache['bytes written from cache'])
assertIsNumber(wtCache['maximum bytes configured'])

var wtConcurrentTrans = wiredTiger['concurrentTransactions']
assertIsNumber(wtConcurrentTrans['write']['out'])
assertIsNumber(wtConcurrentTrans['write']['available'])
assertIsNumber(wtConcurrentTrans['read']['out'])
assertIsNumber(wtConcurrentTrans['read']['out'])

// ismaster, no need to check
// must compatible with MongoDB driver

// replSetGetConfig
var config = db.adminCommand(  {replSetGetConfig: 1} );
var c = config.config;
var m = c.members[0];
assertIsString(c._id);
assertIsNumber(c.version);
assertIsNumber(m._id);
assertIsString(m.host);
assertIsBool(m.arbiterOnly);
assertIsBool(m.buildIndexes);
assertIsBool(m.hidden);
assertIsNumber(m.priority);
assertIsNumberLong(m.slaveDelay);
assertIsNumber(m.votes);

// replSetGetStatus
var s = db.adminCommand(  {replSetGetStatus: 1} );
var m = s.members[0];
assertIsString(s.set);
assertIsISODate(s.date);
assertIsNumber(s.myState);
assertIsNumberLong(s.term);
assertIsNumberLong(s.heartbeatIntervalMillis);
assertIsNumber(m._id);
assertIsString(m.name);
assertIsNumber(m.health);
assertIsNumber(m.state);
assertIsString(m.stateStr);
assertIsNumber(m.uptime);
assertIsTimestamp(m.optime);
assertIsISODate(m.optimeDate);
assertIsTimestamp(m.electionTime);
assertIsISODate(m.electionDate);
assertIsNumber(m.configVersion);


