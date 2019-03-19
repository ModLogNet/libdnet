/*
 * route.c
 *
 * Copyright (c) 2019 Muhammad Moinur Rahmman <5u623l20@protonmail.com>
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 * Copyright (c) 1999 Masaki Hirabaru <masaki@merit.edu>
 * 
 * $Id$
 */

#include "config.h"

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#define route_t	oroute_t	/* XXX - unixware */
#include <net/route.h>
#undef route_t
#include <netinet/in.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dnet.h"

#define ROUNDUP(a) \
	((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

#ifdef HAVE_SOCKADDR_SA_LEN
#define NEXTSA(s) \
	((struct sockaddr *)((u_char *)(s) + ROUNDUP((s)->sa_len)))
#else
#define NEXTSA(s) \
	((struct sockaddr *)((u_char *)(s) + ROUNDUP(sizeof(*(s)))))
#endif

struct route_handle {
	int	fd;
	int	seq;
};

#ifdef DEBUG
static void
route_msg_print(struct rt_msghdr *rtm)
{
	printf("v: %d type: 0x%x flags: 0x%x addrs: 0x%x pid: %d seq: %d\n",
	    rtm->rtm_version, rtm->rtm_type, rtm->rtm_flags,
	    rtm->rtm_addrs, rtm->rtm_pid, rtm->rtm_seq);
}
#endif

static int
route_msg(route_t *r, int type, struct addr *dst, struct addr *gw)
{
	struct addr net;
	struct rt_msghdr *rtm;
	struct sockaddr *sa;
	u_char buf[BUFSIZ];
	pid_t pid;
	int len;

	memset(buf, 0, sizeof(buf));

	rtm = (struct rt_msghdr *)buf;
	rtm->rtm_version = RTM_VERSION;
	if ((rtm->rtm_type = type) != RTM_DELETE)
		rtm->rtm_flags = RTF_UP;
	rtm->rtm_addrs = RTA_DST;
	rtm->rtm_seq = ++r->seq;

	/* Destination */
	sa = (struct sockaddr *)(rtm + 1);
	if (addr_net(dst, &net) < 0 || addr_ntos(&net, sa) < 0)
		return (-1);
	sa = NEXTSA(sa);

	/* Gateway */
	if (gw != NULL && type != RTM_GET) {
		rtm->rtm_flags |= RTF_GATEWAY;
		rtm->rtm_addrs |= RTA_GATEWAY;
		if (addr_ntos(gw, sa) < 0)
			return (-1);
		sa = NEXTSA(sa);
	}
	/* Netmask */
	if (dst->addr_ip == IP_ADDR_ANY || dst->addr_bits < IP_ADDR_BITS) {
		rtm->rtm_addrs |= RTA_NETMASK;
		if (addr_btos(dst->addr_bits, sa) < 0)
			return (-1);
		sa = NEXTSA(sa);
	} else
		rtm->rtm_flags |= RTF_HOST;
	
	rtm->rtm_msglen = (u_char *)sa - buf;
#ifdef DEBUG
	route_msg_print(rtm);
#endif
	if (write(r->fd, buf, rtm->rtm_msglen) < 0)
		return (-1);

	pid = getpid();
	
	while (type == RTM_GET && (len = read(r->fd, buf, sizeof(buf))) > 0) {
		if (len < (int)sizeof(*rtm)) {
			return (-1);
		}
		if (rtm->rtm_type == type && rtm->rtm_pid == pid &&
		    rtm->rtm_seq == r->seq) {
			if (rtm->rtm_errno) {
				errno = rtm->rtm_errno;
				return (-1);
			}
			break;
		}
	}
	if (type == RTM_GET && (rtm->rtm_addrs & (RTA_DST|RTA_GATEWAY)) ==
	    (RTA_DST|RTA_GATEWAY)) {
		sa = (struct sockaddr *)(rtm + 1);
		sa = NEXTSA(sa);
		
		if (addr_ston(sa, gw) < 0 || gw->addr_type != ADDR_TYPE_IP) {
			errno = ESRCH;
			return (-1);
		}
	}
	return (0);
}

route_t *
route_open(void)
{
	route_t *r;
	
	if ((r = calloc(1, sizeof(*r))) != NULL) {
		r->fd = -1;
		if ((r->fd = socket(PF_ROUTE, SOCK_RAW, AF_INET)) < 0)
			return (route_close(r));
	}
	return (r);
}

int
route_add(route_t *r, const struct route_entry *entry)
{
	struct route_entry rtent;
	
	memcpy(&rtent, entry, sizeof(rtent));
	
	if (route_msg(r, RTM_ADD, &rtent.route_dst, &rtent.route_gw) < 0)
		return (-1);
	
	return (0);
}

int
route_delete(route_t *r, const struct route_entry *entry)
{
	struct route_entry rtent;
	
	memcpy(&rtent, entry, sizeof(rtent));
	
	if (route_get(r, &rtent) < 0)
		return (-1);
	
	if (route_msg(r, RTM_DELETE, &rtent.route_dst, &rtent.route_gw) < 0)
		return (-1);
	
	return (0);
}

int
route_get(route_t *r, struct route_entry *entry)
{
	if (route_msg(r, RTM_GET, &entry->route_dst, &entry->route_gw) < 0)
		return (-1);
	
	return (0);
}

#if defined(HAVE_SYS_SYSCTL_H)
int
route_loop(route_t *r, route_handler callback, void *arg)
{
	struct rt_msghdr *rtm;
	struct route_entry entry;
	struct sockaddr *sa;
	char *buf, *lim, *next;
	int ret;
	int mib[6] = { CTL_NET, PF_ROUTE, 0, 0 /* XXX */, NET_RT_DUMP, 0 };
	size_t len;
	
	if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0)
		return (-1);

	if (len == 0)
		return (0);
	
	if ((buf = malloc(len)) == NULL)
		return (-1);
	
	if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
		free(buf);
		return (-1);
	}
	lim = buf + len;
	next = buf;
	for (ret = 0; next < lim; next += rtm->rtm_msglen) {
		rtm = (struct rt_msghdr *)next;
		sa = (struct sockaddr *)(rtm + 1);

		if (addr_ston(sa, &entry.route_dst) < 0 ||
		    (rtm->rtm_addrs & RTA_GATEWAY) == 0)
			continue;

		sa = NEXTSA(sa);
		
		if (addr_ston(sa, &entry.route_gw) < 0)
			continue;
		
		if (entry.route_dst.addr_type != entry.route_gw.addr_type ||
		    (entry.route_dst.addr_type != ADDR_TYPE_IP &&
			entry.route_dst.addr_type != ADDR_TYPE_IP6))
			continue;

		if (rtm->rtm_addrs & RTA_NETMASK) {
			sa = NEXTSA(sa);
			if (addr_stob(sa, &entry.route_dst.addr_bits) < 0)
				continue;
		}
		if ((ret = callback(&entry, arg)) != 0)
			break;
	}
	free(buf);
	
	return (ret);
}
#elif defined(HAVE_NET_RADIX_H)
/* XXX - Tru64, others? */
#include <nlist.h>

