/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/tun.c                                                *
 * Created:     2004-12-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2018 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include <config.h>

#include <lib/tun.h>

#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#if defined(HAVE_POLL_H)
#include <poll.h>
#elif defined(HAVE_SYS_POLL_H)
#include <sys/poll.h>
#endif
#include <sys/socket.h>

#include <linux/if.h>
#include <linux/if_tun.h>


/*
 * Flags: IFF_TUN   - TUN device (no Ethernet headers)
 *        IFF_TAP   - TAP device
 *
 *        IFF_NO_PI - Do not provide packet information
 */

static
int tuntap_open (const char *name, int tap)
{
	int    fd;
	struct ifreq req;

	fd = open ("/dev/net/tun", O_RDWR);

	if (fd < 0) {
		return (-1);
	}

	memset (&req, 0, sizeof (req));

	req.ifr_flags = (tap ? IFF_TAP : IFF_TUN) | IFF_NO_PI;
	strncpy (req.ifr_name, name, IFNAMSIZ - 1);

	if (ioctl (fd, TUNSETIFF, (void *) &req) < 0) {
		close (fd);
		return (-1);
	}

	return (fd);
}

int tun_open (const char *name)
{
	return (tuntap_open (name, 0));
}

int tap_open (const char *name)
{
	return (tuntap_open (name, 1));
}

void tun_close (int fd)
{
	close (fd);
}

int tun_set_packet (int fd, const void *buf, unsigned cnt)
{
	ssize_t r;

	r = write (fd, buf, cnt);

	if (r < 0) {
		return (1);
	}

	if ((unsigned) r != cnt) {
		return (1);
	}

	return (0);
}

int tun_get_packet (int fd, void *buf, unsigned *cnt)
{
	ssize_t r;

	r = read (fd, buf, *cnt);
	if (r < 0) {
		return (1);
	}

	*cnt = (unsigned) r;

	return (0);
}

int tun_check_packet (int fd)
{
	int           r;
	struct pollfd pfd[1];

	pfd[0].fd = fd;
	pfd[0].events = POLLIN;

	r = poll (pfd, 1, 0);
	if (r < 0) {
		return (0);
	}

	if ((pfd[0].revents & POLLIN) == 0) {
		return (0);
	}

	return (1);
}

