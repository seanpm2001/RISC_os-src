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
#ident	"$Header: passmgmt.c,v 1.1.1.2.1.1.1.2 90/11/02 17:49:30 beacker Exp $"
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#include <stdio.h>
#include <shadow.h>
#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef RISCOS
#include <bsd/syslog.h>
#endif

#define CMT_SIZE	(128+1)	/* Argument sizes + 1 (for '\0') */
#define DIR_SIZE	(256+1)
#define SHL_SIZE	(256+1)
#define ENTRY_LENGTH	512	/* Max length of an /etc/passwd entry */
#define UID_MIN		100	/* Lower bound of default UID */

#define M_MASK		01	/* Masks for the optn_mask variable */
#define L_MASK		02	/* It keeps track of which options   */
#define C_MASK		04	/* have been entered		     */
#define H_MASK		010
#define U_MASK		020
#define G_MASK		040
#define S_MASK		0100
#define O_MASK		0200
#define A_MASK		0400
#define D_MASK		01000

					/* flags for info_mask */
#define LOGNAME_EXIST	01		/* logname exists */
#define BOTH_FILES	02		/* touch both password files */
#define WRITE_P_ENTRY	04		/* write out password entry */
#define WRITE_S_ENTRY	010		/* write out shadow entry */
#define NEED_DEF_UID	020		/* need default uid */
#define FOUND		040		/* found the entry in password file */
#define LOCKED		0100		/* did we lock the password file */
#define SHAD_FLAG	0200		/* running with shadow file flag */

char defdir[] = "/usr/" ;	/* default home directory for new user */
char pwdflr[] =	"x" ;		/* password string for /etc/passwd */
char lkstring[] = "*LK*" ;	/* lock string for shadow password */
char nullstr[] = "" ;		/* null string */

/* Declare all functions that do not return integers.  This is here
   to get rid of some lint messages */

void	uid_bcom(), add_ublk(), bad_perm(), bad_usage(), bad_arg(),
	bad_uid(), bad_pasf(), file_error(), bad_news(), no_lock(),
	add_uid(), rid_tmpf(), ck_p_sz(), ck_s_sz(), bad_name() ;

static FILE *fp_ptemp, *fp_stemp ;

/* The uid_blk structure is used in the search for the default
   uid.  Each uid_blk represent a range of uid(s) that are currently
   used on the system. */

struct uid_blk	  { 			
		  struct uid_blk *link ;
		  int low ;		/* low bound for this uid block */
		  int high ; 		/* high bound for this uid block */
		  } ;

struct uid_blk *uid_sp ;
char *prognamp ;			/* program name */
extern int errno ;
int optn_mask = 0, info_mask = 0 ;
#ifdef RISCOS
struct passwd *jgetpwent();
static char line[BUFSIZ+1];
static char saveline[BUFSIZ+1];
int shadow_exist = 0;
#endif

