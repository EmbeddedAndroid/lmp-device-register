/*
 * Copyright (c) 2026 Tyler Baker
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef OSTREE_CMDLINE_H
#define OSTREE_CMDLINE_H

#include <string>

/*
 * Parse a /proc/cmdline-style string and return the booted ostree
 * deployment's OS name, or "" if it cannot be determined. Pure
 * function; no I/O, no globals, safe to unit-test.
 */
std::string parse_ostree_osname_from_cmdline(const std::string &cmdline);

#endif
