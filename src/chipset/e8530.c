/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e8530.c                                          *
 * Created:     2007-11-11 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2020 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include <stdlib.h>
#include <stdio.h>

#include "e8530.h"


#define DEBUG_SCC 0

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define scc_get_chn(chn) (((chn) == 0) ? 'A' : 'B')


static
void e8530_init_chn (e8530_t *scc, unsigned chn)
{
	e8530_chn_t *c;

	c = &scc->chn[chn];

	c->txd_empty = 1;
	c->rxd_empty = 1;

	c->int_on_next_rx = 0;

	c->char_clk_cnt = 0;
	c->char_clk_div = 16384;

	c->read_char_cnt = 0;
	c->read_char_max = 1;

	c->write_char_cnt = 0;
	c->write_char_max = 1;

	c->rtxc = 0;

	c->tx_i = 0;
	c->tx_j = 0;

	c->rx_i = 0;
	c->rx_j = 0;

	c->set_inp_ext = NULL;
	c->set_inp = NULL;

	c->set_out_ext = NULL;
	c->set_out = NULL;

	c->set_rts_ext = NULL;
	c->set_rts = NULL;

	c->set_comm_ext = NULL;
	c->set_comm = NULL;
}

void e8530_init (e8530_t *scc)
{
	scc->index = 0;

	scc->pclk = 0;

	e8530_init_chn (scc, 0);
	e8530_init_chn (scc, 1);

	scc->irq_ext = NULL;
	scc->irq = NULL;
	scc->irq_val = 0;
}

void e8530_free (e8530_t *scc)
{
}

e8530_t *e8530_new (void)
{
	e8530_t *scc;

	if ((scc = malloc (sizeof (e8530_t))) == NULL) {
		return (NULL);
	}

	e8530_init (scc);

	return (scc);
}

void e8530_del (e8530_t *scc)
{
	if (scc != NULL) {
		e8530_free (scc);
		free (scc);
	}
}

void e8530_set_irq_fct (e8530_t *scc, void *ext, void *fct)
{
	scc->irq_ext = ext;
	scc->irq = fct;
}

void e8530_set_inp_fct (e8530_t *scc, unsigned chn, void *ext, void *fct)
{
	if (chn < 2) {
		scc->chn[chn].set_inp_ext = ext;
		scc->chn[chn].set_inp = fct;
	}
}

void e8530_set_out_fct (e8530_t *scc, unsigned chn, void *ext, void *fct)
{
	if (chn < 2) {
		scc->chn[chn].set_out_ext = ext;
		scc->chn[chn].set_out = fct;
	}
}

void e8530_set_rts_fct (e8530_t *scc, unsigned chn, void *ext, void *fct)
{
	if (chn < 2) {
		scc->chn[chn].set_rts_ext = ext;
		scc->chn[chn].set_rts = fct;
	}
}

void e8530_set_comm_fct (e8530_t *scc, unsigned chn, void *ext, void *fct)
{
	if (chn < 2) {
		scc->chn[chn].set_comm_ext = ext;
		scc->chn[chn].set_comm = fct;
	}
}

void e8530_set_multichar (e8530_t *scc, unsigned chn, unsigned read_max, unsigned write_max)
{
	if (chn > 1) {
		return;
	}

	scc->chn[chn].read_char_cnt = 0;
	scc->chn[chn].read_char_max = (read_max < 1) ? 1 : read_max;

	scc->chn[chn].write_char_cnt = 0;
	scc->chn[chn].write_char_max = (write_max < 1) ? 1 : write_max;
}

void e8530_set_clock (e8530_t *scc, unsigned long pclk, unsigned long rtxca, unsigned long rtxcb)
{
	scc->pclk = pclk;
	scc->chn[0].rtxc = rtxca;
	scc->chn[1].rtxc = rtxcb;
}

static
void e8530_set_irq (e8530_t *scc, unsigned char val)
{
	if (scc->irq_val == val) {
		return;
	}

	scc->irq_val = val;

	if (scc->irq != NULL) {
		scc->irq (scc->irq_ext, val);
	}
}

static
void e8530_set_rts (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_chn_t *c;

	c = &scc->chn[chn];

	if (c->set_rts != NULL) {
		c->set_rts (c->set_rts_ext, val != 0);
	}
}

#define ES_INT_COND 0x01
#define TX_INT_COND 0x02
#define RX_INT_COND 0x04
#define SC_INT_COND 0x08