main (argc, argv)
int	argc ;
char  **argv ;
{
	int c, i ;
	char *lognamp, *char_p ;

	extern char *optarg, *malloc() ;
	extern int optind ;
#ifndef RISCOS
	extern struct passwd *getpwent(), *getpwnam() ;
#endif

	struct passwd *pw_ptr1p, passwd_st ;
	struct spwd *sp_ptr1p, shadow_st ;
	struct stat statbuf ;

	/* Get program name */
	prognamp = argv[0] ;

	/* Check identity */
	if ( getuid () != (unsigned short) 0 )
		bad_perm () ;

#ifdef RISCOS
	openlog("passmgmt",LOG_PID,LOG_AUTH);
#endif
	/* Lock the password file(s) */

	if ( lckpwdf() != 0 )
		no_lock () ;
	info_mask |= LOCKED ;		/* remember we locked */

	/* Check for the existence of shadow password file */
	if ( !access (SHADOW,0) )		/* if shadow is there */
	{
		shadow_exist ++;
		info_mask |= SHAD_FLAG ;
	}

	/* initialize the two structures */

	passwd_st.pw_name = nullstr ;		/* login name */
	if ( SHAD_FLAG & info_mask )		/* if shadow is running */
		passwd_st.pw_passwd = pwdflr ;	/* bogus password */
	else
		passwd_st.pw_passwd = lkstring; /* bogus password */
	passwd_st.pw_uid = -1 ;			/* no uid */
	passwd_st.pw_gid = 1 ;			/* default gid */
	passwd_st.pw_age = nullstr ;		/* no aging info. */
	passwd_st.pw_comment = nullstr ;	/* no comments */
	passwd_st.pw_gecos = nullstr ;		/* no comments */
	passwd_st.pw_dir = nullstr ;		/* no default directory */
	passwd_st.pw_shell = nullstr ; 		/* no default shell */

	if ( SHAD_FLAG & info_mask )		/* if shadow is running */
		{
		shadow_st.sp_namp = nullstr ; 	/* no name */
		shadow_st.sp_pwdp = lkstring ; 	/* locked password */
		shadow_st.sp_lstchg = -1 ; 	/* no lastchanged date */
		shadow_st.sp_min = -1 ; 	/* no min */
		shadow_st.sp_max = -1 ; 	/* no max */
		}

	/* parse the command line */

	while ( (c = getopt (argc, argv, "ml:c:h:u:g:s:oad")) != -1 )

		switch (c) 
		   {

		   case 'm' :
			    /* Modify */

			    if ( (A_MASK|D_MASK|M_MASK) & optn_mask )
			         bad_usage ("Invalid combination of options") ;

			    optn_mask |= M_MASK ;
			    break ;

		   case 'l' :
			    /* Change logname */

			    if ( (A_MASK|D_MASK|L_MASK) & optn_mask )
			         bad_usage ("Invalid combination of options") ;

			    if ( strpbrk ( optarg, ":\n" ) ||
				 strlen (optarg) == 0  )
			       	bad_arg ("Invalid argument to option -l") ;

			    optn_mask |= L_MASK ;
			    passwd_st.pw_name = optarg ;
			    shadow_st.sp_namp = optarg ;
			    break ;

		   case 'c' :
			    /* The comment */

			    if ( (D_MASK|C_MASK) & optn_mask )
			         bad_usage ("Invalid combination of options") ;

			    if ( strlen (optarg) > CMT_SIZE ||
				 strpbrk ( optarg, ":\n" ) )
				bad_arg ("Invalid argument to option -c") ;

			    optn_mask |= C_MASK ;
			    passwd_st.pw_comment = optarg ;
			    passwd_st.pw_gecos = optarg ;
			    break ;

		   case 'h' :
			    /* The home directory */

			    if ( (D_MASK|H_MASK) & optn_mask )
			         bad_usage ("Invalid combination of options") ;

			    if ( strlen (optarg) > DIR_SIZE ||
				 strpbrk ( optarg, ":\n" ) )
				bad_arg ("Invalid argument to option -h") ;
			
			    optn_mask |= H_MASK ;
			    passwd_st.pw_dir = optarg ;
			    break ;

		   case 'u' :
			    /* The uid */

			    if ( (D_MASK|U_MASK) & optn_mask )
			         bad_usage ("Invalid combination of options") ;

			    optn_mask |= U_MASK ;
			    passwd_st.pw_uid = (int) strtol (optarg,&char_p,10) ;
			    if ( (*char_p != '\0') ||
			         ( passwd_st.pw_uid < 0 ) ||  
				 ( strlen (optarg) == 0 ) )
				bad_arg ("Invalid argument to option -u") ;

			    break ;

		   case 'g' :
			    /* The gid */

			    if ( (D_MASK|G_MASK) & optn_mask )
			         bad_usage ("Invalid combination of options") ;

			    optn_mask |= G_MASK ;
			    passwd_st.pw_gid = (int) strtol (optarg,&char_p,10) ;

			    if ( (*char_p != '\0') ||
			         ( passwd_st.pw_gid < 0 ) ||
				 ( strlen (optarg) == 0 ) )
				bad_arg ("Invalid argument to option -g") ;
			    break ;

		   case 's' :
			    /* The shell */

			    if ( (D_MASK|S_MASK) & optn_mask )
			         bad_usage ("Invalid combination of options") ;

			    if ( strlen (optarg) > SHL_SIZE ||
				 strpbrk ( optarg, ":\n" ) )
				bad_arg ("Invalid argument to option -s") ;

			    optn_mask |= S_MASK ;
			    passwd_st.pw_shell = optarg ;
			    break ;

		   case 'o' :
			    /* Override unique uid  */

			    if ( (D_MASK|O_MASK) & optn_mask )
			         bad_usage ("Invalid combination of options") ;

			    optn_mask |= O_MASK ;
			    break ;

		   case 'a' :
			    /* Add */

			    if ( (A_MASK|M_MASK|D_MASK|L_MASK) & optn_mask )
			         bad_usage ("Invalid combination of options") ;

			    optn_mask |= A_MASK ;
			    break ;

		   case 'd' :
			    /* Modify */

			    if ( (D_MASK|M_MASK|L_MASK|C_MASK|
				  H_MASK|U_MASK|G_MASK|S_MASK|
				  O_MASK|A_MASK) & optn_mask )
			         bad_usage ("Invalid combination of options") ;

			    optn_mask |= D_MASK ;
			    break ;

		   case '?' :

			    bad_usage ("") ;
			    break ;
		   }

	/* check command syntax for the following errors */
	/* too few or too many arguments */
	/* no -a -m or -d option */
	/* -o without -u */
	/* -m with no other option */

	if ( optind == argc || argc > (optind+1) || 
	     !((A_MASK|M_MASK|D_MASK) & optn_mask) ||
	     ((optn_mask & O_MASK) && !(optn_mask & U_MASK)) ||
	     ((optn_mask & M_MASK) && !(optn_mask & 
	         (L_MASK|C_MASK|H_MASK|U_MASK|G_MASK|S_MASK))) )
		bad_usage ("Invalid command syntax") ;

	/* null string argument or bad characters ? */
	if ( ( strlen ( argv[optind] ) == 0 ) ||
	       strpbrk ( argv[optind], ":\n" ) )
		bad_arg ("Invalid name") ;

	lognamp = argv [optind] ;

	/* if we are adding a new user or modifying an existing user
	   (not the logname), then copy logname into the two data
	   structures */

	if ( (A_MASK & optn_mask) || 
	     ((M_MASK & optn_mask) && !(optn_mask & L_MASK)) )
		{
		passwd_st.pw_name = argv [optind] ;
		if ( SHAD_FLAG & info_mask )	/* if running shadow */
			shadow_st.sp_namp = argv [optind] ;
		}

	/* Put in directory if we are adding and we need a default */

	if ( !( optn_mask & H_MASK) && ( optn_mask & A_MASK ) )
		{
		if ( (passwd_st.pw_dir = malloc ((unsigned) DIR_SIZE)) == NULL )
			file_error () ;

		*passwd_st.pw_dir = '\0' ;
		(void) strcat ( passwd_st.pw_dir, defdir ) ;
		(void) strcat ( passwd_st.pw_dir, lognamp ) ;
		}

	/* Check the number of password files we are touching */

	if ( (SHAD_FLAG & info_mask) && !( (M_MASK & optn_mask) && 
	    !(L_MASK & optn_mask) ) )
		info_mask |= BOTH_FILES ;
	else    
		info_mask &= ~BOTH_FILES ; /* just in case */

	/* Open the temporary file(s) with appropriate permission mask */
	/* and the appropriate owner */

	if ( stat ( PASSWD, &statbuf ) < 0 ) 
		file_error () ;

	(void) umask ( ~( statbuf.st_mode & 0444 ) ) ;

 	if ( (fp_ptemp = fopen ( PASSTEMP , "w" )) == NULL )
		file_error () ;

	if ( chown ( PASSTEMP, statbuf.st_uid, statbuf.st_gid ) != NULL )
		{
		(void) fclose ( fp_ptemp ) ;

		if ( unlink ( PASSTEMP ) )
			(void) fprintf ( stderr, "%s: WARNING cannot unlink %s\n", 
					 prognamp, PASSTEMP ) ;

		file_error () ;
		}

	if ( info_mask & BOTH_FILES )
		{
		if ( stat ( SHADOW, &statbuf ) < 0 ) 
			{
			rid_tmpf () ;
			file_error () ;
			}

		(void) umask ( ~( statbuf.st_mode & 0400 ) ) ;

	 	if ( (fp_stemp = fopen ( SHADTEMP , "w" )) == NULL )
			{
			rid_tmpf () ;
			file_error () ;
			}

		if ( chown ( SHADTEMP, statbuf.st_uid, statbuf.st_gid ) != NULL )
			{
			rid_tmpf () ;
			file_error () ;
			}
		} 

	/* Default uid needed ? */

	if ( !( optn_mask & U_MASK ) && ( optn_mask & A_MASK ) )
		{
		/* mark it in the information mask */
		info_mask |= NEED_DEF_UID ;

		/* create the head of the uid number list */
		if ( (uid_sp = (struct uid_blk *) malloc((unsigned) sizeof(struct uid_blk))) == NULL )
			{
			rid_tmpf () ;
			file_error () ;
			}

		uid_sp->link = NULL ;
		uid_sp->low = (UID_MIN -1) ;
		uid_sp->high = (UID_MIN -1) ;
		}

	/* The while loop for reading PASSWD entries */

	while ( (pw_ptr1p = (struct passwd *) jgetpwent()) != NULL )
	    {
	       /* handle bad lines by just passing them through
		* this lets the YP + and - lines through
		* Other errors are posted to the syslog
		*/
	        if ( pw_ptr1p == (struct passwd *)-1)
	        {
		    fprintf(fp_ptemp,"%s",saveline);
		    continue;
	        }
		info_mask |= WRITE_P_ENTRY ;

		/* Set up the uid usage blocks to find the first
		   available uid above UID_MIN, if needed */

		if ( info_mask & NEED_DEF_UID )
			add_uid ( pw_ptr1p->pw_uid ) ;

		/* Check for unique UID */

		if ( strcmp ( lognamp, pw_ptr1p->pw_name )		 &&
		     ( pw_ptr1p->pw_uid == passwd_st.pw_uid )            &&
		     ( (optn_mask & U_MASK) && !(optn_mask & O_MASK) )  )
			{
			rid_tmpf () ;		/* get rid of temp files */
			bad_uid () ;
			}

		/* Check for unique new logname */

		if ( strcmp ( lognamp, pw_ptr1p->pw_name )		 &&
		     optn_mask & L_MASK					 &&
		     !strcmp ( pw_ptr1p->pw_name, passwd_st.pw_name )   )
		     {
		     rid_tmpf () ;
		     if ( (SHAD_FLAG & info_mask) &&
				!getspnam ( pw_ptr1p->pw_name ) )
			bad_pasf () ;
		     else
			bad_name ( "logname already exists" ) ;
		     }

		if ( !strcmp ( lognamp, pw_ptr1p->pw_name ) )
			{

			/* no good if we want to add an existing logname */
			if ( optn_mask & A_MASK )
				{
				rid_tmpf () ;
				if ( (SHAD_FLAG & info_mask) &&
						!getspnam ( lognamp ) )
					bad_pasf () ;
				else
					bad_name ( "name already exists" ) ;
				}

			/* remember we found it */
			info_mask |= FOUND ;

			/* Do not write it out on the fly */
			if ( optn_mask & D_MASK )
				info_mask &= ~WRITE_P_ENTRY ;

			if ( optn_mask & M_MASK )
				{

				if ( (SHAD_FLAG & info_mask) &&
						!getspnam ( lognamp ) )
					{
					rid_tmpf () ;
					bad_pasf () ;
					}
				if ( optn_mask & L_MASK )
					pw_ptr1p->pw_name = passwd_st.pw_name ;

				if ( optn_mask & U_MASK )
					pw_ptr1p->pw_uid = passwd_st.pw_uid ;

				if ( optn_mask & G_MASK )
					pw_ptr1p->pw_gid = passwd_st.pw_gid ;

				if ( optn_mask & C_MASK )
					{
					pw_ptr1p->pw_comment = passwd_st.pw_comment ;
							
					pw_ptr1p->pw_gecos = passwd_st.pw_comment ;
					}

				if ( optn_mask & H_MASK )
					pw_ptr1p->pw_dir = passwd_st.pw_dir ;
				
				if ( optn_mask & S_MASK )
					pw_ptr1p->pw_shell = passwd_st.pw_shell ;
				ck_p_sz ( pw_ptr1p ) ;  /* check entry size */
				}
			}

		if ( info_mask & WRITE_P_ENTRY )
			{
			if ( putpwent ( pw_ptr1p, fp_ptemp ) )
				{
				rid_tmpf () ;
				file_error () ;
				}
			}

		} /* End of while */

	/* Cannot find the target entry and we are deleting or modifying */

	if ( !(info_mask & FOUND) && ( optn_mask & (D_MASK|M_MASK) ) ) 
		{
		rid_tmpf () ;
		if ( (SHAD_FLAG & info_mask) && getspnam ( lognamp ) )
			bad_pasf () ;
		else
			bad_name ( "name does not exist" ) ;
		}

	/* First available uid above UID_MIN is ... */

	if ( info_mask & NEED_DEF_UID )
		passwd_st.pw_uid = uid_sp->high + 1 ;

	/* Write out the added entry now */

	if ( optn_mask & A_MASK )
		{
		ck_p_sz ( &passwd_st ) ;	/* Check entry size */
		if ( putpwent ( &passwd_st, fp_ptemp ) )
			{
			rid_tmpf () ;
			file_error () ;
			}
		}

	(void) fclose ( fp_ptemp ) ;

	/* Now we are done with PASSWD */

	/* Do this if we are touching both password files */

	if ( info_mask & BOTH_FILES )
		{
		info_mask &= ~FOUND ;		/* Reset FOUND flag */

		/* The while loop for reading SHADOW entries */
		info_mask |= WRITE_S_ENTRY ;

		while ( (sp_ptr1p = (struct spwd *) getspent()) != NULL )
			{

			/* See if the new logname already exist in the
			   shadow passwd file */
			if ( ( optn_mask & M_MASK ) &&
			     strcmp ( lognamp, shadow_st.sp_namp ) &&
			     ( !strcmp ( sp_ptr1p->sp_namp,
					 shadow_st.sp_namp ) ) )
				{
				rid_tmpf () ;
				bad_pasf () ;
				}

			if ( !strcmp ( lognamp, sp_ptr1p->sp_namp ) )
			      {
			      info_mask |= FOUND ;
			      if ( optn_mask & A_MASK ) 
					{
					rid_tmpf () ;
					bad_pasf () ;	/* password file
							   inconsistent */
					}

			      if ( optn_mask & M_MASK )
					{
			      		sp_ptr1p->sp_namp = shadow_st.sp_namp ;
					ck_s_sz ( sp_ptr1p ) ;
					}

			      if ( optn_mask & D_MASK )
			      	    	info_mask &= ~WRITE_S_ENTRY ;
			      }

			if ( info_mask & WRITE_S_ENTRY )
				{
				if ( putspent ( sp_ptr1p, fp_stemp ) )
					{
					rid_tmpf () ;
					file_error () ;
					}
				}
			else	
				info_mask |= WRITE_S_ENTRY ;

			} /* End of while */

		/* If we cannot find the entry and we are deleting or
		   modifying */

		if ( !(info_mask & FOUND) && (optn_mask & (D_MASK|M_MASK)) )
			{
			rid_tmpf () ;
			bad_pasf () ;
			}

		if ( optn_mask & A_MASK )
			{
			ck_s_sz ( &shadow_st ) ;
			if ( putspent ( &shadow_st, fp_stemp ) )
				{
				rid_tmpf () ;
				file_error () ;
				}
			}

		(void) fclose (fp_stemp) ;

		/* Done with SHADOW */
		} /* End of if info_mask */

	/* ignore all signals */

	for ( i = 1 ; i < NSIG ; i++ )
		(void ) sigset ( i, SIG_IGN ) ;

	errno = 0 ;		/* For correcting sigset to SIGKILL */

	if (unlink (OPASSWD) && access (OPASSWD, 0) == 0)
		file_error () ;

	if (link (PASSWD, OPASSWD))
		file_error () ;

	if (unlink (PASSWD))
		file_error () ;

	if (link (PASSTEMP, PASSWD))
		{
		if (link (OPASSWD, PASSWD))
			bad_news () ;

		file_error () ;
		}

	if (unlink (PASSTEMP))
		(void) fprintf (stderr, 
		       "%s: WARNING cannot unlink %s\n", prognamp, PASSTEMP ) ;

	if ( info_mask & BOTH_FILES )
		{

		if (unlink (OSHADOW) && access (OSHADOW, 0) == 0)
			{
			if ( rec_pwd() ) 
				bad_news () ;
			else
				file_error () ;
			}

		if (link (SHADOW, OSHADOW))
			{
			if ( rec_pwd() ) 
				bad_news () ;
			else
				file_error () ;
			}
		
		if (unlink (SHADOW))
			{
			if ( rec_pwd() ) 
				bad_news () ;
			else
				file_error () ;
			}

		if (link (SHADTEMP, SHADOW))
			{
			if (link (OSHADOW, SHADOW))
				bad_news () ;

			if ( rec_pwd() ) 
				bad_news () ;
			else
				file_error () ;
			}

		if (unlink (SHADTEMP))
			(void) fprintf (stderr, 
			       "%s: WARNING cannot unlink %s\n", prognamp, SHADTEMP) ;

		}

	ulckpwdf () ;

}  /* end of main */

