# Copyright 1999-2014 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/app-text/libmwaw/libmwaw-0.3.1.ebuild,v 1.2 2014/10/10 20:17:48 maekke Exp $

EAPI=4

inherit eutils git-r3 versionator multilib user

DESCRIPTION="Library parsing many pre-OSX MAC text formats"

HOMEPAGE="https://github.com/umanwizard/libpagemaker/"

SRC_URI=""
EGIT_REPO_URI="https://github.com/umanwizard/libpagemaker.git"

LICENSE="LGPL-2.1"
SLOT="0"
KEYWORDS="~amd64 ~arm ~x86"
#IUSE="doc static-libs"

S=${WORKDIR}/libpagemaker-${PV}

src_prepare() {
	autoreconf
}

src_configure() {
	econf 
}

src_install() {
	default
}
