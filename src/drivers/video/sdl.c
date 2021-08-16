/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/sdl.c                                      *
 * Created:     2003-09-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2012 Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include <drivers/video/terminal.h>
#include <drivers/video/keys.h>
#include <drivers/video/sdl.h>
#include <lib/log.h>

static int sdl_set_window_size (sdl_t *sdl, unsigned w, unsigned h, int force);


static sdl_keymap_t keymap[] = {
	{ SDLK_ESCAPE,       PCE_KEY_ESC },
	{ SDLK_F1,           PCE_KEY_F1 },
	{ SDLK_F2,           PCE_KEY_F2 },
	{ SDLK_F3,           PCE_KEY_F3 },
	{ SDLK_F4,           PCE_KEY_F4 },
	{ SDLK_F5,           PCE_KEY_F5 },
	{ SDLK_F6,           PCE_KEY_F6 },
	{ SDLK_F7,           PCE_KEY_F7 },
	{ SDLK_F8,           PCE_KEY_F8 },
	{ SDLK_F9,           PCE_KEY_F9 },
	{ SDLK_F10,          PCE_KEY_F10 },
	{ SDLK_F11,          PCE_KEY_F11 },
	{ SDLK_F12,          PCE_KEY_F12 },

	{ SDLK_PRINT,        PCE_KEY_PRTSCN },
	{ SDLK_SCROLLOCK,    PCE_KEY_SCRLK },
	{ SDLK_PAUSE,        PCE_KEY_PAUSE },

	{ SDLK_BACKQUOTE,    PCE_KEY_BACKQUOTE },
	{ SDLK_1,            PCE_KEY_1 },
	{ SDLK_2,            PCE_KEY_2 },
	{ SDLK_3,            PCE_KEY_3 },
	{ SDLK_4,            PCE_KEY_4 },
	{ SDLK_5,            PCE_KEY_5 },
	{ SDLK_6,            PCE_KEY_6 },
	{ SDLK_7,            PCE_KEY_7 },
	{ SDLK_8,            PCE_KEY_8 },
	{ SDLK_9,            PCE_KEY_9 },
	{ SDLK_0,            PCE_KEY_0 },
	{ SDLK_MINUS,        PCE_KEY_MINUS },
	{ 0x00bd,            PCE_KEY_MINUS },
	{ SDLK_UNDERSCORE,   PCE_KEY_UNDERSCORE },
	{ SDLK_EQUALS,       PCE_KEY_EQUAL },
	{ 0x00bb,            PCE_KEY_EQUAL },
	{ SDLK_BACKSPACE,    PCE_KEY_BACKSPACE },
	{ SDLK_PERCENT,      PCE_KEY_PERCENT },

	{ SDLK_TAB,          PCE_KEY_TAB },
	{ SDLK_q,            PCE_KEY_Q },
	{ SDLK_w,            PCE_KEY_W },
	{ SDLK_e,            PCE_KEY_E },
	{ SDLK_r,            PCE_KEY_R },
	{ SDLK_t,            PCE_KEY_T },
	{ SDLK_y,            PCE_KEY_Y },
	{ SDLK_u,            PCE_KEY_U },
	{ SDLK_i,            PCE_KEY_I },
	{ SDLK_o,            PCE_KEY_O },
	{ SDLK_p,            PCE_KEY_P },
	{ SDLK_LEFTBRACKET,  PCE_KEY_LBRACKET },
	{ SDLK_RIGHTBRACKET, PCE_KEY_RBRACKET },
	{ SDLK_LEFTBRACE,    PCE_KEY_LBRACE },
	{ SDLK_RIGHTBRACE,   PCE_KEY_RBRACE },
	{ SDLK_RETURN,       PCE_KEY_RETURN },

	{ SDLK_CAPSLOCK,     PCE_KEY_CAPSLOCK },
	{ SDLK_a,            PCE_KEY_A },
	{ SDLK_s,            PCE_KEY_S },
	{ SDLK_d,            PCE_KEY_D },
	{ SDLK_f,            PCE_KEY_F },
	{ SDLK_g,            PCE_KEY_G },
	{ SDLK_h,            PCE_KEY_H },
	{ SDLK_j,            PCE_KEY_J },
	{ SDLK_k,            PCE_KEY_K },
	{ SDLK_l,            PCE_KEY_L },
	{ SDLK_COLON,        PCE_KEY_COLON },
	{ SDLK_SEMICOLON,    PCE_KEY_SEMICOLON },
	{ 0x00ba,            PCE_KEY_SEMICOLON },
	{ SDLK_QUOTE,        PCE_KEY_QUOTE },
	{ SDLK_BACKSLASH,    PCE_KEY_BACKSLASH },
	{ 0x00dc,            PCE_KEY_BACKSLASH },

	{ SDLK_LSHIFT,       PCE_KEY_LSHIFT },
	{ SDLK_LESS,         PCE_KEY_LESS },
	{ SDLK_GREATER,      PCE_KEY_GREATER },
	{ SDLK_z,            PCE_KEY_Z },
	{ SDLK_x,            PCE_KEY_X },
	{ SDLK_c,            PCE_KEY_C },
	{ SDLK_v,            PCE_KEY_V },
	{ SDLK_b,            PCE_KEY_B },
	{ SDLK_n,            PCE_KEY_N },
	{ SDLK_m,            PCE_KEY_M },
	{ SDLK_COMMA,        PCE_KEY_COMMA },
	{ SDLK_PERIOD,       PCE_KEY_PERIOD },
	{ SDLK_SLASH,        PCE_KEY_SLASH },
	{ SDLK_QUESTION,     PCE_KEY_QUESTION },
	{ SDLK_RSHIFT,       PCE_KEY_RSHIFT },

	{ SDLK_LCTRL,        PCE_KEY_LCTRL },
	{ SDLK_LMETA,        PCE_KEY_LMETA },
	{ SDLK_LSUPER,       PCE_KEY_LSUPER },
	{ SDLK_LALT,         PCE_KEY_LALT },
	{ SDLK_MODE,         PCE_KEY_MODE },
	{ SDLK_SPACE,        PCE_KEY_SPACE },
	{ SDLK_RALT,         PCE_KEY_RALT },
	{ SDLK_RMETA,        PCE_KEY_RMETA },
	{ SDLK_RSUPER,       PCE_KEY_RSUPER },
	{ 0x0465,            PCE_KEY_RSUPER },
	{ SDLK_MENU,         PCE_KEY_MENU },
	{ SDLK_RCTRL,        PCE_KEY_RCTRL },

	{ SDLK_NUMLOCK,      PCE_KEY_NUMLOCK },
	{ SDLK_KP_DIVIDE,    PCE_KEY_KP_SLASH },
	{ SDLK_KP_MULTIPLY,  PCE_KEY_KP_STAR },
	{ SDLK_KP_MINUS,     PCE_KEY_KP_MINUS },
	{ SDLK_KP7,          PCE_KEY_KP_7 },
	{ SDLK_KP8,          PCE_KEY_KP_8 },
	{ SDLK_KP9,          PCE_KEY_KP_9 },
	{ SDLK_KP_PLUS,      PCE_KEY_KP_PLUS },
	{ SDLK_KP4,          PCE_KEY_KP_4 },
	{ SDLK_KP5,          PCE_KEY_KP_5 },
	{ SDLK_KP6,          PCE_KEY_KP_6 },
	{ SDLK_KP1,          PCE_KEY_KP_1 },
	{ SDLK_KP2,          PCE_KEY_KP_2 },
	{ SDLK_KP3,          PCE_KEY_KP_3 },
	{ SDLK_KP_ENTER,     PCE_KEY_KP_ENTER },
	{ SDLK_KP0,          PCE_KEY_KP_0 },
	{ SDLK_KP_PERIOD,    PCE_KEY_KP_PERIOD },
	{ SDLK_INSERT,       PCE_KEY_INS },
	{ SDLK_HOME,         PCE_KEY_HOME },
	{ SDLK_PAGEUP,       PCE_KEY_PAGEUP },
	{ SDLK_DELETE,       PCE_KEY_DEL },
	{ SDLK_END,          PCE_KEY_END },
	{ SDLK_PAGEDOWN,     PCE_KEY_PAGEDN },
	{ SDLK_UP,           PCE_KEY_UP },
	{ SDLK_LEFT,         PCE_KEY_LEFT },
	{ SDLK_DOWN,         PCE_KEY_DOWN },
	{ SDLK_RIGHT,        PCE_KEY_RIGHT },

	{ SDLK_UNDO,         PCE_KEY_UNDO },
	{ SDLK_HELP,         PCE_KEY_HELP },
	{ SDLK_EXCLAIM,      PCE_KEY_EXCLAM },
	{ SDLK_QUOTEDBL,     PCE_KEY_QUOTEDBL },
	{ SDLK_HASH,         PCE_KEY_HASH },
	{ SDLK_DOLLAR,       PCE_KEY_DOLLAR },
	{ SDLK_AMPERSAND,    PCE_KEY_AMPERSAND },
	{ SDLK_LEFTPAREN,    PCE_KEY_LPAREN },
	{ SDLK_RIGHTPAREN,   PCE_KEY_RPAREN },
	{ SDLK_ASTERISK,     PCE_KEY_ASTERISK },
	{ SDLK_PLUS,         PCE_KEY_PLUS },
	{ SDLK_LEFTBRACE,    PCE_KEY_LPAREN },
	{ SDLK_RIGHTBRACE,   PCE_KEY_RPAREN },
	{ SDLK_BAR,          PCE_KEY_BAR },
	{ SDLK_CARET,        PCE_KEY_ASCIICIRCUM },
	{ SDLK_TILDE,        PCE_KEY_ASCIITILDE },
	{ SDLK_AT,           PCE_KEY_AT },

	{ 0,                 PCE_KEY_NONE }
};