/* Try to recover the old password file */

int
rec_pwd ()
{
	if ( unlink ( PASSWD ) || link ( OPASSWD, PASSWD ) )
		return (-1) ;

	return (0) ;
}

/* combine two uid_blk's */

void
uid_bcom ( uid_p )
struct uid_blk *uid_p ;
{
	struct uid_blk *uid_tp ;

	uid_tp = uid_p->link ;
	uid_p->high = uid_tp->high ;
	uid_p->link = uid_tp->link ;

	free ( uid_tp ) ;
}

/* add a new uid_blk */

void
add_ublk ( num, uid_p )
int num ;
struct uid_blk *uid_p ;
{
	struct uid_blk *uid_tp ;

	if ( (uid_tp = (struct uid_blk *) malloc((unsigned) 
		       sizeof(struct uid_blk)) ) == NULL )
		{
		rid_tmpf () ;
		file_error () ;
		}

	uid_tp->high = uid_tp->low = num ;
	uid_tp->link = uid_p->link ;
	uid_p->link = uid_tp ;
}

/*
	Here we are using a linked list of uid_blk to keep track of all
	the used uids.  Each uid_blk represents a range of used uid,
	with low represents the low inclusive end and high represents
	the high inclusive end.  In the beginning, we initialize a linked
	list of one uid_blk with low = high = (UID_MIN-1).  This was
	done in main(). 
	Each time we read in another used uid, we add it onto the linked 
	list by either making a new uid_blk, decrementing the low of 
	an existing uid_blk, incrementing the high of an existing 
	uid_blk, or combining two existing uid_blks.  After we finished
	building this linked list, the first available uid above or
	equal to UID_MIN is the high of the first uid_blk in the linked
	list + 1.
*/
/* add_uid() adds uid to the link list of used uids */
void
add_uid ( uid )
int uid ;
{
   struct uid_blk *uid_p ;
   /* Only keep track of the ones above UID_MIN */

   if ( uid >= UID_MIN )
      {
      uid_p = uid_sp ;

      while ( uid_p != NULL )
   	{

   	if ( uid_p->link != NULL )
   	   {

	   if ( uid >= uid_p->link->low )
	   	uid_p = uid_p->link ;

	   else if ( uid >= uid_p->low && 
		     uid <= uid_p->high )
		   {
		   uid_p = NULL ;
		   }

	        else if (uid == (uid_p->high+1))
		        {

		        if ( ++uid_p->high == (uid_p->link->low - 1) )
			   {
			   uid_bcom (uid_p) ;
			   }
		           uid_p = NULL ;
		         }

		     else if ( uid == (uid_p->link->low - 1) )
			     {
			     uid_p->link->low -- ;
			     uid_p = NULL ;
			     }

			  else if ( uid < uid_p->link->low )
			     	  {
			     	  add_ublk ( uid, uid_p ) ;
			     	  uid_p = NULL ;
			     	  }
	   } /* if uid_p->link */

	else

	   {
	
	   if ( uid == (uid_p->high + 1) )
	      {
	      uid_p->high++ ;
	      uid_p = NULL ;
	      }

	   else if ( uid >= uid_p->low && 
		     uid <= uid_p->high )
		   {
		   uid_p = NULL ;
		   }

		else
		   {
		   add_ublk ( uid, uid_p ) ;
		   uid_p = NULL ;
		   }

	   } /* else */
	
	} /* while uid_p */

      } /* if uid */
}

