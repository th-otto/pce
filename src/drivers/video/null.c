/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/null.c                                     *
 * Created:     2003-10-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2015 Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include <drivers/video/terminal.h>
#include <drivers/video/null.h>


static
void null_free (null_t *nt)
{
}

static
void null_del (void *ext)
{
	null_t *nt = (null_t *)ext;
	if (nt != NULL) {
		null_free (nt);
		free (nt);
	}
}

static
int null_open (void *ext, unsigned w, unsigned h)
{
	return (0);
}

static
int null_close (void *ext)
{
	return (0);
}

static
int null_set_msg_trm (void *ext, const char *msg, const char *val)
{
	if (strcmp (msg, "term.grab") == 0) {
		return (0);
	}
	else if (strcmp (msg, "term.release") == 0) {
		return (0);
	}
	else if (strcmp (msg, "term.title") == 0) {
		return (0);
	}
	else if (strcmp (msg, "term.fullscreen.toggle") == 0) {
		return (0);
	}
	else if (strcmp (msg, "term.fullscreen") == 0) {
		return (0);
	}

	return (-1);
}

static
void null_update (void *ext)
{
}

static
int null_check (void *ext)
{
	return 1;
}

static
void null_init (null_t *nt, ini_sct_t *ini)
{
	trm_init (&nt->trm, nt);
	nt->trm.name = "null";

	nt->trm.del = null_del;
	nt->trm.open = null_open;
	nt->trm.close = null_close;
	nt->trm.set_msg_trm = null_set_msg_trm;
	nt->trm.update = null_update;
	nt->trm.check = null_check;
	nt->trm.grab = 0;
}

terminal_t *null_new (ini_sct_t *ini)
{
	null_t *nt;

	nt = malloc (sizeof (null_t));
	if (nt == NULL) {
		return (NULL);
	}

	null_init (nt, ini);

	return (&nt->trm);
}
