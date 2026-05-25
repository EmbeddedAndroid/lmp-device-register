/*
 * Copyright (c) 2026 Tyler Baker
 *
 * SPDX-License-Identifier: MIT
 */

#include "ostree_cmdline.h"

#include <cstdio>
#include <cstdlib>
#include <string>

static int g_failures = 0;

static void check(const char *name, const std::string &expected,
		  const std::string &actual)
{
	if (expected == actual) {
		std::printf("ok    %s\n", name);
		return;
	}
	std::printf("FAIL  %s: expected=\"%s\" actual=\"%s\"\n",
		    name, expected.c_str(), actual.c_str());
	g_failures++;
}

int main(void)
{
	/* Stock LmP boot shape. */
	check("lmp boot.0",
	      "lmp",
	      parse_ostree_osname_from_cmdline(
		      "ro root=PARTUUID=abc "
		      "ostree=/ostree/boot.0/lmp/9fa5/0 quiet"));

	/* Stock LmP rollback slot. */
	check("lmp boot.1",
	      "lmp",
	      parse_ostree_osname_from_cmdline(
		      "ostree=/ostree/boot.1/lmp/9fa5/0"));

	/* qcom-distro-sota deploys under nodistro. The "No merge
	 * deployment" failure mode this whole change targets. */
	check("qcom nodistro",
	      "nodistro",
	      parse_ostree_osname_from_cmdline(
		      "ro ostree=/ostree/boot.0/nodistro/abcd1234/0 "
		      "console=ttyMSM0,115200n8"));

	/* Pre-boot/initramfs shape: /ostree/deploy/<os>/deploy/<sum>.N. */
	check("deploy shape",
	      "lmp",
	      parse_ostree_osname_from_cmdline(
		      "ostree=/ostree/deploy/lmp/deploy/9fa5.0"));

	/* Token surrounded by tabs and a trailing newline. */
	check("tab-separated",
	      "lmp",
	      parse_ostree_osname_from_cmdline(
		      "ro\tostree=/ostree/boot.0/lmp/9fa5/0\n"));

	/* Multiple kernel arguments after ostree=. */
	check("trailing args",
	      "lmp",
	      parse_ostree_osname_from_cmdline(
		      "ostree=/ostree/boot.0/lmp/9fa5/0 init=/sbin/init"));

	/* No ostree= token at all - returns empty so caller falls back
	 * to the server-supplied default. */
	check("no ostree token",
	      "",
	      parse_ostree_osname_from_cmdline(
		      "ro root=PARTUUID=abc init=/sbin/init"));

	/* Malformed path: too few components. */
	check("short path",
	      "",
	      parse_ostree_osname_from_cmdline("ostree=/ostree/boot.0"));

	/* Unknown shape (second component is neither 'deploy' nor
	 * 'boot.<n>') - silent failure, returns "". */
	check("unknown shape",
	      "",
	      parse_ostree_osname_from_cmdline(
		      "ostree=/ostree/garbage/lmp/9fa5/0"));

	/* Empty osname component - silent failure. */
	check("empty osname",
	      "",
	      parse_ostree_osname_from_cmdline(
		      "ostree=/ostree/boot.0//9fa5/0"));

	/* Empty cmdline. */
	check("empty cmdline",
	      "",
	      parse_ostree_osname_from_cmdline(""));

	/* Non-LmP OS name with hyphens preserved verbatim. */
	check("hyphenated osname",
	      "my-custom-os",
	      parse_ostree_osname_from_cmdline(
		      "ostree=/ostree/boot.0/my-custom-os/abcd/0"));

	if (g_failures > 0) {
		std::printf("\n%d test(s) failed\n", g_failures);
		return EXIT_FAILURE;
	}
	std::printf("\nall tests passed\n");
	return EXIT_SUCCESS;
}
