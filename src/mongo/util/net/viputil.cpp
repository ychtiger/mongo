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

#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kNetwork

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "mongo/util/log.h"
#include "mongo/util/net/slb_ctk_user.h"
#include "mongo/util/net/vtoa_user.h"
#include "mongo/util/net/viputil.h"

namespace mongo {

    void VipUtil::convertVipToStr(int32_t addr, std::string &str_addr)
    {
        char str[64] = {0x0};
        unsigned char *bytes = (unsigned char *)&addr;
        sprintf(str, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
        str_addr.assign(str);
    }

    bool VipUtil::getVipAddr(const int fd, 
            const std::string &client_host, const int32_t client_port, 
            const std::string &dst_host, const int32_t dst_port, 
            std::string &vip_host, int32_t &vip_port)
    {
        int ret = 0;
        struct slb_ctk_vs_entry *e;
        struct slb_ctk_get_vs *vs;
        socklen_t len = sizeof(struct slb_ctk_get_vs) + sizeof(struct slb_ctk_vs_entry);
        char buf[CTK_VS_BUF_LEN] = {0x0};
        vs = (struct slb_ctk_get_vs *)buf;
        vs->protocol = IPPROTO_TCP;
        vs->caddr = inet_addr(client_host.c_str());
        vs->cport = htons(client_port);
        vs->daddr = inet_addr(dst_host.c_str());
        vs->dport = htons(dst_port);
        if ((ret = getsockopt(fd, IPPROTO_IP, CTK_PROXY_SO_GET_VS, vs, &len)) < 0)
        {
            LOG(3) << "Access by rip" 
                << " clientAddr=" << client_host << ":" << client_port 
                << " serverAddr=" << dst_host << ":" << dst_port;
            return false;
        }
        e = (struct slb_ctk_vs_entry *)vs->entrytable;
        convertVipToStr(e->vaddr, vip_host);
        vip_port = ntohs(e->vport);
        LOG(3) << "Access by vip"
              << " clientAddr=" << client_host << ":" << client_port
              << " serverAddr=" << dst_host << ":" << dst_port 
              << " vipAddr=" << vip_host << ":" << vip_port;
        return true;
    }

    int VipUtil::getVip4rds(int sockfd, struct vtoa_get_vs4rds *vs, socklen_t *len)
    {	
        struct sockaddr_in saddr, daddr;
        int ret = 0;
        socklen_t saddrlen, daddrlen;

        if (*len != sizeof(struct vtoa_get_vs4rds) + sizeof(struct vtoa_vs)) {
            return -EINVAL;
        }

        saddrlen = sizeof(saddr);
        if ((ret = getpeername(sockfd, (struct sockaddr *)&saddr, &saddrlen)) < 0) {
            return ret;
        }

        daddrlen = sizeof(daddr);
        if ((ret = getsockname(sockfd, (struct sockaddr *)&daddr, &daddrlen)) < 0) {
            return ret;
        }

        vs->protocol = IPPROTO_TCP;
        vs->caddr = saddr.sin_addr.s_addr;
        vs->cport = saddr.sin_port;
        vs->daddr = daddr.sin_addr.s_addr;
        vs->dport = daddr.sin_port;

        std::string shost, dhost;
        convertVipToStr(saddr.sin_addr.s_addr, shost);
        convertVipToStr(daddr.sin_addr.s_addr, dhost);
        
        return getsockopt(sockfd, IPPROTO_IP, VTOA_SO_GET_VS4RDS, vs, len);
    }

    bool VipUtil::getVipAddr(int fd, 
            std::string &vip_host, int32_t &vip_port) {
        char buf[CTK_VS_BUF_LEN] = {0x0};
        struct vtoa_get_vs4rds *vs = (struct vtoa_get_vs4rds*)buf;
        struct vtoa_vs *e;
        socklen_t len = sizeof(struct vtoa_get_vs4rds) + sizeof(struct vtoa_vs);
        if (getVip4rds(fd, vs, &len) < 0) {
            return false;
        }
        e = vs->entrytable;
        convertVipToStr(e->vaddr, vip_host);
        vip_port = ntohs(e->vport);
        LOG(2) << "Access by vip fd=" << fd 
               << " vipAddr=" << vip_host << ":" << vip_port;
        return true;
    }
}

