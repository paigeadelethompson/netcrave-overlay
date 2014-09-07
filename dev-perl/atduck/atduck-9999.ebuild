# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/sys-apps/kexec-tools/kexec-tools-9999.ebuild,v 1.12 2013/12/28 20:27:31 jlec Exp $

EAPI=5

inherit git-r3

DESCRIPTION="Hayes AT-compatible modem emulator"
HOMEPAGE="https://github.com/nandhp/atduck"
SRC_URI=""
EGIT_REPO_URI="https://github.com/nandhp/atduck.git"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS=""

#SRC_TEST="do"

DEPEND="dev-perl/IO-Tty"

#TODO can use SLiRP? 
src_prepare() { 
	pod2man atduck.pod > atduck.7
	mkdir -p share/ATduck
	mkdir -p share/man/man7
	cp lib/ATduck share/ATduck
	cp -r slirp/ share/ATduck
	cp atduck.7 share/man/man7
}
src_compile() {
	if [ -f Makefile ] || [ -f GNUmakefile ] || [ -f makefile ]; then
		emake || die "emake failed"
	fi
}
