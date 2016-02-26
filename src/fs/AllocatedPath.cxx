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
#include "AllocatedPath.hxx"
#include "Domain.hxx"
#include "Charset.hxx"
#include "util/Error.hxx"
#include "Compiler.h"

/* no inlining, please */
AllocatedPath::~AllocatedPath() {}

AllocatedPath
AllocatedPath::FromUTF8(const char *path_utf8)
{
#if defined(HAVE_FS_CHARSET) || defined(WIN32)
	return AllocatedPath(::PathFromUTF8(path_utf8));
#else
	return FromFS(path_utf8);
#endif
}

AllocatedPath
AllocatedPath::FromUTF8(const char *path_utf8, Error &error)
{
	AllocatedPath path = FromUTF8(path_utf8);
	if (path.IsNull())
		error.Format(path_domain,
			     "Failed to convert to file system charset: %s",
			     path_utf8);

	return path;
}

AllocatedPath
AllocatedPath::GetDirectoryName() const
{
	return FromFS(PathTraitsFS::GetParent(c_str()));
}

std::string
AllocatedPath::ToUTF8() const
{
	return ::PathToUTF8(c_str());
}

void
AllocatedPath::ChopSeparators()
{
	size_t l = length();
	const auto *p = data();

	while (l >= 2 && PathTraitsFS::IsSeparator(p[l - 1])) {
		--l;

#if GCC_CHECK_VERSION(4,7)
		value.pop_back();
#else
		value.erase(value.end() - 1, value.end());
#endif
	}
}
