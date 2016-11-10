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

#ifndef VTOA_USER_H_INCLUDE
#define VTOA_USER_H_INCLUDE

#include <linux/types.h>

struct vtoa_vs {
	__u32		vid;	/* VPC ID */
	__be32		vaddr;	/* vip */
	__be16		vport;	/* vport */
};

struct vtoa_get_vs {
	struct vtoa_vs vs;
};

struct vtoa_get_vs4rds {
	/* which connection*/
	__u16 protocol;
	__be32 caddr;           /* client address */
	__be16 cport;
	__be32 daddr;           /* destination address */
	__be16 dport;

	/* the virtual servers */
	struct vtoa_vs entrytable[0];
};

#define VTOA_BASE_CTL		(64+1024+64+64+64+64)	/* base */

#define VTOA_SO_GET_VS		(VTOA_BASE_CTL+1)
#define VTOA_SO_GET_VS4RDS	(VTOA_BASE_CTL+2)

#endif