void
bad_perm()
{
	(void) fprintf (stderr, "%s: Permission denied\n", prognamp ) ;
	exit (1) ;
}

void
bad_usage(sp)
char *sp ;
{
	if ( strlen (sp) != 0 )
		(void) fprintf (stderr,"%s: %s\n", prognamp, sp) ;
	(void) fprintf (stderr,"Usage:\n") ;
	(void) fprintf (stderr,"%s -a [-c comment] [-h homedir] [-u uid [-o]] [-g gid] \n", prognamp ) ;
	(void) fprintf (stderr,"            [-s shell] name\n") ;
	(void) fprintf (stderr,"%s -m [-c comment] [-h homedir] [-u uid [-o]] [-g gid] \n", prognamp ) ;
	(void) fprintf (stderr,"            [-s shell] [-l logname] name\n") ;
	(void) fprintf (stderr,"%s -d name\n", prognamp ) ;
	if ( info_mask & LOCKED )
		ulckpwdf () ;
	exit (2) ;
}

void
bad_arg(s)
char *s ;
{
	(void) fprintf (stderr, "%s: %s\n",prognamp, s) ;

	if ( info_mask & LOCKED )
		ulckpwdf () ;
	exit (3) ;
}

void
bad_name(s)
char *s ;
{
	(void) fprintf (stderr, "%s: %s\n",prognamp, s) ;
	ulckpwdf () ;
	exit (9) ;
}

