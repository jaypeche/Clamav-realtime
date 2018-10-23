# Copyright 1999-2018 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header:


EAPI="4"

inherit eutils linux-info git-2 xdg-utils

DESCRIPTION="clamav-realtime: ClamAV realtime scanner"
HOMEPAGE="http://www.pingwho.org"

EGIT_REPO_URI="git://github.com/jaypeche/Clamav-realtime.git"
EGIT_BRANCH="master"
EGIT_COMMIT="master"
GIT_SRC_PREFIX="${S}/${PN}-${PV}"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64 ~x86 ~arm"
IUSE="+X +gtk"

DEPEND="sys-libs/glibc
	x11-libs/gtk+
        >=x11-libs/libnotify-0.7.5-r1
        >=app-antivirus/clamav-0.97.8-r2
        >=sys-fs/inotify-tools-3.13-r1"
RDEPEND="${DEPEND}"


src_unpack() {
    git-2_src_unpack ${A}
    cd "${GIT_SRC_PREFIX}"
    epatch "${FILESDIR}/${PN}-gentoo.patch" || die "epatch failed !"
    epatch "${FILESDIR}/${PN}_quarantine_mountpoint_fix.patch" || die "epatch failed"
}

pkg_setup() {
	if ! linux_config_exists; then
		eerror "Kernel configuration file doesn't exist."
	elif ! linux_chkconfig_present INOTIFY_USER; then
		eerror "WARNING : CONFIG_INOTIFY_USER not enabled in kernel."
	fi
}

src_compile() {
     cd "${GIT_SRC_PREFIX}"
    if [ -f Makefile ] || [ -f GNUmakefile ] || [ -f makefile ]; then
        emake || die "emake failed"
    fi
}

src_install() {
    cd "${GIT_SRC_PREFIX}"
    emake DESTDIR="${D}" install || die "einstall failed"
}

pkg_postinst() {
    einfo
    xdg_desktop_database_update || die "Update database failed !"

    einfo
    einfo "NOTE: clamav-realtime was installed in /usr/local/bin"
    einfo "      For security, and for a better graphical integration,"
    einfo "      it must be run as user !"
    einfo
}
