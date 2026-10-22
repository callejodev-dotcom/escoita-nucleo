/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 escoita_nucleo
  vendor:             escoita
  version:            0.3.0
  name:               Nucleo comun dos plugins de Escoita
  description:        Texto en galego, aviso das versions novas, presets propios, A/B e desfacer, MIDI para sintes e limitador
  website:            https://www.escoita.link
  license:            AGPLv3
  minimumCppStandard: 17

  dependencies:       juce_core juce_data_structures juce_audio_basics juce_audio_processors juce_gui_basics

 END_JUCE_MODULE_DECLARATION
*******************************************************************************/

// O núcleo común dos plugins de Escoita (Barullo Tune, Barullo FM, Barullo 7FM, Barullo Xuno…).
// O que vai aquí mellora todos á vez: cada plugin fixa unha versión e sóbea cando quere.
//
//   texto/           U(), maiusculas(), numero(), textoDeBytes(), lerTexto()
//   actualizacions/  Actualizacions: o aviso das versións novas
//   estado/          Estado: presets propios, A/B e desfacer
//   midi/            Sostido (o pedal), escollerVoz() e Voces (reparto de voces), MPE
//                    (escoita_Voces.h vai sen JUCE: os motores sen JUCE poden incluílo só)
//   audio/           Teito: limitador de seguridade (sen JUCE tamén)
//   ui/              MandoEditable: un mando ao que se lle escribe o valor e que se move fino
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
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "texto/escoita_Texto.h"
#include "actualizacions/escoita_Actualizacions.h"
#include "estado/escoita_Estado.h"
#include "midi/escoita_Midi.h"
#include "audio/escoita_Teito.h"
#include "ui/escoita_MandoEditable.h"
