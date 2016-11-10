/**
 *    Copyright (C) 2016 Aliyun Inc.
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

/** Unit tests for AuditOpFilter. */

#include "mongo/db/server_options_helpers.h"
#include "mongo/logger/audit_event.h"

#include "mongo/unittest/unittest.h"

namespace mongo {
namespace {

TEST(AuditOpFilter, parseNormal) {
    std::string filterStr = "auth,admin,query";
    int opFilter = 0;
    ASSERT_TRUE(parseAuditOpFilter(filterStr, opFilter));
    ASSERT_EQUALS(opFilter, logger::opAuth | logger::opAdmin | logger::opQuery);

    // trim
    filterStr = "insert, update";
    ASSERT_TRUE(parseAuditOpFilter(filterStr, opFilter));
    ASSERT_EQUALS(opFilter, logger::opInsert | logger::opUpdate);

    // all and off
    filterStr = "all";
    ASSERT_TRUE(parseAuditOpFilter(filterStr, opFilter));
    ASSERT_EQUALS(opFilter, 0xFFFFFFFF);

    filterStr = "off";
    ASSERT_TRUE(parseAuditOpFilter(filterStr, opFilter));
    ASSERT_EQUALS(opFilter, 0);
}

TEST(AuditOpFilter, parseAbNormal) {
    std::string filterStr = "test";
    int opFilter = 0;
    ASSERT_FALSE(parseAuditOpFilter(filterStr, opFilter));
    ASSERT_EQUALS(opFilter, 0);

    filterStr = "admin,auht,query";
    ASSERT_FALSE(parseAuditOpFilter(filterStr, opFilter));
    ASSERT_EQUALS(opFilter, 0);

    filterStr = "all,auth";
    ASSERT_FALSE(parseAuditOpFilter(filterStr, opFilter));
    ASSERT_EQUALS(opFilter, 0);

    filterStr = "auth,off";
    ASSERT_FALSE(parseAuditOpFilter(filterStr, opFilter));
    ASSERT_EQUALS(opFilter, 0);

    filterStr = "all,off";
    ASSERT_FALSE(parseAuditOpFilter(filterStr, opFilter));
    ASSERT_EQUALS(opFilter, 0);
}

}  // namespace
}  // namespace mongo
