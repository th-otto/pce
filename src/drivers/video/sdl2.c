/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/sdl2.c                                     *
 * Created:     2015-06-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2015-2019 Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include <drivers/video/terminal.h>
#include <drivers/video/keys.h>
#include <drivers/video/sdl2.h>
#include <lib/log.h>


static sdl2_keymap_t keymap[] = {
	{ SDL_SCANCODE_ESCAPE,       PCE_KEY_ESC },
	{ SDL_SCANCODE_F1,           PCE_KEY_F1 },
	{ SDL_SCANCODE_F2,           PCE_KEY_F2 },
	{ SDL_SCANCODE_F3,           PCE_KEY_F3 },
	{ SDL_SCANCODE_F4,           PCE_KEY_F4 },
	{ SDL_SCANCODE_F5,           PCE_KEY_F5 },
	{ SDL_SCANCODE_F6,           PCE_KEY_F6 },
	{ SDL_SCANCODE_F7,           PCE_KEY_F7 },
	{ SDL_SCANCODE_F8,           PCE_KEY_F8 },
	{ SDL_SCANCODE_F9,           PCE_KEY_F9 },
	{ SDL_SCANCODE_F10,          PCE_KEY_F10 },
	{ SDL_SCANCODE_F11,          PCE_KEY_F11 },
	{ SDL_SCANCODE_F12,          PCE_KEY_F12 },

	{ SDL_SCANCODE_PRINTSCREEN,  PCE_KEY_PRTSCN },
	{ SDL_SCANCODE_SCROLLLOCK,   PCE_KEY_SCRLK },
	{ SDL_SCANCODE_PAUSE,        PCE_KEY_PAUSE },

	{ SDL_SCANCODE_GRAVE,        PCE_KEY_BACKQUOTE },
	{ SDL_SCANCODE_1,            PCE_KEY_1 },
	{ SDL_SCANCODE_2,            PCE_KEY_2 },
	{ SDL_SCANCODE_3,            PCE_KEY_3 },
	{ SDL_SCANCODE_4,            PCE_KEY_4 },
	{ SDL_SCANCODE_5,            PCE_KEY_5 },
	{ SDL_SCANCODE_6,            PCE_KEY_6 },
	{ SDL_SCANCODE_7,            PCE_KEY_7 },
	{ SDL_SCANCODE_8,            PCE_KEY_8 },
	{ SDL_SCANCODE_9,            PCE_KEY_9 },
	{ SDL_SCANCODE_0,            PCE_KEY_0 },
	{ SDL_SCANCODE_MINUS,        PCE_KEY_MINUS },
	{ SDL_SCANCODE_EQUALS,       PCE_KEY_EQUAL },
	{ SDL_SCANCODE_BACKSPACE,    PCE_KEY_BACKSPACE },

	{ SDL_SCANCODE_TAB,          PCE_KEY_TAB },
	{ SDL_SCANCODE_Q,            PCE_KEY_Q },
	{ SDL_SCANCODE_W,            PCE_KEY_W },
	{ SDL_SCANCODE_E,            PCE_KEY_E },
	{ SDL_SCANCODE_R,            PCE_KEY_R },
	{ SDL_SCANCODE_T,            PCE_KEY_T },
	{ SDL_SCANCODE_Y,            PCE_KEY_Y },
	{ SDL_SCANCODE_U,            PCE_KEY_U },
	{ SDL_SCANCODE_I,            PCE_KEY_I },
	{ SDL_SCANCODE_O,            PCE_KEY_O },
	{ SDL_SCANCODE_P,            PCE_KEY_P },
	{ SDL_SCANCODE_LEFTBRACKET,  PCE_KEY_LBRACKET },
	{ SDL_SCANCODE_RIGHTBRACKET, PCE_KEY_RBRACKET },
	{ SDL_SCANCODE_RETURN,       PCE_KEY_RETURN },

	{ SDL_SCANCODE_CAPSLOCK,     PCE_KEY_CAPSLOCK },
	{ SDL_SCANCODE_A,            PCE_KEY_A },
	{ SDL_SCANCODE_S,            PCE_KEY_S },
	{ SDL_SCANCODE_D,            PCE_KEY_D },
	{ SDL_SCANCODE_F,            PCE_KEY_F },
	{ SDL_SCANCODE_G,            PCE_KEY_G },
	{ SDL_SCANCODE_H,            PCE_KEY_H },
	{ SDL_SCANCODE_J,            PCE_KEY_J },
	{ SDL_SCANCODE_K,            PCE_KEY_K },
	{ SDL_SCANCODE_L,            PCE_KEY_L },
	{ SDL_SCANCODE_SEMICOLON,    PCE_KEY_SEMICOLON },
	{ SDL_SCANCODE_APOSTROPHE,   PCE_KEY_QUOTE },
	{ SDL_SCANCODE_BACKSLASH,    PCE_KEY_BACKSLASH },

	{ SDL_SCANCODE_LSHIFT,       PCE_KEY_LSHIFT },
	{ SDL_SCANCODE_NONUSBACKSLASH, PCE_KEY_BACKSLASH },
	{ SDL_SCANCODE_Z,            PCE_KEY_Z },
	{ SDL_SCANCODE_X,            PCE_KEY_X },
	{ SDL_SCANCODE_C,            PCE_KEY_C },
	{ SDL_SCANCODE_V,            PCE_KEY_V },
	{ SDL_SCANCODE_B,            PCE_KEY_B },
	{ SDL_SCANCODE_N,            PCE_KEY_N },
	{ SDL_SCANCODE_M,            PCE_KEY_M },
	{ SDL_SCANCODE_COMMA,        PCE_KEY_COMMA },
	{ SDL_SCANCODE_PERIOD,       PCE_KEY_PERIOD },
	{ SDL_SCANCODE_SLASH,        PCE_KEY_SLASH },
	{ SDL_SCANCODE_RSHIFT,       PCE_KEY_RSHIFT },

	{ SDL_SCANCODE_LCTRL,        PCE_KEY_LCTRL },
	{ SDL_SCANCODE_LGUI,         PCE_KEY_LSUPER },
	{ SDL_SCANCODE_LALT,         PCE_KEY_LALT },
	{ SDL_SCANCODE_MODE,         PCE_KEY_MODE },
	{ SDL_SCANCODE_SPACE,        PCE_KEY_SPACE },
	{ SDL_SCANCODE_RALT,         PCE_KEY_RALT },
	{ SDL_SCANCODE_RGUI,         PCE_KEY_RSUPER },
	{ SDL_SCANCODE_MENU,         PCE_KEY_MENU },
	{ SDL_SCANCODE_RCTRL,        PCE_KEY_RCTRL },

	{ SDL_SCANCODE_NUMLOCKCLEAR, PCE_KEY_NUMLOCK },
	{ SDL_SCANCODE_KP_DIVIDE,    PCE_KEY_KP_SLASH },
	{ SDL_SCANCODE_KP_MULTIPLY,  PCE_KEY_KP_STAR },
	{ SDL_SCANCODE_KP_MINUS,     PCE_KEY_KP_MINUS },
	{ SDL_SCANCODE_KP_7,         PCE_KEY_KP_7 },
	{ SDL_SCANCODE_KP_8,         PCE_KEY_KP_8 },
	{ SDL_SCANCODE_KP_9,         PCE_KEY_KP_9 },
	{ SDL_SCANCODE_KP_PLUS,      PCE_KEY_KP_PLUS },
	{ SDL_SCANCODE_KP_4,         PCE_KEY_KP_4 },
	{ SDL_SCANCODE_KP_5,         PCE_KEY_KP_5 },
	{ SDL_SCANCODE_KP_6,         PCE_KEY_KP_6 },
	{ SDL_SCANCODE_KP_1,         PCE_KEY_KP_1 },
	{ SDL_SCANCODE_KP_2,         PCE_KEY_KP_2 },
	{ SDL_SCANCODE_KP_3,         PCE_KEY_KP_3 },
	{ SDL_SCANCODE_KP_ENTER,     PCE_KEY_KP_ENTER },
	{ SDL_SCANCODE_KP_0,         PCE_KEY_KP_0 },
	{ SDL_SCANCODE_KP_PERIOD,    PCE_KEY_KP_PERIOD },
	{ SDL_SCANCODE_INSERT,       PCE_KEY_INS },
	{ SDL_SCANCODE_HOME,         PCE_KEY_HOME },
	{ SDL_SCANCODE_PAGEUP,       PCE_KEY_PAGEUP },
	{ SDL_SCANCODE_DELETE,       PCE_KEY_DEL },
	{ SDL_SCANCODE_END,          PCE_KEY_END },
	{ SDL_SCANCODE_PAGEDOWN,     PCE_KEY_PAGEDN },
	{ SDL_SCANCODE_UP,           PCE_KEY_UP },
	{ SDL_SCANCODE_LEFT,         PCE_KEY_LEFT },
	{ SDL_SCANCODE_DOWN,         PCE_KEY_DOWN },
	{ SDL_SCANCODE_RIGHT,        PCE_KEY_RIGHT },
	{ 0,                         PCE_KEY_NONE }
};


