# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=5

inherit git-2 autotools multilib multilib-build autotools-multilib
#cmake-multilib

MULTILIB_COMPAT=( abi_x86_{32,64} )
DESCRIPTION="libpthread_workqueue is a portable implementation of the pthread_workqueue API first introduced in Mac OS X."
HOMEPAGE="http://sourceforge.net/projects/libpwq/"
#SRC_URI="mirror://sourceforge/libpwq/${P}.tar.gz"
EGIT_REPO_URI="https://github.com/mheily/libpwq.git"

LICENSE="BSD"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""

DEPEND=""
RDEPEND="${DEPEND}"

src_unpack() {
	git-2_src_unpack
}

src_prepare() {
	multilib_copy_sources
}

my_configure() {
	libtoolize
	autoreconf
	automake --add-missing
	autoreconf 
	#.... ^ .... fix
	autotools-utils_src_configure 
#"${@}"; sed -i "/CFLAGS/s|\$| -D_XOPEN_SOURCE=600 -D__EXTENSIONS__ -D_GNU_SOURCE -std=c99 -I../${PF}/include -I../${PF}/src/|" config.mk
#	cp ../${PF}/Makefile ../${PF}-${ABI}/Makefile
#	sed -i 's,src/,../${PF}/src/,g' "../${PF}-${ABI}/config.mk"
}

src_configure() {
	multilib_parallel_foreach_abi my_configure
	#cmake-multilib_src_configure
}

src_compile() {
	multilib_foreach_abi make 
}

src_install() {
	#emake DESTDIR="${D}" install
	autotools-multilib_src_install
	#cmake-multilib_src_install
	dodoc ChangeLog
}