static
void sdl_set_keymap (sdl_t *sdl, SDLKey src, pce_key_t dst)
{
	unsigned     i;
	sdl_keymap_t *tmp;

	for (i = 0; i < sdl->keymap_cnt; i++) {
		if (sdl->keymap[i].sdlkey == src) {
			sdl->keymap[i].pcekey = dst;
			return;
		}
	}

	tmp = realloc (sdl->keymap, (sdl->keymap_cnt + 1) * sizeof (sdl_keymap_t));

	if (tmp == NULL) {
		return;
	}

	tmp[sdl->keymap_cnt].sdlkey = src;
	tmp[sdl->keymap_cnt].pcekey = dst;

	sdl->keymap = tmp;
	sdl->keymap_cnt += 1;
}

static
void sdl_init_keymap_default (sdl_t *sdl)
{
	unsigned i, n;

	sdl->keymap_cnt = 0;
	sdl->keymap = NULL;

	n = 0;
	while (keymap[n].pcekey != PCE_KEY_NONE) {
		n += 1;
	}

	sdl->keymap = malloc (n * sizeof (sdl_keymap_t));

	if (sdl->keymap == NULL) {
		return;
	}

	for (i = 0; i < n; i++) {
		sdl->keymap[i] = keymap[i];
	}

	sdl->keymap_cnt = n;
}

