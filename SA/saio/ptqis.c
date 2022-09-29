#ident "$Header: ptqis.c,v 1.1 90/02/28 21:57:05 chungc Exp $"
/*	%Q%	%I%	%M%	*/
/* $Copyright$ */

/*
 * ptqis.c -- Pizazz SCSI cartridge tape controller standalone driver
 */

#include "sys/errno.h"
#include "sys/param.h"
#include "sys/mtio.h"
#include "machine/cpu.h"
#include "mipsvme/vmereg.h"
#include "saio/tpd.h"
#include "saio/saio.h"
#define 	i_lun i_ctlr
#include "saio/saioctl.h"

#define NOREGS
#include "mips/rambo.h"
#include "mips/scsi_3030sa.h"

#define NTARGETS	7
#define NLUNS		8
/* #define WR_DELAY	20000000  /* read/write delay */
#define WR_DELAY	20000000  /* read/write delay */
#define SPACE_DELAY	0x40000000  /* space filemark delay */
#define REWIND_DELAY	0x40000000  /* rewind delay */
#define WAITREADY	100  /* number of times to try TUN */
#define TP_BLKSIZ       (32 * 512)
#define TP_SHIFT        5	
#define TP_MASK         0x1f	
#define TP_BLKS_PER_REC	32
#define FM		0x80	/* Indicates that a File Mark has been read */
#define VALID		0x80	/* Indicates that the residual length field
				 * is defined (request sense command) */
#define UNIT_ATN	0x06	/* Sense Key indicates cartridge was changed
				 * or Viper was reset */
#define NOT_READY	0x02	/* Indicates the Viper can not be accessed */
#define BLANK_CHECK	0x08	/* Sense Key indicates a no-data condition */
#define SENSE_CNT	8	/* our default scsi sense count */
#define READ_DMA	1	/* use rambo for data xfer */
#define WRITE_DMA	0	/* use rambo for data xfer */

static char *TapeBuf;

struct	ts_softc {
	u_char	ts_open;	/* prevent multiple opens */
	u_char	ts_lastiow;	/* last operation was a write */
	int	ts_xfer;	/* # of bytes xfered by last read */
	int	ts_curfile;	/* last file read/written on tape */
	int	ts_nxtrec;	/* next record on tape to read/write */
	int	ts_resid;	/* residual byte count */
	struct  scsi_iopb scsi_iopb; /* single threaded */
	u_char	sense[16]; 	/* Last Request Sense bytes */
} ts_softc;
static struct scsi_unit scsi_unit; /* global unit defines */

static char tbufspace[ALIGNED_BUF_SIZE + TP_BLKSIZ + 64];

/*
 * _tsinit -- initialize driver global data
 */
_ptsinit()
{
	register struct scsi_iopb *iopb;
	register struct scsi_unit *un;

	if (sizeof (struct tp_dir) > IOB_FS)
		_io_abort ("bad size in iob for tp");
	bzero(&ts_softc,  sizeof(ts_softc));
	bzero(&scsi_unit, sizeof(scsi_unit));

	un = &scsi_unit;
	/* be consistent with unix level code, we'll assume 4k is
	 * a good maximum size for non-aligned stuff */
	/* un->un_buf_64 = (char*)align_malloc(ALIGNED_BUF_SIZE,64); */
	un->un_buf_64 = (char*)(((unsigned int)tbufspace + 64) & ~63);
	/* minimum dma granularity for RAMBO is 64 bytes 16-word alligned */
	un->un_dmastartmask = 0x3f;
	un->un_dmaaddmask   = 0x3f;
	un->un_dmacntmask   = 0x3f;
	iopb = &ts_softc.scsi_iopb;
	iopb->scsi_un = un;

	TapeBuf = (char *)(un->un_buf_64 + ALIGNED_BUF_SIZE);
}

/*
 * _tsopen -- open tape device
 */