static
void e8530_set_int_cond (e8530_t *scc, unsigned chn, unsigned char cond)
{
	e8530_chn_t *c0, *c;

	c = &scc->chn[chn];
	c0 = &scc->chn[0];

	if ((cond & ES_INT_COND) && (c->wr[1] & 0x01)) {
		/* ext */

		/* should check if IP is already set */

		c0->rr[3] |= (chn == 0) ? 0x08 : 0x01;

		c->rr0_latch_msk = c->wr[15];
		c->rr0_latch_val = c->rr[0];
	}

	if ((cond & TX_INT_COND) && (c->wr[1] & 0x02)) {
		/* transmit interrupt */
		c0->rr[3] |= (chn == 0) ? 0x10 : 0x02;
	}

#ifdef SDLC_LOCALTALK_ENABLE
	if (cond & (RX_INT_COND | SC_INT_COND)) {
		unsigned set_rx_int = 0;
		switch((c->wr[1] & 0x18) >> 3) {
			case 0x01: /* "Rx Int On First Character or Special Condition" */
				if(scc->chn[chn].int_on_1st_rx || (cond & SC_INT_COND) ) {
					scc->chn[chn].int_on_1st_rx = 0;
					set_rx_int = 1;
				}
				break;
			case 0x10: /* "Int On All Rx Characters or Special Condition" */
				set_rx_int = 1;
				break;
			case 0x11: /* "Rx In On Special Condition Only" */
				if(cond & SC_INT_COND) {
					set_rx_int = 1;
				}
				break;
		}
		if(set_rx_int) {
#ifdef SDLC_DEBUG
			fprintf (stderr, "SCC %c: rx interrupt set\n", scc_get_chn (chn));
#endif
			/* receive interrupt */
			c0->rr[3] |= (chn == 0) ? 0x20 : 0x04;
		}
	}
#else
	if ((cond & RX_INT_COND) && ((c->wr[1] & 0x18) == 0x10)) {
		/* receive interrupt */
		int data, spec, irq;

		data = (c->rxd_empty == 0);
		spec = (c->rr[0] & 0x80) != 0;
		irq = 0;

		switch ((c->wr[1] >> 3) & 3) {
		case 0: /* disabled */
			irq = 0;
			break;

		case 1: /* rx int on first char / special condition */
			irq = (data && c->int_on_next_rx) || spec;
			break;

		case 2: /* rx int on all chars / special condition */
			irq = data || spec;
			break;

		case 3: /* rx int on special condition */
			irq = spec;
			break;
		}

		if (data) {
			c->int_on_next_rx = 0;
		}

		if (irq) {
			c0->rr[3] |= (chn == 0) ? 0x20 : 0x04;
		}
	}
#endif

	if ((c0->wr[9] & 0x08) == 0) {
		/* MIE == 0 */
		e8530_set_irq (scc, 0);
		return;
	}

	e8530_set_irq (scc, c0->rr[3] != 0);
}

static
void e8530_clr_int_cond (e8530_t *scc, unsigned chn, unsigned char cond)
{
	e8530_chn_t *c, *c0;

	c = &scc->chn[chn];
	c0 = &scc->chn[0];

	if (cond & 0x01) {
		/* ext */

		c0->rr[3] &= (chn == 0) ? ~0x08 : ~0x01;

		c->rr0_latch_msk = 0;
	}

	if (cond & 0x02) {
		/* transmit interrupt */
		c0->rr[3] &= (chn == 0) ? ~0x10 : ~0x02;
	}

	if (cond & 0x04) {
		/* receive interrupt */
		c0->rr[3] &= (chn == 0) ? ~0x20 : ~0x04;
	}

	if ((c0->wr[9] & 0x08) == 0) {
		/* MIE == 0 */
		e8530_set_irq (scc, 0);
		return;
	}

	e8530_set_irq (scc, c0->rr[3] != 0);
}

#ifdef SDLC_LOCALTALK_ENABLE
static
unsigned e8530_check_for_sdlc_packet(e8530_t *scc, unsigned chn);

unsigned sdlc_frame_available = 0;

void e8530_sdlc_frame_available() {
	sdlc_frame_available = 1;
}
#endif

/*
 * Move a character from the input queue to RxD and adjust interrupt
 * conditions.
 */
static
void e8530_check_rxd (e8530_t *scc, unsigned chn)
{
	unsigned int_cond = RX_INT_COND;
	e8530_chn_t *c;

	c = &scc->chn[chn];

#ifdef SDLC_LOCALTALK_ENABLE
	if(sdlc_frame_available) {
		e8530_check_for_sdlc_packet(scc,chn);
	}
#endif

	if (c->rx_i == c->rx_j) {
		return;
	}

	if (c->read_char_cnt == 0) {
		return;
	}

	if ((c->wr[3] & 0x01) == 0) {
		/* receiver disabled */
		return;
	}

	if (c->rxd_empty == 0) {
		/* should overwrite old character */
		return;
	}

#ifdef SDLC_LOCALTALK_ENABLE
	if((scc->chn[chn].wr[4] & 0x30) == 0x20) {
		/* When in SDLC mode... */
		if((c->rx_j+1) == c->rx_i) {
			/* If we are reading the last byte in rxbuf, we signal an end of frame */
			/* Set End of Frame bit in RR1 */
#ifdef SDLC_DEBUG
			fprintf (stderr, "SCC %c: setting End of Frame bit\n", scc_get_chn (chn));
#endif
			scc->chn[chn].rr[1] |= 0x80;
			/* Raise interrupt for special condition */
			int_cond |= SC_INT_COND;
		} else {
			/* ...otherwise, if not last byte in rxbuf */
#ifdef SDLC_DEBUG
			fprintf (stderr, "SCC %c: resetting hunt bit and End of Frame bit\n", scc_get_chn (chn));
#endif
			/* Clear hunt mode */
			scc->chn[chn].rr[0] &= ~0x10;
			/* Clear End of Frame (SDLC) bit in RR1 */
			scc->chn[chn].rr[1] &= ~0x80;
		}
	}
#endif

	c->read_char_cnt -= 1;

	c->rr[8] = c->rxbuf[c->rx_j];
	c->rx_j = (c->rx_j + 1) % E8530_BUF_MAX;

	if (c->set_inp != NULL) {
		c->set_inp (c->set_inp_ext, 1);
	}

	c->rr[0] |= 0x01;
	c->rxd_empty = 0;

	e8530_set_int_cond (scc, chn, int_cond);
}

