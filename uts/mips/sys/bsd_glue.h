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
/* $Header: bsd_glue.h,v 1.2.3.2 90/05/10 06:06:14 wje Exp $ */
/*
 * $Header: bsd_glue.h,v 1.2.3.2 90/05/10 06:06:14 wje Exp $
 */
/*
 * These defines are needed to convert bsd defines into the appropriate sysv ones.
 * By doing this we can use the same header files in the stand alone code as here.
 */
#define NBPG NBPC
#define PGOFSET POFFMASK

