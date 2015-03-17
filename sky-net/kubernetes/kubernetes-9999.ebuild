# Copyright 1999-2014 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/net-irc/unrealircd/unrealircd-3.2.10.4.ebuild,v 1.2 2014/09/30 07:54:01 nimiux Exp $

EAPI=4

inherit git-r3

DESCRIPTION="Container Cluster Manager from Google"
HOMEPAGE="http://kubernetes.io"
SRC_URI=""
SLOT="0"

EGIT_REPO_URI="https://github.com/GoogleCloudPlatform/kubernetes.git"

LICENSE="GPL-2"

KEYWORDS="experimental"

#src_prepare() {
#}

src_unpack() {
	git-r3_fetch
	git-r3_checkout
}

#src_configure() {
#
#}

src_compile() {
	die
	make
}

src_install() {
	make install
}
