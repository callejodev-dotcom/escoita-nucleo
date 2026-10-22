// Probas do núcleo: o que comparten todos os plugins ten que estar ben en todos os sistemas
#include <JuceHeader.h>
#include <cstdio>

using namespace escoita;

int main (int argc, char** argv)
{
    // --rede: pregunta de verdade a GitHub (só no destino sen PROBA) e di que sabe
    if (argc > 1 && juce::String (argv[1]) == "--rede")
    {
        juce::SharedResourcePointer<Actualizacions> a;
        for (int i = 0; i < 100 && a->newVersion().isEmpty(); ++i) juce::Thread::sleep (100);
        std::printf ("versión nova: «%s» (esta é a %s)\n", a->newVersion().toRawUTF8(), Actualizacions::currentVersion().toRawUTF8());
        return 0;
    }

    int fallos = 0;
    auto proba = [&] (const char* que, bool ok) { std::printf ("%-52s %s\n", que, ok ? "OK" : "FALLO"); fallos += ok ? 0 : 1; };

    proba ("versións: compáranse os números (0.10 despois de 0.9)",
           versionNova ("v0.3.1", "0.3.0") && versionNova ("0.10.0", "0.9.9") && versionNova ("1.0", "0.9.9")
           && ! versionNova ("v0.3.0", "0.3.0") && ! versionNova ("0.2.9", "0.3.0") && ! versionNova ("", "0.3.0") && ! versionNova ("lixo", "0.3.0"));

    proba ("maiúsculas con acentos e ñ",
           maiusculas (U ("Corrección, Saída, Armonías, fóra, maiúsculas, año, ü")) == U ("CORRECCIÓN, SAÍDA, ARMONÍAS, FÓRA, MAIÚSCULAS, AÑO, Ü")
           && maiusculas (U ("Do♯3 −5")) == U ("DO♯3 −5"));

    proba ("números á galega (coma e menos de verdade)",
           numero (263.84, 1) == U ("263,8") && numero (-3.5, 1) == U ("−3,5") && numero (440.0, 0) == "440");

    {
        const char latin1[] = "Escala diat\xF3nica";
        const unsigned char cortado[] = { 'a', 'b', 0xC3 };            // unha «ó» cortada
        const unsigned char bom[] = { 0xEF, 0xBB, 0xBF, 'o', 'l', 'a' };
        const unsigned char cero[] = { 'u', 'n', 0, 'd', 'o', 'u', 's' };
        const unsigned char longo[] = { 0xC0, 0xAF };                    // «/» longa de máis: non é UTF-8
        const auto tc = textoDeBytes (cortado, sizeof (cortado));
        proba ("texto de fóra: Latin-1, UTF-8 cortado, BOM e cero",
               textoDeBytes (latin1, sizeof (latin1) - 1) == U ("Escala diatónica")
               && tc.length() == 3 && juce::CharPointer_UTF8::isValidString (tc.toRawUTF8(), (int) tc.getNumBytesAsUTF8())
               && textoDeBytes (bom, sizeof (bom)) == "ola" && textoDeBytes (cero, sizeof (cero)) == "un"
               && textoDeBytes (longo, sizeof (longo)).length() == 2 && textoDeBytes (nullptr, 5).isEmpty());
    }
    {
        const auto f = juce::File::createTempFile (".txt");
        f.replaceWithData ("Mi \xE9 a nota", 11);
        const bool ok = lerTexto (f) == U ("Mi é a nota") && lerTexto (f, 4).isEmpty() && lerTexto (f.getSiblingFile ("non-existe.txt")).isEmpty();
        f.deleteFile();
        proba ("ler un ficheiro de texto (e non se é grande de máis)", ok);
    }
   #if ESCOITA_SEN_REDE
    {
        // nas probas non se sae á rede: sen ESCOITA_NOVA_VERSION non hai versión nova
        juce::SharedResourcePointer<Actualizacions> a;
        const auto v = juce::SystemStats::getEnvironmentVariable ("ESCOITA_NOVA_VERSION", {});
        proba ("aviso das versións: sen rede nas probas", a->newVersion() == (versionNova (v, ESCOITA_VERSION) ? v : juce::String()));
    }
   #endif
    std::printf (fallos == 0 ? "TODO OK\n" : "%d FALLOS\n", fallos);
    return fallos == 0 ? 0 : 1;
}
