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
/* $Header: ypclnt.h,v 1.1.1.2 90/05/07 21:31:42 wje Exp $ */
/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *	 1.12 86/12/19 
 */


/*
 * ypclnt.h
 * This defines the symbols used in the c language
 * interface to the yp client functions.  A description of this interface
 * can be read in ypclnt(3N).
 */

/*
 * Failure reason codes.  The success condition is indicated by a functional
 * value of "0".
 */
#define YPERR_BADARGS 1			/* Args to function are bad */
#define YPERR_RPC 2			/* RPC failure */
#define YPERR_DOMAIN 3			/* Can't bind to a server which serves
					 *   this domain. */
#define YPERR_MAP 4			/* No such map in server's domain */
#define YPERR_KEY 5			/* No such key in map */
#define YPERR_YPERR 6			/* Internal yp server or client
					 *   interface error */
#define YPERR_RESRC 7			/* Local resource allocation failure */
#define YPERR_NOMORE 8			/* No more records in map database */
#define YPERR_PMAP 9			/* Can't communicate with portmapper */
#define YPERR_YPBIND 10			/* Can't communicate with ypbind */
#define YPERR_YPSERV 11			/* Can't communicate with ypserv */
#define YPERR_NODOM 12			/* Local domain name not set */
#define YPERR_BADDB 13			/*  yp data base is bad */
#define YPERR_VERS 14			/* YP version mismatch */
#define YPERR_ACCESS 15			/* Access violation */
#define YPERR_BUSY 16			/* Database is busy */

/*
 * Types of update operations
 */
#define YPOP_CHANGE 1			/* change, do not add */
#define YPOP_INSERT 2			/* add, do not change */
#define YPOP_DELETE 3			/* delete this entry */
#define YPOP_STORE  4			/* add, or change */
 
 

/*           
 * Data definitions
 */

/*
 * struct ypall_callback * is the arg which must be passed to yp_all
 */

struct ypall_callback {
	int (*foreach)();		/* Return non-0 to stop getting
					 *  called */
	char *data;			/* Opaque pointer for use of callback
					 *   function */
};

/*
 * External yp client function references. 
 */
extern int yp_bind();
extern int _yp_dobind();
extern void yp_unbind();
extern int yp_get_default_domain ();
extern int yp_match ();
extern int yp_first ();
extern int yp_next();
extern int yp_master();
extern int yp_order();
extern int yp_all();
extern int yp_match();
extern char *yperr_string();
extern int ypprot_err();

/*
 * Global yp data structures
 */
