/* the Music Player Daemon (MPD)
 * Copyright (C) 2003-2007 by Warren Dukes (warren.dukes@gmail.com)
 * This project's homepage is: http://www.musicpd.org
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MPD_VOLUME_H
#define MPD_VOLUME_H

#include <stdio.h>

#define VOLUME_MIXER_OSS	"oss"
#define VOLUME_MIXER_ALSA	"alsa"
#define VOLUME_MIXER_SOFTWARE	"software"
#define VOLUME_MIXER_HARDWARE	"hardware"
#define VOLUME_MIXER_DISABLED	"disabled"

void volume_init(void);

void volume_finish(void);

int volume_level_get(void);

int volume_level_change(int change, int rel);

void read_sw_volume_state(FILE *fp);

void save_sw_volume_state(FILE *fp);

#endif
