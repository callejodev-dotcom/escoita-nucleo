#!/bin/bash
# Empaqueta un plugin de Escoita para Mac: un instalador .pkg (universal: Apple Silicon + Intel)
# e un .zip cos bundles soltos.
# Uso: mac.sh <produto.env> <carpeta de artefactos Release> <versión> <carpeta de saída>
#
# produto.env (no plugin, xunto ao LEEME-mac.txt) di NOME, CURTO e ID; ver o LEEME do núcleo.
# Firma: con estas variables de contorno, firma co teu certificado de Apple e notariza (ábrese
# sen avisos); se non, firma «ad hoc» (funciona, pero macOS avisa a primeira vez):
#   MACOS_DEV_ID         "Developer ID Application: Nome (EQUIPO)"
#   MACOS_INSTALLER_ID   "Developer ID Installer: Nome (EQUIPO)"
#   APPLE_ID, APPLE_TEAM_ID, APPLE_APP_PASSWORD   (para notarizar)
set -eo pipefail
trap 'echo "::error::$(basename "$0") liña $LINENO: $BASH_COMMAND"' ERR
PRODUTO="$1"; ART="$2"; VER="$3"; OUT="$4"
source "$PRODUTO"
NAME="$NOME"
DOCS="$(cd "$(dirname "$PRODUTO")" && pwd)"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"   # ruta absoluta: o .zip faise desde outra carpeta
WORK="$(mktemp -d)"
ROOT="$WORK/root"
P="$ROOT/Library/Audio/Plug-Ins"
mkdir -p "$P/VST3" "$P/Components" "$P/CLAP" "$ROOT/Applications"
cp -R "$ART/VST3/$NAME.vst3" "$P/VST3/"
[ -d "$ART/AU/$NAME.component" ] && cp -R "$ART/AU/$NAME.component" "$P/Components/"
[ -d "$ART/CLAP/$NAME.clap" ] && cp -R "$ART/CLAP/$NAME.clap" "$P/CLAP/"
cp -R "$ART/Standalone/$NAME.app" "$ROOT/Applications/"
rmdir "$P/Components" "$P/CLAP" 2>/dev/null || true

# comprobar que é universal
lipo -info "$ROOT/Applications/$NAME.app/Contents/MacOS/$NAME"

SIGN_ID="${MACOS_DEV_ID:--}"
for b in "$P/VST3/$NAME.vst3" "$P/Components/$NAME.component" "$P/CLAP/$NAME.clap" "$ROOT/Applications/$NAME.app"; do
  [ -e "$b" ] || continue
  if [ "$SIGN_ID" = "-" ]; then
    codesign --force --deep --sign - "$b"
  else
    codesign --force --deep --options runtime --timestamp --sign "$SIGN_ID" "$b"
  fi
done

# .pkg: que o instalador non «recoloque» os bundles se atopa outra copia no disco
pkgbuild --analyze --root "$ROOT" "$WORK/comp.plist"
i=0
while /usr/libexec/PlistBuddy -c "Print :$i" "$WORK/comp.plist" >/dev/null 2>&1; do
  /usr/libexec/PlistBuddy -c "Delete :$i:BundleIsRelocatable" "$WORK/comp.plist" >/dev/null 2>&1 || true
  /usr/libexec/PlistBuddy -c "Add :$i:BundleIsRelocatable bool false" "$WORK/comp.plist"
  i=$((i+1))
done
pkgbuild --root "$ROOT" --component-plist "$WORK/comp.plist" --identifier "$ID" \
         --version "$VER" --install-location / "$WORK/$CURTO-component.pkg"

PKG="$OUT/$CURTO-$VER-mac.pkg"
if [ -n "$MACOS_INSTALLER_ID" ]; then
  productbuild --package "$WORK/$CURTO-component.pkg" --sign "$MACOS_INSTALLER_ID" "$PKG"
else
  productbuild --package "$WORK/$CURTO-component.pkg" "$PKG"
fi

# notarizar (só cunha conta de desenvolvedor de Apple)
if [ -n "$APPLE_ID" ] && [ -n "$APPLE_TEAM_ID" ] && [ -n "$APPLE_APP_PASSWORD" ] && [ -n "$MACOS_INSTALLER_ID" ]; then
  xcrun notarytool submit "$PKG" --apple-id "$APPLE_ID" --team-id "$APPLE_TEAM_ID" --password "$APPLE_APP_PASSWORD" --wait
  xcrun stapler staple "$PKG"
fi

# .zip cos bundles soltos, para quen prefira copialos á man
ZIPDIR="$WORK/$CURTO-$VER-mac"
mkdir -p "$ZIPDIR"
for b in "$P/VST3/$NAME.vst3" "$P/Components/$NAME.component" "$P/CLAP/$NAME.clap" "$ROOT/Applications/$NAME.app"; do
  [ -e "$b" ] && cp -R "$b" "$ZIPDIR/"
done
[ -f "$DOCS/LEEME-mac.txt" ] && cp "$DOCS/LEEME-mac.txt" "$ZIPDIR/LEEME.txt"
(cd "$WORK" && ditto -c -k --sequesterRsrc --keepParent "$CURTO-$VER-mac" "$OUT/$CURTO-$VER-mac.zip")
echo "Feito: $PKG e $OUT/$CURTO-$VER-mac.zip"