#ifdef SDLC_LOCALTALK_ENABLE
static void e8530_set_rr0 (e8530_t *scc, unsigned chn, unsigned char val);

static
void e8530_set_tx_underrun (e8530_t *scc, unsigned chn) {
	if((scc->chn[chn].rr[0] & 0x40) == 0) {
		/* A 0-to-1 transition of the Tx Underrun/EOM bit triggers an External/Status interrupt */
		e8530_set_rr0 (scc, chn, scc->chn[chn].rr[0] |= 0x40);
	}
}
#endif

#ifdef SDLC_LOCALTALK_ENABLE
static
void e8530_send_sdlc_packet (e8530_t *scc, unsigned chn) {
	e8530_chn_t   *c;

	if(chn != 1) {
		/* Only port B does LocalTalk on a Mac */
		return;
	}

	c = &scc->chn[chn];

	/* Transmit the packet */
#ifdef SDLC_DEBUG
	fprintf (stderr, "SCC %c: transmitting packet type 0x%x from node 0x%x to node 0x%x.\n", scc_get_chn (chn), c->txbuf[2], c->txbuf[1], c->txbuf[0] );
#endif
#ifdef EMSCRIPTEN
	EM_ASM_({
		if(typeof e8530_send_sdlc_packet_js === "function") {
			e8530_send_sdlc_packet_js($0,$1);
		}
	}, c->txbuf, c->tx_i);
#endif

	/* Get ready for next frame */
	c->sdlc_frame_in_progress = 0;
	c->tx_i = 0;
	c->tx_j = 0;
}

static
unsigned e8530_check_for_sdlc_packet(e8530_t *scc, unsigned chn) {
	e8530_chn_t   *c;
	unsigned      packet_size = 0;

	if(!sdlc_frame_available) {
		return 0;
	}

	if(chn != 1) {
		/* Only port B does LocalTalk on a Mac */
		return 0;
	}

	c = &scc->chn[chn];

	if(c->wr[5] & 0x08) {
		/* If TxEnable is set, don't fill the read buffer as the host
		   is transmitting and listening on the line for collisions */
		return 0;
	}

	if((c->rx_i > 0) && (c->rx_j != c->rx_i)) {
#ifdef SDLC_DEBUG
		fprintf (stderr, "SCC %c: abandoned packet (%d bytes out of %d bytes read)\n", scc_get_chn (chn), c->rx_j, c->rx_i);
#endif
	}

	/* e8530_poll_sdlc_packet_js may set this flag again if there is more than one packet in the queue */
	sdlc_frame_available = 0;

	/* This ensures we read at least one character from the frame */
	c->rxd_empty = 1;

#ifdef EMSCRIPTEN
	packet_size = EM_ASM_INT({
		if(typeof e8530_poll_sdlc_packet_js === "function") {
			return e8530_poll_sdlc_packet_js($0,$1);
		}
	}, c->rxbuf, E8530_BUF_MAX-2);
#endif

	if(packet_size) {
#ifdef SDLC_DEBUG
		fprintf (stderr, "SCC %c: got network packet of size %d\n", scc_get_chn (chn), packet_size);
#endif
		c->rx_i      = packet_size;
		c->rx_j      = 0;

		/* Append CRC bytes */
		c->rxbuf[c->rx_i] = 0x00;
		c->rx_i = (c->rx_i + 1) % E8530_BUF_MAX;

		c->rxbuf[c->rx_i] = 0x00;
		c->rx_i = (c->rx_i + 1) % E8530_BUF_MAX;
	} else {
		c->rx_i      = 0;
		c->rx_j      = 0;
	}

	return 1;
}
#endif

/*
 * Move a character from TxD to the output queue and adjust interrupt
 * conditions.
 */
static
void e8530_check_txd (e8530_t *scc, unsigned chn)
{
	e8530_chn_t   *c;
	unsigned char val;

	c = &scc->chn[chn];

	if (c->write_char_cnt == 0) {
		return;
	}

	if (((c->tx_i + 1) % E8530_BUF_MAX) == c->tx_j) {
		return;
	}

	if (c->txd_empty) {
		/* tx underrun */
		c->rr[0] |= 0x40;
		return;
	}

	c->write_char_cnt -= 1;

	val = c->wr[8];

	c->txbuf[c->tx_i] = val;
	c->tx_i = (c->tx_i + 1) % E8530_BUF_MAX;

#ifdef SDLC_LOCALTALK_ENABLE
	if((c->wr[4] & 0x30) == 0x20) {
		/* When in SDLC mode... */
		/* Set a flag to let us know we are tranmitting a frame. */
		c->sdlc_frame_in_progress = 1;
	} else 
#endif
	{
		/* When in regular serial mode, notify char device of new data */
		if (c->set_out != NULL) {
			c->set_out (c->set_out_ext, val);
		}
	}

	c->rr[0] |= 0x04;
	c->txd_empty = 1;

	e8530_set_int_cond (scc, chn, TX_INT_COND);

#if DEBUG_SCC
	fprintf (stderr, "SCC %c: send %02X\n", scc_get_chn (chn), val);
#endif
}


static
unsigned e8530_get_clock_mode (e8530_t *scc, unsigned chn)
{
	switch ((scc->chn[chn].wr[4] >> 6) & 3) {
	case 0:
		return (1);

	case 1:
		return (16);

	case 2:
		return (32);

	case 3:
		return (64);
	}

	return (0);
}

