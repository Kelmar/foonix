/*************************************************************************/
/*
 * $Id: ata.h 44 2010-02-19 23:53:25Z kfiresun $
 */
/*************************************************************************/
/*
 * Copyright (c) 1998, 2001 Manuel Bouyer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Manuel Bouyer.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)wdreg.h	7.1 (Berkeley) 5/9/91
 */
/*************************************************************************/

#ifndef __FOO_ATA_H__
#define __FOO_ATA_H__

/*************************************************************************/

#include "cdefs.h"

__BEGIN_DECLS

/*************************************************************************/

struct ata_command_TYPE
{
    uint32_t	flags;	    /* (i) Controlling flags */

    uint8_t	command;    /* (i) The ATA command to issue */

    uint8_t	features;   /* (i) ATA features */

    uint8_t	sect_num;   /* (i) Starting sector number */
    uint8_t	sect_cnt;   /* (i) Number of sectors to perform op on. */
    uint8_t	head;	    /* (i) Head */
    uint16_t	cylinder;   /* (i) Cylinder/LBA address */

    uint32_t	timeout;    /* (i) Timeout for command, -1 for no timeout. */

    uint8_t	status;	    /* (o) ATA status register result */
    uint8_t	error;	    /* (o) ATA error register result */

    void       *data;	    /* (*) I/O data buffer (if needed) */
    size_t	data_sz;    /* (i) Size of the above buffer in bytes */
} __attribute__((packed));

typedef struct ata_command_TYPE ata_command_t;

/*************************************************************************/

typedef struct ata_device_TYPE ata_device_t;

struct ata_bus_TYPE
{
    int		    id;		 /* A unique ID of some sort */
    uint32_t	    dev_count;	 /* Number of devices on this bus */

    io_port_t	    base_port1;  /* Base I/O port 1 address */
    io_port_t	    base_port2;	 /* Base I/O port 2 address */

    ata_device_t    *devices[2]; /* Reversed linked list of devices */
};

typedef struct ata_bus_TYPE ata_bus_t;

/*************************************************************************/

struct ata_device_TYPE
{
    uint32_t	id;
    char	model[40];
    char	revision[8];
    char	serial[20];

    uint32_t	flags;

    uint32_t	sec_flags;
};

/*************************************************************************/

#define ATADEV_IS_ATAPI		0x01

#define ATADEV_SEC_SUPPORTED	 0x01
#define ATADEV_SEC_FROZEN	 0x02
#define ATADEV_SEC_ENABLED	 0x04
#define ATADEV_SEC_LOCKED	 0x08
#define ATADEV_SEC_COUNT_EXP	 0x10
#define ATADEV_SEC_SUPP_ENHANCED 0x20

/*************************************************************************/
/*
 * I shamfully stole this out of the NetBSD atareg.h file....  As such, I
 * shall reproduce the copyright here.  -- Bryce
 */

/* Commands for Disk Controller. */
#define	WDCC_NOP		0x00	/* Always fail with "aborted command" */
#define	WDCC_RECAL		0x10	/* disk restore code -- resets cntlr */

#define	WDCC_READ		0x20	/* disk read code */
#define	WDCC_WRITE		0x30	/* disk write code */
#define	 WDCC__LONG		0x02	/* modifier -- access ecc bytes */
#define	 WDCC__NORETRY		0x01	/* modifier -- no retrys */

#define	WDCC_FORMAT		0x50	/* disk format code */
#define	WDCC_DIAGNOSE		0x90	/* controller diagnostic */
#define	WDCC_IDP		0x91	/* initialize drive parameters */

#define	WDCC_SMART		0xb0	/* Self Mon, Analysis, Reporting Tech */

#define	WDCC_READMULTI		0xc4	/* read multiple */
#define	WDCC_WRITEMULTI		0xc5	/* write multiple */
#define	WDCC_SETMULTI		0xc6	/* set multiple mode */

#define	WDCC_READDMA		0xc8	/* read with DMA */
#define	WDCC_WRITEDMA		0xca	/* write with DMA */

