# Copyright 1999-2014 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/sys-fs/s3fs/s3fs-1.78.ebuild,v 1.1 2014/09/27 00:04:17 radhermit Exp $

EAPI=5

inherit autotools git-r3

DESCRIPTION="Amazon mounting S3 via fuse"
HOMEPAGE="https://github.com/s3fs-fuse/s3fs-fuse/"
EGIT_REPO_URI="https://github.com/s3fs-fuse/s3fs-fuse.git"
LICENSE="GPL-2"
SLOT="0"
KEYWORDS="testing"

CDEPEND=">=dev-libs/libxml2-2.6:2
	dev-libs/openssl
	>=net-misc/curl-7.0
	>=sys-fs/fuse-2.8.4"
RDEPEND="${CDEPEND}
	app-misc/mime-types"
DEPEND="${CDEPEND}
	virtual/pkgconfig"

RESTRICT="test"

src_prepare() {
	eautoreconf
}

src_configure() {
        econf
}

src_install() {
        default
}

