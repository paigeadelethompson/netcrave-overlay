EAPI=5
inherit git-2 eutils

DESCRIPTION="SSCEP is a command line client for the SCEP protocol"

HOMEPAGE="http://www.cynops.de/oss/CertNanny/"

EGIT_REPO_URI="https://github.com/certnanny/sscep.git"
SRC_URI=""

#LICENSE="LGPL-3"

SLOT="0"
KEYWORDS="netcrave"

src_prepare() {
	epatch "${FILESDIR}/Makefile.patch"
}

src_configure() {
	./Configure
}

src_install() {
	insinto /opt/sscep
	exeinto /opt/sscep/bin
	doins sscep.conf
	doexe sscep_dyn
	dodoc README TODO VERSION COPYRIGHT HISTORY
	dosym /opt/sscep/bin/sscep_dyn /usr/bin/sscep
}

