#!/bin/bash

cd ../
git archive --prefix=ali-dds-kernel-$3/ -o src.tar.gz HEAD
cd rpm
rpm_create ali-dds-kernel.spec -v $3 -r $4 -s