static
void sdl_init_keymap_user (sdl_t *sdl, ini_sct_t *sct)
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

		sdl_set_keymap (sdl, (SDLKey) sdlkey, pcekey);
	}
}

static
void sdl_grab_mouse (void *ext, int grab)
{
	sdl_t *sdl = (sdl_t *)ext;
	if (grab) {
		sdl->grab = 1;
		SDL_ShowCursor (0);
		SDL_WM_GrabInput (SDL_GRAB_ON);
	}
	else {
		SDL_ShowCursor (1);
		SDL_WM_GrabInput (SDL_GRAB_OFF);
		sdl->grab = 0;
	}
}

static
void sdl_set_fullscreen (sdl_t *sdl, int val)
{
	if ((val != 0) == (sdl->fullscreen != 0)) {
		return;
	}

	if (sdl->scr != NULL) {
		SDL_WM_ToggleFullScreen (sdl->scr);
	}

	sdl->fullscreen = (val != 0);

#ifdef PCE_HOST_WINDOWS
	/*
	 * SDL under windows does not support toggling full screen mode
	 * after a surface has been created. Setting the window size
	 * to the current size will free and then reallocate the
	 * surface.
	 */
	sdl_set_window_size (sdl, sdl->wdw_w, sdl->wdw_h, 1);

	/* Invalidate the entire terminal to force an update. */
	sdl->trm.update_x = 0;
	sdl->trm.update_y = 0;
	sdl->trm.update_w = sdl->trm.w;
	sdl->trm.update_h = sdl->trm.h;

	trm_update (&sdl->trm);
#endif
}

static
int sdl_set_window_size (sdl_t *sdl, unsigned w, unsigned h, int force)
{
	unsigned long flags;

	if (sdl->scr != NULL) {
		if (force == 0) {
			if ((sdl->wdw_w == w) && (sdl->wdw_h == h)) {
				return (0);
			}
		}

		SDL_FreeSurface (sdl->scr);
	}

	sdl->scr = NULL;

	flags = SDL_HWSURFACE;

	if (sdl->fullscreen) {
		flags |= SDL_FULLSCREEN;
	}

	if (sdl->dsp_bpp == 2) {
		sdl->scr = SDL_SetVideoMode (w, h, 16, flags);
		sdl->scr_bpp = 2;
	}
	else if (sdl->dsp_bpp == 4) {
		sdl->scr = SDL_SetVideoMode (w, h, 32, flags);
		sdl->scr_bpp = 4;
	}

	if (sdl->scr == NULL) {
		sdl->scr = SDL_SetVideoMode (w, h, 16, flags);
		sdl->scr_bpp = 2;
	}

	if (sdl->scr == NULL) {
		return (1);
	}

	sdl->wdw_w = w;
	sdl->wdw_h = h;

	return (0);
}

