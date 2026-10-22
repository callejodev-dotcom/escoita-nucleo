/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 escoita_nucleo
  vendor:             escoita
  version:            0.1.0
  name:               Nucleo comun dos plugins de Escoita
  description:        Texto en galego, lectura segura de ficheiros e aviso das versions novas
  website:            https://www.escoita.link
  license:            AGPLv3
  minimumCppStandard: 17

  dependencies:       juce_core juce_data_structures

 END_JUCE_MODULE_DECLARATION
*******************************************************************************/

// O núcleo común dos plugins de Escoita (Barullo Tune, Barullo FM, Barullo 7FM, Barullo Xuno…).
// O que vai aquí mellora todos á vez: cada plugin fixa unha versión e sóbea cando quere.
//
// Cada destino que o use debe definir (escoita_produto() en CMake failo só):
//   ESCOITA_PRODUTO   o nome («Barullo Tune»)
//   ESCOITA_VERSION   a versión («0.3.0»)
//   ESCOITA_REPO      o repositorio en GitHub («callejodev-dotcom/barullotune»)
//   ESCOITA_SEN_REDE  (opcional) 1 nas probas: non se sae á rede nin se tocan os axustes
#pragma once
#define ESCOITA_NUCLEO_H_INCLUDED

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

#include "texto/escoita_Texto.h"
#include "actualizacions/escoita_Actualizacions.h"