void
bad_uid()
{
	(void) fprintf (stderr, "%s: UID in use\n", prognamp ) ;

	ulckpwdf () ;
	exit (4) ;
}

void
bad_pasf()
{
	(void) fprintf (stderr, "%s: Inconsistent password files\n", prognamp ) ;

#ifdef RISCOS
	syslog(LOG_ALERT,"Inconsistant password files.");
#endif
	ulckpwdf () ;
	exit (5) ;
}

void
file_error()
{
	(void) fprintf (stderr, "%s: Unexpected failure.  Password files unchanged\n", prognamp ) ;

#ifdef RISCOS
	syslog(LOG_ALERT,"Unexpected failure in password update.");
#endif
	ulckpwdf () ;
	exit (6) ;
}

void
bad_news()
{
	(void) fprintf (stderr, "%s: Unexpected failure.  Password file(s) missing\n", prognamp ) ;

	ulckpwdf () ;
	exit (7) ;
}

void
no_lock ()
{
	(void) fprintf(stderr,"%s: Password file(s) busy.  Try again later\n", prognamp ) ;

	exit (8) ;
}

/* Check for the size of the whole passwd entry */
void
ck_p_sz ( pwp )
struct passwd *pwp ;
{
	char *ctp ;

	if ( (ctp = malloc ((unsigned) ENTRY_LENGTH)) == NULL )
		{
		rid_tmpf () ;
		file_error () ;
		}

