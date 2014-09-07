# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/sys-apps/kexec-tools/kexec-tools-9999.ebuild,v 1.12 2013/12/28 20:27:31 jlec Exp $

EAPI=5

#AUTOTOOLS_AUTORECONF=true

#TODO required kernel config CONFIG_FUNCTION_TRACER, CONFIG_HAVE_FENTRY, CONFIG_MODULES, CONFIG_SYSFS, CONFIG_KALLSYMS_ALL
#TODO symlink has to be created: arch/amd64/ - > arch/x86 (not x86_64?) http://forums.gentoo.org/viewtopic-p-6613753.html x86_64/ just has boot/bzImage..

inherit git-r3 linux-info

DESCRIPTION="dynamic kernel patching"
HOMEPAGE="https://github.com/dynup/kpatch"
SRC_URI=""
EGIT_REPO_URI="https://github.com/dynup/kpatch.git"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS=""

src_compile() {
    if [ -f Makefile ] || [ -f GNUmakefile ] || [ -f makefile ]; then
        emake || die "emake failed"
    fi
}

src_install() {
	if [ -f Makefile ] || [ -f GNUmakefile] || [ -f makefile ] ; then
		emake DESTDIR="${D}" install
	fi

	if ! declare -p DOCS >/dev/null 2>&1 ; then
		local d
		for d in README* ChangeLog AUTHORS NEWS TODO CHANGES THANKS BUGS \
				FAQ CREDITS CHANGELOG ; do
			[[ -s "${d}" ]] && dodoc "${d}"
		done
	elif declare -p DOCS | grep -q "^declare -a " ; then
		dodoc "${DOCS[@]}"
	else
		dodoc ${DOCS}
	fi
}