#define	WDCC_ACKMC		0xdb	/* acknowledge media change */
#define	WDCC_LOCK		0xde	/* lock drawer */
#define	WDCC_UNLOCK		0xdf	/* unlock drawer */

#define	WDCC_FLUSHCACHE		0xe7	/* Flush cache */
#define	WDCC_FLUSHCACHE_EXT	0xea	/* Flush cache ext */
#define	WDCC_IDENTIFY		0xec	/* read parameters from controller */
#define	SET_FEATURES		0xef	/* set features */

#define	WDCC_IDLE		0xe3	/* set idle timer & enter idle mode */
#define	WDCC_IDLE_IMMED		0xe1	/* enter idle mode */
#define	WDCC_SLEEP		0xe6	/* enter sleep mode */
#define	WDCC_STANDBY		0xe2	/* set standby timer & enter standby */
#define	WDCC_STANDBY_IMMED	0xe0	/* enter standby mode */
#define	WDCC_CHECK_PWR		0xe5	/* check power mode */

#define WDCC_SECURITY_FREEZE	0xf5	/* freeze locking state */

/* Big Drive support */
#define	WDCC_READ_EXT		0x24	/* read 48-bit addressing */
#define	WDCC_WRITE_EXT		0x34	/* write 48-bit addressing */

#define	WDCC_READMULTI_EXT	0x29	/* read multiple 48-bit addressing */
#define	WDCC_WRITEMULTI_EXT	0x39	/* write multiple 48-bit addressing */

#define	WDCC_READDMA_EXT	0x25	/* read 48-bit addressing with DMA */
#define	WDCC_WRITEDMA_EXT	0x35	/* write 48-bit addressing with DMA */

#define ATAPI_IDENTIFY_DEVICE	0xA1

/*
 * End of shamfully stolen bits....
 */

#define WDCC_SECURITY_SET_PASS	    0xF1
#define WDCC_SECURITY_UNLOCK	    0xF2
#define WDCC_SECURITY_ERASE_PREPAIR 0xF3
#define WDCC_SECURITY_ERASE_UNIT    0xF4
/*#define WDCC_SECURITY_FREEZE	    0xF5*/ /* Defined by NetBSD */
#define WDCC_SECURITY_DISABLE	    0xF6

/*************************************************************************/

#define ATA_SZ_MODEL 40
#define ATA_SZ_SERIAL 20
#define ATA_SZ_REVISION 8

/*************************************************************************/

/*
 * Another bit of shamefully stolen code..... - Bryce
 */
struct ata_params_TYPE
{
    /* drive info */
    uint16_t	atap_config;		/* 0: general configuration */
#define WDC_CFG_ATAPI_MASK    	0xc000
#define WDC_CFG_ATAPI    	0x8000
#define	ATA_CFG_REMOVABLE	0x0080
#define	ATA_CFG_FIXED		0x0040
#define ATAPI_CFG_TYPE_MASK	0x1f00
#define ATAPI_CFG_TYPE(x) (((x) & ATAPI_CFG_TYPE_MASK) >> 8)
#define	ATAPI_CFG_REMOV		0x0080
#define ATAPI_CFG_DRQ_MASK	0x0060
#define ATAPI_CFG_STD_DRQ	0x0000
#define ATAPI_CFG_IRQ_DRQ	0x0020
#define ATAPI_CFG_ACCEL_DRQ	0x0040
#define ATAPI_CFG_CMD_MASK	0x0003
#define ATAPI_CFG_CMD_12	0x0000
#define ATAPI_CFG_CMD_16	0x0001
/* words 1-9 are ATA only */
    uint16_t	atap_cylinders;		/* 1: # of non-removable cylinders */
    uint16_t	__reserved1;
    uint16_t	atap_heads;		/* 3: # of heads */
    uint16_t	__retired1[2];		/* 4-5: # of unform. bytes/track */
    uint16_t	atap_sectors;		/* 6: # of sectors */
    uint16_t	__retired2[3];