static
unsigned sdl_map_key (sdl_t *sdl, SDLKey key)
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
void sdl_update (void *ext)
{
	sdl_t *sdl = (sdl_t *)ext;
	SDL_Surface         *s;
	Uint32              rmask, gmask, bmask, amask;
	terminal_t          *trm;
	SDL_Rect            dst;
	const unsigned char *buf;
	unsigned            dw, dh;
	unsigned            ux, uy, uw, uh;
	unsigned            fx, fy;
	unsigned            bx, by;

	trm = &sdl->trm;

	trm_get_scale (&sdl->trm, trm->w, trm->h, &fx, &fy);

	dw = fx * trm->w;
	dh = fy * trm->h;

	bx = sdl->border[0] + sdl->border[2];
	by = sdl->border[1] + sdl->border[3];

	if (sdl_set_window_size (sdl, dw + bx, dh + by, 0)) {
		return;
	}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0x00ff0000;
	gmask = 0x0000ff00;
	bmask = 0x000000ff;
	amask = 0xff000000;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	buf = trm_scale (trm, trm->buf, trm->w, trm->h, fx, fy);

	ux = fx * trm->update_x;
	uy = fy * trm->update_y;
	uw = fx * trm->update_w;
	uh = fy * trm->update_h;

	s = SDL_CreateRGBSurfaceFrom (
		(char *) buf + trm->term_bpp * (dw * uy + ux), uw, uh, trm->term_bpp * 8, trm->term_bpp * dw,
		rmask, gmask, bmask, trm->term_bpp == 3 ? 0 : amask
	);

	dst.x = ux + sdl->border[0];
	dst.y = uy + sdl->border[1];

	if (s == NULL) {
		return;
	}


	if (SDL_BlitSurface (s, NULL, sdl->scr, &dst) != 0) {
		fprintf (stderr, "sdl: blit error\n");
	}

	SDL_FreeSurface (s);

	SDL_Flip (sdl->scr);
}


#ifdef __EMSCRIPTEN__
/*
 * Emscripten uses SDL2 style scancodes
 * for a USB keyboard
 */
