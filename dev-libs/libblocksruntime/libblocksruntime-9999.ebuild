# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=5

inherit git-2 multilib-build multilib
MULTILIB_COMPAT=( abi_x86_{32,64} )

DESCRIPTION="https://github.com/mackyle/blocksruntime.git"
HOMEPAGE="http://nickhutchinson.me/libdispatch"
SRC_URI=""

LICENSE="MIT"
SLOT="0"
KEYWORDS="~amd64 ~x86"
IUSE=""

EGIT_REPO_URI="https://github.com/mackyle/blocksruntime.git"

src_unpack() {
	git-2_src_unpack	
}

src_prepare() {
        multilib_copy_sources
}

ustr_make() {
        cd "${BUILD_DIR}" || die

	        case "$ABI" in
	                 x86)
			     CFLAGS="-m32 -O2"
	                     ;;
	                 amd64)
			     CFLAGS="-march=x86-64 -O2"
	                     ;;
	                 *)
			     echo $ABI
	                     ;;
	       	esac
#PUAH!!!
		echo $ABI
	        AR="`which $(echo $(tc-getAR) | awk -F ' ' '{print $1}')`" \
		CC="`which $(echo $(tc-getCC) | awk -F ' ' '{print $1}')`" \
                LDFLAGS="${LDFLAGS}" \
                prefix="${EPREFIX}/usr" \
                SHRDIR="/usr/share/${P}" \
                HIDE= ./buildlib || die
}

ustr_install() {
        cd "${BUILD_DIR}" || die
	DESTDIR="${D}" \
                prefix="${EPREFIX}/usr" \
                libdir="${EPREFIX}/usr/$(get_libdir)" \
                mandir="/usr/share/man" \
                SHRDIR="/usr/share/${P}" \
                DOCSHRDIR="/usr/share/doc/${PF}" \
                HIDE= ./installlib || die
}

src_compile() {
        multilib_foreach_abi ustr_make all-shared
}

#multilib_src_test() {
#        multilib_foreach_abi ustr_make check
#}

src_install() {
        multilib_foreach_abi ustr_install install-multilib-linux
#        dodoc ChangeLog README README-DEVELOPERS AUTHORS NEWS TODO
	 echo true
}