	/* Ensure that the combined length of the individual */
	/* fields will fit in a passwd entry. The 1 accounts for the */
	/* newline and the 6 accounts for the colons (:'s) */
	if ( (	strlen  ( pwp->pw_name ) + 1 +
		sprintf ( ctp, "%d", pwp->pw_uid ) +
		sprintf ( ctp, "%d", pwp->pw_gid ) +
		strlen  ( pwp->pw_comment ) +
		strlen  ( pwp->pw_dir )	+
		strlen  ( pwp->pw_shell ) + 6) > (ENTRY_LENGTH-1) )
		{
		rid_tmpf () ;
		bad_arg ("New password entry too long") ;
		}
}

/* Check for the size of the whole passwd entry */
void
ck_s_sz ( ssp )
struct spwd *ssp ;
{
	char *ctp ;

	if ( (ctp = malloc ((unsigned) ENTRY_LENGTH)) == NULL )
		{
		rid_tmpf () ;

		if ( rec_pwd () )
			bad_news () ;
		file_error () ;
		}

	/* Ensure that the combined length of the individual */
	/* fields will fit in a shadow entry. The 1 accounts for the */
	/* newline and the 4 accounts for the colons (:'s) */
	if ( (	strlen  ( ssp->sp_namp ) + 1 +
		strlen  ( ssp->sp_pwdp ) +
		sprintf ( ctp, "%d", ssp->sp_lstchg ) +
		sprintf ( ctp, "%d", ssp->sp_min ) +
		sprintf ( ctp, "%d", ssp->sp_max ) + 4) > (ENTRY_LENGTH-1))
		{
		rid_tmpf () ;
		bad_arg ("New password entry too long") ;
		}
}

