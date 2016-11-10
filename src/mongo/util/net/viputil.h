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

#pragma once

namespace mongo {
    class VipUtil {
    public:
        static void convertVipToStr(int32_t addr, std::string &str_addr);
        static int getVip4rds(int sockfd, struct vtoa_get_vs4rds *vs, socklen_t *len);

        /* 
         * fd: raw fd, create by socket(AF_INET, SOCK_RAW, IPPROTO_RAW)
         * client_host: connection peer host
         * client_port: connection peer port
         * dst_host: connection local host
         * dst_port: connection local port
         * vip_host: vip host
         * vip_port: vip port
         */
        static bool getVipAddr(const int fd, 
                const std::string &client_host, const int32_t client_port, 
                const std::string &dst_host, const int32_t dst_port,
                std::string &vip_host, int32_t &vip_port);

        /**
         * fd: tcp socket fd
         * vip_host: vip host
         * vip_port: vip port
         * vpc_id:   vpc id
         */
        static bool getVipAddr(int fd, 
                std::string &vip_host, int32_t &vip_port, uint32_t &vpc_id);

    };
}