_ptsopen(io)
register struct iob *io;
{
	register struct tp_dir *tpd;
	register struct ts_softc *ts;
	register int target = io->i_unit;
	register int lun = io->i_lun;
	register struct scsi_iopb *iopb;
	u_int status, addr, i, count;
	int tun = 0, flag = 0;

	/*
	 * verify lun and target numbers numbers
	 */
	if (lun >= NLUNS) {
		printf("ts bad lun number %d\n", lun);
		goto bad;
	}
	if (target >= NTARGETS) {
		printf("ts bad target number %d\n", target);
		goto bad;
	}
	/*
	 * tape is unique open device, so refuse if already open.
	 */
	ts = &ts_softc;
	if (ts->ts_open) {
		printf("ts: in use\n");
		goto badio;
	}
	iopb = &ts_softc.scsi_iopb;
	iopb->scsi_target = target ? target:6; /* if 0 => default id = 6 */

	if (_scsi_initp()) {	/* inititialize ncr/rambo chips */
		printf("ts: init of ncr, rambo chips failed\n");
		goto badio;
	}
	/* wait for tape drive to be ready
 	 */
	count = WAITREADY; /* init our down counter */
retryit:
	if (ptsiopb_init(io, C0_TESTRDY, 0, 0, 0, 0))
		goto badio;
	if ((status = scsicmdp(iopb)) == CHECK_CONDITION) {
		if (ptsiopb_init(io, C0_REQSENSE, 0, SENSE_CNT,
				&ts->sense[0], READ_DMA))
			goto badio;
		if (scsicmdp(iopb)) { /* do the op */
			printf("ts: request sense ERROR\n");
			goto badio;	/* no need to retry */
		} else if (ts->sense[2] & UNIT_ATN) {
#ifdef DEBUG
			if (!tun) {
				printf("\nts: unit attention\n");
				tun++;
			} else goto badio; /* only one allowed! */
#endif DEBUG
		} else if (ts->sense[2] & NOT_READY) {
#ifdef DEBUG
			if (!flag) {
				printf("not ready");
				flag++;
			} else printf(".");
#endif DEBUG
		} else {
			printf("TS SENSE DATA: ");
			for (i=0; i < SENSE_CNT; i++)
				printf("%x ",ts->sense[i]);
			printf("\n");
			goto badio;
		}
		if (count--) {
			_scandevs(); /* bounce the LEDS and scan for abort */
			goto retryit;
		}
		printf("\n");
	} else if (iopb->scsi_status) {
		printf("tp: %s, failed open\n",
				(char*)tsprinterr(iopb->scsi_status));
		goto badio;
	} else if (iopb->scsi_hwstatus) {
		printf("tp: %s, failed open\n",
				(char *)tsprinterr(iopb->scsi_hwstatus));
		goto badio;
	}
	/*
	 * Rewind the tape so that it may be moved forward to the
	 * correct file.
	 */
	printf("\nrewinding the tape....."); /* user courtesy */
	if (ptsiopb_init(io, C0_REWIND, 0, 0, 0, 0))
		goto badio;
	if (status = scsicmdp(iopb))
		goto stat;
	printf("done\n"); /* user courtesy */
	if (io->i_part) {
		count = io->i_part;
		printf("\n forward spacing the tape %d files.....",count);
		if (ptsiopb_init(io, C0_SPACE, count, FILES, 0, 0))
			goto badio;
		if (status = scsicmdp(iopb))
			goto stat;
		printf("done\n"); /* user courtesy */
	}
	/*
	 * Read in the volume header and see if it's valid.
	 * Everytime we open a file we read in the header.
	 * This is due to the fact that multiple directories may
	 * exist on one physical tape and we don't know which
	 * one was read in last time.
	 */
	if (ptsiopb_init(io,C0_READ,
			(TP_BLKSIZ>>DEV_BSHIFT),0,&TapeBuf[0],READ_DMA))
		goto badio;
	if (status = scsicmdp(iopb)) {
		goto stat;
	}
	/* TODO: think about how do deal with this statement;
	 * "unless a check condition occurred we xfer'ed the full amount"
	 * is this a true statement? what about a dma screwup? */
	ts->ts_xfer = TP_BLKSIZ; /* just say we read what we wanted to */
	tpd = (struct tp_dir *)io->i_fs_tape;
	bcopy (TapeBuf, tpd, sizeof (struct tp_dir));
	if (io->i_fstype != DTFS_NONE) {
		switch (io->i_fstype) {
		      case DTFS_AUTO:
				if (!is_tpd (tpd)) {
					io->i_fstype = DTFS_NONE;
					break;
				}
				else {
					/*
					 * If more than one type of volume
					 * header is ever possible we would
					 * then need to do some mapping
					 * like vh_mapfstype();
					 */
					io->i_fstype = DTFS_TPD;
					break;
				}

			case DTFS_TPD:
				if (!is_tpd (tpd)) {
					printf ("bad volume header\n");
					goto badio;
				}
		}
	}
	/*
	 * open successful
	 */
	ts->ts_curfile = 0;
	ts->ts_lastiow = 0;
	ts->ts_resid = 0;
	ts->ts_nxtrec = 0;
	return (0);
stat:
	if (status == CHECK_CONDITION) {
		if (ptsiopb_init(io, C0_REQSENSE, 0, SENSE_CNT,
					&ts->sense[0], READ_DMA))
			goto badio;
		if (status = scsicmdp(iopb)) { /* do the op */
			printf("ts: request sense ERROR\n");
			goto badio;	/* no need to retry */
		} else if (ts->sense[2] & UNIT_ATN)
			printf("ts: unit attention\n");
		printf("SENSE DATA: ");
		for (i=0; i < 16; i++)
			printf("%x ",ts->sense[i]);
		printf("\n");
	} else
		printf("ts: %s, failed open\n",(char*)tsprinterr(status));
badio:
	io->i_errno = EIO;
	return (-1);
bad:
	io->i_errno = ENXIO;
	return (-1);
}

