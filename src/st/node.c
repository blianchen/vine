/*
 * node.c
 *
 *  Created on: Mar 9, 2015
 *      Author: blc
 */

#include <unistd.h>
#include <stdint.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <exception/net_exception.h>
#include <exception/sys_exception.h>
#include <logger.h>
#include <timeutil.h>

#include "common.h"

///////////////////////////////////////////////////////////
////
////////////////////////////////////////////////////////////
//#include <ifaddrs.h>
// getifaddrs 不能获得mac地址
static uint64_t vpmd_uid() {
	int fd;
	struct ifreq ifq[16];
	struct ifconf ifc;

	uint64_t rv;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0) {
		THROW(net_exception, "Create socket error: %s(errno: %d)", getLastErrorText(), getLastError());
		return 0;
	}
	ifc.ifc_len = sizeof(ifq);
	ifc.ifc_buf = (caddr_t)ifq;
	if(ioctl(fd, SIOCGIFCONF, (char *)&ifc)) {
		THROW(net_exception, "ioctl SIOCGIFCONF error: %s(errno: %d)", getLastErrorText(), getLastError());
		return 0;
	}

	int num = ifc.ifc_len / sizeof(struct ifreq);
	int i;
	for(i=0; i<num; i++)
	{
		rv = 0;

		if(ioctl(fd, SIOCGIFFLAGS, (char *)&ifq[i])) {
			LOG_WARN("ioctl SIOCGIFFLAGS error: %s(errno: %d)", getLastErrorText(), getLastError());
			continue;
		}
		if ((ifq[i].ifr_flags & IFF_LOOPBACK) || !(ifq[i].ifr_flags & IFF_UP) ) {
			// is 127.0.0.1 or 接口非激活状态
			continue;
		}
//		printf("flag : %d , %d, ", ifq[i].ifr_flags & IFF_LOOPBACK, ifq[i].ifr_flags & IFF_UP);

		if(ioctl(fd, SIOCGIFHWADDR, (char *)&ifq[i])) {
			LOG_WARN("ioctl SIOCGIFHWADDR error: %s(errno: %d)", getLastErrorText(), getLastError());
			continue;
		}
		// mac, 1 byte
		rv = rv | ifq[i].ifr_hwaddr.sa_data[5];
//		rv = ( ((rv | ifq[i].ifr_hwaddr.sa_data[4]) << 8) | ifq[i].ifr_hwaddr.sa_data[5] ) << 16;
//		printf ("%X:%X:%X:%X:%X:%X \n", (unsigned char) ifq[i].ifr_hwaddr.sa_data[0], (unsigned char) ifq[i].ifr_hwaddr.sa_data[1], (unsigned char) ifq[i].ifr_hwaddr.sa_data[2], (unsigned char) ifq[i].ifr_hwaddr.sa_data[3], (unsigned char) ifq[i].ifr_hwaddr.sa_data[4], (unsigned char) ifq[i].ifr_hwaddr.sa_data[5]);

		// ip, 2 byte
		rv = rv << 16;
		if(ioctl(fd, SIOCGIFADDR, (char *)&ifq[i])) {
			LOG_WARN("ioctl SIOCGIFADDR error: %s(errno: %d)", getLastErrorText(), getLastError());
			continue;
		}
		rv = rv | ( htonl(((struct sockaddr_in*)(&ifq[i].ifr_addr))->sin_addr.s_addr) & 0xffff );
		break;
//		printf("%s", inet_ntoa(((struct sockaddr_in*)(&ifq[i].ifr_addr))->sin_addr));
	}
	close(fd);

	// time in 10 millisecond, 18 bit
	rv = rv << 18;
	struct timeval t;
	if (gettimeofday(&t, NULL) != 0) {
		THROW(sys_exception, "gettimeofday error: %s(errno: %d)", getLastErrorText(), getLastError());
		return rv;
	}
	rv = rv | ( ((t.tv_sec * 100) + (t.tv_usec / 10000)) & 0x3ffff );

	return rv;
}

uint64_t node_uid() {
//	uint64_t nodeid = 0;
//	nodeid = (nodeid | mac_ip_last_byte()) << 18;
//	nodeid = nodeid | (time_now() & 0x3ffff);
	return vpmd_uid();
}
