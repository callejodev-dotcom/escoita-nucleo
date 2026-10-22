# O núcleo común dos plugins de Escoita. Úsase desde o CMakeLists.txt de cada plugin, despois
# de JUCE (ver o LEEME): engade o módulo escoita_nucleo e a función escoita_produto().
cmake_minimum_required(VERSION 3.22)

if(NOT COMMAND juce_add_module)
  message(FATAL_ERROR "escoita-nucleo: engade JUCE antes ca o núcleo")
endif()

juce_add_module("${CMAKE_CURRENT_LIST_DIR}/escoita_nucleo")
# a carpeta do núcleo, para quen a precise (por exemplo, probas sen JUCE que usan probas/Aleatorio.h)
set(ESCOITA_NUCLEO_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "carpeta de escoita-nucleo")

file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/escoita_nucleo/escoita_nucleo.h" _ev REGEX "^ *version: ")
string(REGEX REPLACE "^ *version: *" "" _ev "${_ev}")
message(STATUS "escoita-nucleo ${_ev}: ${CMAKE_CURRENT_LIST_DIR}")

# escoita_produto(<destino> NOME "Barullo Tune" VERSION 0.3.0 REPO callejodev-dotcom/barullotune [PROBA])
#   Enlaza o núcleo e dálle os datos do produto. Con PROBA (os programas de proba), o núcleo
#   non sae á rede nin toca os axustes do usuario, e vense as axudas das probas (Aleatorio.h).
function(escoita_produto destino)
  cmake_parse_arguments(EP "PROBA" "NOME;VERSION;REPO" "" ${ARGN})
  if(NOT EP_NOME OR NOT EP_VERSION OR NOT EP_REPO)
    message(FATAL_ERROR "escoita_produto(${destino}): fan falta NOME, VERSION e REPO")
  endif()
  target_link_libraries(${destino} PRIVATE escoita_nucleo)
  target_compile_definitions(${destino} PUBLIC
    ESCOITA_PRODUTO="${EP_NOME}"
    ESCOITA_VERSION="${EP_VERSION}"
    ESCOITA_REPO="${EP_REPO}")
  if(EP_PROBA)
    target_compile_definitions(${destino} PRIVATE ESCOITA_SEN_REDE=1)
    target_include_directories(${destino} PRIVATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/probas")
  endif()
endfunction()
