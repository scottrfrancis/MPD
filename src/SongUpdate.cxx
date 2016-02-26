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

#include "config.h" /* must be first for large file support */
#include "DetachedSong.hxx"
#include "db/plugins/simple/Song.hxx"
#include "db/plugins/simple/Directory.hxx"
#include "storage/StorageInterface.hxx"
#include "storage/FileInfo.hxx"
#include "util/UriUtil.hxx"
#include "util/Error.hxx"
#include "fs/AllocatedPath.hxx"
#include "fs/Traits.hxx"
#include "fs/FileInfo.hxx"
#include "decoder/DecoderList.hxx"
#include "tag/Tag.hxx"
#include "tag/TagBuilder.hxx"
#include "TagFile.hxx"
#include "TagStream.hxx"

#ifdef ENABLE_ARCHIVE
#include "TagArchive.hxx"
#endif

#include <assert.h>
#include <string.h>
#include <sys/stat.h>

#ifdef ENABLE_DATABASE

Song *
Song::LoadFile(Storage &storage, const char *path_utf8, Directory &parent)
{
	assert(!uri_has_scheme(path_utf8));
	assert(strchr(path_utf8, '\n') == nullptr);

	Song *song = NewFile(path_utf8, parent);
	if (!song->UpdateFile(storage)) {
		song->Free();
		return nullptr;
	}

	return song;
}

#endif

#ifdef ENABLE_DATABASE

bool
Song::UpdateFile(Storage &storage)
{
	const auto &relative_uri = GetURI();

	StorageFileInfo info;
	if (!storage.GetInfo(relative_uri.c_str(), true, info, IgnoreError()))
		return false;

	if (!info.IsRegular())
		return false;

	TagBuilder tag_builder;

	const auto path_fs = storage.MapFS(relative_uri.c_str());
	if (path_fs.IsNull()) {
		const auto absolute_uri =
			storage.MapUTF8(relative_uri.c_str());
		if (!tag_stream_scan(absolute_uri.c_str(), tag_builder))
			return false;
	} else {
		if (!tag_file_scan(path_fs, tag_builder))
			return false;
	}

	mtime = info.mtime;
	tag_builder.Commit(tag);
	return true;
}

#endif

#ifdef ENABLE_ARCHIVE

Song *
Song::LoadFromArchive(ArchiveFile &archive, const char *name_utf8,
		      Directory &parent)
{
	assert(!uri_has_scheme(name_utf8));
	assert(strchr(name_utf8, '\n') == nullptr);

	Song *song = NewFile(name_utf8, parent);

	if (!song->UpdateFileInArchive(archive)) {
		song->Free();
		return nullptr;
	}

	return song;
}

bool
Song::UpdateFileInArchive(ArchiveFile &archive)
{
	assert(parent != nullptr);
	assert(parent->device == DEVICE_INARCHIVE);

	std::string path_utf8(uri);

	for (const Directory *directory = parent;
	     directory->parent != nullptr &&
		     directory->parent->device == DEVICE_INARCHIVE;
	     directory = directory->parent) {
		path_utf8.insert(path_utf8.begin(), '/');
		path_utf8.insert(0, directory->GetName());
	}

	TagBuilder tag_builder;
	if (!tag_archive_scan(archive, path_utf8.c_str(), tag_builder))
		return false;

	tag_builder.Commit(tag);
	return true;
}

#endif

bool
DetachedSong::LoadFile(Path path)
{
	FileInfo fi;
	if (!GetFileInfo(path, fi) || !fi.IsRegular())
		return false;

	TagBuilder tag_builder;
	if (!tag_file_scan(path, tag_builder))
		return false;

	mtime = fi.GetModificationTime();
	tag_builder.Commit(tag);
	return true;
}

bool
DetachedSong::Update()
{
	if (IsAbsoluteFile()) {
		const AllocatedPath path_fs =
			AllocatedPath::FromUTF8(GetRealURI());
		if (path_fs.IsNull())
			return false;

		return LoadFile(path_fs);
	} else if (IsRemote()) {
		TagBuilder tag_builder;
		if (!tag_stream_scan(uri.c_str(), tag_builder))
			return false;

		mtime = 0;
		tag_builder.Commit(tag);
		return true;
	} else
		// TODO: implement
		return false;
}
