# escoita-nucleo

O núcleo común dos plugins de [Escoita](https://www.escoita.link): Barullo Tune, Barullo FM, Barullo 7FM e Barullo Xuno. O que se mellora aquí mellora todos. Cada plugin fixa unha versión do núcleo e sóbea cando quere; a súa CI e as súas probas comproban que nada se rompe antes de publicar.

## Que ten

| Peza | Que é |
|---|---|
| `escoita_nucleo/` | Módulo de JUCE (`escoita::`) |
| ↳ `texto/` | `U()` (literais UTF-8), `maiusculas()` (tamén con acentos: o sistema non sempre as sabe), `numero()` (coma decimal e −), `textoDeBytes()` e `lerTexto()` (texto de fóra en UTF-8 ou Latin-1, nunca inválido) |
| ↳ `actualizacions/` | `Actualizacions`: unha vez ao día pregunta a GitHub cal é a última versión do produto (sen mandar datos) e di se hai unha nova (en Linux, co programa curl ou wget do sistema). O plugin debuxa o aviso ao seu xeito e deixa desactivalo |
| ↳ `estado/` | `Estado`: presets propios (ficheiros XML en Documentos/<produto>/Presets), A/B e desfacer, sobre o APVTS. Os sintes engaden o estado que non é parámetro (sons editados, memoria) con `extra`/`aplicarExtra`, e os seus cambios desfanse con `cambio()`. `engadirAoMenu()` e `gardarComo()` poñen as entradas nun menú (cada plugin co seu aspecto) |
| ↳ `midi/` | `Sostido` (o pedal de sostemento entre o MIDI e o motor), `escollerVoz()` e `Voces` (reparto: a libre que leva máis tempo libre, se non a que leva máis tempo tocando; ou rotando, coma os Juno) e `MPE` (bend, presión e CC74 por canle, coa mestra; rango do bend por RPN e configuración MPE) |
| ↳ `audio/` | `Teito`: limitador de seguridade (−1 dBFS, ataque instantáneo, sen latencia) |
| ↳ `ui/` | `MandoEditable`: un mando (calquera juce::Slider) ligado a un parámetro ao que se lle escribe o valor coma no DAW (clic dereito, Cmd/Ctrl-clic ou intro) e que se move fino con maiúsculas; o aspecto é o de cada plugin |
| `CMakeLists.txt` | Engade o módulo e a función `escoita_produto()` |
| `.github/workflows/plugin.yml` | A CI común: Mac universal, Windows e Linux; probas do plugin, pluginval, auval, clap-validator, empaquetado e publicación ao etiquetar |
| `empaquetado/` | `mac.sh` (.pkg e .zip), `linux.sh` (.tar.gz con instalador), `windows.iss` (Inno Setup en galego, con `Galician.isl`) |
| `probas/` | As probas do núcleo e `Aleatorio.h` (números aleatorios iguais en todos os sistemas, para as probas dos plugins) |

Non van aquí o aspecto de cada plugin (cada un é o seu aparello) nin os motores (o chip, o DX7, o Juno, o afinador).

## Usalo nun plugin

No `CMakeLists.txt`, despois de JUCE:

```cmake
# Núcleo común de Escoita: a versión fixada ou, se existe, a carpeta irmá ../escoita-nucleo
# (para traballar no núcleo e probalo nos plugins á vez); tamén -DESCOITA_NUCLEO=<carpeta>
set(ESCOITA_NUCLEO_VERSION "v0.2.0")
if(NOT ESCOITA_NUCLEO AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../escoita-nucleo/escoita_nucleo")
  set(ESCOITA_NUCLEO "${CMAKE_CURRENT_SOURCE_DIR}/../escoita-nucleo")
endif()
if(ESCOITA_NUCLEO)
  add_subdirectory("${ESCOITA_NUCLEO}" escoita-nucleo)
else()
  include(FetchContent)
  FetchContent_Declare(escoita_nucleo GIT_REPOSITORY https://github.com/callejodev-dotcom/escoita-nucleo.git
                       GIT_TAG ${ESCOITA_NUCLEO_VERSION} GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(escoita_nucleo)
endif()

escoita_produto(BarulloTune NOME "Barullo Tune" VERSION ${PROJECT_VERSION} REPO callejodev-dotcom/barullotune)
# e nas probas (sen rede, con Aleatorio.h):
escoita_produto(BTRenderTest NOME "Barullo Tune" VERSION ${PROJECT_VERSION} REPO callejodev-dotcom/barullotune PROBA)
```

No código: `#include <escoita_nucleo/escoita_nucleo.h>` (ou `JuceHeader.h`). O aviso das versións:

```cpp
juce::SharedResourcePointer<escoita::Actualizacions> versions;   // no editor
// cada pouco (no temporizador): versions->newVersion() → "0.3.1" ou baleiro
// ao premer o aviso: juce::URL (escoita::Actualizacions::downloadsUrl()).launchInDefaultBrowser();
// para desactivalo: versions->setEnabled (false);
```

Para a CI, `.github/workflows/build.yml`:

```yaml
name: Compilar e publicar Barullo Tune
on:
  push: { branches: ["**"], tags: ["v*"] }
  workflow_dispatch:
jobs:
  plugin:
    uses: callejodev-dotcom/escoita-nucleo/.github/workflows/plugin.yml@v0.2.0
    secrets: inherit
    permissions:
      contents: write
    with:
      sanitizadores: "BTRenderTest"   # opcional
```

E no plugin:

- `packaging/produto.env`:
  ```sh
  NOME="Barullo Tune"            # nome dos bundles
  CURTO="BarulloTune"            # destino de CMake e prefixo dos ficheiros
  BINARIO="barullo-tune"         # a app en Linux
  ID="link.escoita.barullotune"  # identificador do .pkg
  DESCRICION="Corrector de afinación"
  AU="aumf Btun Esco"            # tipo, subtipo e fabricante, para auval (baleiro: sen AU)
  WINDOWS_APPID="3D8E1F6A-5B2C-4E7D-A9F0-7C6B5A4D3E21"   # GUID propio, sen chaves; non cambialo nunca
  ```
  Unha **serie** (varios plugins nun repo e nun só instalador, coma Barullo Enxebres) pon ademais
  a lista dos plugins, un por liña «Nome dos ficheiros|Destino de CMake|AU» (o nome dos bundles,
  sen tiles: o `codesign` de Apple e o cargador de VST3 trabúcanse con eles; o nome que se ve no
  DAW pode levalos, con `PLUGIN_NAME` en `juce_add_plugin`). `NOME` e `CURTO` son entón os do
  paquete (o instalador, os ficheiros e o `project()` de CMake, que leva a versión):
  ```sh
  PLUGINS="Barullo Filtro|BarulloFiltro|aufx Bfil Esco
  Barullo Chorus|BarulloChorus|aufx Bcho Esco"
  ```
  Unha serie pode ir sen aplicación independente: o instalador só leva o que haxa.
- `packaging/LEEME-mac.txt`, `packaging/LEEME-windows.txt` e `packaging/NOTAS-VERSION.md` (o texto da versión publicada).
- `tools/probas-ci.sh` (opcional): as probas do plugin. Chámase desde a raíz con `mac`, `windows`, `linux` ou `sanitizadores`; a compilación está en `build/` (en `build/<Destino>_artefacts/Release/`, ou `RelWithDebInfo` cos sanitizadores).
- As opcións de CMake `BARULLO_TESTS`, `BARULLO_WERROR` e `BARULLO_SANITIZE`, se as ten.

Presets, A/B e desfacer:

```cpp
// no procesador (o UndoManager declárase antes ca o APVTS, e o Estado despois)
juce::UndoManager undo;
juce::AudioProcessorValueTreeState params { *this, &undo, "Estado", capa() };
escoita::Estado estado { params, &undo, [] { escoita::Estado::Opcions o; o.extension = "bfmpreset"; o.excluir = { "bypass" }; return o; }() };
// na interface: un clic, un paso (estado.novoPaso()); o A/B con estado.lado() / cambiarLado();
// o menú: juce::PopupMenu m; estado.engadirAoMenu (m, [this] { refrescar(); }); m.showMenuAsync (…);
```

## Mellorar o núcleo

1. Clona `escoita-nucleo` ao lado dos plugins (`~/…/escoita-nucleo` xunto a `~/…/barullotune`): os plugins colleno só ao compilar en local.
2. Cambia, proba aquí (`cmake -S probas -B build && cmake --build build && build/ProbaNucleo_artefacts/Release/ProbaNucleo`) e nos plugins.
3. Sobe a versión en `escoita_nucleo/escoita_nucleo.h`, etiqueta (`git tag v0.3.0 && git push origin v0.3.0`).
4. En cada plugin, cando queiras: `ESCOITA_NUCLEO_VERSION` e o `@v…` do workflow á versión nova. A súa CI dirá se todo segue ben.

## Licenza

GNU Affero General Public License v3.0 (`LICENSE`), coma os plugins. `empaquetado/Galician.isl` é a tradución non oficial de Inno Setup ao galego, de MdL.
