# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=5
inherit git-r3

DESCRIPTION="Torvalds/linux"
HOMEPAGE="http://kernel.org"
SRC_URI=""
EGIT_BRANCH="master"
EGIT_REPO_URI="https://github.com/torvalds/linux.git"
LICENSE="GPL-2"
SLOT="0"
KEYWORDS=""

src_prepare() {
	ln -s x86 arch/amd64
}

src_compile() {
	make mrproper
}
src_install() {
	dodir /usr/src/git-sources
	cp -rvp * ${ED}/usr/src/git-sources
}