/* Get rid of the temp files */
void
rid_tmpf ()
{
	(void) fclose ( fp_ptemp ) ;

	if ( unlink ( PASSTEMP ) )
		(void) fprintf ( stderr, "%s: WARNING cannot unlink %s\n", 
				 prognamp, PASSTEMP ) ;

	if ( info_mask & BOTH_FILES )
		{
		(void) fclose ( fp_stemp ) ;

		if ( unlink ( SHADTEMP ) )
			(void) fprintf ( stderr, 
					 "%s: WARNING cannot unlink %s\n", 
					 prognamp, SHADTEMP ) ;
		}
}
/* this duplicates code from passwd.c.  This should be in a lib, but
 * interaction for YP entries is messy (they have to stay in the same
 * place in the file to preserve their semantics).
 */
FILE *pwf=NULL;
struct passwd passwd;
#ifndef MAXUID
#define MAXUID 65536
#endif MAXUID
#ifndef MAXGID
#define MAXGID 65536
#endif MAXGID
static char *
jpwskip(p)
register char *p;
{
	while(*p && *p != ':' && *p != '\n')
		++p;
	if(*p == '\n')
		*p = '\0';
	else if(*p)
		*p++ = '\0';
	return(p);
}

struct passwd *
jgetpwent(a)
	int a;
{

	extern struct passwd *jfgetpwent();

	if(pwf == NULL) {
		if((pwf = fopen(PASSWD, "r")) == NULL)
			return(NULL);
	}
	return (jfgetpwent(pwf));
}

