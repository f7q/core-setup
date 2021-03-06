// Copyright (c) .NET Foundation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pal.h"
#include "utils.h"
#include "deps_entry.h"
#include "trace.h"


bool deps_entry_t::to_path(const pal::string_t& base, bool look_in_base, pal::string_t* str) const
{
    pal::string_t& candidate = *str;

    candidate.clear();

    // Base directory must be present to obtain full path
    if (base.empty())
    {
        return false;
    }

    // Entry relative path contains '/' separator, sanitize it to use
    // platform separator. Perf: avoid extra copy if it matters.
    pal::string_t pal_relative_path = relative_path;
    if (_X('/') != DIR_SEPARATOR)
    {
        replace_char(&pal_relative_path, _X('/'), DIR_SEPARATOR);
    }

    // Reserve space for the path below
    candidate.reserve(base.length() +
        pal_relative_path.length() + 3);

    candidate.assign(base);
    pal::string_t sub_path = look_in_base ? get_filename(pal_relative_path) : pal_relative_path;
    append_path(&candidate, sub_path.c_str());

    bool exists = pal::file_exists(candidate);
    const pal::char_t* query_type = look_in_base ? _X("Local") : _X("Relative");
    if (!exists)
    {
        trace::verbose(_X("    %s path query did not exist %s"), query_type, candidate.c_str());
        candidate.clear();
    }
    else
    {
        trace::verbose(_X("    %s path query exists %s"), query_type, candidate.c_str());
    }
    return exists;
}

// -----------------------------------------------------------------------------
// Given a "base" directory, yield the local path of this file
//
// Parameters:
//    base - The base directory to look for the relative path of this entry
//    str  - If the method returns true, contains the file path for this deps
//           entry relative to the "base" directory
//
// Returns:
//    If the file exists in the path relative to the "base" directory.
//
bool deps_entry_t::to_dir_path(const pal::string_t& base, pal::string_t* str) const
{
    if (asset_type == asset_types::resources)
    {
        pal::string_t pal_relative_path = relative_path;
        if (_X('/') != DIR_SEPARATOR)
        {
            replace_char(&pal_relative_path, _X('/'), DIR_SEPARATOR);
        }

        // Resources are represented as "<ietf-code>/<ResourceAssemblyName.dll>" in the deps.json.
        // The <ietf-code> is the "directory" in the pal_relative_path below, so extract it.
        pal::string_t ietf_dir = get_directory(pal_relative_path);
        pal::string_t ietf = ietf_dir;

        // get_directory returns with DIR_SEPERATOR appended that we need to remove.
        auto sep_pos = ietf.find_last_of(DIR_SEPARATOR);
        if (sep_pos != pal::string_t::npos)
        {
            ietf = ietf.erase(sep_pos, pal::string_t::npos);
        }
        
        pal::string_t base_ietf_dir = base;
        append_path(&base_ietf_dir, ietf.c_str());
        trace::verbose(_X("Detected a resource asset, will query dir/ietf-tag/resource base: %s asset: %s"), base_ietf_dir.c_str(), asset_name.c_str());
        return to_path(base_ietf_dir, true, str);
    }
    return to_path(base, true, str);
}
// -----------------------------------------------------------------------------
// Given a "base" directory, yield the relative path of this file in the package
// layout.
//
// Parameters:
//    base - The base directory to look for the relative path of this entry
//    str  - If the method returns true, contains the file path for this deps
//           entry relative to the "base" directory
//
// Returns:
//    If the file exists in the path relative to the "base" directory.
//
bool deps_entry_t::to_rel_path(const pal::string_t& base, pal::string_t* str) const
{
    return to_path(base, false, str);
}

// -----------------------------------------------------------------------------
// Given a "base" directory, yield the relative path of this file in the package
// layout.
//
// Parameters:
//    base - The base directory to look for the relative path of this entry
//    str  - If the method returns true, contains the file path for this deps
//           entry relative to the "base" directory
//
// Returns:
//    If the file exists in the path relative to the "base" directory.
//
bool deps_entry_t::to_full_path(const pal::string_t& base, pal::string_t* str) const
{
    str->clear();

    // Base directory must be present to obtain full path
    if (base.empty())
    {
        return false;
    }

    pal::string_t new_base = base;

    if (library_path.empty())
    {
        append_path(&new_base, library_name.c_str());
        append_path(&new_base, library_version.c_str());
    }
    else
    {
        append_path(&new_base, library_path.c_str());
    }

    return to_rel_path(new_base, str);
}