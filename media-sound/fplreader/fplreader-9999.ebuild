EAPI=5
inherit git-2

DESCRIPTION="foobar2000 FPL binary playlist parser and converter"
HOMEPAGE="https://dex.ycnrg.org/display/ycdocs/fplreader"

EGIT_REPO_URI="https://github.com/yellowcrescent/fplreader.git"
SRC_URI=""

LICENSE="LGPL-3"

SLOT="0"
KEYWORDS="netcrave"


src_compile() {
       gcc fplreader.cpp -o fplreader
}

src_install() {
	dobin fplreader
}

