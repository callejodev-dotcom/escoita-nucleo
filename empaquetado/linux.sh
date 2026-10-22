#!/bin/bash
# Empaqueta un plugin de Escoita (ou unha serie de plugins) para Linux (x86_64): un .tar.gz con
# VST3, LV2, CLAP (se o hai), a aplicación (se a hai) e un instalador para o usuario (sen sudo).
# Uso: linux.sh <produto.env> <carpeta da compilación> <versión> <carpeta de saída>
# produto.env di NOME, CURTO, BINARIO e DESCRICION e, nunha serie, PLUGINS. Os artefactos de cada
# plugin están en <compilación>/<Destino>_artefacts/Release.
set -eo pipefail
trap 'echo "::error::$(basename "$0") liña $LINENO: $BASH_COMMAND"' ERR
PRODUTO="$1"; BUILD="$2"; VER="$3"; OUT="$4"
source "$PRODUTO"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
WORK="$(mktemp -d)"
D="$WORK/$CURTO-$VER-linux-x86_64"
mkdir -p "$D"
bash "$(dirname "$0")/plugins.sh" "$PRODUTO" > "$WORK/plugins.txt"
APP=""
NOMES=()
while IFS='|' read -r N T _; do
  A="$BUILD/${T}_artefacts/Release"
  NOMES+=("$N")
  cp -R "$A/VST3/$N.vst3" "$D/"
  strip "$D/$N.vst3/Contents/x86_64-linux/$N.so" || true
  if [ -e "$A/LV2/$N.lv2" ]; then cp -R "$A/LV2/$N.lv2" "$D/"; strip "$D/$N.lv2/lib$N.so" || true; fi
  if [ -e "$A/CLAP/$N.clap" ]; then cp -R "$A/CLAP/$N.clap" "$D/"; fi
  if [ -z "$APP" ] && [ -e "$A/Standalone/$N" ]; then cp "$A/Standalone/$N" "$D/$BINARIO"; strip "$D/$BINARIO" || true; APP="$N"; fi
done < "$WORK/plugins.txt"

FORMATOS="VST3 (~/.vst3)"
if ls "$D"/*.lv2 >/dev/null 2>&1; then FORMATOS="$FORMATOS, LV2 (~/.lv2)"; fi
if ls "$D"/*.clap >/dev/null 2>&1; then FORMATOS="$FORMATOS, CLAP (~/.clap)"; fi

# o instalador: copia cada bundle (e a app, se a hai) para o usuario
{
  echo '#!/bin/sh'
  echo "# Instala $NOME para o teu usuario (sen sudo)"
  echo 'set -e'
  echo 'cd "$(dirname "$0")"'
  echo 'mkdir -p "$HOME/.vst3" "$HOME/.lv2" "$HOME/.clap"'
  for N in "${NOMES[@]}"; do
    echo "rm -rf \"\$HOME/.vst3/$N.vst3\" \"\$HOME/.lv2/$N.lv2\" \"\$HOME/.clap/$N.clap\""
    echo "cp -R \"$N.vst3\" \"\$HOME/.vst3/\""
    echo "[ -e \"$N.lv2\" ] && cp -R \"$N.lv2\" \"\$HOME/.lv2/\""
    echo "[ -e \"$N.clap\" ] && cp -R \"$N.clap\" \"\$HOME/.clap/\""
  done
  if [ -n "$APP" ]; then
    echo 'mkdir -p "$HOME/.local/bin" "$HOME/.local/share/applications"'
    echo "cp $BINARIO \"\$HOME/.local/bin/\""
    echo "cat > \"\$HOME/.local/share/applications/$BINARIO.desktop\" <<EOD"
    echo '[Desktop Entry]'
    echo 'Type=Application'
    echo "Name=$APP"
    echo "Comment=$DESCRICION (Escoita)"
    echo "Exec=\$HOME/.local/bin/$BINARIO"
    echo 'Categories=AudioVideo;Audio;Midi;'
    echo 'EOD'
    echo "echo \"Instalado. Volve abrir o teu DAW e busca «${NOME}»; a app está no menú ou en ~/.local/bin/$BINARIO\""
  elif [ ${#NOMES[@]} -gt 1 ]; then
    echo "echo \"Instalados: $(printf '%s, ' "${NOMES[@]}" | sed 's/, $//'). Volve abrir o teu DAW.\""
  else
    echo "echo \"Instalado. Volve abrir o teu DAW e busca «${NOME}».\""
  fi
} > "$D/instalar.sh"
{
  echo '#!/bin/sh'
  for N in "${NOMES[@]}"; do echo "rm -rf \"\$HOME/.vst3/$N.vst3\" \"\$HOME/.lv2/$N.lv2\" \"\$HOME/.clap/$N.clap\""; done
  if [ -n "$APP" ]; then echo "rm -f \"\$HOME/.local/bin/$BINARIO\" \"\$HOME/.local/share/applications/$BINARIO.desktop\""; fi
  echo "echo \"$NOME desinstalado.\""
} > "$D/desinstalar.sh"
chmod +x "$D/instalar.sh" "$D/desinstalar.sh"
if [ -n "$APP" ]; then chmod +x "$D/$BINARIO"; fi
NOME_MAIUS="$(echo "$NOME" | tr '[:lower:]' '[:upper:]')"
{
  echo "$NOME_MAIUS para Linux (x86_64)"
  echo "Unha ferramenta de Escoita · https://www.escoita.link"
  echo
  echo "Instalar:    ./instalar.sh      (para o teu usuario, sen sudo)"
  echo "Desinstalar: ./desinstalar.sh"
  echo
  if [ ${#NOMES[@]} -gt 1 ]; then echo "Plugins: $(printf '%s, ' "${NOMES[@]}" | sed 's/, $//')."; fi
  if [ -n "$APP" ]; then
    echo "Formatos: $FORMATOS e aplicación independente ($BINARIO)."
    echo "A aplicación precisa ALSA ou JACK; o plugin funciona en calquera DAW con eses formatos"
  else
    echo "Formatos: $FORMATOS."
    echo "Funcionan en calquera DAW con eses formatos"
  fi
  echo "(Bitwig, Reaper, Ardour, Carla…)."
} > "$D/LEEME.txt"
tar -C "$WORK" -czf "$OUT/$CURTO-$VER-linux-x86_64.tar.gz" "$CURTO-$VER-linux-x86_64"
echo "Feito: $OUT/$CURTO-$VER-linux-x86_64.tar.gz"