    char	atap_serial[ATA_SZ_SERIAL];	/* 10-19: serial number */
    uint16_t	__retired3[2];
    uint16_t	__obsolete1;
    char	atap_revision[ATA_SZ_REVISION];	/* 23-26: firmware revision */
    char	atap_model[ATA_SZ_MODEL];		/* 27-46: model number */
    uint16_t	atap_multi;		/* 47: maximum sectors per irq (ATA) */
    uint16_t	__reserved2;
    uint16_t	atap_capabilities1;	/* 49: capability flags */
#define WDC_CAP_IORDY	0x0800
#define WDC_CAP_IORDY_DSBL 0x0400
#define	WDC_CAP_LBA	0x0200
#define	WDC_CAP_DMA	0x0100
#define ATA_CAP_STBY	0x2000
#define ATAPI_CAP_INTERL_DMA	0x8000
#define ATAPI_CAP_CMD_QUEUE	0x4000
#define	ATAPI_CAP_OVERLP	0X2000
#define ATAPI_CAP_ATA_RST	0x1000
    uint16_t	atap_capabilities2;	/* 50: capability flags (ATA) */
    uint8_t	__junk2;
    uint8_t	atap_oldpiotiming;	/* 51: old PIO timing mode */
    uint8_t	__junk3;
    uint8_t	atap_olddmatiming;	/* 52: old DMA timing mode (ATA) */
    uint16_t	atap_extensions;	/* 53: extensions supported */
#define WDC_EXT_UDMA_MODES	0x0004
#define WDC_EXT_MODES		0x0002
#define WDC_EXT_GEOM		0x0001
/* words 54-62 are ATA only */
    uint16_t	atap_curcylinders;	/* 54: current logical cylinders */
    uint16_t	atap_curheads;		/* 55: current logical heads */
    uint16_t	atap_cursectors;	/* 56: current logical sectors/tracks */
    uint16_t	atap_curcapacity[2];	/* 57-58: current capacity */
    uint16_t	atap_curmulti;		/* 59: current multi-sector setting */
#define WDC_MULTI_VALID 0x0100
#define WDC_MULTI_MASK  0x00ff
    uint16_t	atap_capacity[2];  	/* 60-61: total capacity (LBA only) */
    uint16_t	__retired4;
    uint8_t	atap_dmamode_supp; 	/* 63: multiword DMA mode supported */
    uint8_t	atap_dmamode_act; 	/*     multiword DMA mode active */
    uint8_t	atap_piomode_supp;       /* 64: PIO mode supported */
    uint8_t	__junk4;
    uint16_t	atap_dmatiming_mimi;	/* 65: minimum DMA cycle time */
    uint16_t	atap_dmatiming_recom;	/* 66: recommended DMA cycle time */
    uint16_t	atap_piotiming;    	/* 67: mini PIO cycle time without FC */
    uint16_t	atap_piotiming_iordy;	/* 68: mini PIO cycle time with IORDY FC */
    uint16_t	__reserved3[2];
/* words 71-72 are ATAPI only */
    uint16_t	atap_pkt_br;		/* 71: time (ns) to bus release */
    uint16_t	atap_pkt_bsyclr;	/* 72: tme to clear BSY after service */
    uint16_t	__reserved4[2];
    uint16_t	atap_queuedepth;   	/* 75: */
#define WDC_QUEUE_DEPTH_MASK 0x0F
    uint16_t   atap_sata_caps;/* 76: */
#define SATA_SIGNAL_GEN1	0x02
#define SATA_SIGNAL_GEN2	0x04
#define SATA_NATIVE_CMDQ	0x0100
#define SATA_HOST_PWR_MGMT	0x0200
    uint16_t   atap_sata_reserved;    /* 77: */
    uint16_t   atap_sata_features_supp;    /* 78: */
#define SATA_NONZERO_OFFSETS	0x02
#define SATA_DMA_SETUP_AUTO	0x04
#define SATA_DRIVE_PWR_MGMT	0x08
    uint16_t   atap_sata_features_en;    /* 79: */
    uint16_t	atap_ata_major;  	/* 80: Major version number */
#define	WDC_VER_ATA1	0x0002
#define	WDC_VER_ATA2	0x0004
#define	WDC_VER_ATA3	0x0008
#define	WDC_VER_ATA4	0x0010
#define	WDC_VER_ATA5	0x0020
#define	WDC_VER_ATA6	0x0040
#define	WDC_VER_ATA7	0x0080
    uint16_t   atap_ata_minor;  	/* 81: Minor version number */
    uint16_t	atap_cmd_set1;    	/* 82: command set supported */
#define	WDC_CMD1_NOP	0x4000		/*	NOP */
#define	WDC_CMD1_RB	0x2000		/*	READ BUFFER */
#define	WDC_CMD1_WB	0x1000		/*	WRITE BUFFER */
/*			0x0800			Obsolete */
#define	WDC_CMD1_HPA	0x0400		/*	Host Protected Area */
#define	WDC_CMD1_DVRST	0x0200		/*	DEVICE RESET */
#define	WDC_CMD1_SRV	0x0100		/*	SERVICE */
#define	WDC_CMD1_RLSE	0x0080		/*	release interrupt */
#define	WDC_CMD1_AHEAD	0x0040		/*	look-ahead */
#define	WDC_CMD1_CACHE	0x0020		/*	write cache */
#define	WDC_CMD1_PKT	0x0010		/*	PACKET */
#define	WDC_CMD1_PM	0x0008		/*	Power Management */
#define	WDC_CMD1_REMOV	0x0004		/*	Removable Media */
#define	WDC_CMD1_SEC	0x0002		/*	Security Mode */
#define	WDC_CMD1_SMART	0x0001		/*	SMART */
    uint16_t	atap_cmd_set2;    	/* 83: command set supported */
#define	ATA_CMD2_FCE	0x2000		/*	FLUSH CACHE EXT */
#define	WDC_CMD2_FC	0x1000		/*	FLUSH CACHE */
#define	WDC_CMD2_DCO	0x0800		/*	Device Configuration Overlay */
#define	ATA_CMD2_LBA48	0x0400		/*	48-bit Address */
#define	WDC_CMD2_AAM	0x0200		/*	Automatic Acoustic Management */
#define	WDC_CMD2_SM	0x0100		/*	SET MAX security extension */
#define	WDC_CMD2_SFREQ	0x0040		/*	SET FEATURE is required
						to spin-up after power-up */
#define	WDC_CMD2_PUIS	0x0020		/*	Power-Up In Standby */
#define	WDC_CMD2_RMSN	0x0010		/*	Removable Media Status Notify */
#define	ATA_CMD2_APM	0x0008		/*	Advanced Power Management */
#define	ATA_CMD2_CFA	0x0004		/*	CFA */
#define	ATA_CMD2_RWQ	0x0002		/*	READ/WRITE DMA QUEUED */
#define	WDC_CMD2_DM	0x0001		/*	DOWNLOAD MICROCODE */
    uint16_t	atap_cmd_ext;		/* 84: command/features supp. ext. */
#define	ATA_CMDE_TLCONT	0x1000		/*	Time-limited R/W Continuous */
#define	ATA_CMDE_TL	0x0800		/*	Time-limited R/W */
#define	ATA_CMDE_URGW	0x0400		/*	URG for WRITE STREAM DMA/PIO */
#define	ATA_CMDE_URGR	0x0200		/*	URG for READ STREAM DMA/PIO */
#define	ATA_CMDE_WWN	0x0100		/*	World Wide name */
#define	ATA_CMDE_WQFE	0x0080		/*	WRITE DMA QUEUED FUA EXT */
#define	ATA_CMDE_WFE	0x0040		/*	WRITE DMA/MULTIPLE FUA EXT */
#define	ATA_CMDE_GPL	0x0020		/*	General Purpose Logging */
#define	ATA_CMDE_STREAM	0x0010		/*	Streaming */
#define	ATA_CMDE_MCPTC	0x0008		/*	Media Card Pass Through Cmd */
#define	ATA_CMDE_MS	0x0004		/*	Media serial number */
#define	ATA_CMDE_SST	0x0002		/*	SMART self-test */
#define	ATA_CMDE_SEL	0x0001		/*	SMART error logging */
    uint16_t	atap_cmd1_en;		/* 85: cmd/features enabled */
/* bits are the same as atap_cmd_set1 */
    uint16_t	atap_cmd2_en;		/* 86: cmd/features enabled */
/* bits are the same as atap_cmd_set2 */
    uint16_t	atap_cmd_def;		/* 87: cmd/features default */
    uint8_t	atap_udmamode_supp; 	/* 88: Ultra-DMA mode supported */
    uint8_t	atap_udmamode_act; 	/*     Ultra-DMA mode active */
/* 89-92 are ATA-only */
    uint16_t	atap_seu_time;		/* 89: Sec. Erase Unit compl. time */
    uint16_t	atap_eseu_time;		/* 90: Enhanced SEU compl. time */
    uint16_t	atap_apm_val;		/* 91: current APM value */
    uint16_t	__reserved6[35];	/* 92-126: reserved */
    uint16_t	atap_rmsn_supp;		/* 127: remov. media status notif. */
#define WDC_RMSN_SUPP_MASK 0x0003
#define WDC_RMSN_SUPP 0x0001
    uint16_t	atap_sec_st;		/* 128: security status */
#define WDC_SEC_LEV_MAX	0x0100
#define WDC_SEC_ESE_SUPP 0x0020
#define WDC_SEC_EXP	0x0010
#define WDC_SEC_FROZEN	0x0008
#define WDC_SEC_LOCKED	0x0004
#define WDC_SEC_EN	0x0002
#define WDC_SEC_SUPP	0x0001
} __attribute__((packed));

