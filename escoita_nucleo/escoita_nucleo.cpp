#ifdef ESCOITA_NUCLEO_H_INCLUDED
 #error "Este ficheiro compílase só (é o módulo); non o inclúas"
#endif

#include "escoita_nucleo.h"

#ifndef ESCOITA_PRODUTO
 #error "Falta ESCOITA_PRODUTO (usa escoita_produto() no CMakeLists.txt)"
#endif
#ifndef ESCOITA_VERSION
 #error "Falta ESCOITA_VERSION (usa escoita_produto() no CMakeLists.txt)"
#endif
#ifndef ESCOITA_REPO
 #error "Falta ESCOITA_REPO (usa escoita_produto() no CMakeLists.txt)"
#endif

#include "texto/escoita_Texto.cpp"
#include "actualizacions/escoita_Actualizacions.cpp"
