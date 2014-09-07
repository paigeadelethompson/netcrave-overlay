# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/sys-apps/kexec-tools/kexec-tools-9999.ebuild,v 1.12 2013/12/28 20:27:31 jlec Exp $

EAPI=5
inherit git-r3

DESCRIPTION="Live kernel patching by SuSE"
HOMEPAGE="https://www.suse.com/promo/kgraft.html"
SRC_URI=""
EGIT_REPO_URI="git://git.kernel.org/pub/scm/linux/kernel/git/jirislaby/kgraft.git"
LICENSE="GPL-2"
SLOT="0"
KEYWORDS=""

#src_prepare() {
#	ln -s x86 arch/amd64
# }


src_compile() {
	make mrproper
}
src_install() {
	dodir /usr/src/kgraft
	cp -rvp * ${ED}/usr/src/kgraft

}