struct passwd *
jfgetpwent(f)
FILE *f;
{
	register char *p;
	char *endp;
	long	x, strtol();
	char *memchr();

	if (f == (FILE *)NULL)
		return(NULL);
	p = fgets(line, BUFSIZ, f);
	if(p == NULL)
		return(NULL);
	strcpy(saveline,line);
	if ( *p == '+' || *p == '-')
	{
	    /* NIS escape, let caller worry about what to do with it*/
	    return((struct passwd *)-1);
	}
	passwd.pw_name = p;
	p = jpwskip(p);
	/* the real fgetpwent gives back the shadow pw here, which
	 * should not be done for the passwd pgm itself
	 * Rather, if the shadow exists, give the passwd as "x", which
	 * is what the pwconv did to passwd.
	 */
	passwd.pw_passwd = (shadow_exist) ? "x":p;
	p = jpwskip(p);
	if (p == NULL || *p == ':')
	{
		/* check for non-null uid */
		syslog(LOG_ALERT,"Bad passwd entry: %s",saveline);
		return ((struct passwd *)-1);
	}
	x = strtol(p, &endp, 10);	
	if (endp != memchr(p, ':', strlen(p)))
	{
		syslog(LOG_ALERT,"Bad passwd entry: %s",saveline);
		/* check for numeric value */
		return ((struct passwd *)-1);
	}
	p = jpwskip(p);
	passwd.pw_uid = (x < 0 || x > MAXUID)? (MAXUID+1): x;
	if ( x < 0 || x > MAXUID)
	{
	    syslog(LOG_ALERT,"Bad uid in passwd entry: %s",saveline);
	}
	if (p == NULL || *p == ':')
	{
		syslog(LOG_ALERT,"Bad passwd entry: %s",saveline);
		/* check for non-null uid */
		return ((struct passwd *)-1);
	}
	x = strtol(p, &endp, 10);	
	if (endp != memchr(p, ':', strlen(p)))
	{
		syslog(LOG_ALERT,"Bad passwd entry: %s",saveline);
		/* check for numeric value */
		return ((struct passwd *)-1);
	}
	p = jpwskip(p);
	passwd.pw_gid = (x < 0 || x > MAXGID)? (MAXGID+1): x;
	if ( x < 0 || x > MAXGID)
	{
	    syslog(LOG_ALERT,"Bad gid in passwd entry: %s",saveline);
	}
	passwd.pw_comment = p;
	passwd.pw_gecos = p;
	p = jpwskip(p);
	passwd.pw_dir = p;
	p = jpwskip(p);
	passwd.pw_shell = p;
	(void) jpwskip(p);

	p = passwd.pw_passwd;
	while(*p && *p != ',')
		p++;
	if(*p)
		*p++ = '\0';
#ifdef SYSTYPE_BSD43
	passwd.pw_quota = 0;
#else SYSTYPE_BSD43
	passwd.pw_age = p;
#endif SYSTYPE_BSD43
	return(&passwd);
}

jsetpwent()
{
	if(pwf == NULL) {
		if((pwf = fopen(PASSWD, "r")) == NULL)
			return(NULL);
	}
	else
		rewind(pwf);
}
