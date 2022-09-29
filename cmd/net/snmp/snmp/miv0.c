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
#ident	"$Header: miv0.c,v 1.1.1.2 90/05/09 17:29:50 wje Exp $"

/*
 *	$Header: miv0.c,v 1.1.1.2 90/05/09 17:29:50 wje Exp $
 *	Author: J. Davin
 *	Copyright 1988, 1989, Massachusetts Institute of Technology
 *	See permission and disclaimer notice in file "notice.h"
 */

#include	<notice.h>

#include	<ctypes.h>
#include	<error.h>
#include	<miv.h>
#include	<mix.h>
#include	<mis.h>
#include	<asn.h>

#define		mivNonZero(asn)		\
			asnNonZero (asnValue ((asn)), asnLength ((asn)))

#define		mivNumber(asn)		\
			asnNumber (asnValue ((asn)), asnLength ((asn)))


static	MixStatusType	mivRelease (mix)

MixCookieType		mix;

{
	mix = mix;
	return (smpErrorGeneric);
}


static	MixStatusType	mivNoSet (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;
	return (smpErrorReadOnly);
}

static	MixStatusType	mivCreate (mix, name, namelen, value)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		value;

{
	mix = mix;
	name = name;
	namelen = namelen;
	value = value;
	return (smpErrorGeneric);
}

static	MixStatusType	mivDestroy (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	mix = mix;
	name = name;
	namelen = namelen;
	return (smpErrorGeneric);
}

static	AsnIdType	mivGet (mix, name, namelen)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;

{
	if ((namelen != (MixLengthType) 1) ||
		(*name != (MixNameType) 0)) {
		return ((AsnIdType) 0);
	}
	else {
		return (asnIntl (asnClassUniversal, (AsnTagType) 2,
			*((CIntlPtrType) mix)));
	}
}

static	AsnIdType	mivNext (mix, name, namelenp)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthPtrType	namelenp;

{
	if (*namelenp != (MixLengthType) 0) {
		return ((AsnIdType) 0);
	}
	else {
		*namelenp = (MixLengthType) 1;
		*name = (MixNameType) 0;
		return (asnIntl (asnClassUniversal, (AsnTagType) 2,
			*((CIntlPtrType) mix)));
	}
}

static	MixStatusType	mivSet (mix, name, namelen, asn)

MixCookieType		mix;
MixNamePtrType		name;
MixLengthType		namelen;
AsnIdType		asn;

{
	if ((namelen != (MixLengthType) 1) ||
		(*name != (MixNameType) 0)) {
		return (smpErrorNoSuch);
	}
	else if (asnLength (asn) > (AsnLengthType) sizeof (CIntlType)) {
		return (smpErrorBadValue);
	}
	else if (asnTag (asn) != (AsnTagType) 2) {
		return (smpErrorBadValue);
	}
	else if (asnClass (asn) != asnClassUniversal) {
		return (smpErrorBadValue);
	}
	else {
		*((CIntlPtrType) mix) = (CIntlType) mivNumber (asn);
		return (smpErrorNone);
	}
}

static	MixOpsType	mivOpsRW	= {

			mivRelease,
			mivCreate,
			mivDestroy,
			mivNext,
			mivGet,
			mivSet

			};

MisStatusType		mivIntlRW (name, namelen, address)

MixNamePtrType		name;
MixLengthType		namelen;
CIntlPtrType		address;

{
	return (misExport (name, namelen, & mivOpsRW,
		(MixCookieType) address));
}

static	MixOpsType	mivOpsRO	= {

			mivRelease,
			mivCreate,
			mivDestroy,
			mivNext,
			mivGet,
			mivNoSet

			};

MisStatusType		mivIntlRO (name, namelen, address)

MixNamePtrType		name;
MixLengthType		namelen;
CIntlPtrType		address;

{
	return (misExport (name, namelen, & mivOpsRO,
		(MixCookieType) address));
}

