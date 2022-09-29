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
 * |         928 Arques Avenue                                 |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: mis.h,v 1.1.1.2 90/05/09 17:29:38 wje Exp $ */
#ifndef		_MIS_H_
#define		_MIS_H_

/*
 *	$Header: mis.h,v 1.1.1.2 90/05/09 17:29:38 wje Exp $
 *	Author: J. Davin
 *	Copyright 1988, 1989, Massachusetts Institute of Technology
 *	See permission and disclaimer notice in file "notice.h"
 */

#include	<notice.h>

#include	<ctypes.h>
#include	<error.h>
#include	<mix.h>
#include	<aps.h>

typedef		ErrStatusType		MisStatusType;

typedef		CBoolType		MisAccessType;

CVoidType	misInit ();
MisStatusType	misExport ();
MisAccessType	misCommunityToAccess ();
MixIdType	misCommunityToMib ();
ApsIdType	misCommunityByName ();

#endif		/*	_MIS_H_	*/
