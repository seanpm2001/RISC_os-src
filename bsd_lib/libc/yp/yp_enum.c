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
#ident	"$Header: yp_enum.c,v 1.2.1.2 90/05/07 21:29:41 wje Exp $"
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)yp_enum.c	1.2 88/07/27 4.0NFSSRC Copyr 1988 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.8 87/08/12 
 */


#define NULL 0
#ifdef SYSTYPE_BSD43
#include <sys/time.h>
#endif
#ifdef SYSTYPE_SYSV
#include <bsd/sys/time.h>
#endif
#include <rpc/rpc.h>
#include "yp_prot.h"
#include "ypv1_prot.h"
#include "ypclnt.h"

static char ypsymbol_prefix[] = "YP_";
static int ypsymbol_prefix_length = 3;
static int v2dofirst(), v1dofirst(), v2donext(), v1donext();

extern struct timeval _ypserv_timeout;
extern unsigned int _ypsleeptime;
extern int _yp_dobind();
extern char *malloc();

/*
 * This requests the yp server associated with a given domain to return the
 * first key/value pair from the map data base.  The returned key should be
 * used as an input to the call to ypclnt_next.  This part does the parameter
 * checking, and the do-until-success loop.
 */
int
yp_first (domain, map, key, keylen, val, vallen)
	char *domain;
	char *map;
	char **key;		/* return: key array */
	int  *keylen;		/* return: bytes in key */
	char **val;		/* return: value array */
	int  *vallen;		/* return: bytes in val */
{
	int domlen;
	int maplen;
	struct dom_binding *pdomb;
	int reason;
	int (*dofun)();

	if ( (map == NULL) || (domain == NULL) ) {
		return (YPERR_BADARGS);
	}
	
	domlen = strlen(domain);
	maplen = strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ) {
		return (YPERR_BADARGS);
	}

	for (;;) {
		
		if (reason = _yp_dobind(domain, &pdomb) ) {
			return (reason);
		}

		dofun = (pdomb->dom_vers == YPVERS) ? v2dofirst : v1dofirst;

		reason = (*dofun)(domain, map, pdomb, _ypserv_timeout,
		    key, keylen, val, vallen);

		if (reason == YPERR_RPC) {
			yp_unbind(domain);
			(void) sleep(_ypsleeptime);
		} else {
			break;
		}
	}
	
	return (reason);
}

/*
 * This part of the "get first" interface talks to ypserv.
 */

static int
v2dofirst (domain, map, pdomb, timeout, key, keylen, val, vallen)
	char *domain;
	char *map;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **key;
	int  *keylen;
	char **val;
	int  *vallen;

{
	struct ypreq_nokey req;
	struct ypresp_key_val resp;
	unsigned int retval = 0;

	req.domain = domain;
	req.map = map;
	resp.keydat.dptr = resp.valdat.dptr = NULL;
	resp.keydat.dsize = resp.valdat.dsize = 0;

	/*
	 * Do the get first request.  If the rpc call failed, return with status
	 * from this point.
	 */
	
	if(clnt_call(pdomb->dom_client, YPPROC_FIRST, xdr_ypreq_nokey,
	    &req, xdr_ypresp_key_val, &resp, timeout) != RPC_SUCCESS) {
		return (YPERR_RPC);
	}

	/* See if the request succeeded */
	
	if (resp.status != YP_TRUE) {
		retval = ypprot_err((unsigned) resp.status);
	}

	/* Get some memory which the user can get rid of as he likes */

	if (!retval) {

		if ((*key =
		    (char *) malloc((unsigned)
		        resp.keydat.dsize + 2)) != NULL) {

			if ((*val = (char *) malloc(
			    (unsigned) resp.valdat.dsize + 2) ) == NULL) {
				free((char *) *key);
				retval = YPERR_RESRC;
			}
		
		} else {
			retval = YPERR_RESRC;
		}
	}

	/* Copy the returned key and value byte strings into the new memory */

	if (!retval) {
		*keylen = resp.keydat.dsize;
		bcopy(resp.keydat.dptr, *key, resp.keydat.dsize);
		(*key)[resp.keydat.dsize] = '\n';
		(*key)[resp.keydat.dsize + 1] = '\0';
		
		*vallen = resp.valdat.dsize;
		bcopy(resp.valdat.dptr, *val, resp.valdat.dsize);
		(*val)[resp.valdat.dsize] = '\n';
		(*val)[resp.valdat.dsize + 1] = '\0';
	}
	
	CLNT_FREERES(pdomb->dom_client, xdr_ypresp_key_val, &resp); 
	return (retval);
}

