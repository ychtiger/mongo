var mydb = db.getSiblingDB('readonly_db')
mydb.dropAllUsers();

adminUser = "readOnlyAdmin"
adminPass = "admin" + Math.random();

var admindb = mydb.getMongo().getDB( "admin" );
admindb.createUser({user: adminUser, pwd: adminPass, roles: jsTest.adminUserRoles});

basicUser = "basicUser"
basicPass = "basic" + Math.random();

mydb.createUser({user: basicUser, pwd: basicPass, roles: jsTest.basicUserRoles});

// test just admin user can setReadOnly mode
assert.commandFailed( mydb.runCommand( { setReadOnly:1, duration: 10 } ) )

assert( mydb.auth( basicUser, basicPass ) );
assert.commandFailed( mydb.runCommand( { setReadOnly:1, duration: 10 } ) )

assert( admindb.auth( adminUser, adminPass ) );
assert.commandWorked( admindb.runCommand( { setReadOnly:1, duration: 10 } ) );

// test missed argument
assert.commandFailed( admindb.runCommand( { setReadOnly:1 } ) )

function setReadOnly( durationSecond ) {
    admindb.runCommand( { setReadOnly: 1, duration: durationSecond } )
}

var doc = { a: 1 }

// test avoid insert 
setReadOnly( -1 );
assert.writeError(mydb.foo.insert( doc ));
assert.writeError(admindb.foo.insert( doc ));

setReadOnly( 0 );
assert.writeOK(mydb.foo.insert( doc ));

setReadOnly( 5 );
assert.writeError(mydb.foo.insert( doc ));
sleep(2 * 1000);
result = admindb.runCommand( { setReadOnly: -1 } );

assert.eq( result.readOnly, "enable" )
assert.eq( result.remainSecond, 3 )

sleep(4 * 1000);
assert.writeOK(mydb.foo.insert( doc ));
result = admindb.runCommand( { setReadOnly: -1 } );
assert.eq(result.readOnly, "disable")
assert.eq(result.remainSecond, 0)

setReadOnly( 100 )
setReadOnly( 0 )
result = admindb.runCommand( { setReadOnly: -1 } );
assert.eq(result.readOnly, "disable")
assert.eq(result.remainSecond, 0)