static unsigned int sdl_map_sym(SDL_keysym *keysym)
{
	unsigned int scancode = 0;
	
	switch (keysym->scancode)
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
		/* ??? we get scancode 0x0f also from right bracket and kp-period */
		case SDL_SCANCODE_TAB: scancode = keysym->sym == 0x09 ? 0x0f : keysym->sym == 0x6c ? 0x71 : 0x1b; break;
		case SDL_SCANCODE_Q: scancode = 0x10; break;
		case SDL_SCANCODE_W: scancode = 0x11; break;
		case SDL_SCANCODE_E: scancode = 0x12; break;
		case SDL_SCANCODE_R: scancode = 0x13; break;
		case SDL_SCANCODE_T: scancode = 0x14; break;
		case SDL_SCANCODE_Y: scancode = keysym->sym == 'z' ? 0x15 : 0x2c; break;
		case SDL_SCANCODE_U: scancode = 0x16; break;
		case SDL_SCANCODE_I: scancode = 0x17; break;
		case SDL_SCANCODE_O: scancode = 0x18; break;
		case SDL_SCANCODE_P: scancode = 0x19; break;
		case SDL_SCANCODE_LEFTBRACKET: scancode = 0x1a; break;
		case SDL_SCANCODE_RIGHTBRACKET: scancode = 0x1b; break;
		case SDL_SCANCODE_RETURN: scancode = 0x1c; break;
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
		case SDL_SCANCODE_APOSTROPHE: scancode = keysym->sym == 0x60 ? 0x29 : 0x28; break;
		case SDL_SCANCODE_GRAVE: scancode = keysym->sym == 0x27 ? 0x28 : 0x29; break;
		case SDL_SCANCODE_BACKSLASH: scancode = 0x2b; break;
		case SDL_SCANCODE_NONUSHASH: scancode = 0x2b; break;
		case SDL_SCANCODE_KP_HASH: scancode = 0x2b; break;
		case SDL_SCANCODE_Z: scancode = keysym->sym == 'y' ? 0x2c : 0x15; break;
		case SDL_SCANCODE_X: scancode = 0x2d; break;
		case SDL_SCANCODE_C: scancode = 0x2e; break;
		case SDL_SCANCODE_V: scancode = 0x2f; break;
		case SDL_SCANCODE_B: scancode = 0x30; break;
		case SDL_SCANCODE_N: scancode = 0x31; break;
		case SDL_SCANCODE_M: scancode = 0x32; break;
		case SDL_SCANCODE_COMMA: scancode = 0x33; break;
		case SDL_SCANCODE_PERIOD: scancode = 0x34; break;
		case SDL_SCANCODE_SLASH: scancode = 0x35; break;
		case SDL_SCANCODE_SPACE: scancode = 0x39; break;
		case SDL_SCANCODE_CAPSLOCK: scancode = 0x3a; break;
		case SDL_SCANCODE_F1: scancode = 0x3b; break;
		case SDL_SCANCODE_F2: scancode = 0x3c; break;
		case SDL_SCANCODE_F3: scancode = 0x3d; break;
		case SDL_SCANCODE_F4: scancode = 0x3e; break;
		case SDL_SCANCODE_F5: scancode = 0x3f; break;
		case SDL_SCANCODE_F6: scancode = keysym->sym == 0x3f ? 0x0c : 0x40; break;
		case SDL_SCANCODE_F7: scancode = 0x41; break;
		case SDL_SCANCODE_F8: scancode = 0x42; break;
		case SDL_SCANCODE_F9: scancode = 0x43; break;
		case SDL_SCANCODE_F10: scancode = 0x44; break;
		case SDL_SCANCODE_F11: scancode = 0x62; break;
		case SDL_SCANCODE_F12: scancode = 0x61; break;
		case SDL_SCANCODE_PRINTSCREEN: scancode = 0x62; break;
		case SDL_SCANCODE_SCROLLLOCK: scancode = 0x00; break;
		case SDL_SCANCODE_PAUSE: scancode = 0x00; break;
		case SDL_SCANCODE_INSERT: scancode = 0x52; break;
		case SDL_SCANCODE_HOME: scancode = 0x47; break;
		case SDL_SCANCODE_PAGEUP: scancode = 0x49; break;
		case SDL_SCANCODE_DELETE: scancode = 0x53; break;
		case SDL_SCANCODE_END: scancode = 0x47; break;
		case SDL_SCANCODE_PAGEDOWN: scancode = 0x51; break;
		case SDL_SCANCODE_RIGHT: scancode = 0x4d; break;
		case SDL_SCANCODE_LEFT: scancode = 0x4b; break;
		case SDL_SCANCODE_DOWN: scancode = 0x50; break;
		case SDL_SCANCODE_UP: scancode = 0x48; break;
		case SDL_SCANCODE_KP_LEFTPAREN:
		case SDL_SCANCODE_NUMLOCKCLEAR: scancode = 0x63; break;
		case SDL_SCANCODE_KP_DIVIDE: scancode = 0x65; break;
		case SDL_SCANCODE_KP_MULTIPLY: scancode = 0x66; break;
		case SDL_SCANCODE_KP_MINUS: scancode = 0x4a; break;
		case SDL_SCANCODE_KP_PLUS: scancode = 0x4e; break;
		case SDL_SCANCODE_KP_ENTER: scancode = 0x72; break;
		case SDL_SCANCODE_KP_1: scancode = 0x6d; break;
		case SDL_SCANCODE_KP_2: scancode = 0x6e; break;
		case SDL_SCANCODE_KP_3: scancode = 0x6f; break;
		case SDL_SCANCODE_KP_4: scancode = 0x6a; break;
		case SDL_SCANCODE_KP_5: scancode = 0x6b; break;
		case SDL_SCANCODE_KP_6: scancode = keysym->sym == 0x5e ? 0x07 : 0x6c; break;
		case SDL_SCANCODE_KP_7: scancode = 0x67; break;
		case SDL_SCANCODE_KP_8: scancode = 0x68; break;
		case SDL_SCANCODE_KP_9: scancode = 0x69; break;
		case SDL_SCANCODE_KP_0: scancode = 0x70; break;
		case SDL_SCANCODE_KP_PERIOD: scancode = 0x71; break;
		case SDL_SCANCODE_KP_RIGHTPAREN:
		case SDL_SCANCODE_KP_EQUALS: scancode = 0x64; break;
		case SDL_SCANCODE_NONUSBACKSLASH: scancode = 0x60; break;
		case SDL_SCANCODE_HELP: scancode = 0x62; break;
		case SDL_SCANCODE_UNDO: scancode = 0x61; break;

		case SDL_SCANCODE_LCTRL: scancode = 0x1d; break;
		case SDL_SCANCODE_LSHIFT: scancode = 0x2a; break;
		case SDL_SCANCODE_LALT: scancode = 0x38; break;
		case SDL_SCANCODE_LGUI: scancode = 0x00; break;
		case SDL_SCANCODE_RCTRL: scancode = 0x1d; break;
		case SDL_SCANCODE_RSHIFT: scancode = 0x36; break;
		case SDL_SCANCODE_RALT: scancode = 0x4c; break;
		case SDL_SCANCODE_RGUI: scancode = 0x00; break;

		default: scancode = 0; break;
	}
	keysym->scancode = scancode;
	return scancode;
}

#else