static int
v1dofirst (domain, map, pdomb, timeout, key, keylen, val, vallen)
	char *domain;
	char *map;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **key;
	int  *keylen;
	char **val;
	int  *vallen;

{
	int result;
	
	result = v1prot_dofirst (domain, map, pdomb, timeout,
	    key, keylen, val, vallen);

	if (result) {
		return(result);
	} else {
		return(v1filter (domain, map, pdomb, timeout,
		    key, keylen, val, vallen));
	}
}

int
v1prot_dofirst (domain, map, pdomb, timeout, key, keylen, val, vallen)
	char *domain;
	char *map;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **key;
	int  *keylen;
	char **val;
	int  *vallen;

{
	struct yprequest req;
	struct ypresponse resp;
	unsigned int retval = 0;

	req.yp_reqtype = YPFIRST_REQTYPE;
	req.ypfirst_req_domain = domain;
	req.ypfirst_req_map = map;
	
	resp.ypfirst_resp_keyptr = NULL;
	resp.ypfirst_resp_keysize = 0;
	resp.ypfirst_resp_valptr = NULL;
	resp.ypfirst_resp_valsize = 0;



	/*
	 * Do the get first request.  If the rpc call failed, return with status
	 * from this point.
	 */
	
	if(clnt_call(pdomb->dom_client, YPOLDPROC_FIRST, _xdr_yprequest,
	    &req, _xdr_ypresponse, &resp, timeout) != RPC_SUCCESS) {
		return (YPERR_RPC);
	}

	/* See if the request succeeded */
	
	if (resp.ypfirst_resp_status != YP_TRUE) {
		retval = ypprot_err((unsigned) resp.ypfirst_resp_status);
	}

	/* Get some memory which the user can get rid of as he likes */

	if (!retval) {

		if ((*key =
		    (char *) malloc((unsigned)
		        resp.ypfirst_resp_keysize + 2)) != NULL) {

			if ((*val = (char *) malloc((unsigned)
			    resp.ypfirst_resp_valsize + 2) ) == NULL) {
				free((char *) *key);
				retval = YPERR_RESRC;
			}
		
		} else {
			retval = YPERR_RESRC;
		}
	}

	/* Copy the returned key and value byte strings into the new memory */

	if (!retval) {
		*keylen = resp.ypfirst_resp_keysize;
		bcopy(resp.ypfirst_resp_keyptr,
		    *key, resp.ypfirst_resp_keysize);
		(*key)[resp.ypfirst_resp_keysize] = '\n';
		(*key)[resp.ypfirst_resp_keysize + 1] = '\0';
		
		*vallen = resp.ypfirst_resp_valsize;
		bcopy(resp.ypfirst_resp_valptr, *val,
		    resp.ypfirst_resp_valsize);
		(*val)[resp.ypfirst_resp_valsize] = '\n';
		(*val)[resp.ypfirst_resp_valsize + 1] = '\0';
	}
	
	CLNT_FREERES(pdomb->dom_client, _xdr_ypresponse, &resp); 
	return (retval);
}

/*
 * This requests the yp server associated with a given domain to return the
 * "next" key/value pair from the map data base.  The input key should be
 * one returned by ypclnt_first or a previous call to ypclnt_next.  The
 * returned key should be used as an input to the next call to ypclnt_next.
 * This part does the parameter checking, and the do-until-success loop.
 */
