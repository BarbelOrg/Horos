# Maintainer: KingdomOfCpp
pkgname=horos
pkgver=r3.2deed54
pkgrel=1
pkgdesc="Horos - Environment variable editor GUI application"
arch=('x86_64')
url="https://github.com/BarbelOrg/Horos"
license=('custom')
depends=('qt6-base' 'qt6-declarative')
makedepends=('xmake' 'gcc' 'git' 'qt6-tools')
source=("git+https://github.com/BarbelOrg/horos.git")
sha256sums=('SKIP')

pkgver() {
    cd "$pkgname"
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

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