#define UNDEFINED_OFFSET    ((unsigned int)-1)
static int findScanCodeOffset(SDL_keysym *keysym)
{
	unsigned int scanPC = keysym->scancode;
	int offset = UNDEFINED_OFFSET;

	switch (keysym->sym)
	{
		case SDLK_ESCAPE: offset = scanPC - 0x01; break;
		case SDLK_1: offset = scanPC - 0x02; break;
		case SDLK_2: offset = scanPC - 0x03; break;
		case SDLK_3: offset = scanPC - 0x04; break;
		case SDLK_4: offset = scanPC - 0x05; break;
		case SDLK_5: offset = scanPC - 0x06; break;
		case SDLK_6: offset = scanPC - 0x07; break;
		case SDLK_7: offset = scanPC - 0x08; break;
		case SDLK_8: offset = scanPC - 0x09; break;
		case SDLK_9: offset = scanPC - 0x0a; break;
		case SDLK_0: offset = scanPC - 0x0b; break;
		case SDLK_BACKSPACE: offset = scanPC - 0x0e; break;
		case SDLK_TAB: offset = scanPC - 0x0f; break;
		case SDLK_RETURN: offset = scanPC - 0x1c; break;
		case SDLK_SPACE: offset = scanPC - 0x39; break;
		case SDLK_q: offset = scanPC - 0x10; break;
		case SDLK_w: offset = scanPC - 0x11; break;
		case SDLK_e: offset = scanPC - 0x12; break;
		case SDLK_r: offset = scanPC - 0x13; break;
		case SDLK_t: offset = scanPC - 0x14; break;
		case SDLK_y: offset = scanPC - 0x15; break;
		case SDLK_u: offset = scanPC - 0x16; break;
		case SDLK_i: offset = scanPC - 0x17; break;
		case SDLK_o: offset = scanPC - 0x18; break;
		case SDLK_p: offset = scanPC - 0x19; break;
		case SDLK_a: offset = scanPC - 0x1e; break;
		case SDLK_s: offset = scanPC - 0x1f; break;
		case SDLK_d: offset = scanPC - 0x20; break;
		case SDLK_f: offset = scanPC - 0x21; break;
		case SDLK_g: offset = scanPC - 0x22; break;
		case SDLK_h: offset = scanPC - 0x23; break;
		case SDLK_j: offset = scanPC - 0x24; break;
		case SDLK_k: offset = scanPC - 0x25; break;
		case SDLK_l: offset = scanPC - 0x26; break;
		case SDLK_z: offset = scanPC - 0x2c; break;
		case SDLK_x: offset = scanPC - 0x2d; break;
		case SDLK_c: offset = scanPC - 0x2e; break;
		case SDLK_v: offset = scanPC - 0x2f; break;
		case SDLK_b: offset = scanPC - 0x30; break;
		case SDLK_n: offset = scanPC - 0x31; break;
		case SDLK_m: offset = scanPC - 0x32; break;
		case SDLK_CAPSLOCK: offset = scanPC - 0x3a; break;
		case SDLK_RSHIFT: offset = scanPC - 0x36; break;
		case SDLK_LSHIFT: offset = scanPC - 0x2a; break;
		case SDLK_LCTRL: offset = scanPC - 0x1d; break;
		case SDLK_LALT: offset = scanPC - 0x38; break;
		case SDLK_F1: offset = scanPC - 0x3b; break;
		case SDLK_F2: offset = scanPC - 0x3c; break;
		case SDLK_F3: offset = scanPC - 0x3d; break;
		case SDLK_F4: offset = scanPC - 0x3e; break;
		case SDLK_F5: offset = scanPC - 0x3f; break;
		case SDLK_F6: offset = scanPC - 0x40; break;
		case SDLK_F7: offset = scanPC - 0x41; break;
		case SDLK_F8: offset = scanPC - 0x42; break;
		case SDLK_F9: offset = scanPC - 0x43; break;
		case SDLK_F10: offset = scanPC - 0x44; break;
		default: break;
	}
	if (offset < 0)
		offset = UNDEFINED_OFFSET;
	if (offset >= 0)
	{
		pce_log(MSG_DEB, "Detected scancode offset = %d (key: '%s' with scancode $%02x)\n", offset, SDL_GetKeyName(keysym->sym), scanPC);
	}

	return offset;
}


static unsigned int sdl_map_sym(SDL_keysym *keysym)
{
	unsigned int scancode = 0;
	static unsigned int offset = UNDEFINED_OFFSET;

	switch ((unsigned int) keysym->sym)
	{
		/* Numeric Pad */
		case SDLK_KP_DIVIDE: scancode = 0x65; break;	/* Numpad / */
		case SDLK_KP_MULTIPLY: scancode = 0x66; break;	/* NumPad * */
		case SDLK_KP7: scancode = 0x67; break;	/* NumPad 7 */
		case SDLK_KP8: scancode = 0x68; break;	/* NumPad 8 */
		case SDLK_KP9: scancode = 0x69; break;	/* NumPad 9 */
		case SDLK_KP4: scancode = 0x6a; break;	/* NumPad 4 */
		case SDLK_KP5: scancode = 0x6b; break;	/* NumPad 5 */
		case SDLK_KP6: scancode = 0x6c; break;	/* NumPad 6 */
		case SDLK_KP1: scancode = 0x6d; break;	/* NumPad 1 */
		case SDLK_KP2: scancode = 0x6e; break;	/* NumPad 2 */
		case SDLK_KP3: scancode = 0x6f; break;	/* NumPad 3 */
		case SDLK_KP0: scancode = 0x70; break;	/* NumPad 0 */
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

		case SDLK_NUMLOCK: scancode = 0x63; break;
		
		case SDLK_BACKQUOTE:
		case SDLK_LESS: scancode = 0x60; break;	/* a '<>' key next to short left Shift */

		/* keys not found on some keyboards */
		case SDLK_RCTRL: scancode = 0x1d; break;
		case SDLK_MODE: /* passthru */ /* Alt Gr key according to SDL docs */
		case SDLK_RALT: scancode = 0x4c; break;
	}

	if (scancode == 0)
	{
		/*
		 * Process remaining keys: assume that it's PC101 keyboard
		 * and that it is compatible with Atari ST keyboard (basically
		 * same scancodes but on different platforms with different
		 * base offset (framebuffer = 0, X11 = 8).
		 * Try to detect the offset using a little bit of black magic.
		 * If offset is known then simply pass the scancode.
		 */
		scancode = keysym->scancode;
		if (offset == UNDEFINED_OFFSET)
		{
			offset = findScanCodeOffset(keysym);
		}

		/* offset is defined so pass the scancode directly */
		if (offset != UNDEFINED_OFFSET && scancode > offset)
		{
			scancode -= offset;
		}
	}
	keysym->scancode = scancode;
	return scancode;
}
#endif /* __EMSCRIPTEN__ */