/*
 * _tsclose -- close tape device
 */
_ptsclose(io)
register struct iob *io;
{
	register struct ts_softc *ts;
	register struct scsi_iopb *iopb;
	register ctlr = io->i_lun;
	struct int_unit *isu;
	u_int status, addr, i;

	ts = &ts_softc;
	iopb = &ts_softc.scsi_iopb;
	/*
	 * if tape was open for writing or last operation was a write,
	 * then write two EOF's and backspace over the last one.
	 */
	if ((io->i_flgs == F_WRITE) || ((io->i_flgs & F_WRITE) && 
	   (ts->ts_lastiow))) {
		/* TODO: are 2 FM's necessary? tpqic() uses two! */
		if (ptsiopb_init(io, C0_WRFM, 1, 0, 0, 0))
			goto failed;
		if (status = scsicmdp(iopb))
			goto bad;
	}
	/*
	 * rewind the tape
	 */
	printf("\nrewinding the tape....."); /* user courtesy */
	if (ptsiopb_init(io, C0_REWIND, 0, 0, 0, 0))
		goto failed;
	if (status = scsicmdp(iopb))
		goto bad;
	printf("done\n"); /* user courtesy */

	ts->ts_lastiow = 0;
	ts->ts_open = 0;
	return (0);

bad:
	if (status == CHECK_CONDITION) {
		if (ptsiopb_init(io, C0_REQSENSE, 0, SENSE_CNT,
					&ts->sense[0], READ_DMA))
			goto failed;
		if (status = scsicmdp(iopb)) { /* do the op */
			printf("ts: %s\n",(char *)tsprinterr(status));
		} else {
			printf("SENSE DATA: ");
			for (i=0; i < 16; i++)
				printf("%x ",ts->sense[i]);
			printf("\n");
		}
	} else
		printf("ts: %s\n",(char *)tsprinterr(status));
failed:
	ts->ts_lastiow = 0;
	ts->ts_open = 0;
	io->i_errno = EIO;
	return (-1);
}

/*
 * _tsstrategy -- perform io
 */
