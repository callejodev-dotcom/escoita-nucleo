# As probas do núcleo: cmake -S probas -B build [-DJUCE_DIR=...] && cmake --build build
cmake_minimum_required(VERSION 3.22)
project(ProbasEscoitaNucleo VERSION 0.0.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE: a carpeta irmá ../../JUCE, a que se indique con -DJUCE_DIR=..., ou descargada
if(NOT JUCE_DIR)
  set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../JUCE")
endif()
get_filename_component(JUCE_DIR "${JUCE_DIR}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
if(EXISTS "${JUCE_DIR}/CMakeLists.txt")
  add_subdirectory("${JUCE_DIR}" JUCE)
else()
  include(FetchContent)
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()

add_subdirectory(.. escoita-nucleo)

# dúas veces: a proba (sen rede) e a de verdade (só se compila; con --rede pregunta a GitHub)
foreach(destino ProbaNucleo ProbaNucleoRede)
  juce_add_console_app(${destino} PRODUCT_NAME ${destino})
  juce_generate_juce_header(${destino})
  target_sources(${destino} PRIVATE ProbaNucleo.cpp)
  target_compile_definitions(${destino} PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
  target_link_libraries(${destino} PRIVATE juce::juce_core juce::juce_data_structures
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
  if(MSVC)
    target_compile_options(${destino} PRIVATE /utf-8)
  endif()
endforeach()
# avisos como erros no código propio (o da proba e o do núcleo), non no de JUCE
set_source_files_properties(ProbaNucleo.cpp "${CMAKE_CURRENT_SOURCE_DIR}/../escoita_nucleo/escoita_nucleo.cpp"
  PROPERTIES COMPILE_OPTIONS "$<IF:$<CXX_COMPILER_ID:MSVC>,/WX,-Werror>")
escoita_produto(ProbaNucleo NOME "Proba Escoita" VERSION ${PROJECT_VERSION} REPO callejodev-dotcom/escoita-nucleo PROBA)
escoita_produto(ProbaNucleoRede NOME "Proba Escoita" VERSION ${PROJECT_VERSION} REPO callejodev-dotcom/barullotune)
