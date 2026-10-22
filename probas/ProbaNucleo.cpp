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
    // ---- MIDI: o pedal de sostemento
    {
        Sostido s;
        juce::String rastro;
        s.empezar = [&] (int n, int) { rastro << "+" << n; };
        s.acabar = [&] (int n) { rastro << "-" << n; };
        s.noteOn (60, 100); s.pedal (true); s.noteOff (60);     // sostida: non acaba
        s.noteOn (64, 90); s.noteOn (60, 80);                    // tocar outra vez a sostida: acaba e empeza
        s.noteOff (60); s.noteOff (64);                          // co pedal: as dúas quedan
        s.pedal (false);                                         // soltar: acaban
        s.noteOn (67, 1); s.pedal (true); s.pedal (false);       // a premida non acaba ao soltar o pedal
        proba ("MIDI: o pedal sostén e solta como un piano", rastro == "+60+64-60+60-60-64+67" && ! s.sostida (67));
    }
    // ---- MIDI: o reparto de voces
    {
        Voces v (3);
        const int a = v.escoller(); v.tocar (a, 60);
        const int b = v.escoller(); v.tocar (b, 62);
        const int c = v.escoller(); v.tocar (c, 64);
        const int d = v.escoller();                     // todas ocupadas: róubase a primeira
        v.soltar (b);                                    // libre agora
        const int e = v.escoller();                      // a libre gaña
        v.tocar (e, 65); v.soltar (a); v.soltar (c);     // libres a e c; a soltouse antes
        const int f = v.escoller();
        const bool ok = a == 0 && b == 1 && c == 2 && d == 0 && e == 1 && f == 0 && v.vozDaNota (65) == 1 && v.vozDaNota (99) == -1;
        Voces r (4);
        int seq[6]; for (int i = 0; i < 6; ++i) { seq[i] = r.escollerRotando(); r.tocar (seq[i], 60 + i); r.soltar (seq[i]); }
        const bool rota = seq[0] == 0 && seq[1] == 1 && seq[2] == 2 && seq[3] == 3 && seq[4] == 0 && seq[5] == 1;
        const int ex = escollerVoz (3, [] (int) { return EstadoVoz { true, 0 }; }, [] (int k) { return k == 0; });
        proba ("MIDI: reparto de voces (libre, roubo, rotación)", ok && rota && ex == 1);
    }
    // ---- MIDI: MPE
    {
        MPE m;
        auto cc = [&] (int ch, int n, int val) { m.procesar (juce::MidiMessage::controllerEvent (ch, n, val)); };
        const bool antes = ! m.activo() && m.rangoBend (3) == 2;
        cc (1, 101, 0); cc (1, 100, 6); cc (1, 6, 15);                  // configuración MPE: 15 canles
        const bool activo = m.activo() && m.canlesDeNotas() == 15 && m.rangoBend (3) == 48;
        m.procesar (juce::MidiMessage::pitchWheel (3, 8192 + 4096));    // +0,5 na canle 3: +24 semitonos
        m.procesar (juce::MidiMessage::pitchWheel (1, 8192 - 8192));    // −1 na mestra (rango 2): −2
        m.procesar (juce::MidiMessage::channelPressureChange (3, 127));
        cc (3, 74, 0);
        cc (2, 101, 0); cc (2, 100, 0); cc (2, 6, 12);                  // rango do bend da canle 2
        const bool ok = std::abs (m.bendSemitonos (3) - 22.0) < 1e-9 && std::abs (m.bendSemitonos (1) + 2.0) < 1e-9
                        && juce::exactlyEqual (m.presion (3), 1.0) && juce::exactlyEqual (m.timbre (3), 0.0) && juce::exactlyEqual (m.timbre (5), 0.5) && m.rangoBend (2) == 12;
        proba ("MIDI: MPE (bend por canle e mestra, presión, CC74, RPN)", antes && activo && ok);
    }
    // ---- o teito
    {
        Teito t; t.preparar (48000.0);
        double maxi = 0;
        for (int i = 0; i < 48000; ++i) maxi = std::max (maxi, std::abs (t.procesar (2.0 * std::sin (i * 0.05))));
        Teito q; q.preparar (48000.0);
        const double baixo = q.procesar (0.5);
        double l = 3.0, r = -1.5; Teito e; e.preparar (44100.0); e.procesar (l, r);
        proba ("teito: nunca pasa de −1 dBFS e non toca o que vai baixo",
               maxi <= 0.891 + 1e-12 && juce::exactlyEqual (baixo, 0.5) && std::abs (l - 0.891) < 1e-12 && std::abs (r + 0.4455) < 1e-12);
    }
    // ---- presets propios, A/B e desfacer
    {
        struct Proc : juce::AudioProcessor
        {
            Proc() : juce::AudioProcessor (BusesProperties()), apvts (*this, &undo, "P", capa()) {}
            using juce::AudioProcessor::processBlock;
            static juce::AudioProcessorValueTreeState::ParameterLayout capa()
            {
                juce::AudioProcessorValueTreeState::ParameterLayout l;
                l.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "vol", 1 }, "vol", 0.0f, 10.0f, 5.0f));
                l.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "bypass", 1 }, "bypass", 0.0f, 1.0f, 0.0f));
                return l;
            }
            const juce::String getName() const override { return "p"; }
            void prepareToPlay (double, int) override {}
            void releaseResources() override {}
            void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
            double getTailLengthSeconds() const override { return 0; }
            bool acceptsMidi() const override { return false; }
            bool producesMidi() const override { return false; }
            juce::AudioProcessorEditor* createEditor() override { return nullptr; }
            bool hasEditor() const override { return false; }
            int getNumPrograms() override { return 1; }
            int getCurrentProgram() override { return 0; }
            void setCurrentProgram (int) override {}
            const juce::String getProgramName (int) override { return {}; }
            void changeProgramName (int, const juce::String&) override {}
            void getStateInformation (juce::MemoryBlock&) override {}
            void setStateInformation (const void*, int) override {}
            juce::UndoManager undo;
            juce::AudioProcessorValueTreeState apvts;
        } p;
        int extra = 1;   // estado que non é parámetro
        Estado::Opcions o;
        o.extension = "probapreset"; o.excluir = { "bypass" }; o.version = 3;
        o.extra = [&] { juce::ValueTree t ("X"); t.setProperty ("n", extra, nullptr); return t; };
        o.aplicarExtra = [&] (const juce::ValueTree& t) { extra = (int) t.getProperty ("n"); };
        Estado e (p.apvts, &p.undo, o);
        auto vol = [&] { return p.apvts.getParameter ("vol")->convertFrom0to1 (p.apvts.getParameter ("vol")->getValue()); };
        auto pon = [&] (float v) { auto* r = p.apvts.getParameter ("vol"); r->setValueNotifyingHost (r->convertTo0to1 (v)); };
        auto flush = [&] { p.apvts.copyState(); };   // o APVTS pasa os cambios á árbore (e ao desfacer) aquí
        pon (2.0f); extra = 7;
        e.cambiarLado();                         // B: copia de A
        const bool copia = e.lado() == 1 && std::abs (vol() - 2.0f) < 1e-4f && extra == 7;
        pon (8.0f); extra = 9;
        e.cambiarLado();                         // A outra vez: 2 e 7
        const bool volveA = e.lado() == 0 && std::abs (vol() - 2.0f) < 1e-4f && extra == 7;
        e.cambiarLado();                         // B: 8 e 9
        const bool volveB = std::abs (vol() - 8.0f) < 1e-4f && extra == 9;
        proba ("estado: A/B cos parámetros e o estado propio", copia && volveA && volveB && ! e.foto().getChildWithProperty ("id", "bypass").isValid());

        const auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("escoita-proba-presets");
        dir.deleteRecursively();
        e.setCarpeta (dir);
        const bool gardou = e.gardar ("Proba: son/1");
        pon (1.0f); extra = 2;
        const auto lista = e.presets();
        const bool cargou = lista.size() == 1 && lista[0].getFileName() == "Proba son1.probapreset" && e.cargar (lista[0]);
        const bool valores = std::abs (vol() - 8.0f) < 1e-4f && extra == 9;
        const auto xml = juce::parseXML (lista[0]);
        const bool formato = xml != nullptr && xml->hasTagName ("PARAMS") && xml->getIntAttribute ("version") == 3;
        dir.deleteRecursively();
        proba ("estado: presets propios (gardar, listar, cargar)", gardou && cargou && valores && formato);

        // desfacer: un cambio de parámetro e un do estado propio, cada un no seu paso
        flush(); e.novoPaso();
        pon (4.0f); flush(); e.novoPaso();
        e.cambio ("extra", [&] { extra = 42; }, [&] { extra = 9; });
        e.novoPaso();
        p.undo.undo();
        const bool d1 = extra == 9 && std::abs (vol() - 4.0f) < 1e-4f;
        p.undo.undo();
        const bool d2 = std::abs (vol() - 8.0f) < 1e-4f;
        p.undo.redo(); p.undo.redo();
        const bool d3 = extra == 42 && std::abs (vol() - 4.0f) < 1e-4f;
        proba ("estado: desfacer e refacer (parámetros e estado propio)", d1 && d2 && d3);
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
