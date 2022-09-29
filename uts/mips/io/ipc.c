/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1990 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
#ident	"$Header: ipc.c,v 1.6.4.2.1.1.1.2 90/10/16 10:12:40 beacker Exp $"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#include	"sys/errno.h"
#include	"sys/types.h"
#include	"sys/sbd.h"
#include	"sys/param.h"
#include	"sys/signal.h"
#include	"sys/immu.h"
#include	"sys/region.h"
#include 	"bsd/sys/time.h"
#include	"sys/proc.h"
#include	"sys/fs/s5dir.h"
#include	"sys/psw.h"
#include	"sys/pcb.h"
#include	"sys/user.h"
#include	"sys/ipc.h"

/*
**	Common IPC routines.
*/

/*
**	Check message, semaphore, or shared memory access permissions.
**
**	This routine verifies the requested access permission for the current
**	process.  Super-user is always given permission.  Otherwise, the
**	appropriate bits are checked corresponding to owner, group, or
**	everyone.  Zero is returned on success.  On failure, u.u_error is
**	set to EACCES and one is returned.
**	The arguments must be set up as follows:
**		p - Pointer to permission structure to verify
**		mode - Desired access permissions
*/

ipcaccess(p, mode)
register struct ipc_perm	*p;
register ushort			mode;
{
	if (u.u_uid == 0)
		return(0);
	if (u.u_uid != p->uid && u.u_uid != p->cuid) {
		mode >>= 3;
		if (u.u_gid != p->gid && u.u_gid != p->cgid)
			mode >>= 3;
	}
	if (mode & p->mode)
		return(0);
	u.u_error = EACCES;
	return(1);
}

/*
**	Get message, semaphore, or shared memory structure.
**
**	This routine searches for a matching key based on the given flags
**	and returns a pointer to the appropriate entry.  A structure is
**	allocated if the key doesn't exist and the flags call for it.
**	The arguments must be set up as follows:
**		key - Key to be used
**		flag - Creation flags and access modes
**		base - Base address of appropriate facility structure array
**		cnt - # of entries in facility structure array
**		size - sizeof(facility structure)
**		status - Pointer to status word: set on successful completion
**			only:	0 => existing entry found
**				1 => new entry created
**	Ipcget returns NULL with u.u_error set to an appropriate value if
**	it fails, or a pointer to the initialized entry if it succeeds.
*/

struct ipc_perm *
ipcget(key, flag, base, cnt, size, status)
register struct ipc_perm	*base;
int				cnt,
				flag,
				size,
				*status;
key_t				key;
{
	register struct ipc_perm	*a;	/* ptr to available entry */
	register int			i;	/* loop control */

	if (key == IPC_PRIVATE) {
		for (i = 0;i++ < cnt;
			base = (struct ipc_perm *)(((char *)base) + size)) {
			if (base->mode & IPC_ALLOC)
				continue;
			goto init;
		}
		u.u_error = ENOSPC;
		return(NULL);
	} else {
		for (i = 0, a = NULL;i++ < cnt;
			base = (struct ipc_perm *)(((char *)base) + size)) {
			if (base->mode & IPC_ALLOC) {
				if (base->key == key) {
					if ((flag & (IPC_CREAT | IPC_EXCL)) ==
						(IPC_CREAT | IPC_EXCL)) {
						u.u_error = EEXIST;
						return(NULL);
					}
					if ((flag & 0777) & ~base->mode) {
						u.u_error = EACCES;
						return(NULL);
					}
					*status = 0;
					return(base);
				}
				continue;
			}
			if (a == NULL)
				a = base;
		}
		if (!(flag & IPC_CREAT)) {
			u.u_error = ENOENT;
			return(NULL);
		}
		if (a == NULL) {
			u.u_error = ENOSPC;
			return(NULL);
		}
		base = a;
	}
init:
	*status = 1;
	/* save the IPC_PERMANENT flag if it is set (040000) */
	base->mode = IPC_ALLOC | (flag & 040777);
	base->key = key;
	base->cuid = base->uid = u.u_uid;
	base->cgid = base->gid = u.u_gid;
	return(base);
}
