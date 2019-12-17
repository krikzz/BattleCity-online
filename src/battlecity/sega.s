.text

	.org	0x0000

	dc.l	0x01000000,RST
	dc.l	INT,INT,INT,INT,INT,INT,INT
	dc.l	INT,INT,INT,INT,INT,INT,INT,INT
	dc.l	INT,INT,INT,INT,INT,INT,INT,INT
	dc.l	INT,INT,INT,HBL,INT,VBL,INT,INT
	dc.l	INT,INT,INT,INT,INT,INT,INT,INT
	dc.l	INT,INT,INT,INT,INT,INT,INT,INT
	dc.l	INT,INT,INT,INT,INT,INT,INT,INT
	dc.l	INT,INT,INT,INT,INT,INT,INT

	.ascii	"SEGA SSF        "				    /* Console Name (16) 1 or nothing=ROM, 2=ROM+DATAFILE, 3=RAM*/
	.ascii	"KRIKzz 2014.NOV "				    /* Copyright Information (16) */
	.ascii	"Battle City Online                              "  /* Domestic Name (48) */
	.ascii	"Battle City Online                            "  /* Overseas Name (48) */
	.ascii	"GM 00000000-00"				    /* Serial Number (2, 14) */
	dc.w	0x0000						    /* Checksum (2) */
	.ascii	"JD              "				    /* I/O Support (16) */
	dc.l	0xffffffff 					    /* ROM Start Address (4) */
	dc.l	0x20000 					    /* ROM End Address (4) */
	dc.l	0x00FF0000					    /* Start of Backup RAM (4) */
	dc.l	0x00FFFFFF					    /* End of Backup RAM (4) */
	.ascii	"B                       "                          
	.ascii	"http://krikzz.com  biokrik@gmail.com    "	    /* Memo (40) */
	.ascii	"W               "				    /* Country Support (16) */

RST:
    move    #0x2700, %sr            /* disable interrupts */
    tst.l   0xA10008                /* check CTRL1 and CTRL2 setup */
    bne.b   1f
    tst.w   0xA1000C                /* check CTRL3 setup */
1:
    bne.b   skip_tmss               /* if any controller control port is setup, skip TMSS handling */

*Check Hardware Version Number
    move.b  0xA10001, %d0
    andi.b  #0x0F, %d0              /* VERS */
    beq     2f                      /* 0 = original hardware, TMSS not present */
    move.l  #0x53454741, 0xA14000   /* Store Sega Security Code "SEGA" to TMSS */
2:
    move.w  0xC00004, %d0           /* read VDP Status reg */

skip_tmss:
    move.w  #0x8104,0xC00004        /* display off, vblank disabled */
    move.w  0xC00004, %d0           /* read VDP Status reg */


*Clear Hardware
    lea    vdp_regs, %a0
    move.w #14, %d0 
1:/*setup vdp regs*/
    move.w (%a0)+, 0xC00004
    dbra.w %d0, 1b

    lea     0xC00000, %a1
    lea     0xFF0000, %a0
    move.l  #0, %d0
    move.w  #0x3FFF, %d1
    move.l  #0x40000000, 0xC00004
2:/*clear vram and wram*/
    move.l  %d0, (%a0)+
    move.l  %d0, (%a1)
    dbra    %d1, 2b
    
    move.w #0x1f, %d1
    move.l  #0xC0000000, 0xC00004
3:/*clean cram*/
    move.l  %d0, (%a1)
    dbra    %d1, 3b

    move.w #0x13, %d1
    move.l  #0x40000010, 0xC00004
4:/*clean cram*/
    move.l  %d0, (%a1)
    dbra    %d1, 4b

    move.w #0x2300, %sr
    jmp main


vdp_regs:
dc.w 0x8004 /*h-int off, hv ctr on*/
dc.w 0x8104 /*disp off, v-int off, dma off, cell mode 28*/
dc.w 0x8230 /*APLAN*/
dc.w 0x833C /*WPLAN*/
dc.w 0x8407 /*BPLAN*/
dc.w 0x857E /*SLIST*/
dc.w 0x8700 /*background pal and color*/
dc.w 0x8A00 /*h-int line*/
dc.w 0x8B00 /*ex-int off, vs full, hs full*/
dc.w 0x8C81 /*cell mode 40, shadow and hlihgt off, interlace off*/
dc.w 0x8D3E /*HSCRL*/
dc.w 0x8F02 /*auto inc*/
dc.w 0x9001 /*scroll size 64x32*/
dc.w 0x9100 /*win h pos*/
dc.w 0x9200 /*win v pos*/


INT:
    rte
VBL:
    move.w #1, vb_flag
    move.w #1, vb_flag_net
    add #1, frame_ctr
    rte
HBL:
    rte
