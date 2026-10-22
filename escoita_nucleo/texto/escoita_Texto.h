#pragma once

// Texto: o código vai en UTF-8 e as interfaces en galego (tildes, ñ, ♯, −)
namespace escoita
{
    // un literal UTF-8 do código como juce::String
    inline juce::String U (const char* s) { return juce::String::fromUTF8 (s); }

    // maiúsculas tamén para as letras con acento: o towupper do sistema non sempre as sabe
    // (no Mac, «ó» queda en minúscula)
    juce::String maiusculas (const juce::String&);

    // un número á galega: coma decimal e o signo menos de verdade (−)
    juce::String numero (double valor, int decimais);

    // texto de fóra (un preset, unha escala, un estado do DAW): UTF-8 se o é (sen o BOM),
    // se non Latin-1, que é o que gardan moitos programas de Windows. Remata no primeiro
    // cero. Nunca dá un texto inválido, veña o que veña.
    juce::String textoDeBytes (const void* datos, size_t tamano);

    // un ficheiro de texto pequeno (por riba de «maximo» bytes, nada: non é noso)
    juce::String lerTexto (const juce::File&, juce::int64 maximo = 4 * 1024 * 1024);
}