static
void sdl_event_keydown (sdl_t *sdl, SDL_keysym *sym)
{
	pce_key_t pcekey;

	if ((sym->sym == SDLK_BACKQUOTE) && (sym->mod & KMOD_LCTRL)) {
		sdl_grab_mouse (sdl, 0);
		sdl_set_fullscreen (sdl, 0);
		trm_set_msg_emu (&sdl->trm, "emu.stop", "1");
		return;
	}
	else if (sym->sym == SDLK_PRINT) {
		trm_screenshot (&sdl->trm, NULL);
		return;
	}

	sdl_map_sym(sym);

	pcekey = sdl_map_key (sdl, sym->sym);

	if (sdl->report_keys || pcekey == PCE_KEY_NONE) {
		const char *keyname = pce_key_to_string(pcekey);
		fprintf (stderr, "sdl: scancode=0x%x, key=0x%04x, unicode=0x%04x -> %d (%s)\n", sym->scancode, sym->sym, sym->unicode, pcekey, keyname ? keyname : "<unknown>");
	}

	if (pcekey == PCE_KEY_NONE && sym->scancode == 0) {
		return;
	}

	trm_set_key (&sdl->trm, PCE_KEY_EVENT_DOWN, pcekey, sym->scancode);

	if (sym->sym == SDLK_NUMLOCK) {
		trm_set_key (&sdl->trm, PCE_KEY_EVENT_UP, pcekey, sym->scancode);
	}
}

static
void sdl_event_keyup (sdl_t *sdl, SDL_keysym *sym)
{
	pce_key_t pcekey;

	sdl_map_sym(sym);

	pcekey = sdl_map_key (sdl, sym->sym);

	if (sym->sym == SDLK_PRINT) {
		return;
	}

	if (pcekey != PCE_KEY_NONE || sym->scancode != 0) {
		if (sym->sym == SDLK_NUMLOCK) {
			trm_set_key (&sdl->trm, PCE_KEY_EVENT_DOWN, pcekey, sym->scancode);
		}

		trm_set_key (&sdl->trm, PCE_KEY_EVENT_UP, pcekey, sym->scancode);
	}
}

static
void sdl_event_mouse_button (sdl_t *sdl, int down, unsigned button)
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
			sdl_grab_mouse (sdl, 1);
		}
		return;
	}

	if (sdl->trm.set_mouse == NULL) {
		return;
	}

	trm_set_mouse (&sdl->trm, 0, 0, sdl->button);
}

static
void sdl_event_mouse_motion (sdl_t *sdl, int dx, int dy)
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
int sdl_check (void *ext)
{
	sdl_t *sdl = (sdl_t *)ext;
	unsigned int i;
	SDL_Event evt;

	i = 0;
	while (SDL_PollEvent (&evt) && (i < 8)) {
		switch (evt.type) {
		case SDL_KEYDOWN:
			sdl_event_keydown (sdl, &evt.key.keysym);
			break;

		case SDL_KEYUP:
			sdl_event_keyup (sdl, &evt.key.keysym);
			break;

		case SDL_MOUSEBUTTONDOWN:
			sdl_event_mouse_button (sdl, 1, evt.button.button);
			break;

		case SDL_MOUSEBUTTONUP:
			sdl_event_mouse_button (sdl, 0, evt.button.button);
			break;

		case SDL_MOUSEMOTION:
			sdl_event_mouse_motion (sdl, evt.motion.xrel, evt.motion.yrel);
			break;

		case SDL_VIDEORESIZE:
			break;

		case SDL_QUIT:
			sdl_grab_mouse (sdl, 0);
			trm_set_msg_emu (&sdl->trm, "emu.exit", "0");
			break;

		default:
			break;
		}

		i += 1;
	}
	return i > 0;
}

