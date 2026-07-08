# Maintainer: KingdomOfCpp
pkgname=horos
pkgver="1.0.0"
pkgrel=2
pkgdesc="Horos - Environment variable editor tool"
arch=('x86_64')
url="https://github.com/BarbelOrg/Horos"
license=('custom')
depends=('qt6-base' 'qt6-declarative')
makedepends=('xmake' 'gcc' 'git' 'qt6-tools')
source=("git+https://github.com/BarbelOrg/horos.git")
sha256sums=('SKIP')

build() {
    cd "$pkgname"
    export XMAKE_ROOT=y
    xmake f -m release --toolchain=gcc
    xmake -y
}

package() {
    cd "$pkgname"
    export XMAKE_ROOT=y
    xmake install -o "$pkgdir/usr" -y
}
