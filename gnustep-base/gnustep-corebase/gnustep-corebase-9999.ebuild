# Copyright 1999-2012 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/gnustep-base/libobjc2/libobjc2-1.6.1.ebuild,v 1.1 2012/07/25 12:11:23 voyageur Exp $

EAPI=4
inherit multilib multilib-build subversion

DESCRIPTION="GNUstep CoreBase framework"
HOMEPAGE="http://www.gnustep.org"
SRC_URI=""

ESVN_REPO_URI="svn://svn.gna.org/svn/gnustep/libs/corebase/trunk"
ESVN_PROJECT="corebase"

LICENSE="LGPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86 ~arm"
IUSE="multilib debug"

RDEPEND="=gnustep-base/gnustep-base-9999"
DEPEND="${RDEPEND}
	>=sys-devel/clang-2.9
	=gnustep-base/gnustep-make-9999"

src_unpack() {
	subversion_src_unpack
}

src_prepare() {
	multilib_copy_sources
}

my_configure() {
        cd "${BUILD_DIR}" || die
        case "${ABI}" in
                         x86)
			   
			     export OBJCFLAGS="-fblocks" 
			     export CC="clang -m32" 
			     export CXX="clang++ -m32"
			     # this doesnt help, its the /usr/lib dir isn't getting set right by the 
			     # config script:
			     # export LDFLAGS="${LDFLAGS} -Wl,-b,elf32-i386,-A,i386" for some reason
			     # it keeps going to lib64 instead of lib32
                             
			     econf $(use_enable debug) --libdir=/usr/lib32
			     
			     # even this wont fix it
                             # makefile_path="Source/GNUmakefile"
			     # sed -i 's/lib64/lib32/g' "$makefile_path" || die
		             # so idfk. seems pretty hopeless
			
			     ;;
                         amd64)
			     # try skipping amd64 completely until 32 bit works
			     return
			     export OBJCFLAGS="-fblocks" 
			     export CC="clang" 
			     export CXX="clang++" 
		             econf $(use_enable debug) 
                             ;;
                         *)
                             echo "${ABI}" && die
                             ;;
         esac
}

src_configure() {
	multilib_foreach_abi my_configure
}

my_compile() {
        cd "${BUILD_DIR}" || die
	case "${ABI}" in
                         x86)
			     export OBJCFLAGS="-fblocks"
                             export CC="clang -m32"
                             export CXX="clang++ -m32"
			     emake
                             ;;
                         amd64)
			     return
		             export OBJCFLAGS="-fblocks"
                             export CC="clang" 
                             export CXX="clang++"
			     emake
                             ;;
                         *)
                             echo "${ABI}" && die
                             ;;
         esac

}

src_compile() {
	multilib_foreach_abi my_compile
}

my_install() {
        cd "${BUILD_DIR}" || die
	case "${ABI}" in
			x86)	
				emake DESTDIR="${D}" GNUSTEP_INSTALLATION_DOMAIN=SYSTEM install			
				dodoc README ChangeLog
				;;
			amd64)
				return
				;;
			*)
				echo "${ABI}" && die
				;;
	esac
}

src_install() {
	multilib_foreach_abi my_install
}
