// readonly_cmd.cpp

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

#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kAccessControl

#include "mongo/platform/basic.h"
#include "mongo/db/commands/readonly_cmd.h"

#include <string>
#include <vector>

#include "mongo/util/log.h"
#include "mongo/db/commands.h"
#include "mongo/db/auth/authorization_manager_global.h"

namespace mongo {

    using std::endl;
    using std::string;
    using std::stringstream;

    const static long ONEYEAR = 60 * 60 * 24 * 365; // second

    class SetReadOnlyCommand : public Command {
    public:
        static const char* url() { return "http://dochub.mongodb.org/core/setreadonlycommand"; }
        
        SetReadOnlyCommand() : Command( "setReadOnly" ) { }
        virtual bool isWriteCommandForConfigServer() const { return false; }
        virtual bool slaveOk() const { return true; }
        virtual bool adminOnly() const { return true; }
        virtual void help(stringstream& h) const { 
            h << "set db read-only mode to ENABLE, avoid write operation"; 
        }
        virtual void addRequiredPrivileges(const std::string& dbname,
                                           const BSONObj& cmdObj,
                                           std::vector<Privilege>* out) {
            ActionSet actions;
            actions.addAction(ActionType::readonly);
            out->push_back(Privilege(ResourcePattern::forClusterResource(), actions));
        }
        virtual bool run(OperationContext* txn, const string& dbname, BSONObj& cmdObj, int, string& errmsg, BSONObjBuilder& result, bool fromRepl) {
            bool setMark = false;
            BSONElement setMarkElt = cmdObj.getField("setReadOnly"); 
            if (!setMarkElt.eoo()) {
                setMark = setMarkElt.numberInt() > 0;
            }
            int duration = 0;
            if (setMark) {
                BSONElement durationElt = cmdObj["duration"];
                if (durationElt.eoo()) {
                    errmsg =  "setReadOnly: set read only mode enable, must specifies duration";
                    return false; 
                }
                duration = durationElt.numberInt();
                if (duration > ONEYEAR) {
                    stringstream ss;
                    ss << "setReadOnly: Cannot set duration longer than one year (" << ONEYEAR << " seconds)";
                    errmsg = ss.str();
                    return false;
                }
            }
            log() << "CMD setReadOnly: setReadOnly:" << setMark << " duration:" << duration << endl;

            if (setMark) {
                log() << "db read-only mode is now enable, no writes allowed. db.setReadOnly({\"duration\":0}) to disable read-only" << endl;
                log() << "    For more info see " << SetReadOnlyCommand::url() << endl;
                getGlobalAuthorizationManager()->setReadOnlyExpire(duration);
            }
            long long remain = getGlobalAuthorizationManager()->getReadOnlyRemainSecond();
            // get current read-only mode
            result.append( "readOnly", remain == 0 ? "disable" : "enable" );
            result.appendNumber( "remainSecond", remain );
            result.appendDate( "expireDate", remain > 0 ? 
                Date_t( getGlobalAuthorizationManager()->getReadOnlyExpire() * 1000 ) :
                Date_t( curTimeMillis64()) );
            return true;
        }
    } setReadOnlyCmd;
}