static
unsigned e8530_get_bits_per_char (e8530_t *scc, unsigned chn)
{
	switch ((scc->chn[chn].wr[5] >> 5) & 3) {
	case 0:
		return (5);

	case 1:
		return (7);

	case 2:
		return (6);

	case 3:
		return (8);
	}

	return (0);
}

static
unsigned e8530_get_parity (e8530_t *scc, unsigned chn)
{
	switch (scc->chn[chn].wr[4] & 3) {
	case 0:
		return (0);

	case 1: /* odd */
		return (1);

	case 2:
		return (0);

	case 3: /* even */
		return (2);
	}

	return (0);
}

/*
 * Get 2 * the number of stop bits
 */
static
unsigned e8530_get_stop_bits (e8530_t *scc, unsigned chn)
{
	switch ((scc->chn[chn].wr[4] >> 2) & 3) {
	case 0:
		return (0);

	case 1:
		return (2);

	case 2:
		return (3);

	case 3:
		return (4);

	}

	return (0);
}

/*
 * Update the communication parameters and call the callback function
 */
static
void e8530_set_params (e8530_t *scc, unsigned chn)
{
	unsigned long bps, clk, mul, div;
	e8530_chn_t   *c;

	c = &scc->chn[chn];

	c->bpc = e8530_get_bits_per_char (scc, chn);
	c->parity = e8530_get_parity (scc, chn);
	c->stop = e8530_get_stop_bits (scc, chn);

	mul = e8530_get_clock_mode (scc, chn);

	if (c->stop == 0) {
		/* sync mode */
		mul = 1;
	}

	if (c->wr[14] & 0x01) {
		/* baud rate generator enabled */

		if (c->wr[14] & 0x02) {
			clk = scc->pclk;
		}
		else {
			clk = c->rtxc;
		}

		div = (c->wr[13] << 8) | c->wr[12];
		div = 2 * mul * (div + 2);

		c->char_clk_div = ((2 * c->bpc + c->stop + 2) * div) / 2;

		if (c->parity != 0) {
			c->char_clk_div += div;
		}

		bps = clk / div;
	}
	else {
		c->char_clk_div = 16384;
		bps = 0;
	}

	c->bps = bps;

	if (c->set_comm != NULL) {
		c->set_comm (c->set_comm_ext, c->bps, c->parity, c->bpc, c->stop);
	}

#if DEBUG_SCC
	{
		static const char p[3] = { 'N', 'O', 'E' };
		static const char *s[5] = { "0", "", "1", "1.5", "2" };

		fprintf (stderr, "SCC %c: %lu%c%u%s\n",
			scc_get_chn (chn), c->bps, p[c->parity], c->bpc, s[c->stop]
		);
	}
#endif
}


/*
 * Set new RR0 value reflecting the external status and set an
 * interrupt condition if an enabled status changed.
 */
static
void e8530_set_rr0 (e8530_t *scc, unsigned chn, unsigned char val)
{
	unsigned char old;

	chn &= 1;

	old = scc->chn[chn].rr[0];
	scc->chn[chn].rr[0] = val;

	if ((old ^ val) & scc->chn[chn].wr[15] & 0xfa) {
		e8530_set_int_cond (scc, chn, ES_INT_COND);
	}
}


/*
 * RR0: transmit/receive buffer status and external status
 */
static
unsigned char e8530_get_rr0 (e8530_t *scc, unsigned chn)
{
	unsigned char val;

	val = scc->chn[chn].rr[0] & ~scc->chn[chn].rr0_latch_msk;
	val |= scc->chn[chn].rr0_latch_val & scc->chn[chn].rr0_latch_msk;

	return (val);
}

#ifdef SDLC_LOCALTALK_ENABLE
/*
 * RR1: special receive condition
 */
static
unsigned char e8530_get_rr1 (e8530_t *scc, unsigned chn)
{
	return (scc->chn[chn].rr[1]);
}
#endif

/*
 * RR2: interrupt vector
 * If read from channel B, include status
 */
static
unsigned char e8530_get_rr2 (e8530_t *scc, unsigned chn)
{
	unsigned char val;
	e8530_chn_t   *c0;

	c0 = &scc->chn[0];

	val = c0->rr[2];

	if (chn == 1) {
		unsigned char st;

		/* include status in vector */

		if (c0->rr[3] & 0x20) {
			/* chn a rx */
			st = 0x06 | (0x03 << 3);
		}
		else if (c0->rr[3] & 0x10) {
			/* chn a tx */
			st = 0x04 | (0x01 << 3);
		}
		else if (c0->rr[3] & 0x08) {
			/* chn a ext */
			st = 0x05 | (0x05 << 3);
		}
		else if (c0->rr[3] & 0x04) {
			/* chn b rx */
			st = 0x02 | (0x02 << 3);
		}
		else if (c0->rr[3] & 0x02) {
			/* chn b tx */
			st = 0x00 | (0x00 << 3);
		}
		else if (c0->rr[3] & 0x01) {
			/* chn b ext */
			st = 0x01 | (0x04 << 3);
		}
		else {
			st = 0x03 | (0x06 << 3);
		}

		if (c0->wr[9] & 0x10) {
			/* status high */
			val = (val & 0x70) | ((st & 0x38) << 1);
		}
		else {
			val = (val & 0xf1) | ((st & 0x07) << 1);
		}
	}

	return (val);
}

