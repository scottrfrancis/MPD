/*
 * Copyright 2003-2016 The Music Player Daemon Project
 * http://www.musicpd.org
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"
#include "BufferedOutputStream.hxx"
#include "OutputStream.hxx"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

bool
BufferedOutputStream::AppendToBuffer(const void *data, size_t size) noexcept
{
	auto r = buffer.Write();
	if (r.size < size)
		return false;

	memcpy(r.data, data, size);
	buffer.Append(size);
	return true;
}

void
BufferedOutputStream::Write(const void *data, size_t size)
{
	/* try to append to the current buffer */
	if (AppendToBuffer(data, size))
		return;

	/* not enough room in the buffer - flush it */
	Flush();

	/* see if there's now enough room */
	if (AppendToBuffer(data, size))
		return;

	/* too large for the buffer: direct write */
	os.Write(data, size);
}

void
BufferedOutputStream::Write(const char *p)
{
	Write(p, strlen(p));
}

void
BufferedOutputStream::Format(const char *fmt, ...)
{
	auto r = buffer.Write();
	if (r.IsEmpty()) {
		Flush();
		r = buffer.Write();
	}

	/* format into the buffer */
	va_list ap;
	va_start(ap, fmt);
	size_t size = vsnprintf(r.data, r.size, fmt, ap);
	va_end(ap);

	if (gcc_unlikely(size >= r.size)) {
		/* buffer was not large enough; flush it and try
		   again */

		Flush();

		r = buffer.Write();

		if (gcc_unlikely(size >= r.size)) {
			/* still not enough space: grow the buffer and
			   try again */
			r.size = size + 1;
			r.data = buffer.Write(r.size);
		}

		/* format into the new buffer */
		va_start(ap, fmt);
		size = vsnprintf(r.data, r.size, fmt, ap);
		va_end(ap);

		/* this time, it must fit */
		assert(size < r.size);
	}

	buffer.Append(size);
}

void
BufferedOutputStream::Flush()
{
	auto r = buffer.Read();
	if (r.IsEmpty())
		return;

	os.Write(r.data, r.size);
	buffer.Consume(r.size);
}
