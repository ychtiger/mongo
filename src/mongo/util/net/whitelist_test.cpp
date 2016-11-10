/**
 *    Copyright (C) 2015 Aliyun Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#include <cstdarg>
#include <string>
#include <vector>

#include "mongo/util/concurrency/mutex.h"
#include "mongo/unittest/unittest.h"
#include "mongo/util/net/whitelist.h"

using namespace mongo;

TEST(TestWhiteList, parseItem) {
    {
       IpRange r;
       WhiteList w;
       ASSERT(w.parseItem("127.21.218.100", r));
       ASSERT_EQUALS(r.min, 2132138596);
       ASSERT_EQUALS(r.max, 2132138596);
    }

    {
       IpRange r;
       WhiteList w;
       ASSERT(w.parseItem("127.21.218.100-127.21.218.200", r));
       ASSERT_EQUALS(r.min, 2132138596);
       ASSERT_EQUALS(r.max, 2132138696);
    }

    {
       IpRange r;
       WhiteList w;
       ASSERT(w.parseItem("127.21.218.100/16", r));
       ASSERT_EQUALS(r.min, 2132082688);
       ASSERT_EQUALS(r.max, 2132148223);
    }

    {
       IpRange r;
       WhiteList w;
       ASSERT(w.parseItem("0.0.0.0/0", r));
       ASSERT_EQUALS(r.min, 0);
       ASSERT_EQUALS(r.max, 0xFFFFFFFF);
    }

    {
       IpRange r;
       WhiteList w;
       ASSERT(!w.parseItem("127.21.257.6553", r));
    }

    {
        IpRange r;
        WhiteList w;
        ASSERT(!w.parseItem("127.21.257.333.100", r));
    }

    {
        IpRange r;
        WhiteList w;
        ASSERT(!w.parseItem("", r));
    }

    {
        IpRange r;
        WhiteList w;
        ASSERT(!w.parseItem("hello", r));
    }

    {
       IpRange r;
       WhiteList w;
       ASSERT(!w.parseItem("127.21.257.100", r));
    }

    {
       IpRange r;
       WhiteList w;
       ASSERT(!w.parseItem("127.21.257.", r));
    }

    {
       IpRange r;
       WhiteList w;
       ASSERT(!w.parseItem("127.21.257.100-127.21.257.200", r));
    }

    {
       IpRange r;
       WhiteList w;
       ASSERT(!w.parseItem("127.21.218.200-127.21.218.100", r));
    }

    {
       IpRange r;
       WhiteList w;
       ASSERT(!w.parseItem("127.21.257.100/40", r));
    }
}

TEST(TestWhiteList, parseFromString) {
    {
        WhiteList w;
        ASSERT(w.parseFromString("0.0.0.0/0"));
        ASSERT_EQUALS(w.rangeSize(), 1);
        ASSERT(w.include(0));
        ASSERT(w.include(0xFFFFFFFF));
        ASSERT(w.include(0xFFFFFFFF / 2));
    }

    {
        WhiteList w;
        ASSERT(w.parseFromString("127.21.218.100"));
        ASSERT_EQUALS(w.rangeSize(), 1);
        ASSERT(w.include("127.21.218.100"));
        ASSERT(!w.include("127.21.218.99"));
        ASSERT(!w.include("127.21.218.101"));
    }

    {
        WhiteList w;
        ASSERT(w.parseFromString("127.21.218.100,127.21.218.100-127.21.218.200"));
        ASSERT_EQUALS(w.rangeSize(), 1);
        ASSERT(w.include("127.21.218.100"));
        ASSERT(w.include("127.21.218.150"));
        ASSERT(w.include("127.21.218.200"));
        ASSERT(!w.include("127.21.218.99"));
        ASSERT(!w.include("127.21.218.201"));
    }

    {
        WhiteList w;
        ASSERT(w.parseFromString("127.21.218.100/24"));
        ASSERT_EQUALS(w.rangeSize(), 1);
        ASSERT(w.include("127.21.218.0"));
        ASSERT(w.include("127.21.218.255"));
        ASSERT(!w.include("127.21.217.1"));
        ASSERT(!w.include("127.21.219.1"));
    }

    {
        WhiteList w;
        ASSERT(w.parseFromString("127.21.218.100,127.21.218.100-127.21.218.200,127.21.218.100/24,0.0.0.0/0"));
        ASSERT_EQUALS(w.rangeSize(), 1);
        ASSERT(w.include("0.0.0.0"));
        ASSERT(w.include("255.255.255.255"));
    }

    {
        WhiteList w;
        ASSERT(w.parseFromString("127.21.218.50-127.21.218.80,127.21.218.100-127.21.218.200,127.21.218.150-127.21.218.210"));
        ASSERT_EQUALS(w.rangeSize(), 2);
        ASSERT(w.include("127.21.218.50"));
        ASSERT(w.include("127.21.218.65"));
        ASSERT(w.include("127.21.218.80"));
        ASSERT(w.include("127.21.218.100"));
        ASSERT(w.include("127.21.218.210"));
        ASSERT(w.include("127.21.218.155"));
    }

}

TEST(TestWhiteList, parseFromFile) {

    std::string path = "/tmp/whitelist";
    FILE* f = fopen(path.c_str(), "w");
    ASSERT(f != NULL);
    std::string str = "127.21.218.50-127.21.218.80,127.21.218.100-127.21.218.200,127.21.218.150-127.21.218.210,127.22.1.1/16";
    fwrite(str.c_str(), 1, str.length(), f);
    fclose(f);

    WhiteList w;
    std::string errmsg;
    ASSERT(w.parseFromFile(path, errmsg));
    ASSERT_EQUALS(w.rangeSize(), 3);
    ASSERT(w.include("127.21.218.50"));
    ASSERT(w.include("127.21.218.65"));
    ASSERT(w.include("127.21.218.80"));
    ASSERT(w.include("127.21.218.100"));
    ASSERT(w.include("127.21.218.210"));
    ASSERT(w.include("127.21.218.155"));
    ASSERT(w.include("127.22.0.0"));
    ASSERT(w.include("127.22.255.255"));
    
}