_ptsstrategy(io, func)
register struct iob *io;
register int func;
{
	register struct ts_softc *ts;
	register struct tp_dir *tpd;
	daddr_t blkno;
	struct scsi_iopb *iopb;
	struct int_unit *isu;
	int repcnt, off, newblk, xfer, space = 0;
	int blks, nxtblks, status = 0;
	int ctlr = io->i_lun;
	u_int count, addr, i;

	ts = &ts_softc;
	ts->ts_lastiow = 0;
	iopb = &ts->scsi_iopb;
	/*
	 * If the request is for block zero, func equals a read
	 * and we have a valid tape volume header just return it.
	 * (Normally called from inside the file systems open routine)
	 */
	if (func == READ) {
		tpd = (struct tp_dir *)io->i_fs_tape;
		if (io->i_bn == 0 && (tpd->td_magic == TP_MAGIC) &&
		    io->i_cc == sizeof (struct tp_dir)) {
			bcopy (tpd, io->i_ma, io->i_cc);
			return (io->i_cc);
		}
	}
	/*
	 * Normally this flag is set in the open routine.  In this
	 * environment it's not save to set it there.  If you're
	 * opening a tape with a file system and you specifiy a name
	 * to use.  You could pass the tape controllers open only to
	 * fail inside the file systems open routine. (which calls
	 * strategy). You would then
	 * be left with this flag set and no way to reopen the correct
	 * file unless you rebooted or restarted the standalone program.
	 * When you have reached this point in _tpqicstrategy I know
	 * that you have completed the open.
	 */
	ts->ts_open = 1;

	if (func == WRITE) {
		if ((io->i_bn >> TP_SHIFT) != ts->ts_nxtrec)
			return (-1);
		blks = (io->i_cc + (DEV_BSIZE-1)) >> DEV_BSHIFT;
		if (ptsiopb_init(io, C0_WRITE, blks, 0, io->i_ma, 0))
			goto failed;
		status = scsicmdp(iopb);
		ts->ts_resid = 0;
		ts->ts_nxtrec++;
	}
	else { /* we're doing a READ op */
		newblk = io->i_bn >> TP_SHIFT;
		if (newblk != ts->ts_nxtrec) {
#ifdef DEBUG
printf("strat: newblk = %d nextrec = %d\n",newblk,ts->ts_nxtrec);
#endif DEBUG
			ts->ts_nxtrec++;
			blks = newblk*TP_BLKS_PER_REC;
			nxtblks = ts->ts_nxtrec*TP_BLKS_PER_REC;
#ifdef DEBUG
printf("strat: blks = %d nxtblks = %d\n",blks,nxtblks);
#endif DEBUG
			if (newblk > ts->ts_nxtrec) {
				count = blks - nxtblks;
				if (ptsiopb_init(io, C0_SPACE,
							 count, BLOCKS, 0, 0))
					goto failed;
				status = scsicmdp(iopb);
			} else if (newblk < ts->ts_nxtrec) {
				count = nxtblks - blks;
				count = ~count + 1; /* two's complement */
				if (ptsiopb_init(io, C0_SPACE,
							 count, BLOCKS, 0, 0))
					goto failed;
				status = scsicmdp(iopb);
			}
			ts->ts_nxtrec = newblk;
			if (status) {
				space = 1;
				printf("ts: SPACE cmd error\n");
				goto stat;
			}
			blks = TP_BLKSIZ >> DEV_BSHIFT;
			if (ptsiopb_init(io, C0_READ, blks, 0,
							 &TapeBuf[0],READ_DMA))
				goto failed;
			if (status = scsicmdp(iopb))
				goto stat;
			/* unless we got a CHECK condition we read what we
			 * asked for */
			ts->ts_xfer = TP_BLKSIZ;
		}
		xfer = _min(ts->ts_xfer, io->i_cc);
		/*
		 * Using i_bn here because i_offset is valid for a file
		 * not the tape_file. It's the file systems job to worry
		 * about file offsets.
		 */
		off = (io->i_bn & TP_MASK) << DEV_BSHIFT;
		bcopy (&TapeBuf[off], io->i_ma, xfer);
		ts->ts_xfer -= xfer;
		ts->ts_resid = io->i_cc - xfer;
	}
stat:
	if (status == CHECK_CONDITION) {
		if (ptsiopb_init(io, C0_REQSENSE, 0, SENSE_CNT,
					&ts->sense[0], READ_DMA))
			goto failed;
		if (status = scsicmdp(iopb)) { /* do the op */
			ts->ts_resid = io->i_cc;
			goto badio;
		} else {
#ifdef DEBUG
			printf("SENSE DATA: ");
			for (i=0; i < 16; i++)
				printf("%x ",ts->sense[i]);
			printf("\n");
#endif DEBUG
			if (space) goto badio; /* SPACE cmd error? */
			if (ts->sense[2] & FM) { /* read a file mark? */
				if (ts->sense[0] & VALID) { /* residual? */
					count = ts->sense[6] << DEV_BSHIFT;
				} else count = 0; /* is this OK? */
				ts->ts_xfer = TP_BLKSIZ - count;
				xfer = _min(ts->ts_xfer, io->i_cc);
				off = (io->i_bn & TP_MASK) << DEV_BSHIFT;
#ifdef DEBUG
printf("count= 0x%x i_cc= 0x%x xfer= 0x%x ts->ts_resid= 0x%x\n",count,io->i_cc,xfer,ts->ts_resid);
#endif DEBUG
				bcopy (&TapeBuf[off], io->i_ma, xfer);
				ts->ts_xfer -= xfer;
				ts->ts_resid = io->i_cc - xfer;
			}
			else goto badio;
		}
	} else if (status) goto badio;
	return (io->i_cc - ts->ts_resid);

badio:
	printf("ts: %s\n",(char *)tsprinterr(status));
failed:
	io->i_errno = EIO;
	return (-1);
}
/*
 * _tsioctl -- io controls
 */
