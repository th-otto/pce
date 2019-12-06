/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/mem.c                                       *
 * Created:     2011-03-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2018 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "atarist.h"
#include "dma.h"
#include "mem.h"
#include "psg.h"

#include <chipset/e6850.h>
#include <drivers/char/char.h>


static int st_mem_check_ram_addr(atari_st_t *sim, uint32_t addr)
{
	if (addr >= sim->ram->size && sim->cpu->generate_buserrs)
	{
		switch (sim->memcfg)
		{
		case 0x00: /* both banks 128K */
			if (addr < 0x40000) return 1;
			break;
		case 0x01: /* bank 0 empty, bank 1 512K */
			if (addr < 0x80000) return 1;
			break;
		case 0x02: /* bank 0 empty, bank 1 2MB */
			if (addr < 0x200000) return 1;
			break;
		case 0x03: /* bank 0 empty, bank 1 8MB */
			if (addr < 0x800000) return 1;
			break;
		case 0x04: /* bank 0 512K, bank 1 empty */
			if (addr < 0x80000) return 1;
			break;
		case 0x05: /* bank 0 512K, bank 1 512k */
			if (addr < 0x100000) return 1;
			break;
		case 0x06: /* bank 0 512K, bank 1 2MB */
			if (addr < 0x280000) return 1;
			break;
		case 0x07: /* bank 0 512K, bank 1 8MB */
			if (addr < 0x880000) return 1;
			break;
		case 0x08: /* bank 0 2MB, bank 1 empty */
			if (addr < 0x200000) return 1;
			break;
		case 0x09: /* bank 0 2MB, bank 1 512K */
			if (addr < 0x280000) return 1;
			break;
		case 0x0a: /* bank 0 2MB, bank 1 2MB */
			if (addr < 0x400000) return 1;
			break;
		case 0x0b: /* bank 0 2MB, bank 1 8MB */
			if (addr < 0xa00000) return 1;
			break;
		case 0x0c: /* bank 0 8MB, bank 1 empty */
			if (addr < 0x800000) return 1;
			break;
		case 0x0d: /* bank 0 8MB, bank 1 512K */
			if (addr < 0x880000) return 1;
			break;
		case 0x0e: /* bank 0 8MB, bank 1 2MB */
			if (addr < 0xa00000) return 1;
			break;
		case 0x0f: /* bank 0 8MB, bank 1 8MB */
			return 1;
		}
		e68_set_bus_error (sim->cpu, addr, 1, 0);
		return 0;
	}
	return 1;
}

unsigned char st_mem_get_uint8 (void *ext, unsigned long addr)
{
	atari_st_t *sim = ext;

	if (addr < 0xf00000)
	{
		if (st_mem_check_ram_addr(sim, addr))
		{
			if (addr < sim->ram->size)
				return sim->ram->data[addr];
			return 0;
		}
		return 0xff;
	}

	if ((addr >= 0xff8900) && (addr < 0xff8940)) {
		/* DMA sound */

		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, addr, 1, 0);
		}

		return (0);
	}

	if ((addr >= 0xff8a00) && (addr < 0xff8a40)) {
		/* blitter */
		e68_set_bus_error (sim->cpu, addr, 1, 0);
		return (0);
	}

	if ((addr >= 0xfffc20) && (addr < 0xfffc40)) {
		if (sim->model & (PCE_ST_MEGA | PCE_ST_RTC)) {
			return (rp5c15_get_uint8 (&sim->rtc, (addr - 0xfffc20) / 2));
		}
		else {
			return (0);
		}
	}

	switch (addr) {
	case 0xff8001: /* memory configuration */
		return (sim->memcfg);

	case 0xff8609:
		return (st_dma_get_addr (&sim->dma, 0));

	case 0xff860b:
		return (st_dma_get_addr (&sim->dma, 1));

	case 0xff860d:
		return (st_dma_get_addr (&sim->dma, 2));

	case 0xff860e: /* FDC density */
	case 0xff860f: /* FDC density */
		return (0);

	case 0xff8800:
		/* YM2149 */
		return (st_psg_get_data (&sim->psg));

	case 0xff8802:
		/* YM2149 */
		return (st_psg_get_select (&sim->psg));

	case 0xfffc00:
		return (e6850_get_uint8 (&sim->acia0, 0));

	case 0xfffc02:
		return (e6850_get_uint8 (&sim->acia0, 1));

	case 0xfffc04:
		return (e6850_get_uint8 (&sim->acia1, 0));

	case 0xfffc06:
		return (e6850_get_uint8 (&sim->acia1, 1));

	case 0xff8e01: /* VME bus */
	case 0xff8e03:
	case 0xff8e05:
	case 0xff8e07:
	case 0xff8e09:
	case 0xff8e0b:
	case 0xff8e0d:
	case 0xff8e0f:
		if ((sim->model & (PCE_ST_MEGA|PCE_ST_TT)) == 0) {
			e68_set_bus_error (sim->cpu, addr, 1, 0);
		}
		return 0;

	case 0xff9000: /* No bus error here */
	case 0xff9001: /* No bus error here */
	case 0xff9200: /* Joypad fire buttons + MegaSTE DIP Switches */
	case 0xff9201: /* Joypad fire buttons + MegaSTE DIP Switches */
	case 0xff9202: /* Joypad directions/buttons/selection */
	case 0xff9203: /* Joypad directions/buttons/selection */
	case 0xff9211: /* Joypad 0 X position (?) */
	case 0xff9213: /* Joypad 0 Y position (?) */
	case 0xff9215: /* Joypad 1 X position (?) */
	case 0xff9217: /* Joypad 1 Y position (?) */
	case 0xff9220: /* Lightpen X position */
	case 0xff9221: /* Lightpen X position */
	case 0xff9222: /* Lightpen Y position */
	case 0xff9223: /* Lightpen Y position */
		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, addr, 1, 0);
		}
		return 0;

	default:
		e68_set_bus_error (sim->cpu, addr, 1, 0);
		break;
	}

	return (0);
}

