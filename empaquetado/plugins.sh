#!/bin/bash
# Os plugins dun produto, un por liña «Nome|Destino|AU»: os dunha serie (PLUGINS en produto.env)
# ou, se non hai serie, o único (NOME, CURTO e AU).
# Uso: plugins.sh <produto.env>
set -eo pipefail
source "$1"
if [ -n "$PLUGINS" ]; then
  printf '%s\n' "$PLUGINS" | sed -e 's/^[[:space:]]*//' -e '/^$/d'
else
  printf '%s|%s|%s\n' "$NOME" "$CURTO" "$AU"
fi
