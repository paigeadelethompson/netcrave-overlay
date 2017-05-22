EAPI=5
inherit git-2 eutils

DESCRIPTION="Gerbil Scheme"
HOMEPAGE="https://github.com/vyzo/gerbil"

EGIT_REPO_URI="https://github.com/vyzo/gerbil.git"
SRC_URI=""

LICENSE="LGPL-2.1"

SLOT="0"
KEYWORDS="netcrave"

RDEPEND="dev-scheme/gambit"

src_compile() {
	cd src
	./build.sh || die "gerbil build script failed"
	cd ..
        epatch "${FILESDIR}/GERBIL_HOME.patch"

}

src_install() {
	insinto /opt/gerbil
	exeinto /opt/gerbil/bin
	doins -r bootstrap etc lib src
	doexe bin/gxc bin/gxi
	dosym /opt/gerbil/bin/gxc /usr/bin/gxc
	dosym /opt/gerbil/bin/gxi /usr/bin/gxi
	dosym /opt/gerbil/bin/gxi /usr/bin/gxi-script
	dosym /opt/gerbil/etc/gerbil.el /etc/gerbil.el
	dodoc ChangeLog README.md doc/guide.md doc/srfi.md
}

