#pragma once

// MIDI para sintes: o pedal de sostemento, o reparto de voces e a expresión por canle (MPE).
// Nada disto reserva memoria nin bloquea: vale no fío do audio.
namespace escoita
{
    // O pedal de sostemento (CC64) entre o MIDI e o motor: o motor só ve as notas que
    // empezan e as que acaban de verdade. Unha nota sostida que se volve tocar acaba e
    // empeza de novo (coma nun piano).
    class Sostido
    {
    public:
        std::function<void (int nota, int velocidade)> empezar;   // ao motor
        std::function<void (int nota)> acabar;

        void noteOn (int n, int vel);
        void noteOff (int n);
        void pedal (bool abaixo);
        bool pedalAbaixo() const { return pedalOn; }
        bool sostida (int n) const { return n >= 0 && n < 128 && sostidas[(size_t) n]; }
        // esquece todo (as notas que soan acábaas o motor: allNotesOff dela)
        void reiniciar() { sostidas.fill (false); premidas.fill (false); pedalOn = false; }

    private:
        std::array<bool, 128> premidas {}, sostidas {};
        bool pedalOn = false;
    };

    // Reparto de voces: para unha nota nova, a voz libre que leva máis tempo libre; se non hai
    // ningunha libre, róubase a que leva máis tempo tocando. «estado» di de cada voz se está
    // libre e cando cambiou por última vez (un contador que sobe); «excluida», as que non valen.
    struct EstadoVoz { bool libre; uint64_t cambio; };
    int escollerVoz (int voces, const std::function<EstadoVoz (int)>& estado, const std::function<bool (int)>& excluida = {});

    // O mesmo, gardando o estado: para un sinte novo, a forma doada
    class Voces
    {
    public:
        explicit Voces (int n = 8) : notas ((size_t) n, -1), cambios ((size_t) n, 0) {}
        int numero() const { return (int) notas.size(); }
        int escoller() const;                     // a voz para unha nota nova (libre ou roubada)
        int escollerRotando();                    // coma os Juno: a seguinte despois da última, preferindo as libres
        void tocar (int voz, int nota) { notas[(size_t) voz] = nota; cambios[(size_t) voz] = ++reloxo; ultima = voz; }
        void soltar (int voz) { notas[(size_t) voz] = -1; cambios[(size_t) voz] = ++reloxo; }
        int nota (int voz) const { return notas[(size_t) voz]; }
        int vozDaNota (int nota) const;           // a voz que toca esa nota, ou −1
        void reiniciar() { std::fill (notas.begin(), notas.end(), -1); std::fill (cambios.begin(), cambios.end(), 0); reloxo = 0; ultima = -1; }

    private:
        std::vector<int> notas;
        std::vector<uint64_t> cambios;
        uint64_t reloxo = 0;
        int ultima = -1;
    };

    // Expresión por canle (MPE, zona baixa: a canle 1 é a mestra e as 2–16 levan unha nota
    // cada unha). Sen MPE todo vai pola canle 1 e é coma sempre. Le tamén o rango do bend de
    // cada canle (RPN 0) e a mensaxe de configuración de MPE (RPN 6).
    class MPE
    {
    public:
        MPE() { reiniciar(); }
        void reiniciar();
        void procesar (const juce::MidiMessage&);     // pitch bend, presión, CC74 e RPN
        // a expresión que lle toca a unha nota que vai pola canle «c» (1..16)
        double bendSemitonos (int c) const;           // o da canle máis o da mestra, cada un co seu rango
        double bend (int c) const { return bendCanle[canle (c)]; }   // −1..1, só o da canle
        double presion (int c) const { return presionCanle[canle (c)]; }   // 0..1
        double timbre (int c) const { return timbreCanle[canle (c)]; }     // 0..1 (CC74; 0,5 se non chegou)
        int rangoBend (int c) const { return rango[canle (c)]; }           // semitonos
        bool activo() const { return membros > 0; }   // chegou unha configuración MPE con canles
        int canlesDeNotas() const { return membros; }

    private:
        static int canle (int c) { return juce::jlimit (1, 16, c); }
        double bendCanle[17], presionCanle[17], timbreCanle[17];
        int rango[17];
        int rpnMsb[17], rpnLsb[17];
        int membros = 0;
    };
}
