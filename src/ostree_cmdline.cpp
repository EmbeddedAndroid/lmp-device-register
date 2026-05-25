/*
 * Copyright (c) 2026 Tyler Baker
 *
 * SPDX-License-Identifier: MIT
 */

#include "ostree_cmdline.h"

#include <vector>

#include <boost/tokenizer.hpp>

/*
 * parse_ostree_osname_from_cmdline extracts the booted ostree
 * deployment's OS name from a /proc/cmdline-style string.
 *
 * ostreed sets the kernel command line to include an `ostree=` token
 * pointing at the deployment that owns the currently-booted rootfs.
 * Two shapes are observed in the wild:
 *
 *   ostree=/ostree/boot.<N>/<osname>/<checksum>/<index>
 *     (the kernel's view, set by ostree-finalize-staged on boot)
 *
 *   ostree=/ostree/deploy/<osname>/deploy/<checksum>.<index>
 *     (some pre-boot stages, e.g. initramfs)
 *
 * <osname> is at index 2 of the slash-separated path components in
 * both layouts.
 *
 * Returns "" if no `ostree=` token is found, the path does not match
 * one of the expected shapes, or the OS name component is empty.
 * Failures are silent so that registration falls back to the server-
 * supplied sota.toml default (current behaviour) rather than aborting.
 */
std::string parse_ostree_osname_from_cmdline(const std::string &cmdline)
{
	static const std::string token = "ostree=";

	std::string::size_type pos = cmdline.find(token);
	if (pos == std::string::npos)
		return "";

	pos += token.size();
	std::string::size_type end = cmdline.find_first_of(" \t\n\r", pos);
	std::string path = cmdline.substr(pos,
		end == std::string::npos ? std::string::npos : end - pos);

	boost::char_separator<char> sep("/");
	boost::tokenizer<boost::char_separator<char>> tok(path, sep);
	std::vector<std::string> parts(tok.begin(), tok.end());
	if (parts.size() < 3 || parts[0] != "ostree")
		return "";

	const std::string &second = parts[1];
	const bool deploy_shape = second == "deploy";
	const bool boot_shape   = second.rfind("boot.", 0) == 0;
	if (!deploy_shape && !boot_shape)
		return "";

	const std::string &osname = parts[2];
	if (osname.empty())
		return "";

	return osname;
}
