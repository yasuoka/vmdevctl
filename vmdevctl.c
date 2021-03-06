/*
 * Copyright (c) 2007 YASUOKA Masahiko <yasuoka@yasuoka.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/types.h>

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define	VMNDEV		52

typedef struct {
	int	 device_id;
	char	 device_name[40];
	bool	 connected;
} vm_device_info;

static int   vm_get_device_info (int, vm_device_info *);
static int   vm_device_set_connect (int, int);
static void  usage (void);


static void
usage(void)
{
	extern char	*__progname;

	fprintf(stderr, "usage: %s [device_id onoff]\n", __progname);
}

int
main(int argc, char *argv[])
{
	int		 i, ch, devid, onoff = 0, found;
	vm_device_info	 dev;
	extern int	 optind;

	while ((ch = getopt(argc, argv, "h")) != -1)
		switch (ch) {
		case 'h':
		case '?':
			usage();
			exit(1);
		}

	argc -= optind;
	argv += optind;

	found = 0;
	devid = -1;
	if (argc == 2) {
		devid = atoi(argv[0]);
		if (devid < 0 || VMNDEV <= devid) {
			fprintf(stderr, "device-id must be 0-%d.\n",
			    VMNDEV - 1);
			exit(EX_USAGE);
		}
		if (strcmp(argv[1], "on") == 0)
			onoff = 1;
		else if (strcmp(argv[1], "off") == 0)
			onoff = 0;
		else {
			fprintf(stderr, "onoff must be 'on' or 'off'.\n");
			exit(EX_USAGE);
		}
		argc -= 2;
		argv += 2;
	}
	if (argc != 0) {
		usage();
		exit(EX_USAGE);
	}

	printf("---------------------------------------------------\n");
	printf("Id. Device Name                          Connected\n");
	printf("===================================================\n");
	for (i = 0; i < VMNDEV; i++) {
		if (vm_get_device_info(i, &dev) == 0) {
			printf("%2d: %-36s %s",
			    dev.device_id, dev.device_name,
			    (dev.connected)? "true" : "false");
			if (devid >= 0 && dev.device_id == devid) {
				found = 1;
				if (vm_device_set_connect(devid, onoff) != 0) {
					fprintf(stderr,
					    "vm_device_set_connect() "
					    "failed.\n");
					exit(1);
				}
				if (vm_get_device_info(i, &dev) != 0) {
					fprintf(stderr,
					    "vm_get_device_info() failed.\n");
					exit(1);
				}
				printf(" => %s",
				    (dev.connected)? "true" : "false");
			}
			printf("\n");
		}
	}
	printf("--------------------------------------------------\n");
	if (devid > 0 && !found) {
		fprintf(stderr, "device id not found: %d\n", devid);
		exit(1);
	}

	exit(EXIT_SUCCESS);
}

static int
vm_get_device_info(int device_id, vm_device_info *pdevinfo)
{
	int		 off;
	uint32_t	 ebx, rval, info;

	pdevinfo->device_name[36] = '\0';
	off = 0;
	while (off < 40) {
		ebx = device_id << 16 | off;
		__asm__ __volatile__(
			    "movl	$0x564D5868, %%eax	\n\t"
			    "movl	%0, %%ebx		\n\t"
			    "mov	$0xb, %%ecx		\n\t"
			    "mov	$0x5658, %%dx		\n\t"
			    "inl	%%dx, %%eax		\n\t"
			    "movl	%%eax, %1		\n\t"
			    "movl	%%ebx, %2		\n"
		:	"=m"(ebx), "=m"(rval), "=m"(info)
		::      "%eax", "%ebx", "%ecx", "%edx");
		if (rval != 1)
			return (1);
		if (off < 36) {
			pdevinfo->device_name[off++] = info & 0xff;
			pdevinfo->device_name[off++] = (info >>  8) & 0xff;
			pdevinfo->device_name[off++] = (info >> 16) & 0xff;
			pdevinfo->device_name[off++] = (info >> 24) & 0xff;
		} else {
			off += 4;
			if ((info & 1) != 0)
				pdevinfo->connected = true;
			else
				pdevinfo->connected = false;
		}
	}
	pdevinfo->device_id = device_id;

	return (0);
}

static int
vm_device_set_connect(int device_id, int conn)
{
	uint32_t	 ebx, rval;

	ebx = device_id;
	if (conn)
		ebx |= 0x80000000L;
	__asm__ __volatile__(
		"movl	$0x564D5868, %%eax	\n\t"
		"movl	%0, %%ebx		\n\t"
		"mov	$0xc, %%ecx		\n\t"
		"mov	$0x5658, %%dx		\n\t"
		"inl	%%dx, %%eax		\n\t"
		"movl	%%eax, %1		\n\t"
	:	"=m"(ebx), "=m"(rval)
	::      "%eax", "%ebx", "%ecx", "%edx");

	return ((rval == 1)? 0 : 1);
}