/*
 * RR3: interrupt pending
 */
static
unsigned char e8530_get_rr3 (e8530_t *scc, unsigned chn)
{
	if (chn != 0) {
		return (0);
	}

	return (scc->chn[0].rr[3]);
}

/*
 * RR8: receive buffer
 */
static
unsigned char e8530_get_rr8 (e8530_t *scc, unsigned chn)
{
	e8530_chn_t *c;
	unsigned char val;

	c = &scc->chn[chn];

	val = c->rr[8];
#ifdef SDLC_DEBUG
	fprintf (stderr, "SCC %c: get RR8: %02x\n", scc_get_chn (chn), val);
#endif

	c->rr[0] &= ~0x01;
	c->rxd_empty = 1;

	e8530_clr_int_cond (scc, chn, 0x04);

	e8530_check_rxd (scc, chn);

	return (val);
}

static
unsigned char e8530_get_reg (e8530_t *scc, unsigned chn, unsigned reg)
{
	unsigned char val;

	chn &= 1;

	switch (reg) {
	case 0x00:
		val = e8530_get_rr0 (scc, chn);
		break;

#ifdef SDLC_LOCALTALK_ENABLE
	case 0x01:
		val = e8530_get_rr1 (scc, chn);
		break;
#endif

	case 0x02:
		val = e8530_get_rr2 (scc, chn);
		break;

	case 0x03:
		val = e8530_get_rr3 (scc, chn);
		break;

	case 0x08:
		val = e8530_get_rr8 (scc, chn);
		break;

	default:
		val = scc->chn[chn].rr[reg & 15];
		break;
	}


	scc->index = 0;

#if DEBUG_SCC
	fprintf (stderr, "SCC %c: get RR%u -> %02x\n",
		scc_get_chn (chn), reg, val
	);
#endif

	return (val);
}

/*
 * WR0: command register
 */
static
void e8530_set_wr0 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[0] = val;

	scc->index = val & 7;

	switch ((val >> 3) & 7) {
	case 0x00: /* null command */
		break;

	case 0x01: /* point high */
		scc->index += 8;
		break;

	case 0x02: /* reset external/status interrupts */
		e8530_clr_int_cond (scc, chn, 0x01);
		break;

	case 0x03: /* send abort */
#ifdef SDLC_LOCALTALK_ENABLE
		fprintf (stderr, "SCC %c: Send abort\n", scc_get_chn (chn));
		e8530_set_tx_underrun(scc, chn);
#endif
		break;

	case 0x04: /* enable interrupt on next rx character */
#ifdef SDLC_LOCALTALK_ENABLE
		if((scc->chn[chn].wr[1] & 0x18) == 0x08) {
			scc->chn[chn].int_on_1st_rx = 1;
#ifdef SDLC_DEBUG
		fprintf (stderr, "SCC %c: Enable interrupt on next rx character\n", scc_get_chn (chn));
#endif
		}
#endif
		scc->chn[chn].int_on_next_rx = 1;
		break;

	case 0x05: /* reset tx interrupt pending */
		e8530_clr_int_cond (scc, chn, 0x02);
		break;

	case 0x06: /* error reset */
#ifdef SDLC_LOCALTALK_ENABLE
#ifdef SDLC_DEBUG
		fprintf (stderr, "SCC %c: Error reset (RR1 was 0x%x)\n", scc_get_chn (chn), scc->chn[chn].rr[1]);
#endif
		/* Clear End of Frame (SDLC) bit in RR1 */
		scc->chn[chn].rr[1] &= ~0x80;
		/* Clear Rx Fifo Overrun */
		scc->chn[chn].rr[1] &= ~0x20;
#endif
		scc->chn[chn].rr[1] &= 0x7f;
		break;

	case 0x07: /* reset highest ius */
		break;
	}

#ifdef SDLC_LOCALTALK_ENABLE
	switch ((val >> 6) & 0x03) {
		case 0x00: /* Null command */
			break;
		case 0x01: /* Reset Rx CRC Checker */
			break;
		case 0x10: /* Reset Tx CRC Generator */
			break;
		case 0x11: /* Reset Tx Underrun/EOM Latch */
			fprintf (stderr, "SCC %c: Reset Tx Underrun/EOM Latch\n", scc_get_chn (chn));
			/* Michael Fort's vMac code uses this condition to detect the end of an SDLC frame,
			 * but at least under System 6.0 this condition never seems to get triggered.
			 * I will leave this check here, but I also do the same when the transmitter is
			 * disabled, which seems to be a better condition. */
			if(scc->chn[chn].sdlc_frame_in_progress) {
				e8530_send_sdlc_packet(scc, chn);
			}
			break;
	}
#endif
}

/*
 * WR1: transmit/receive interrupt and data transfer mode definition
 */
static
void e8530_set_wr1 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[1] = val;

#ifdef SDLC_LOCALTALK_ENABLE
#ifdef SDLC_DEBUG
	char *msg;
	switch((scc->chn[chn].wr[1] >> 3) & 0x03) {
		case 0x00: msg = "SCC %c: Rx Int Disable\n"; break;
		case 0x01: msg = "SCC %c: Rx Int On First Character or Special Condition\n"; break;
		case 0x10: msg = "SCC %c: Int On All Rx Characters or Special Condition\n"; break;
		case 0x11: msg = "SCC %c: Int On Special Condition Only\n"; break;
	};
	fprintf (stderr, msg, scc_get_chn (chn));
