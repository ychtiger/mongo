#!/bin/bash
chmod 600 ./jstests/libs/authTestsKey
scons -j 12 --nostrip=NOSTRIP --opt=off --ssl=SSL --prefix=`echo $HOME`/mongo-install/ all install
./buildscripts/errorcodes.py --report=markdown -o errors.md