unsigned short st_mem_get_uint16 (void *ext, unsigned long addr)
{
	atari_st_t *sim = ext;

	if (addr < 0xf00000) {
		if (st_mem_check_ram_addr(sim, addr))
		{
			if (addr < sim->ram->size)
				return buf_get_uint16_be(sim->ram->data, addr);
			return 0;
		}
		return 0xffff;
	}

	if ((addr >= 0xff8900) && (addr < 0xff8940)) {
		/* dma sound */

		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, addr, 2, 0);
		}

		return (0);
	}

	if ((addr >= 0xff8a00) && (addr < 0xff8a40)) {
		/* blitter */
		e68_set_bus_error (sim->cpu, addr, 2, 0);
		return (0);
	}

	switch (addr) {
	case 0xfa0000: /* cartridge */
	case 0xfa0002:
		break;

	case 0xff8604:
		return (st_dma_get_disk (&sim->dma));

	case 0xff8606:
		return (st_dma_get_status (&sim->dma));

	case 0xff8608:
		return (st_dma_get_addr (&sim->dma, 0));

	case 0xff860a:
		return (st_dma_get_addr (&sim->dma, 1));

	case 0xff860c:
		return (st_dma_get_addr (&sim->dma, 2));

	case 0xff860e: /* FDC density */
		return (0);

	case 0xff9000: /* No bus error here */
	case 0xff9200: /* Joypad fire buttons + MegaSTE DIP Switches */
	case 0xff9202: /* Joypad directions/buttons/selection */
	case 0xff9220: /* Lightpen X position */
	case 0xff9222: /* Lightpen Y position */
		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, addr, 2, 0);
		}
		return 0;

	case 0xfffc00:
	case 0xfffc02:
	case 0xfffc04:
	case 0xfffc06:
		return (st_mem_get_uint8 (ext, addr) << 8);

	default:
		e68_set_bus_error (sim->cpu, addr, 2, 0);
		break;
	}

	return (0);
}

unsigned long st_mem_get_uint32 (void *ext, unsigned long addr)
{
	unsigned long val;
	atari_st_t    *sim = ext;

	val = mem_get_uint16_be (sim->mem, addr);
	val <<= 16;
	if (!sim->cpu->bus_error)
		val |= mem_get_uint16_be (sim->mem, addr + 2);

	return (val);
}

