#!/bin/bash
# Empaqueta un plugin de Escoita para Linux (x86_64): un .tar.gz con VST3, LV2, CLAP (se o hai),
# a aplicación e un instalador para o usuario (sen sudo).
# Uso: linux.sh <produto.env> <carpeta de artefactos Release> <versión> <carpeta de saída>
# produto.env di NOME, CURTO, BINARIO e DESCRICION.
set -eo pipefail
trap 'echo "::error::$(basename "$0") liña $LINENO: $BASH_COMMAND"' ERR
PRODUTO="$1"; ART="$2"; VER="$3"; OUT="$4"
source "$PRODUTO"
NAME="$NOME"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
WORK="$(mktemp -d)"
D="$WORK/$CURTO-$VER-linux-x86_64"
mkdir -p "$D"
cp -R "$ART/VST3/$NAME.vst3" "$D/"
[ -e "$ART/LV2/$NAME.lv2" ] && cp -R "$ART/LV2/$NAME.lv2" "$D/"
[ -e "$ART/CLAP/$NAME.clap" ] && cp -R "$ART/CLAP/$NAME.clap" "$D/"
cp "$ART/Standalone/$NAME" "$D/$BINARIO"
strip "$D/$BINARIO" "$D/$NAME.vst3/Contents/x86_64-linux/$NAME.so" || true
[ -e "$D/$NAME.lv2/lib$NAME.so" ] && { strip "$D/$NAME.lv2/lib$NAME.so" || true; }

FORMATOS="VST3 (~/.vst3)"
[ -e "$D/$NAME.lv2" ] && FORMATOS="$FORMATOS, LV2 (~/.lv2)"
[ -e "$D/$NAME.clap" ] && FORMATOS="$FORMATOS, CLAP (~/.clap)"

cat > "$D/instalar.sh" <<EOS
#!/bin/sh
# Instala $NAME para o teu usuario (sen sudo)
set -e
cd "\$(dirname "\$0")"
mkdir -p "\$HOME/.vst3" "\$HOME/.lv2" "\$HOME/.clap" "\$HOME/.local/bin" "\$HOME/.local/share/applications"
rm -rf "\$HOME/.vst3/$NAME.vst3" "\$HOME/.lv2/$NAME.lv2" "\$HOME/.clap/$NAME.clap"
cp -R "$NAME.vst3" "\$HOME/.vst3/"
[ -e "$NAME.lv2" ] && cp -R "$NAME.lv2" "\$HOME/.lv2/"
[ -e "$NAME.clap" ] && cp -R "$NAME.clap" "\$HOME/.clap/"
cp $BINARIO "\$HOME/.local/bin/"
cat > "\$HOME/.local/share/applications/$BINARIO.desktop" <<EOD
[Desktop Entry]
Type=Application
Name=$NAME
Comment=$DESCRICION (Escoita)
Exec=\$HOME/.local/bin/$BINARIO
Categories=AudioVideo;Audio;Midi;
EOD
echo "Instalado. Volve abrir o teu DAW e busca «$NAME»; a app está no menú ou en ~/.local/bin/$BINARIO"
EOS
cat > "$D/desinstalar.sh" <<EOS
#!/bin/sh
rm -rf "\$HOME/.vst3/$NAME.vst3" "\$HOME/.lv2/$NAME.lv2" "\$HOME/.clap/$NAME.clap" "\$HOME/.local/bin/$BINARIO" "\$HOME/.local/share/applications/$BINARIO.desktop"
echo "$NAME desinstalado."
EOS
chmod +x "$D/instalar.sh" "$D/desinstalar.sh" "$D/$BINARIO"
NOME_MAIUS="$(echo "$NAME" | tr '[:lower:]' '[:upper:]')"
cat > "$D/LEEME.txt" <<EOS
$NOME_MAIUS para Linux (x86_64)
Unha ferramenta de Escoita · https://www.escoita.link

Instalar:    ./instalar.sh      (para o teu usuario, sen sudo)
Desinstalar: ./desinstalar.sh

Formatos: $FORMATOS e aplicación independente ($BINARIO).
A aplicación precisa ALSA ou JACK; o plugin funciona en calquera DAW con eses formatos
(Bitwig, Reaper, Ardour, Carla…).
EOS
tar -C "$WORK" -czf "$OUT/$CURTO-$VER-linux-x86_64.tar.gz" "$CURTO-$VER-linux-x86_64"
echo "Feito: $OUT/$CURTO-$VER-linux-x86_64.tar.gz"
