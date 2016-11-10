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

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <string>

#include "mongo/db/repl/repl_set_iocheck.h"
#include "mongo/util/exit.h"
#include "mongo/util/time_support.h"
#include "mongo/db/storage/storage_options.h"
#include "mongo/db/server_parameters.h"
#include "mongo/util/log.h"

namespace mongo {
 
int kBlockSize = 4096;
int checkFileFd = -1;
volatile unsigned long long lastCheckTime = 0;

// modify these param by setParameter online
MONGO_EXPORT_SERVER_PARAMETER(ioCheckEnable, bool, true);
MONGO_EXPORT_SERVER_PARAMETER(ioCheckInterval, int, 10);
MONGO_EXPORT_SERVER_PARAMETER(ioHangMaxSeconds, int, 30);
MONGO_EXPORT_SERVER_PARAMETER(ioHangMinSeconds, int, 1);

// export current hang status by getParameter
MONGO_EXPORT_SERVER_PARAMETER(ioHanging, bool, false);  

// periodically write(DIRECT_IO) a file in dbpath to detect io hang
// we think io hang if write never return or cost longer than ioHangMaxSeconds
// we reset the iohang status when latancey decreased to ioHangMinSeconds

void runCheckIO() {
    unsigned long long checkBegin = 0;
    unsigned long long checkEnd = 0;
    std::string checkFile = storageGlobalParams.dbpath + "/iocheck.dat";
    void *buf = memalign(kBlockSize, kBlockSize);
    memset(buf, 0, kBlockSize);
    while (!inShutdown()) {
        checkBegin = curTimeMillis64();
        if (ioCheckEnable) {
            if (checkFileFd < 0) {
                checkFileFd = open(checkFile.c_str(), O_DIRECT | O_CREAT | O_RDWR, 0644);
                // shouldn't happend, just let it fail and try next time
                // we don't want to bother the main thread
                if (checkFileFd < 0) {
                    warning() << "open iocheck file failed " << strerror(errno);
                }
            }

            if (checkFileFd >= 0) {
                sprintf((char*)buf, "%llu", checkBegin);
                if (pwrite(checkFileFd, buf, kBlockSize, 0) < 0) {
                    warning() << "write iocheck file failed " << strerror(errno);
                }
            }
        } 
        checkEnd = curTimeMillis64();

        if (isIOHang(checkEnd)) {
            // when IO hang detected, reset when io latency decrease to ioHangMinSeconds
            if ((checkEnd - checkBegin) <= static_cast<unsigned>(ioHangMinSeconds * 1000)) {
                lastCheckTime =  checkEnd;
            }
        } else {
            lastCheckTime = checkEnd;
        }

        // server may shutdown during sleep
        for (int i = 0; i < ioCheckInterval && !inShutdown(); i++) {
            sleepsecs(1);
        }
    }

    if (checkFileFd >= 0) {
        close(checkFileFd);
    }

    free(buf);
}

bool isIOHang(unsigned long long now) {
    bool hang = false;
    if (ioCheckEnable && lastCheckTime != 0) {
        if (now - lastCheckTime > static_cast<unsigned>(ioHangMaxSeconds * 1000)) {
            hang = true;
        }
    } 
    ioHanging = hang;
    return hang;
}

bool isIOHang() {
    unsigned long long now = curTimeMillis64();
    return isIOHang(now);
}

}