#endif
	if((scc->chn[chn].wr[1] & 0x18) == 0x08) {
		scc->chn[chn].int_on_1st_rx = 1;
	}
#endif

	e8530_set_int_cond (scc, chn, 0x00);
}

/*
 * WR2: interrupt vector
 */
static
void e8530_set_wr2 (e8530_t *scc, unsigned char val)
{
	scc->chn[0].wr[2] = val;
	scc->chn[1].wr[2] = val;

	scc->chn[0].rr[2] = val;
	scc->chn[1].rr[2] = val;
}

/*
 * WR3: receive parameters and control
 */
static
void e8530_set_wr3 (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_chn_t *c;

	c = &scc->chn[chn];

	if (c->wr[3] ^ val) {
		if (val & 0x01) {
#if DEBUG_SCC >= 1
			fprintf (stderr, "SCC %c: receiver enable\n", scc_get_chn (chn));
#endif
			c->int_on_next_rx = 1;
			c->rxd_empty = 1;
		}
		else {
			c->rr[0] &= ~0x01;
			e8530_clr_int_cond (scc, chn, 0x04);
#if DEBUG_SCC >= 1
			fprintf (stderr, "SCC %c: receiver disable\n", scc_get_chn (chn));
#endif
		}
	}

	if (val & 0x10) {
#if DEBUG_SCC >= 1
		if ((c->rr[0] & 0x10) == 0) {
			fprintf (stderr, "SCC %c: sync/hunt mode\n", scc_get_chn (chn));
		}
#endif
		c->rr[0] |= 0x10;
	}

	c->wr[3] = val;
}

/*
 * WR4: transmit/receive miscellaneous parameters and modes
 */
static
void e8530_set_wr4 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[4] = val;

#ifdef SDLC_LOCALTALK_ENABLE
#ifdef SDLC_DEBUG
	const char *msg;
	switch((val >> 4) & 0x03) {
		case 0x00: msg = "SCC %c: 8-Bit Sync Character\n"; break;
		case 0x01: msg = "SCC %c: 16-Bit Sync Character\n"; break;
		case 0x10: msg = "SCC %c: SDLC Mode (01111110 Flag)\n"; break;
		case 0x11: msg = "SCC %c: External Sync Mode\n"; break;
	};
	fprintf (stderr, msg, scc_get_chn (chn));
#endif

	if(((val >> 4) & 0x03) != 0x02) {
#ifdef SDLC_DEBUG
		fprintf (stderr, "SCC %c: resetting End of Frame bit\n", scc_get_chn (chn));
#endif
		/* Clear End of Frame (SDLC) bit in RR1 on any mode other than SDLC */
		scc->chn[chn].rr[1] &= ~0x80;
	}
#endif

	e8530_set_params (scc, chn);
}

/*
 * WR5: transmit parameters and controls
 */
static
void e8530_set_wr5 (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_chn_t   *c;
	unsigned char old;

	c = &scc->chn[chn];

	old = c->wr[5];
	c->wr[5] = val;

	if ((old ^ val) & 0x02) {
		e8530_set_rts (scc, chn, (val & 0x02) != 0);
	}

#ifdef SDLC_LOCALTALK_ENABLE
	if ((old ^ val) & 0x08) {
#ifdef SDLC_DEBUG
		fprintf (stderr, "SCC %c: Tx Enable = %c\n", scc_get_chn (chn), (val & 0x08) ? '1' : '0');
#endif
		if((val & 0x08) == 0) {
			/* On of the last things AppleTalk under System 6.0 does after transmitting a
			 * packet is turn off the transmitter. There seems to be no other indication, so
			 * this is a good place to write our SDLC frame trailer */
			if(c->sdlc_frame_in_progress) {
				e8530_send_sdlc_packet(scc, chn);
			}
		}
	}
#endif

	e8530_set_params (scc, chn);
}

#ifdef SDLC_LOCALTALK_ENABLE
/*
 * WR6: sync characters of SDLC Address Field
 */
static
void e8530_set_wr6 (e8530_t *scc, unsigned chn, unsigned char val)
{
#ifdef SDLC_DEBUG
	fprintf (stderr, "SCC %c: SDLC address is 0x%x\n", scc_get_chn (chn), val);
#endif

	scc->chn[chn].wr[6] = val;
}
#endif

/*
 * WR8: transmit buffer
 */
static
void e8530_set_wr8 (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_chn_t *c;

	c = &scc->chn[chn];

	c->wr[8] = val;

	c->rr[0] &= ~0x04;
	c->txd_empty = 0;

	e8530_clr_int_cond (scc, chn, 0x02);

	e8530_check_txd (scc, chn);

#if DEBUG_SCC
	fprintf (stderr, "SCC %c: send %02X\n", scc_get_chn (chn), val);
#endif
}

/*
 * WR9: Master interrupt control
 */
static
void e8530_set_wr9 (e8530_t *scc, unsigned char val)
{
	unsigned char old;

	old = scc->chn[0].wr[9];

	scc->chn[0].wr[9] = val;
	scc->chn[1].wr[9] = val;

	if ((old ^ val) & val & 0x08) {
		/* MIE was enabled, re-check pending interrupts */
		e8530_set_int_cond (scc, 0, 0);
		e8530_set_int_cond (scc, 1, 0);
	}
}

/*
 * WR10: miscellaneous transmitter/receiver control bits
 */
static
void e8530_set_wr10 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[10] = val;
}