static
int sdl_set_msg_trm (void *ext, const char *msg, const char *val)
{
	sdl_t *sdl = (sdl_t *)ext;
	if (val == NULL) {
		val = "";
	}

	if (strcmp (msg, "term.grab") == 0) {
		sdl_grab_mouse (sdl, 1);
		return (0);
	}
	else if (strcmp (msg, "term.release") == 0) {
		sdl_grab_mouse (sdl, 0);
		return (0);
	}
	else if (strcmp (msg, "term.title") == 0) {
#ifndef __EMSCRIPTEN__
		SDL_WM_SetCaption (val, val);
#endif
		return (0);
	}
	else if (strcmp (msg, "term.set_border_x") == 0) {
		sdl->border[0] = strtoul (val, NULL, 0);
		sdl->border[2] = sdl->border[0];
		sdl_update (sdl);
		return (0);
	}
	else if (strcmp (msg, "term.set_border_y") == 0) {
		sdl->border[1] = strtoul (val, NULL, 0);
		sdl->border[3] = sdl->border[0];
		sdl_update (sdl);
		return (0);
	}
	else if (strcmp (msg, "term.fullscreen.toggle") == 0) {
		sdl_set_fullscreen (sdl, !sdl->fullscreen);
		return (0);
	}
	else if (strcmp (msg, "term.fullscreen") == 0) {
		int v;

		v = strtol (val, NULL, 0);

		sdl_set_fullscreen (sdl, v != 0);

		return (0);
	}

	return (-1);
}

static
void sdl_del (void *sdl)
{
	free (sdl);
}

static
int sdl_open (void *ext, unsigned w, unsigned h)
{
	sdl_t *sdl = (sdl_t *)ext;
	unsigned             fx, fy, bx, by;
	const SDL_VideoInfo *inf;

	if ((w == 0) || (h == 0)) {
		w = 640;
		h = 480;
	}

	trm_get_scale (&sdl->trm, w, h, &fx, &fy);

	bx = sdl->border[0] + sdl->border[2];
	by = sdl->border[1] + sdl->border[3];

	if (SDL_WasInit (SDL_INIT_VIDEO) == 0) {
		if (SDL_InitSubSystem (SDL_INIT_VIDEO) < 0) {
			return (1);
		}
	}

	inf = SDL_GetVideoInfo();

	sdl->dsp_bpp = inf->vfmt->BytesPerPixel;
	sdl->scr_bpp = 0;

#ifndef __EMSCRIPTEN__
	SDL_WM_SetCaption ("pce", "pce");
#endif
	SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
#ifndef __EMSCRIPTEN__
	SDL_EventState (SDL_MOUSEMOTION, SDL_ENABLE);
#endif

	sdl_set_window_size (sdl, fx * w + bx, fy * h + by, 0);

	return (0);
}

static
int sdl_close (void *ext)
{
	sdl_t *sdl = (sdl_t *)ext;
	sdl_grab_mouse (sdl, 0);

	if (sdl->scr != NULL) {
		SDL_FreeSurface (sdl->scr);

		sdl->scr = NULL;
	}

	return (0);
}

static
void sdl_init (sdl_t *sdl, ini_sct_t *sct)
{
	int fs, rep;

	trm_init (&sdl->trm, sdl);
	sdl->trm.name = "sdl";

	sdl->trm.del = sdl_del;
	sdl->trm.open = sdl_open;
	sdl->trm.close = sdl_close;
	sdl->trm.set_msg_trm = sdl_set_msg_trm;
	sdl->trm.update = sdl_update;
	sdl->trm.check = sdl_check;
	sdl->trm.grab = sdl_grab_mouse;

	sdl->scr = NULL;

	sdl->wdw_w = 0;
	sdl->wdw_h = 0;

	sdl->button = 0;

	ini_get_bool (sct, "fullscreen", &fs, 0);
	sdl->fullscreen = (fs != 0);

	ini_get_uint16 (sct, "border", &sdl->border[0], 0);
	sdl->border[1] = sdl->border[0];
	sdl->border[2] = sdl->border[0];
	sdl->border[3] = sdl->border[0];

	sdl->grab = 0;

	sdl->dsp_bpp = 0;
	sdl->scr_bpp = 0;

	ini_get_bool (sct, "report_keys", &rep, 0);
	sdl->report_keys = (rep != 0);

	sdl_init_keymap_default (sdl);
	sdl_init_keymap_user (sdl, sct);
}

terminal_t *sdl_new (ini_sct_t *sct)
{
	sdl_t *sdl;

	sdl = malloc (sizeof (sdl_t));
	if (sdl == NULL) {
		return (NULL);
	}

	sdl_init (sdl, sct);

	return (&sdl->trm);
}