int
yp_next (domain, map, inkey, inkeylen, outkey, outkeylen, val, vallen)
	char *domain;
	char *map;
	char *inkey;
	int  inkeylen;
	char **outkey;		/* return: key array associated with val */
	int  *outkeylen;	/* return: bytes in key */
	char **val;		/* return: value array associated with outkey */
	int  *vallen;		/* return: bytes in val */
{
	int domlen;
	int maplen;
	struct dom_binding *pdomb;
	int reason;
	int (*dofun)();


	if ( (map == NULL) || (domain == NULL) || (inkey == NULL) ) {
		return(YPERR_BADARGS);
	}
	
	domlen = strlen(domain);
	maplen = strlen(map);
	
	if ( (domlen == 0) || (domlen > YPMAXDOMAIN) ||
	    (maplen == 0) || (maplen > YPMAXMAP) ) {
		return(YPERR_BADARGS);
	}

	for (;;) {
		if (reason = _yp_dobind(domain, &pdomb) ) {
			return(reason);
		}

		dofun = (pdomb->dom_vers == YPVERS) ? v2donext : v1donext;

		reason = (*dofun)(domain, map, inkey, inkeylen, pdomb,
		    _ypserv_timeout, outkey, outkeylen, val, vallen);

		if (reason == YPERR_RPC) {
			yp_unbind(domain);
			(void) sleep(_ypsleeptime);
		} else {
			break;
		}
	}
	
	return(reason);
}

/*
 * This part of the "get next" interface talks to ypserv.
 */
static int
v2donext (domain, map, inkey, inkeylen, pdomb, timeout, outkey, outkeylen,
    val, vallen)
	char *domain;
	char *map;
	char *inkey;
	int  inkeylen;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **outkey;		/* return: key array associated with val */
	int  *outkeylen;	/* return: bytes in key */
	char **val;		/* return: value array associated with outkey */
	int  *vallen;		/* return: bytes in val */

{
	struct ypreq_key req;
	struct ypresp_key_val resp;
	unsigned int retval = 0;

	req.domain = domain;
	req.map = map;
	req.keydat.dptr = inkey;
	req.keydat.dsize = inkeylen;
	
	resp.keydat.dptr = resp.valdat.dptr = NULL;
	resp.keydat.dsize = resp.valdat.dsize = 0;

	/*
	 * Do the get next request.  If the rpc call failed, return with status
	 * from this point.
	 */
	
	if(clnt_call(pdomb->dom_client,
	    YPPROC_NEXT, xdr_ypreq_key, &req, xdr_ypresp_key_val, &resp,
	    timeout) != RPC_SUCCESS) {
		return(YPERR_RPC);
	}

	/* See if the request succeeded */
	
	if (resp.status != YP_TRUE) {
		retval = ypprot_err((unsigned) resp.status);
	}

	/* Get some memory which the user can get rid of as he likes */

	if (!retval) {
		if ( (*outkey = (char *) malloc((unsigned)
		    resp.keydat.dsize + 2) ) != NULL) {

			if ( (*val = (char *) malloc((unsigned)
			    resp.valdat.dsize + 2) ) == NULL) {
				free((char *) *outkey);
				retval = YPERR_RESRC;
			}
		
		} else {
			retval = YPERR_RESRC;
		}
	}

	/* Copy the returned key and value byte strings into the new memory */

	if (!retval) {
		*outkeylen = resp.keydat.dsize;
		bcopy(resp.keydat.dptr, *outkey,
		    resp.keydat.dsize);
		(*outkey)[resp.keydat.dsize] = '\n';
		(*outkey)[resp.keydat.dsize + 1] = '\0';
		
		*vallen = resp.valdat.dsize;
		bcopy(resp.valdat.dptr, *val, resp.valdat.dsize);
		(*val)[resp.valdat.dsize] = '\n';
		(*val)[resp.valdat.dsize + 1] = '\0';
	}
	
	CLNT_FREERES(pdomb->dom_client, xdr_ypresp_key_val, &resp);
	return(retval);
}

static int
v1donext (domain, map, inkey, inkeylen, pdomb, timeout,
    outkey, outkeylen, val, vallen)
	char *domain;
	char *map;
	char *inkey;
	int  inkeylen;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **outkey;		/* return: key array associated with val */
	int  *outkeylen;	/* return: bytes in key */
	char **val;		/* return: value array associated with outkey */
	int  *vallen;		/* return: bytes in val */

{
	int result;
	
	result = v1prot_donext (domain, map, inkey, inkeylen, pdomb, timeout,
	    outkey, outkeylen, val, vallen);

	if (result) {
		return(result);
	} else {
		return(v1filter (domain, map, pdomb, timeout,
		    outkey, outkeylen, val, vallen));
	}
}

