/** test builtin user features 
 *  builtin-user can be created by localhost only
 */

// RUN-TEST: python buildscripts/resmoke.py jstests/auth/builtin_user.js

var NUM_NODES = 3;
var rsTest = new ReplSetTest({ nodes: NUM_NODES });
rsTest.startSet({ oplogSize: 10, keyFile: 'jstests/libs/key1' });
rsTest.initiate();
rsTest.awaitSecondaryNodes();

var setupConn = rsTest.getPrimary();
var admin = setupConn.getDB('admin')

// setup initial data 
var normalUser1 = 'normalUser1';
var normalUser2 = 'normalUser2';
var normalUser3 = 'normalUser3';
var normalUser4 = 'normalUser4';
var normalUser5 = 'normalUser5';
var normalUser6 = 'normalUser6';
var builtinUser1 = '__cloud_admin1';
var builtinUser2 = '__cloud_admin2';
var builtinUser3 = '__cloud_admin3';
var builtinUser4 = '__cloud_admin4';
var builtinUser5 = '__cloud_admin5';
var builtinUser6 = '__cloud_admin6';

// create 2 normal users and 2 builtin users
admin.createUser({ user: builtinUser1, pwd: 'root', roles: ['__system'] });

// builtin user's view
var builtinConn = new Mongo("localhost:" + setupConn.port);
var builtinAdmin = builtinConn.getDB('admin');
builtinAdmin.auth(builtinUser1, 'root');

// create users by builtin user
builtinAdmin.createUser({ user: builtinUser2, pwd: 'root', roles: ['root'] });
builtinAdmin.createUser({ user: normalUser1, pwd: 'root', roles: ['__system'] });
builtinAdmin.createUser({ user: normalUser2, pwd: 'root', roles: ['root'] });

// normal user's view
var normalConn = new Mongo("localhost:" + setupConn.port);
var normalAdmin = normalConn.getDB('admin');
normalAdmin.auth(normalUser1, 'root');

// usersInfo Command
jsTest.log('Test usersInfo Command')
var userList = normalAdmin.getUsers();
assert.eq(userList.length, 2)

var userList = builtinAdmin.getUsers();
assert.eq(userList.length, 4)

// createUser Command
jsTest.log('Test createUser Command')
normalAdmin.createUser({ user: normalUser3, pwd: 'root', roles: ['root'] });
assert.throws(function() { normalAdmin.createUser({ user: builtinUser3, pwd: 'root', roles: ['root'] }) },
        [], "normal user cannnot create builtin user");

builtinAdmin.createUser({ user: normalUser4, pwd: 'root', roles: ['root'] });
builtinAdmin.createUser({ user: builtinUser4, pwd: 'root', roles: ['root'] });

var userList = normalAdmin.getUsers();
assert.eq(userList.length, 4)

var userList = builtinAdmin.getUsers();
assert.eq(userList.length, 7)

// dropUser Command
jsTest.log('Test dropUser Command')
normalAdmin.dropUser(normalUser2);
assert.throws(function() { normalAdmin.dropUser(cloudUser2) }, 
        [], 'normal user cannot drop builtin user')

builtinAdmin.runCommand({dropUser: normalUser4})
builtinAdmin.runCommand({dropUser: builtinUser4})

var userList = normalAdmin.getUsers();
assert.eq(userList.length, 2)

var userList = builtinAdmin.getUsers();
assert.eq(userList.length, 4)

// updateUser Command
jsTest.log('Test updateUser Command');
builtinAdmin.createUser({ user: normalUser5, pwd: 'root', roles: ['root'] });
builtinAdmin.createUser({ user: builtinUser5, pwd: 'root', roles: ['root'] });

normalAdmin.updateUser(normalUser5, {pwd: 'root2'});
assert.throws(function() { normalAdmin.updateUser(builtinUser5, {pwd: 'root2'}) }, 
        [], 'normal user cannot update builtin user');

builtinAdmin.updateUser(normalUser5, {pwd: 'root3'});
builtinAdmin.updateUser(builtinUser5, {pwd: 'root3'});

// grantRolesToUser
jsTest.log('Test grantRolesToUser Command');
normalAdmin.grantRolesToUser(normalUser5, ['root']);
assert.throws(function() { normalAdmin.grantRolesToUser(builtinUser5, ['root']) }, 
        [], 'normal user cannot grant roles to builtin user');

builtinAdmin.grantRolesToUser(normalUser5, ['root']);
builtinAdmin.grantRolesToUser(builtinUser5, ['root']);