static
void sdl2_set_keymap (sdl2_t *sdl, SDL_Scancode src, pce_key_t dst)
{
	unsigned      i;
	sdl2_keymap_t *tmp;

	for (i = 0; i < sdl->keymap_cnt; i++) {
		if (sdl->keymap[i].sdlkey == src) {
			sdl->keymap[i].pcekey = dst;
			return;
		}
	}

	tmp = realloc (sdl->keymap, (sdl->keymap_cnt + 1) * sizeof (sdl2_keymap_t));

	if (tmp == NULL) {
		return;
	}

	tmp[sdl->keymap_cnt].sdlkey = src;
	tmp[sdl->keymap_cnt].pcekey = dst;

	sdl->keymap = tmp;
	sdl->keymap_cnt += 1;
}

static
void sdl2_init_keymap_default (sdl2_t *sdl)
{
	unsigned i, n;

	sdl->keymap_cnt = 0;
	sdl->keymap = NULL;

	n = 0;
	while (keymap[n].pcekey != PCE_KEY_NONE) {
		n += 1;
	}

	sdl->keymap = malloc (n * sizeof (sdl2_keymap_t));

	if (sdl->keymap == NULL) {
		return;
	}

	for (i = 0; i < n; i++) {
		sdl->keymap[i] = keymap[i];
	}

	sdl->keymap_cnt = n;
}

