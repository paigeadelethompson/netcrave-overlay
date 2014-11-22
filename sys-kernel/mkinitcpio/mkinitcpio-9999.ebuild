# Copyright 1999-2014 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/dev-python/libvirt-python/libvirt-python-9999.ebuild,v 1.2 2014/11/17 20:12:56 tamiko Exp $


# TODO - requirements and use 
#    awk (gawk, gawk)
#    bash
#    coreutils
#    filesystem>=2011.10-1
#    findutils
#    grep
#    gzip
#    kmod
#    libarchive
#    mkinitcpio-busybox>=1.19.4-2
#    systemd
#    util-linux>=2.23
#    bzip2 (optional) - Use bzip2 compression for the initramfs image
#    lz4 (optional) - Use lz4 compression for the initramfs image
#    lzop (optional) - Use lzo compression for the initramfs image
#    mkinitcpio-nfs-utils (optional) - Support for root filesystem on NFS
#    xz (optional) - Use lzma or xz compression for the initramfs image
# TODO busybox IUSE (arch has a mkinitcpio-busybox thats specifically for mkinitcpio I think...)

EAPI=5

MY_P="${P/_rc/-rc}"

#inherit eutils distutils-r1

if [[ ${PV} = *9999* ]]; then
   inherit git-r3
   EGIT_REPO_URI="https://github.com/paigeadele/mkinitcpio.git"
   EGIT_BRANCH="gentoo"
   SRC_URI=""
   KEYWORDS=""
else
	SRC_URI=" http://mirror.nl.leaseweb.net/archlinux/core/os/i686/${MY_P}-any.pkg.tar.xz"
	KEYWORDS="~amd64 ~x86"
fi
S="${WORKDIR}/${P%_rc*}"

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
    # TODO should look at the genkernel ebuild for how to make a static busybox build Im pretty sure it rolls its own 
    dosym /bin/busybox /usr/lib/initcpio/busybox 
}