/*
 * Okay, we don't have to keep holding our heads in shame now.
 */

typedef struct ata_params_TYPE ata_params_t;

/*************************************************************************/

#define ATA_STATUS_BSY	0x80	/* Busy */
#define ATA_STATUS_RDY	0x40	/* Device Ready */
#define ATA_STATUS_DF	0x20	/* Device Fault */
#define ATA_STATUS_DSC	0x10	/* Seek Complete */
#define ATA_STATUS_DRQ	0x08	/* Data xfer Requested */
#define ATA_STATUS_CORR	0x04	/* Data Corrected */
#define ATA_STATUS_IDX	0x02	/* Index Mark */
#define ATA_STATUS_ERR	0x01	/* Error */

#define ATA_ERROR_BBK	0x80	/* Bad Block (also UDMA bad CRC) */
#define ATA_ERROR_UNC	0x40	/* Uncorrectable Data Error */
#define ATA_ERROR_MC	0x20	/* Media Changed */
#define ATA_ERROR_IDNF	0x10	/* ID mark not found */
#define	ATA_ERROR_MCR	0x08	/* Media Change Requested */
#define	ATA_ERROR_ABRT	0x04	/* Command Aborted */
#define ATA_ERROR_TK0NF	0x02	/* Track 0 not found */
#define ATA_ERROR_AMNF	0x01	/* Address mark not found */

/*************************************************************************/

#define ATA_ERR_NOERR	     0
#define ATA_ERR_TIMEOUT	    -1
#define ATA_ERR_BADPARAM    -2
#define ATA_ERR_ABRTCMD     -3

/*************************************************************************/

#define ATA_FLAG_READ	0x00000001  /* Reading data from the ATA device. */
#define ATA_FLAG_WRITE	0x00000002  /* Writing data to the ATA device. */
#define ATA_FLAG_DATA	0x00000003  /* Data operation */

/*************************************************************************/

void init_ata(void);
ata_bus_t *get_ata_bus(unsigned int id);
int do_ata_command(ata_bus_t *, ata_command_t *);

void wipe_drive(ata_bus_t *, int drive, const char *passwd, size_t psz);

/*************************************************************************/

__END_DECLS

#endif /* __FOO_ATA_H__ */

/*************************************************************************/