_ptsioctl(io, cmd, arg)
register struct iob *io;
register int cmd;
register caddr_t arg;
{
	io->i_errno = EINVAL;
	return (-1);
}
/*
** initialize an io parameter block 
*/
ptsiopb_init(io, cmd, blkcount, bcount, addr, r_w)
register struct iob *io;
register cmd, blkcount, bcount;
register unsigned addr;
{
	register struct ts_softc *ts;
	register struct scsi_iopb *iopb;
	struct volume_header *vh;
	struct device_parameters *devp;
	register unsigned hold_addr;
	register struct scsi_unit *un;

	ts = &ts_softc;
	iopb = &ts_softc.scsi_iopb;
	/*
	 * clear out previous status
 	 */
	iopb->scsi_status = 0;
	iopb->scsi_hwstatus = 0;

	if (addr) {	/* we're got a dma xfer */
		un = &scsi_unit;
		if (blkcount) {
			if (addr & un->un_dmaaddmask) {
			     printf("tpis: i/o buffer not aligned (%x)\n",addr);
			     io->i_errno = EIO;
			     return (1);
			}
			iopb->scsi_count = blkcount*DEV_BSIZE;
			iopb->scsi_bufaddr = PHYS_TO_K1(addr);
			iopb->scsi_flags = DMA_XFER;
		} else if (bcount) {
			if (bcount & un->un_dmacntmask) { /* not mod 64? */
			     if (bcount > ALIGNED_BUF_SIZE) {
			         printf("tpis: bcount (%x) > MAX(%x)\n",
					bcount,ALIGNED_BUF_SIZE);
			         io->i_errno = EIO;
			         return (1);
			     }
			}
			/* use these to be consistent with unix code */
			iopb->scsi_count0 = bcount;
			iopb->scsi_bufaddr0 = PHYS_TO_K1(addr);
			iopb->scsi_flags = PTM_XFER; /* indicate sub-blk dma */
		} else printf("tsiopb_init: trying to dma with no count!!!\n");
#ifdef DEBUG
printf("tpis: addr= 0x%x count= 0x%x\n",iopb->scsi_bufaddr,iopb->scsi_count);
#endif DEBUG
	} else {
		iopb->scsi_flags = NO_XFER;	/* no data in/out phase */
	}
	/*
	** SCSI Command Descriptor Bytes in the IOPB
	*/
	iopb->cmd_b0 = cmd;		/* SCSI Command */
	if (cmd == C0_READ || cmd == C0_WRITE || cmd == C0_WRFM ||
					cmd == C0_SPACE) {
		if (cmd == C0_WRFM) {
			iopb->cmd_b1 = io->i_lun<<5;
			iopb->scsi_time = WR_DELAY;
		} else if (cmd == C0_SPACE) {
			iopb->cmd_b1 = (io->i_lun<<5)|bcount; /* space code */
			iopb->scsi_time = SPACE_DELAY;
		} else {
			iopb->cmd_b1 = (io->i_lun<<5)|1; /* Fixed block size */
			iopb->scsi_time = WR_DELAY;
		}
		iopb->cmd_b2 = HB(blkcount);
		iopb->cmd_b3 = MB(blkcount);
		iopb->cmd_b4 = LB(blkcount);
        	return (0);
	}
	if (cmd == C0_REWIND)
		iopb->scsi_time = REWIND_DELAY;
	else
		iopb->scsi_time = WR_DELAY;
	iopb->cmd_b1 = io->i_lun<<5;
	iopb->cmd_b2 = 0;
	iopb->cmd_b3 = 0;
	iopb->cmd_b4 = bcount;  /* byte count */
	iopb->cmd_b5 = 0;
        return (0);
}