void st_mem_set_uint8 (void *ext, unsigned long addr, unsigned char val)
{
	atari_st_t *sim = ext;

	if (addr < 0xf00000) {
		if (st_mem_check_ram_addr(sim, addr))
		{
			if (addr < sim->ram->size)
				sim->ram->data[addr] = val;
		}
		return;
	}

	if ((addr >= 0xff8808) && (addr < 0xff8900)) {
		/* psg */
		e68_set_bus_error (sim->cpu, addr, 1, 1);
		return;
	}

	if ((addr >= 0xff8900) && (addr < 0xff8940)) {
		/* dma sound */

		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, addr, 1, 1);
		}

		return;
	}

	if ((addr >= 0xff8a00) && (addr < 0xff8a40)) {
		/* blitter */
		e68_set_bus_error (sim->cpu, addr, 1, 1);
		return;
	}

	if ((addr >= 0xfffc20) && (addr < 0xfffc40)) {
		if (sim->model & (PCE_ST_MEGA | PCE_ST_RTC))
		{
			rp5c15_set_uint8 (&sim->rtc, (addr - 0xfffc20) / 2, val);
		} else
		{
			e68_set_bus_error (sim->cpu, addr, 1, 1);
		}

		return;
	}

	switch (addr) {
	case 0xff8001: /* memory configuration */
		sim->memcfg = val;
		break;

	case 0xff8609:
		st_dma_set_addr (&sim->dma, 0, val);
		break;

	case 0xff860b:
		st_dma_set_addr (&sim->dma, 1, val);
		break;

	case 0xff860d:
		st_dma_set_addr (&sim->dma, 2, val);
		break;

	case 0xff860e: /* FDC density */
	case 0xff860f: /* FDC density */
		break;

	case 0xff8800:
	case 0xff8804:
		st_psg_set_select (&sim->psg, val);
		break;

	case 0xff8802:
	case 0xff8806:
		st_psg_set_data (&sim->psg, val);
		break;

	case 0xff8e01: /* VME bus */
	case 0xff8e03:
	case 0xff8e05:
	case 0xff8e07:
	case 0xff8e09:
	case 0xff8e0b:
	case 0xff8e0d:
	case 0xff8e0f:
		if ((sim->model & PCE_ST_MEGA) == 0) {
			e68_set_bus_error (sim->cpu, addr, 1, 1);
		}
		break;

	case 0xff9000: /* No bus error here */
	case 0xff9001: /* No bus error here */
	case 0xff9200: /* Joypad fire buttons + MegaSTE DIP Switches */
	case 0xff9201: /* Joypad fire buttons + MegaSTE DIP Switches */
	case 0xff9202: /* Joypad directions/buttons/selection */
	case 0xff9203: /* Joypad directions/buttons/selection */
	case 0xff9211: /* Joypad 0 X position (?) */
	case 0xff9213: /* Joypad 0 Y position (?) */
	case 0xff9215: /* Joypad 1 X position (?) */
	case 0xff9217: /* Joypad 1 Y position (?) */
	case 0xff9220: /* Lightpen X position */
	case 0xff9221: /* Lightpen X position */
	case 0xff9222: /* Lightpen Y position */
	case 0xff9223: /* Lightpen Y position */
		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, addr, 1, 1);
		}
		break;

	case 0xfffc00:
		e6850_set_uint8 (&sim->acia0, 0, val);
		break;

	case 0xfffc02:
		e6850_set_uint8 (&sim->acia0, 1, val);
		break;

	case 0xfffc04:
		e6850_set_uint8 (&sim->acia1, 0, val);
		break;

	case 0xfffc06:
		e6850_set_uint8 (&sim->acia1, 1, val);
		break;

	default:
		e68_set_bus_error (sim->cpu, addr, 1, 1);
		break;
	}
}

void st_mem_set_uint16 (void *ext, unsigned long addr, unsigned short val)
{
	atari_st_t *sim = ext;

	if (addr < 0xf00000) {
		if (st_mem_check_ram_addr(sim, addr))
		{
			if (addr < sim->ram->size)
				buf_set_uint16_be(sim->ram->data, addr, val);
		}
		return;
	}

	if ((addr >= 0xff8804) && (addr < 0xff8900)) {
		/* psg */
		e68_set_bus_error (sim->cpu, addr, 2, 1);
		return;
	}

	if ((addr >= 0xff8900) && (addr < 0xff8940)) {
		/* dma sound */

		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, addr, 2, 1);
		}

		return;
	}

	if ((addr >= 0xff8a00) && (addr < 0xff8a40)) {
		/* blitter */
		e68_set_bus_error (sim->cpu, addr, 2, 1);
		return;
	}

	switch (addr) {
	case 0xff8604:
		st_dma_set_disk (&sim->dma, val);
		break;

	case 0xff8606:
		st_dma_set_mode (&sim->dma, val);
		break;

	case 0xff860e: /* FDC density */
		break;

	case 0xff8800:
		st_psg_set_select (&sim->psg, val >> 8);
		break;

	case 0xff8802:
		st_psg_set_data (&sim->psg, val >> 8);
		break;

	case 0xff9000: /* No bus error here */
	case 0xff9200: /* Joypad fire buttons + MegaSTE DIP Switches */
	case 0xff9202: /* Joypad directions/buttons/selection */
	case 0xff9220: /* Lightpen X position */
	case 0xff9222: /* Lightpen Y position */
		if ((sim->model & PCE_ST_STE) == 0) {
			e68_set_bus_error (sim->cpu, addr, 2, 1);
		}
		break;

	case 0xfffc00:
	case 0xfffc02:
	case 0xfffc04:
	case 0xfffc06:
		st_mem_set_uint8 (ext, addr, val >> 8);
		break;

	default:
		e68_set_bus_error (sim->cpu, addr, 2, 1);
		break;
	}
}

void st_mem_set_uint32 (void *ext, unsigned long addr, unsigned long val)
{
	atari_st_t *sim = ext;

	mem_set_uint16_be (sim->mem, addr, val >> 16);
	if (!sim->cpu->bus_error)
		mem_set_uint16_be (sim->mem, addr + 2, val & 0xffff);
}