/*
 * WR11: clock mode control
 */
static
void e8530_set_wr11 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[11] = val;
}

/*
 * WR12: baud rate generator low byte
 */
static
void e8530_set_wr12 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[12] = val;

	e8530_set_params (scc, chn);
}

/*
 * WR13: baud rate generator high byte
 */
static
void e8530_set_wr13 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[13] = val;

	e8530_set_params (scc, chn);
}

/*
 * WR14: miscellaneous control bits
 */
static
void e8530_set_wr14 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[14] = val;

#if DEBUG_SCC
	if ((val & 0xe0) != 0) {
		fprintf (stderr, "SCC %c: dpll cmd: %u\n",
			scc_get_chn (chn), (val >> 5) & 7
		);
	}
#endif

	e8530_set_params (scc, chn);
}

/*
 * WR15: external/status interrupt control
 */
static
void e8530_set_wr15 (e8530_t *scc, unsigned chn, unsigned char val)
{
	scc->chn[chn].wr[15] = val;
	scc->chn[chn].rr[15] = val;

#ifdef SDLC_LOCALTALK_ENABLE
	if(val & 0x10) {
		fprintf (stderr, "SCC %c: Sync/Hunt IE set in WR15\n", scc_get_chn (chn));
	}
	if(val & 0x40) {
		fprintf (stderr, "SCC %c: Tx Underrun/EOM IE set in WR15\n", scc_get_chn (chn));
	}
#endif
}

void e8530_set_reg (e8530_t *scc, unsigned chn, unsigned reg, unsigned char val)
{
	chn &= 1;

#if DEBUG_SCC
	fprintf (stderr, "SCC %c: set WR%u <- %02x\n",
		scc_get_chn (chn), reg, val
	);
#endif

	switch (reg) {
	case 0x00:
		e8530_set_wr0 (scc, chn, val);
		break;

	case 0x01:
		e8530_set_wr1 (scc, chn, val);
		break;

	case 0x02:
		e8530_set_wr2 (scc, val);
		break;

	case 0x03:
		e8530_set_wr3 (scc, chn, val);
		break;

	case 0x04:
		e8530_set_wr4 (scc, chn, val);
		break;

	case 0x05:
		e8530_set_wr5 (scc, chn, val);
		break;

#ifdef SDLC_LOCALTALK_ENABLE
	case 0x06:
		e8530_set_wr6 (scc, chn, val);
		break;
#endif

	case 0x08:
		e8530_set_wr8 (scc, chn, val);
		break;

	case 0x09:
		e8530_set_wr9 (scc, val);
		break;

	case 0x0a:
		e8530_set_wr10 (scc, chn, val);
		break;

	case 0x0b:
		e8530_set_wr11 (scc, chn, val);
		break;

	case 0x0c:
		e8530_set_wr12 (scc, chn, val);
		break;

	case 0x0d:
		e8530_set_wr13 (scc, chn, val);
		break;

	case 0x0e:
		e8530_set_wr14 (scc, chn, val);
		break;

	case 0x0f:
		e8530_set_wr15 (scc, chn, val);
		break;

	default:
		scc->chn[chn].wr[reg & 15] = val;
		break;
	}

	if (reg != 0) {
		scc->index = 0;
	}
}


unsigned char e8530_get_ctl (e8530_t *scc, unsigned chn)
{
	return (e8530_get_reg (scc, chn & 1, scc->index));
}

unsigned char e8530_get_ctl_a (e8530_t *scc)
{
	return (e8530_get_reg (scc, 0, scc->index));
}

unsigned char e8530_get_ctl_b (e8530_t *scc)
{
	return (e8530_get_reg (scc, 1, scc->index));
}

void e8530_set_ctl (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_set_reg (scc, chn & 1, scc->index, val);
}

void e8530_set_ctl_a (e8530_t *scc, unsigned char val)
{
	e8530_set_reg (scc, 0, scc->index, val);
}

void e8530_set_ctl_b (e8530_t *scc, unsigned char val)
{
	e8530_set_reg (scc, 1, scc->index, val);
}

unsigned char e8530_get_data (e8530_t *scc, unsigned chn)
{
	return (e8530_get_reg (scc, chn & 1, 8));
}

unsigned char e8530_get_data_a (e8530_t *scc)
{
	return (e8530_get_reg (scc, 0, 8));
}

unsigned char e8530_get_data_b (e8530_t *scc)
{
	return (e8530_get_reg (scc, 1, 8));
}

void e8530_set_data (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_set_reg (scc, chn & 1, 8, val);
}

void e8530_set_data_a (e8530_t *scc, unsigned char val)
{
	e8530_set_reg (scc, 0, 8, val);
}

void e8530_set_data_b (e8530_t *scc, unsigned char val)
{
	e8530_set_reg (scc, 1, 8, val);
}


void e8530_set_dcd (e8530_t *scc, unsigned chn, unsigned char val)
{
	unsigned char old;

	chn &= 1;
	old = scc->chn[chn].rr[0];

	if (val) {
		old &= ~0x08;
	}
	else {
		old |= 0x08;
	}

	e8530_set_rr0 (scc, chn, old);
}

void e8530_set_dcd_a (e8530_t *scc, unsigned char val)
{
	e8530_set_dcd (scc, 0, val);
}

void e8530_set_dcd_b (e8530_t *scc, unsigned char val)
{
	e8530_set_dcd (scc, 1, val);
}

