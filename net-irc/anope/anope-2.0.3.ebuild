# Copyright 1999-2016 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Id$

EAPI=6

inherit cmake-utils eutils multilib versionator user

DESCRIPTION="Anope IRC Services"
HOMEPAGE="https://anope.org"
SRC_URI="https://github.com/anope/anope/releases/download/${PV}/${P}-source.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE="mysql sqlite pcre posix gnutls ssl tre ldap anope_modules_sql_auth
	anope_modules_sql_log anope_modules_sql_oper anope_modules_ldap_auth
	anope_modules_ldap_oper"

REQUIRED_USE="anope_modules_sql_auth? ( || ( mysql sqlite ) )
	anope_modules_sql_oper? ( || ( mysql sqlite ) )
	anope_modules_sql_log? ( || ( mysql sqlite ) )
	anope_modules_ldap_auth? ( ldap )
	anope_modules_ldap_oper? ( ldap )"


DEPEND="
	mysql? ( virtual/mysql )
	ssl? ( dev-libs/openssl:= )
	gnutls? ( net-libs/gnutls dev-libs/libgcrypt:0 )
	ldap? ( net-nds/openldap )
	pcre? ( dev-libs/libpcre )
	sqlite? ( >=dev-db/sqlite-3.0 )
	tre? ( dev-libs/tre )"
RDEPEND="${DEPEND}"

pkg_setup() {
	enewgroup anope
	enewuser anope -1 -1 -1 anope
}

src_unpack() {
	unpack ${A}
	mv "${P}-source" "${P}" || die
}

anope_use_extra() {
	use $1 && ln -s "${P}/modules/extra/$2.cpp" "${P}/modules/$2.cpp"
}

src_prepare() {	
	#use mysql && ln -s ./modules/m_mysql.cpp ./modules/extra/m_mysql.cpp
	anope_use_extra mysql "m_mysql"
	anope_use_extra sqlite "m_sqlite"
	anope_use_extra ssl "m_ssl_openssl"
	anope_use_extra gnutls "m_ssl_gnutls"
	anope_use_extra posix "m_regex_posix"
	anope_use_extra pcre "m_regex_pcre"
	anope_use_extra tre "m_regex_tre"
	anope_use_extra ldap "m_ldap"
	anope_use_extra anope_modules_sql_auth "m_sql_authentication"
	anope_use_extra anope_modules_sql_log "m_sql_log"
	anope_use_extra anope_modules_sql_oper "m_sql_oper"
	anope_use_extra anope_modules_ldap_auth "m_ldap_authentication"
	anope_use_extra anope_modules_ldap_oper "m_ldap_oper"

	unset -f anope_use_extra

	eapply_user
}

src_configure() {
	local mycmakeargs=(
		-DCMAKE_INSTALL_PREFIX="${ROOT}"
		-DBIN_DIR="${ROOT}usr/bin"
		-DDB_DIR="${ROOT}var/lib/anope"
		-DCONF_DIR="${ROOT}etc/anope/conf"
		-DLIB_DIR="${ROOT}usr/lib/anope"
		-DLOCALE_DIR="${ROOT}etc/anope/locale"
		-DLOGS_DIR="${ROOT}var/log/anope"
	)
	cmake-utils_src_configure
}

src_install() {
	cmake-utils_src_install
}
