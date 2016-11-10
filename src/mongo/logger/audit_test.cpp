/*    Copyright (C) 2016 Aliyun Inc.
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
 *    must comply with the GNU Affero General Public License in all respects
 *    for all of the code used other than as permitted herein. If you modify
 *    file(s) with this exception, you may extend this exception to your
 *    version of the file(s), but you are not obligated to do so. If you do not
 *    wish to do so, delete this exception statement from your version. If you
 *    delete this exception statement from all source files in the program,
 *    then also delete it in the license file.
 */

#include "mongo/platform/basic.h"

#include "mongo/logger/audit_test.h"

#include <sstream>
#include <string>
#include <vector>

#include "mongo/bson/bsonobjbuilder.h"
#include "mongo/db/auth/user_name.h"
#include "mongo/db/auth/role_name.h"
#include "mongo/logger/audit_event_utf8_encoder.h"
#include "mongo/platform/compiler.h"
#include "mongo/unittest/unittest.h"
#include "mongo/util/time_support.h"
#include "mongo/util/mongoutils/str.h"

using namespace mongo::logger;

namespace mongo {
namespace {

typedef AuditTest<AuditEventJSONEncoder> AuditTestJSONEncoder;

/**
 * Verifies that the encoded log line contains string.
 */
void testEncodedLogLine(const AuditEventEphemeral& event,
                        Encoder<AuditEventEphemeral>& encoder,
                        const std::string& expectedSubstring) {
    std::ostringstream os;
    ASSERT_TRUE(encoder.encode(event, os));
    std::string s = os.str();
    if (s.find(expectedSubstring) == std::string::npos) {
        FAIL(str::stream() << "encoded log line does not contain substring \"" << expectedSubstring
                           << "\". log line: " << s);
    }
}

// Tests pass through of log component:
//     AuditEventEphemeral -> AuditEventJSONEncoder
TEST_F(AuditTestJSONEncoder, AuditEventEncoder) {
    std::string atype("authenticate");
    Date_t d = Date_t::now();
    std::string localHost = "127.0.0.1";
    int localPort = 27017;
    std::string remoteHost = "10.0.0.1";
    int remotePort = 30000;
    std::vector<UserName> userNames;
    std::vector<RoleName> roleNames;

    long long latencyMicros = 100;

    BSONObjBuilder builder;
    builder.append("user", "user");
    builder.append("db", "test");
    builder.append("mechanism", "SCRAM-SHA-1");
    BSONObj param = builder.obj();

    AuditEventJSONEncoder jsonEncoder;
    AuditEventAliCloudDBEncoder aliEncoder;
    AuditEventEphemeral event(atype, d, localHost, localPort, remoteHost, remotePort, &userNames, &roleNames, latencyMicros, &param, ErrorCodes::OK);
    testEncodedLogLine(event, jsonEncoder, atype);
    testEncodedLogLine(event, aliEncoder, "100");

    globalAuditLogDomain()->append(event);

    ASSERT_EQUALS(1U, _logLines.size());
    ASSERT_NOT_EQUALS(_logLines[0].find(atype), std::string::npos);
}

}  // namespace
}  // namespace mongo
