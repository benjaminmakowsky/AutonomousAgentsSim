/*
 * XPilotNG/SDL, an SDL/OpenGL XPilot client.
 *
 * Copyright (C) 2003-2004 Juha Lindstr�m <juhal@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xpclient_sdl.h"

/* from talk.c */
selection_t selection;
int Startup_server_motd(void);

void Play_beep(void)
{
    fprintf(stderr, "\aBEEP\n");
}

int Startup_server_motd(void)
{
    return 0;
}

int Handle_motd(long off, char *buf, int len, long filesize) 
{
    fwrite(buf + off, 1, len, stdout);
    return 0;
}

void Raise_window(void) {}

void Colors_init_style_colors(void) {}