// revokeRolesFromUser
jsTest.log('Test revokeRolesFromUser Command');
normalAdmin.revokeRolesFromUser(normalUser5, ['root']);
assert.throws(function() { normalAdmin.revokeRolesFromUser(builtinUser5, ['root']) }, 
        [], 'normal user cannot revoke roles from builtin user');

builtinAdmin.revokeRolesFromUser(normalUser5, ['root']);
builtinAdmin.revokeRolesFromUser(builtinUser5, ['root']);

// filter builtin users for db.system.users.find()
jsTest.log('Test find()')
var result = normalAdmin.system.users.find();
assert.eq(result.toArray().length, 3);
var result = builtinAdmin.system.users.find();
assert.eq(result.toArray().length, 6);

// filter builtin users for find command on system.users collection
jsTest.log('Test find Command')
var result = normalAdmin.runCommand({find: 'system.users'});
assert.eq(result.cursor.firstBatch.length, 3);
var result = normalAdmin.runCommand({find: 'system.users', filter: {user: builtinUser1}});
assert.eq(result.cursor.firstBatch.length, 0);
var result = builtinAdmin.runCommand({find: 'system.users'});
assert.eq(result.cursor.firstBatch.length, 6);
var result = builtinAdmin.runCommand({find: 'system.users', filter: {user: builtinUser1}});
assert.eq(result.cursor.firstBatch.length, 1);

// forbidden command on system.users collection
// group, count, distinct, aggregate, mapreduce
jsTest.log('Test aggregate Command')
var result = normalAdmin.runCommand({count: 'system.users', query: {}});
assert.eq(result.n, 3);
var result = builtinAdmin.runCommand({count: 'system.users', query: {}});
assert.eq(result.n, 6);

var result = normalAdmin.runCommand({distinct: 'system.users', key: 'user', query: {}});
assert.eq(result.values.length, 3);
var result = builtinAdmin.runCommand({distinct: 'system.users', key: 'user', query: {}});
assert.eq(result.values.length, 6);

assert.commandFailed(normalAdmin.runCommand(
            {group: {ns: 'system.users', key: 'user', $reduce: tojson, initial: {}}}));
assert.commandWorked(builtinAdmin.runCommand(
            {group: {ns: 'system.users', key: 'user', $reduce: tojson, initial: {}}}));

assert.commandFailed(normalAdmin.runCommand(
            {aggregate: 'system.users', pipeline: [{$sort: {user: 1}}]}));
assert.commandWorked(builtinAdmin.runCommand(
            {aggregate: 'system.users', pipeline: [{$sort: {user: 1}}]}));

// insert/update/delete command on system.users collection
jsTest.log('Test insert/delete/update/findAndModify Command')
assert.commandFailed(normalAdmin.runCommand(
            {findAndModify: 'system.users', query: {user: normalUser5}, remove: true}));
assert.commandWorked(builtinAdmin.runCommand(
            {findAndModify: 'system.users', query: {user: normalUser5}, remove: true}));

assert.commandFailed(normalAdmin.runCommand(
            {delete: 'system.users', deletes: [{q: {user: normalUser6}, limit: 1}]}));
assert.commandWorked(builtinAdmin.runCommand(
            {delete: 'system.users', deletes: [{q: {user: normalUser6}, limit: 1}]}));

assert.commandFailed(normalAdmin.runCommand(
            {update: 'system.users', updates: [{q: {user: normalUser6}, u: {$set: {user: normalUser6}}}]}));
assert.commandWorked(builtinAdmin.runCommand(
            {update: 'system.users', updates: [{q: {user: normalUser6}, u: {$set: {user: normalUser6}}}]}));

var userList = normalAdmin.getUsers();
assert.eq(userList.length, 2)

var userList = builtinAdmin.getUsers();
assert.eq(userList.length, 5)

assert.commandFailed(normalAdmin.runCommand(
            {insert: 'system.users', documents: [{user: builtinUser6, db: 'admin'}, {user: normalUser6, db: 'admin'}]}));

assert.commandWorked(builtinAdmin.runCommand(
            {insert: 'system.users', documents: [{user: builtinUser6, db: 'admin'}, {user: normalUser6, db: 'admin'}]}));

var userList = normalAdmin.getUsers();
assert.eq(userList.length, 3)

var userList = builtinAdmin.getUsers();
assert.eq(userList.length, 7)

// dropAllUserFromDatabase command
jsTest.log('Test dropAllUserFromDatabase Command')
normalAdmin.dropAllUsers()

var userList = builtinAdmin.getUsers();
assert.eq(userList.length, 4)

builtinAdmin.dropAllUsers()

// logout
normalAdmin.logout();
builtinAdmin.logout();

// stop ReplSet
rsTest.stopSet();