static
void sdl2_init_keymap_user (sdl2_t *sdl, ini_sct_t *sct)
{
	const char    *str;
	ini_val_t     *val;
	unsigned long sdlkey;
	pce_key_t     pcekey;

	val = NULL;

	while (1) {
		val = ini_next_val (sct, val, "keymap");

		if (val == NULL) {
			break;
		}

		str = ini_val_get_str (val);

		if (str == NULL) {
			continue;
		}

		if (pce_key_get_map (str, &sdlkey, &pcekey)) {
			continue;
		}

		sdl2_set_keymap (sdl, (SDL_Scancode) sdlkey, pcekey);
	}
}

static
void sdl2_grab_mouse (void *ext, int grab)
{
	sdl2_t *sdl = (sdl2_t *)ext;

	sdl->grab = (grab != 0);

	if (sdl->window != NULL) {
		SDL_SetWindowGrab (sdl->window, sdl->grab ? SDL_TRUE : SDL_FALSE);
		SDL_SetRelativeMouseMode (sdl->grab ? SDL_TRUE : SDL_FALSE);
	}
}

static
void sdl2_set_fullscreen (sdl2_t *sdl, int val)
{
	if ((val != 0) == (sdl->fullscreen != 0)) {
		return;
	}

	sdl->fullscreen = (val != 0);

	if (sdl->window != NULL) {
		SDL_SetWindowFullscreen (sdl->window, val ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	}
}

static
int sdl2_set_window_size (sdl2_t *sdl, unsigned w, unsigned h)
{
	if ((w == 0) || (h == 0)) {
		return (1);
	}

	if ((sdl->wdw_w == w) && (sdl->wdw_h == h)) {
		return (0);
	}

	SDL_SetWindowSize (sdl->window, w, h);

	sdl->wdw_w = w;
	sdl->wdw_h = h;

	return (0);
}

static
int sdl2_set_window_size_auto (sdl2_t *sdl)
{
	unsigned fx, fy, tw, th, ww, wh;

	tw = sdl->trm.w;
	th = sdl->trm.h;

	trm_get_scale (&sdl->trm, tw, th, &fx, &fy);

	ww = fx * tw;
	wh = fy * th;

	if (sdl2_set_window_size (sdl, ww, wh)) {
		return (1);
	}

	return (0);
}

static
int sdl2_set_frame_size (sdl2_t *sdl)
{
	unsigned tw, th;

	tw = sdl->trm.w;
	th = sdl->trm.h;

	if (sdl->autosize) {
		sdl2_set_window_size_auto (sdl);
	}

	if ((sdl->txt_w == tw) && (sdl->txt_h == th)) {
		return (0);
	}

	if (sdl->texture != NULL) {
		SDL_DestroyTexture (sdl->texture);
		sdl->texture = NULL;
	}

	sdl->texture = SDL_CreateTexture (sdl->render,
		SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, tw, th
	);

	if (sdl->texture == NULL) {
		fprintf (stderr, "sdl2: texture\n");
		return (1);
	}

	sdl->txt_w = tw;
	sdl->txt_h = th;

	return (0);
}

static
unsigned sdl2_map_key (sdl2_t *sdl, SDL_Scancode key)
{
	unsigned i;

	for (i = 0; i < sdl->keymap_cnt; i++) {
		if (sdl->keymap[i].sdlkey == key) {
			return (sdl->keymap[i].pcekey);
		}
	}

	return (PCE_KEY_NONE);
}

static
void sdl2_update (void *ext)
{
	sdl2_t *sdl = (sdl2_t *)ext;
	terminal_t *trm;
	void       *pixels;
	int        pitch;

	trm = &sdl->trm;

	sdl->update = 0;

	if ((trm->w == 0) || (trm->h == 0)) {
		return;
	}

	if (sdl2_set_frame_size (sdl)) {
		return;
	}

	if ((sdl->texture == NULL) || (sdl->render == NULL)) {
		return;
	}

	SDL_LockTexture (sdl->texture, NULL, &pixels, &pitch);
	memcpy (pixels, trm->buf, trm->term_bpp * trm->w * trm->h);
	SDL_UnlockTexture (sdl->texture);

	SDL_RenderCopy (sdl->render, sdl->texture, NULL, NULL);
	SDL_RenderPresent (sdl->render);
}


static unsigned int sdl2_map_sym(SDL_Keysym *keysym)
{
	unsigned int scancode = 0;

	switch ((unsigned int) keysym->scancode)
	{
		case SDL_SCANCODE_ESCAPE: scancode = 0x01; break;
		case SDL_SCANCODE_1: scancode = 0x02; break;
		case SDL_SCANCODE_2: scancode = 0x03; break;
		case SDL_SCANCODE_3: scancode = 0x04; break;
		case SDL_SCANCODE_4: scancode = 0x05; break;
		case SDL_SCANCODE_5: scancode = 0x06; break;
		case SDL_SCANCODE_6: scancode = 0x07; break;
		case SDL_SCANCODE_7: scancode = 0x08; break;
		case SDL_SCANCODE_8: scancode = 0x09; break;
		case SDL_SCANCODE_9: scancode = 0x0a; break;
		case SDL_SCANCODE_0: scancode = 0x0b; break;
		case SDL_SCANCODE_MINUS: scancode = 0x0c; break;
		case SDL_SCANCODE_EQUALS: scancode = 0x0d; break;
		case SDL_SCANCODE_BACKSPACE: scancode = 0x0e; break;
		case SDL_SCANCODE_TAB: scancode = 0x0f; break;
		case SDL_SCANCODE_Q: scancode = 0x10; break;
		case SDL_SCANCODE_W: scancode = 0x11; break;
		case SDL_SCANCODE_E: scancode = 0x12; break;
		case SDL_SCANCODE_R: scancode = 0x13; break;
		case SDL_SCANCODE_T: scancode = 0x14; break;
		case SDL_SCANCODE_Y: scancode = 0x15; break;
		case SDL_SCANCODE_U: scancode = 0x16; break;
		case SDL_SCANCODE_I: scancode = 0x17; break;
		case SDL_SCANCODE_O: scancode = 0x18; break;
		case SDL_SCANCODE_P: scancode = 0x19; break;
		case SDL_SCANCODE_LEFTBRACKET: scancode = 0x1a; break;
		case SDL_SCANCODE_RIGHTBRACKET: scancode = 0x1b; break;
		case SDL_SCANCODE_RETURN: scancode = 0x1c; break;
		case SDL_SCANCODE_LCTRL: scancode = 0x1d; break;
		case SDL_SCANCODE_A: scancode = 0x1e; break;
		case SDL_SCANCODE_S: scancode = 0x1f; break;
		case SDL_SCANCODE_D: scancode = 0x20; break;
		case SDL_SCANCODE_F: scancode = 0x21; break;
		case SDL_SCANCODE_G: scancode = 0x22; break;
		case SDL_SCANCODE_H: scancode = 0x23; break;
		case SDL_SCANCODE_J: scancode = 0x24; break;
		case SDL_SCANCODE_K: scancode = 0x25; break;
		case SDL_SCANCODE_L: scancode = 0x26; break;
		case SDL_SCANCODE_SEMICOLON: scancode = 0x27; break;
		case SDL_SCANCODE_APOSTROPHE: scancode = 0x28; break;
		case SDL_SCANCODE_GRAVE: scancode = 0x29; break;
		case SDL_SCANCODE_LSHIFT: scancode = 0x2a; break;
		case SDL_SCANCODE_BACKSLASH: scancode = 0x2b; break;
		case SDL_SCANCODE_Z: scancode = 0x2c; break;
		case SDL_SCANCODE_X: scancode = 0x2d; break;
		case SDL_SCANCODE_C: scancode = 0x2e; break;
		case SDL_SCANCODE_V: scancode = 0x2f; break;
		case SDL_SCANCODE_B: scancode = 0x30; break;
		case SDL_SCANCODE_N: scancode = 0x31; break;
		case SDL_SCANCODE_M: scancode = 0x32; break;
		case SDL_SCANCODE_COMMA: scancode = 0x33; break;
		case SDL_SCANCODE_PERIOD: scancode = 0x34; break;
		case SDL_SCANCODE_SLASH: scancode = 0x35; break;
		case SDL_SCANCODE_RSHIFT: scancode = 0x36; break;
		case SDL_SCANCODE_PRINTSCREEN: scancode = 0x37; break;
		case SDL_SCANCODE_LALT: scancode = 0x38; break;
		case SDL_SCANCODE_SPACE: scancode = 0x39; break;
		case SDL_SCANCODE_CAPSLOCK: scancode = 0x3a; break;
		case SDL_SCANCODE_F1: scancode = 0x3b; break;
		case SDL_SCANCODE_F2: scancode = 0x3c; break;
		case SDL_SCANCODE_F3: scancode = 0x3d; break;
		case SDL_SCANCODE_F4: scancode = 0x3e; break;
		case SDL_SCANCODE_F5: scancode = 0x3f; break;
		case SDL_SCANCODE_F6: scancode = 0x40; break;
		case SDL_SCANCODE_F7: scancode = 0x41; break;
		case SDL_SCANCODE_F8: scancode = 0x42; break;
		case SDL_SCANCODE_F9: scancode = 0x43; break;
		case SDL_SCANCODE_F10: scancode = 0x44; break;

		case SDL_SCANCODE_NONUSBACKSLASH: scancode = 0x60; break;
		case SDL_SCANCODE_KP_LEFTPAREN: scancode = 0x63; break;
		case SDL_SCANCODE_KP_RIGHTPAREN: scancode = 0x64; break;
		
		case SDL_SCANCODE_SCROLLLOCK: scancode = 0x00; break;
		case SDL_SCANCODE_PAUSE: scancode = 0x00; break;
	}
	if (scancode != 0)
	{
		keysym->scancode = scancode;
		return scancode;
	}

	switch ((unsigned int) keysym->sym)
	{
		/* Numeric Pad */
		case SDLK_KP_DIVIDE: scancode = 0x65; break;	/* Numpad / */
		case SDLK_KP_MULTIPLY: scancode = 0x66; break;	/* NumPad * */
		case SDLK_KP_7: scancode = 0x67; break;	/* NumPad 7 */
		case SDLK_KP_8: scancode = 0x68; break;	/* NumPad 8 */
		case SDLK_KP_9: scancode = 0x69; break;	/* NumPad 9 */
		case SDLK_KP_4: scancode = 0x6a; break;	/* NumPad 4 */
		case SDLK_KP_5: scancode = 0x6b; break;	/* NumPad 5 */
		case SDLK_KP_6: scancode = 0x6c; break;	/* NumPad 6 */
		case SDLK_KP_1: scancode = 0x6d; break;	/* NumPad 1 */
		case SDLK_KP_2: scancode = 0x6e; break;	/* NumPad 2 */
		case SDLK_KP_3: scancode = 0x6f; break;	/* NumPad 3 */
		case SDLK_KP_0: scancode = 0x70; break;	/* NumPad 0 */
		case SDLK_KP_PERIOD: scancode = 0x71; break;	/* NumPad . */
		case SDLK_KP_ENTER: scancode = 0x72; break;	/* NumPad Enter */
		case SDLK_KP_MINUS: scancode = 0x4a; break;	/* NumPad - */
		case SDLK_KP_PLUS: scancode = 0x4e; break;	/* NumPad + */

		/* Special Keys */
		case SDLK_F11: scancode = 0x62; break;	/* F11 => Help */
		case SDLK_F12: scancode = 0x61; break;	/* F12 => Undo */
		case SDLK_HOME: scancode = 0x47; break;	/* Home */
		case SDLK_UP: scancode = 0x48; break;	/* Arrow Up */
		case SDLK_PAGEUP: scancode = 0x49; break;	/* Page Up */
		case SDLK_LEFT: scancode = 0x4b; break;	/* Arrow Left */
		case SDLK_RIGHT: scancode = 0x4d; break;	/* Arrow Right */
		case SDLK_END: scancode = 0x4f; break;	/* Milan's scancode for End */
		case SDLK_DOWN: scancode = 0x50; break;	/* Arrow Down */
		case SDLK_PAGEDOWN: scancode = 0x51; break;	/* Page Down */
		case SDLK_INSERT: scancode = 0x52; break;	/* Insert */
		case SDLK_DELETE: scancode = 0x53; break;	/* Delete */

		case SDLK_NUMLOCKCLEAR: scancode = 0x63; break;
		
		case SDLK_BACKQUOTE:
		case SDLK_LESS: scancode = 0x60; break;	/* a '<>' key next to short left Shift */

		/* keys not found on some keyboards */
		case SDLK_RCTRL: scancode = 0x1d; break;
		case SDLK_MODE: /* passthru */ /* Alt Gr key according to SDL docs */
		case SDLK_RALT: scancode = 0x4c; break;
	}

	keysym->scancode = scancode;
	return scancode;
}


static
void sdl2_event_keydown (sdl2_t *sdl, SDL_Keysym *sym)
{
	pce_key_t pcekey;

	if (sdl->ignore_keys) {
		return;
	}

	if (sym->scancode == SDL_SCANCODE_GRAVE) {
		if (sdl->grave_down) {
			return;
		}
		else if (sym->mod & KMOD_LCTRL) {
			sdl->grave_down = 1;
			sdl2_grab_mouse (sdl, 0);
			sdl2_set_fullscreen (sdl, 0);
			trm_set_msg_emu (&sdl->trm, "emu.stop", "1");
			return;
		}
	}
	else if (sym->scancode == SDL_SCANCODE_PRINTSCREEN) {
		trm_screenshot (&sdl->trm, NULL);
		return;
	}

	sdl2_map_sym(sym);

	pcekey = sdl2_map_key (sdl, sym->scancode);

	if (sdl->report_keys || (pcekey == PCE_KEY_NONE)) {
		const char *keyname = pce_key_to_string(pcekey);
		fprintf (stderr, "sdl: scancode=0x%x, key=0x%04x, unicode=0x%04x -> %d (%s)\n", sym->scancode, sym->sym, sym->unused, pcekey, keyname ? keyname : "<unknown>");
	}

	if (pcekey == PCE_KEY_NONE) {
		return;
	}

	trm_set_key (&sdl->trm, PCE_KEY_EVENT_DOWN, pcekey, sym->scancode);

	if (sym->scancode == SDL_SCANCODE_NUMLOCKCLEAR) {
		trm_set_key (&sdl->trm, PCE_KEY_EVENT_UP, pcekey, sym->scancode);
	}
}

static
void sdl2_event_keyup (sdl2_t *sdl, SDL_Keysym *sym)
{
	pce_key_t pcekey;

	if (sdl->ignore_keys) {
		return;
	}

	sdl2_map_sym(sym);

	pcekey = sdl2_map_key (sdl, sym->scancode);

	if (sym->scancode == SDL_SCANCODE_GRAVE) {
		if (sdl->grave_down) {
			sdl->grave_down = 0;
			return;
		}
	}
	else if (sym->scancode == SDL_SCANCODE_PRINTSCREEN) {
		return;
	}

	if (pcekey != PCE_KEY_NONE) {
		if (sym->scancode == SDL_SCANCODE_NUMLOCKCLEAR) {
			trm_set_key (&sdl->trm, PCE_KEY_EVENT_DOWN, pcekey, sym->scancode);
		}

		trm_set_key (&sdl->trm, PCE_KEY_EVENT_UP, pcekey, sym->scancode);
	}
}

static
void sdl2_event_mouse_button (sdl2_t *sdl, int down, unsigned button)
{
	if (button == 0) {
		return;
	}

	if (button == 2) {
		button = 3;
	}
	else if (button == 3) {
		button = 2;
	}

	button -= 1;

	if (down) {
		sdl->button |= 1U << button;
	}
	else {
		sdl->button &= ~(1U << button);
	}

	if (sdl->grab == 0) {
		if (down == 0) {
			sdl2_grab_mouse (sdl, 1);
		}
		return;
	}

	if (sdl->trm.set_mouse == NULL) {
		return;
	}

	trm_set_mouse (&sdl->trm, 0, 0, sdl->button);
}

static
void sdl2_event_mouse_motion (sdl2_t *sdl, int dx, int dy)
{
	unsigned but, val;

	if (sdl->grab == 0) {
		return;
	}

	if (sdl->trm.set_mouse == NULL) {
		return;
	}

	val = 0;
	but = SDL_GetMouseState (NULL, NULL);

	if (but & SDL_BUTTON (1)) {
		val |= 1;
	}

	if (but & SDL_BUTTON (3)) {
		val |= 2;
	}

	trm_set_mouse (&sdl->trm, dx, dy, val);
}

static
void sdl2_event_window (sdl2_t *sdl, SDL_WindowEvent *evt)
{
	if (sdl->window == NULL) {
		return;
	}

	if (evt->windowID != SDL_GetWindowID (sdl->window)) {
		return;
	}

	switch (evt->event) {
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		sdl->update = 1;
		break;

	case SDL_WINDOWEVENT_RESIZED:
		sdl->wdw_w = evt->data1;
		sdl->wdw_h = evt->data2;
		sdl->autosize = 0;
		sdl->update = 1;
		break;

	case SDL_WINDOWEVENT_RESTORED:
		sdl->update = 1;
		break;

	case SDL_WINDOWEVENT_EXPOSED:
		sdl->update = 1;
		break;

	case SDL_WINDOWEVENT_MOVED:
		break;

	case SDL_WINDOWEVENT_ENTER:
		break;

	case SDL_WINDOWEVENT_LEAVE:
		break;

	case SDL_WINDOWEVENT_FOCUS_GAINED:
		sdl->ignore_keys = 1;
		break;

	case SDL_WINDOWEVENT_FOCUS_LOST:
		break;

	case SDL_WINDOWEVENT_SHOWN:
		sdl->update = 1;
		break;

	case SDL_WINDOWEVENT_HIDDEN:
		break;

	case SDL_WINDOWEVENT_MINIMIZED:
		break;

	case SDL_WINDOWEVENT_MAXIMIZED:
		sdl->update = 1;
		break;

	case SDL_WINDOWEVENT_TAKE_FOCUS:
		break;

	default:
		fprintf (stderr, "sdl2: window event %u\n", evt->event);
		break;
	}
}

static
int sdl2_check (void *ext)
{
	sdl2_t *sdl = (sdl2_t *)ext;
	unsigned int i;
	SDL_Event evt;

	i = 0;
	while (SDL_PollEvent (&evt)) {
		switch (evt.type) {
		case SDL_KEYDOWN:
			sdl2_event_keydown (sdl, &evt.key.keysym);
			break;

		case SDL_KEYUP:
			sdl2_event_keyup (sdl, &evt.key.keysym);
			break;

		case SDL_TEXTINPUT:
			break;

		case SDL_MOUSEBUTTONDOWN:
			sdl2_event_mouse_button (sdl, 1, evt.button.button);
			break;

		case SDL_MOUSEBUTTONUP:
			sdl2_event_mouse_button (sdl, 0, evt.button.button);
			break;

		case SDL_MOUSEMOTION:
			sdl2_event_mouse_motion (sdl, evt.motion.xrel, evt.motion.yrel);
			break;

		case SDL_WINDOWEVENT:
			sdl2_event_window (sdl, &evt.window);
			break;

		case SDL_QUIT:
			sdl2_grab_mouse (sdl, 0);
			trm_set_msg_emu (&sdl->trm, "emu.exit", "1");
			break;

		case SDL_AUDIODEVICEADDED:
			break;

		default:
			fprintf (stderr, "sdl2: event %u\n", evt.type);
			break;
		}

		i += 1;
	}

	if (sdl->update) {
		sdl2_update (sdl);
	}

	if (sdl->ignore_keys) {
		sdl->ignore_keys = 0;
	}
	return i > 0;
}

static
int sdl2_set_msg_trm (void *ext, const char *msg, const char *val)
{
	sdl2_t *sdl = (sdl2_t *)ext;

	if (val == NULL) {
		val = "";
	}

	if (strcmp (msg, "term.grab") == 0) {
		sdl2_grab_mouse (sdl, 1);
		return (0);
	}
	else if (strcmp (msg, "term.release") == 0) {
		sdl2_grab_mouse (sdl, 0);
		return (0);
	}
	else if (strcmp (msg, "term.title") == 0) {
#ifndef __EMSCRIPTEN__
		if (sdl->window != NULL) {
			SDL_SetWindowTitle (sdl->window, val);
		}
#endif
		return (0);
	}
	else if (strcmp (msg, "term.fullscreen.toggle") == 0) {
		sdl2_set_fullscreen (sdl, !sdl->fullscreen);
		return (0);
	}
	else if (strcmp (msg, "term.fullscreen") == 0) {
		int v;

		v = strtol (val, NULL, 0);

		sdl2_set_fullscreen (sdl, v != 0);

		return (0);
	}
	else if (strcmp (msg, "term.autosize") == 0) {
		sdl->autosize = 1;
		sdl->update = 1;
		return (0);
	}

	return (-1);
}

static
void sdl2_del (void *sdl)
{
	free (sdl);
}

static
int sdl2_open (void *ext, unsigned w, unsigned h)
{
	sdl2_t *sdl = (sdl2_t *)ext;
	unsigned x, y;
	unsigned fx, fy;
	unsigned flags;

	trm_get_scale (&sdl->trm, w, h, &fx, &fy);

	w *= fx;
	h *= fy;

	if ((w == 0) || (h == 0)) {
		w = 512;
		h = 384;
	}

	if (SDL_WasInit (SDL_INIT_VIDEO) == 0) {
		if (SDL_InitSubSystem (SDL_INIT_VIDEO) < 0) {
			return (1);
		}
	}

	sdl->window = NULL;
	sdl->render = NULL;
	sdl->texture = NULL;

	SDL_EventState (SDL_MOUSEMOTION, SDL_ENABLE);

	SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_SetHint (SDL_HINT_GRAB_KEYBOARD, "1");
	SDL_SetHint (SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");

	flags = SDL_WINDOW_RESIZABLE;

	if (sdl->fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	x = SDL_WINDOWPOS_UNDEFINED;
	y = SDL_WINDOWPOS_UNDEFINED;

	sdl->window = SDL_CreateWindow ("pce", x, y, w, h, flags);

	if (sdl->window == NULL) {
		fprintf (stderr, "sdl2: window\n");
		return (1);
	}

	SDL_ShowWindow (sdl->window);

	sdl->wdw_w = w;
	sdl->wdw_h = h;

	sdl->render = SDL_CreateRenderer (sdl->window, -1, 0);

	if (sdl->render == NULL) {
		fprintf (stderr, "sdl2: renderer\n");
		return (1);
	}

	return (0);
}

static
int sdl2_close (void *ext)
{
	sdl2_t *sdl = (sdl2_t *)ext;

	sdl2_grab_mouse (sdl, 0);

	if (sdl->texture != NULL) {
		SDL_DestroyTexture (sdl->texture);
		sdl->texture = NULL;
	}

	if (sdl->render != NULL) {
		SDL_DestroyRenderer (sdl->render);
		sdl->render = NULL;
	}

	if (sdl->window != NULL) {
		SDL_DestroyWindow (sdl->window);
		sdl->window = NULL;
	}

	return (0);
}

static
void sdl2_init (sdl2_t *sdl, ini_sct_t *sct)
{
	int fs, rep;

	trm_init (&sdl->trm, sdl);

	sdl->trm.del = sdl2_del;
	sdl->trm.open = sdl2_open;
	sdl->trm.close = sdl2_close;
	sdl->trm.set_msg_trm = sdl2_set_msg_trm;
	sdl->trm.update = sdl2_update;
	sdl->trm.check = sdl2_check;
	sdl->trm.grab = sdl2_grab_mouse;

	sdl->window = NULL;
	sdl->render = NULL;
	sdl->texture = NULL;

	sdl->txt_w = 0;
	sdl->txt_h = 0;

	sdl->wdw_w = 0;
	sdl->wdw_h = 0;

	sdl->update = 0;

	sdl->button = 0;

	ini_get_bool (sct, "fullscreen", &fs, 0);
	sdl->fullscreen = (fs != 0);

	sdl->grab = 0;

	ini_get_bool (sct, "report_keys", &rep, 0);
	sdl->report_keys = (rep != 0);

	sdl->autosize = 1;

	sdl->grave_down = 0;
	sdl->ignore_keys = 0;

	sdl2_init_keymap_default (sdl);
	sdl2_init_keymap_user (sdl, sct);

	SDL_SetHint (SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
}

terminal_t *sdl2_new (ini_sct_t *sct)
{
	sdl2_t *sdl;

	if ((sdl = malloc (sizeof (sdl2_t))) == NULL) {
		return (NULL);
	}

	sdl2_init (sdl, sct);

	return (&sdl->trm);
}