int
v1prot_donext (domain, map, inkey, inkeylen, pdomb, timeout,
    outkey, outkeylen, val, vallen)
	char *domain;
	char *map;
	char *inkey;
	int  inkeylen;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **outkey;		/* return: key array associated with val */
	int  *outkeylen;	/* return: bytes in key */
	char **val;		/* return: value array associated with outkey */
	int  *vallen;		/* return: bytes in val */

{
	struct yprequest req;
	struct ypresponse resp;
	unsigned int retval = 0;

	req.yp_reqtype = YPNEXT_REQTYPE;
	req.ypnext_req_domain = domain;
	req.ypnext_req_map = map;
	req.ypnext_req_keyptr = inkey;
	req.ypnext_req_keysize = inkeylen;
	
	resp.ypnext_resp_keyptr = NULL;
	resp.ypnext_resp_keysize = 0;
	resp.ypnext_resp_valptr = NULL;
	resp.ypnext_resp_valsize = 0;
	
	/*
	 * Do the get next request.  If the rpc call failed, return with status
	 * from this point.
	 */
	
	if(clnt_call(pdomb->dom_client, YPOLDPROC_NEXT, _xdr_yprequest,
	    &req, _xdr_ypresponse, &resp, timeout) != RPC_SUCCESS) {
		return(YPERR_RPC);
	}

	if (resp.ypnext_resp_status != YP_TRUE) {
		retval = ypprot_err((unsigned) resp.ypnext_resp_status);
	}

	/* Get some memory which the user can get rid of as he likes */

	if (!retval) {
		if ( (*outkey = (char *) malloc((unsigned)
		    resp.ypnext_resp_keysize + 2) ) != NULL) {

			if ( (*val = (char *) malloc((unsigned)
			    resp.ypnext_resp_valsize + 2) ) == NULL) {
				free((char *) *outkey);
				retval = YPERR_RESRC;
			}
		
		} else {
			retval = YPERR_RESRC;
		}
	}

	/* Copy the returned key and value byte strings into the new memory */

	if (!retval) {
		*outkeylen = resp.ypnext_resp_keysize;
		bcopy(resp.ypnext_resp_keyptr, *outkey,
		    resp.ypnext_resp_keysize);
		(*outkey)[resp.ypnext_resp_keysize] = '\n';
		(*outkey)[resp.ypnext_resp_keysize + 1] = '\0';
		
		*vallen = resp.ypnext_resp_valsize;
		bcopy(resp.ypnext_resp_valptr, *val, resp.ypnext_resp_valsize);
		(*val)[resp.ypnext_resp_valsize] = '\n';
		(*val)[resp.ypnext_resp_valsize + 1] = '\0';
	}
	
	CLNT_FREERES(pdomb->dom_client, _xdr_ypresponse, &resp);
	return(retval);
}

/*
 * This supplies client side yp-private key filtration.  It is needed in
 * speaking with v.1 protocol-speaking servers.
 *
 * This continues to get "next" key-value pairs from the map while the
 * key-value pairs which come back have keys which are yp private symbols.
 */
static int
v1filter (domain, map, pdomb, timeout, key, keylen, val, vallen)
	char *domain;
	char *map;
	struct dom_binding *pdomb;
	struct timeval timeout;
	char **key;		/* return: key array */
	int  *keylen;		/* return: bytes in key */
	char **val;		/* return: value array */
	int  *vallen;		/* return: bytes in val */
{
	char *inkey;
	int inkeylen;
	int result = 0;

	/*
	 * Keep trying to get the next key-value pair as long as we
	 * (1) continue to succeed, and
	 * (2) the key we get back is a yp reserved symbol.
	 */

 	inkey = NULL;
	inkeylen = 0;
 
	while ( (!result) &&
	    (!bcmp(*key, ypsymbol_prefix, ypsymbol_prefix_length) ) ) {
		inkey = *key;
		inkeylen = *keylen;
		*key = NULL;
		*keylen = 0;
		free(*val);
		*val = NULL;
		*vallen = 0;
		result = v1prot_donext (domain, map, inkey, inkeylen,
		    pdomb, timeout, key, keylen, val, vallen);
		free(inkey);
		inkeylen = 0;
	}

	return(result);
}

