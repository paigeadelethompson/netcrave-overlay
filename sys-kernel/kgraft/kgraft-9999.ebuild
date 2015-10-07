# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=5
inherit git-r3

DESCRIPTION="Live kernel patching by SuSE"
HOMEPAGE="https://www.suse.com/promo/kgraft.html"
SRC_URI=""
EGIT_BRANCH="kgraft"
EGIT_REPO_URI="git://git.kernel.org/pub/scm/linux/kernel/git/jirislaby/kgraft.git"
LICENSE="GPL-2"
SLOT="0"
KEYWORDS="netcrave"

src_prepare() {
	ln -s x86 arch/amd64
}

src_compile() {
	make mrproper
}
src_install() {
	dodir /usr/src/kgraft
	cp -rvp * ${ED}/usr/src/kgraft

}