void e8530_set_cts (e8530_t *scc, unsigned chn, unsigned char val)
{
	unsigned char rr0;

	chn &= 1;
	rr0 = scc->chn[chn].rr[0];

	if (val) {
		rr0 &= ~0x20;
	}
	else {
		rr0 |= 0x20;
	}

	e8530_set_rr0 (scc, chn, rr0);
}

void e8530_set_cts_a (e8530_t *scc, unsigned char val)
{
	e8530_set_cts (scc, 0, val);
}

void e8530_set_cts_b (e8530_t *scc, unsigned char val)
{
	e8530_set_cts (scc, 1, val);
}

void e8530_receive (e8530_t *scc, unsigned chn, unsigned char val)
{
	e8530_chn_t *c;

	chn &= 1;
	c = &scc->chn[chn];

#ifdef SDLC_LOCALTALK_ENABLE
	if((scc->chn[chn].wr[4] & 0x30) == 0x20) {
		/* In SDLC packet mode, we don't use the serial interface */
		return;
	}
#endif

	if (((c->rx_i + 1) % E8530_BUF_MAX) != c->rx_j) {
		c->rxbuf[c->rx_i] = val;
		c->rx_i = (c->rx_i + 1) % E8530_BUF_MAX;
	}
}

void e8530_receive_a (e8530_t *scc, unsigned char val)
{
	e8530_receive (scc, 0, val);
}

void e8530_receive_b (e8530_t *scc, unsigned char val)
{
	e8530_receive (scc, 1, val);
}

unsigned char e8530_send (e8530_t *scc, unsigned chn)
{
#ifdef SDLC_LOCALTALK_ENABLE
	if((scc->chn[chn].wr[4] & 0x30) == 0x20) {
		/* In SDLC packet mode, we don't use the serial interface */
		return (0);
	}
#endif

	unsigned char val;
	e8530_chn_t   *c;

	chn &= 1;
	c = &scc->chn[chn];

	if (c->tx_i == c->tx_j) {
		return (0);
	}

	val = c->txbuf[c->tx_j];
	c->tx_j = (c->tx_j + 1) % E8530_BUF_MAX;

	return (val);
}

unsigned char e8530_send_a (e8530_t *scc)
{
	return (e8530_send (scc, 0));
}

unsigned char e8530_send_b (e8530_t *scc)
{
	return (e8530_send (scc, 1));
}

int e8530_inp_full (e8530_t *scc, unsigned chn)
{
	e8530_chn_t *c;

	chn &= 1;
	c = &scc->chn[chn];

#ifdef SDLC_LOCALTALK_ENABLE
	if((scc->chn[chn].wr[4] & 0x30) == 0x20) {
		/* In SDLC packet mode, we don't use the serial interface */
		return (1);
	}
#endif

	if (((c->rx_i + 1) % E8530_BUF_MAX) == c->rx_j) {
		return (1);
	}

	return (0);
}

int e8530_out_empty (e8530_t *scc, unsigned chn)
{
	e8530_chn_t *c;

	chn &= 1;
	c = &scc->chn[chn];

#ifdef SDLC_LOCALTALK_ENABLE
	if((scc->chn[chn].wr[4] & 0x30) == 0x20) {
		/* In SDLC packet mode, we don't use the serial interface */
		return (1);
	}
#endif

	if (c->tx_i == c->tx_j) {
		return (1);
	}

	return (0);
}

static
void e8530_reset_channel (e8530_t *scc, unsigned chn)
{
	e8530_chn_t *c;

	chn &= 1;
	c = &scc->chn[chn];

	c->rr[0] |= 0x04; /* tx empty */
#if 0
	c->rr[0] |= 0x20; /* cts */
#endif
	c->rr[1] |= 0x01; /* all sent */

	c->rr0_latch_msk = 0;

	c->bps = 0;
	c->parity = 0;
	c->bpc = 0;
	c->stop = 0;

	c->tx_i = 0;
	c->tx_j = 0;

	c->rx_i = 0;
	c->rx_j = 0;
}

void e8530_reset (e8530_t *scc)
{
	unsigned i;

	scc->index = 0;

	for (i = 0; i < 16; i++) {
		scc->chn[0].rr[i] = 0;
		scc->chn[0].wr[i] = 0;
		scc->chn[1].rr[i] = 0;
		scc->chn[1].wr[i] = 0;
	}

	e8530_reset_channel (scc, 0);
	e8530_reset_channel (scc, 1);

	e8530_set_rts (scc, 0, 0);
	e8530_set_rts (scc, 1, 0);

	e8530_set_irq (scc, 0);
}

static inline
void e8530_chn_clock (e8530_t *scc, unsigned chn, unsigned n)
{
	e8530_chn_t *c;

	c = &scc->chn[chn];

	if (n < c->char_clk_cnt) {
		c->char_clk_cnt -= n;
		return;
	}
	else {
		n -= c->char_clk_cnt;
	}

	if (n > c->char_clk_div) {
		n = n % c->char_clk_div;
	}

	c->char_clk_cnt = c->char_clk_div - n;

	c->read_char_cnt = c->read_char_max;
	c->write_char_cnt = c->write_char_max;

	e8530_check_rxd (scc, chn);
	e8530_check_txd (scc, chn);
}

void e8530_clock (e8530_t *scc, unsigned n)
{
	e8530_chn_clock (scc, 0, n);
	e8530_chn_clock (scc, 1, n);
}