static int
_kread(int fd, void *addr, void *buf, int len)
{
	if (lseek(fd, (off_t)addr, SEEK_SET) == (off_t)-1L)
		return (-1);
	return (read(fd, buf, len) == len ? 0 : -1);
}

static int
_radix_walk(int fd, struct radix_node *rn, route_handler callback, void *arg)
{
	struct radix_node rnode;
	struct rtentry rt;
	struct sockaddr_in sin;
	struct route_entry entry;
	int ret = 0;
 again:
	_kread(fd, rn, &rnode, sizeof(rnode));
	if (rnode.rn_b < 0) {
		if (!(rnode.rn_flags & RNF_ROOT)) {
			_kread(fd, rn, &rt, sizeof(rt));
			_kread(fd, rt_key(&rt), &sin, sizeof(sin));
			addr_ston((struct sockaddr *)&sin, &entry.route_dst);
			if (!(rt.rt_flags & RTF_HOST)) {
				_kread(fd, rt_mask(&rt), &sin, sizeof(sin));
				addr_stob((struct sockaddr *)&sin,
				    &entry.route_dst.addr_bits);
			}
			_kread(fd, rt.rt_gateway, &sin, sizeof(sin));
			addr_ston((struct sockaddr *)&sin, &entry.route_gw);
			if ((ret = callback(&entry, arg)) != 0)
				return (ret);
		}
		if ((rn = rnode.rn_dupedkey))
			goto again;
	} else {
		rn = rnode.rn_r;
		if ((ret = _radix_walk(fd, rnode.rn_l, callback, arg)) != 0)
			return (ret);
		if ((ret = _radix_walk(fd, rn, callback, arg)) != 0)
			return (ret);
	}
	return (ret);
}

int
route_loop(route_t *r, route_handler callback, void *arg)
{
	struct radix_node_head *rnh, head;
	struct nlist nl[2];
	int fd, ret = 0;

	memset(nl, 0, sizeof(nl));
	nl[0].n_name = "radix_node_head";
	
	if (knlist(nl) < 0 || nl[0].n_type == 0 ||
	    (fd = open("/dev/kmem", O_RDONLY, 0)) < 0)
		return (-1);
	
	for (_kread(fd, (void *)nl[0].n_value, &rnh, sizeof(rnh));
	    rnh != NULL; rnh = head.rnh_next) {
		_kread(fd, rnh, &head, sizeof(head));
		/* XXX - only IPv4 for now... */
		if (head.rnh_af == AF_INET) {
			if ((ret = _radix_walk(fd, head.rnh_treetop,
				 callback, arg)) != 0)
				break;
		}
	}
	close(fd);
	return (ret);
}
#else
int
route_loop(route_t *r, route_handler callback, void *arg)
{
	errno = ENOSYS;
	return (-1);
}
#endif

route_t *
route_close(route_t *r)
{
	if (r != NULL) {
		if (r->fd >= 0)
			close(r->fd);
		free(r);
	}
	return (NULL);
}
