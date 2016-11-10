// netvip_cmd.cpp

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

#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kReplication

#include "mongo/platform/basic.h"

#include <string>

#include "mongo/util/log.h"
#include "mongo/util/net/hostandport.h"
#include "mongo/db/client.h"
#include "mongo/db/commands.h"
#include "mongo/db/repl/replication_coordinator_global.h"

namespace mongo {

    using std::endl;
    using std::string;
    using std::stringstream;

    using namespace mongo::repl;

    class NetVipCommand : public Command {
    public:
        static const char* url() { return "http://dochub.mongodb.org/core/netvipcommand"; }


        NetVipCommand() : Command("netvip") { }

        virtual bool isWriteCommandForConfigServer() const { return false; }
        virtual bool slaveOk() const { return true; }
        virtual bool adminOnly() const { return true; }
        virtual void help(stringstream& h) const {
            h << "set vip address to replicaset hosts list";
        }

        virtual void addRequiredPrivileges(const std::string& dbname,
                                           const BSONObj& cmdObj,
                                           std::vector<Privilege>* out) {
            ActionSet actions;
            actions.addAction(ActionType::netvip);
            out->push_back(Privilege(ResourcePattern::forClusterResource(), actions));
        }

        virtual bool run(OperationContext* txn,
                         const string& dbname,
                         BSONObj& cmdObj,
                         int,
                         string& errmsg,
                         BSONObjBuilder& result) {
            int firstNumber = cmdObj.firstElement().numberInt();
            bool setMark = false;
            if (firstNumber == 1)
                setMark = true;
            else if (firstNumber == -1)
                setMark = false;
            else {
                errmsg = "bad option";
                return false;
            }
            ReplicationCoordinator* replCoord = getGlobalReplicationCoordinator();
            result.append("vip", toStringArray(replCoord->getNetVip()));

            HostAndPort remote = txn->getClient()->getRemote();
            log() << "CMD netvip: netvip:" << setMark
                  << " from " << remote.host() << ":" << remote.port() << endl;

            if (setMark) {
                std::vector<BSONElement> vipElts = cmdObj.getField("vip").Array();
                std::vector<HostAndPort> tmpNetVip;
                for (size_t i = 0; i < vipElts.size(); ++i) {
                    BSONElement &elt = vipElts[i];
                    LOG(3) << "CMD netvip seq " << i << " " << elt.toString() << endl;
                    HostAndPort hp(elt.String());
                    if (!hp.hasPort()) {
                        errmsg = "must specifically port";
                        return false;
                    }
                    tmpNetVip.push_back(hp);
                }
                replCoord->setNetVip(tmpNetVip);
            }
            return true;
        }

    private:
        std::vector<std::string> toStringArray(const std::vector<HostAndPort> &hosts) {
            std::vector<std::string> addrs;
            for (size_t i = 0; i < hosts.size(); ++i) {
                addrs.push_back(hosts[i].toString());
            }
            return addrs;
        }
    } netVipCmd;
}


