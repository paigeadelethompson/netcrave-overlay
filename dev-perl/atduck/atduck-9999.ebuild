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
src_compile() { 
	pod2man atduck.pod > atduck.7
	dodir /usr/share/ATduck
	dodir /usr/share/man/man7
	cp lib/ATduck ${ED}/usr/share/ATduck
	cp -r slirp/ ${ED}/usr/share/ATduck
	cp atduck.7 ${ED}/usr/share/man/man7
}
