# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/net-irc/anope/anope-1.8.7-r1.ebuild,v 1.2 2013/02/18 18:48:48 gurligebis Exp $

EAPI=4

inherit cmake-utils eutils git-r3 multilib versionator user

DESCRIPTION="Anope IRC Services"
HOMEPAGE="http://www.anope.org"
SRC_URI=""
EGIT_REPO_URI="https://github.com/anope/anope.git"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~mainline ~amd64 ~x86"
IUSE="mysql"

DEPEND="mysql? ( virtual/mysql )"
RDEPEND="${DEPEND}"

pkg_pretend() {
	local replaced_version
	for replaced_version in ${REPLACING_VERSIONS} ; do
		if ! version_is_at_least 1.8.7 ${replaced_version} && [[ -f ${ROOT}opt/anope/data/nick.db ]]; then
			eerror "It looks like you have an older version of Anope installed."
			eerror "To upgrade, shut down Anope and copy your databases to"
			eerror "${ROOT}var/lib/anope and your configuration to ${ROOT}etc/anope"
			eerror "You can do this by:"
			eerror "# mkdir -p ${ROOT}var/lib/anope ${ROOT}etc/anope"
			eerror "# chown anope:anope ${ROOT}var/lib/anope"
			eerror "# mv ${ROOT}opt/anope/data/*.db ${ROOT}var/lib/anope"
			eerror "# mv ${ROOT}opt/anope/data/services.conf ${ROOT}etc/anope"
			die "Please move your anope database files from /opt/anope/data"
		fi
	done
}

pkg_setup() {
	enewgroup anope
	enewuser anope -1 -1 -1 anope
}

src_configure() {
#	local mycmakeargs=(
#		-DCMAKE_INSTALL_PREFIX="${D}/usr"
#	)
	cmake-utils_src_configure

}

src_install() {
	cmake-utils_src_install
}


