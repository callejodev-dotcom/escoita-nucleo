#pragma once
#include "escoita_Voces.h"

// MIDI para sintes: o pedal de sostemento e o reparto de voces (en escoita_Voces.h, sen JUCE) e a
// expresión por canle (MPE). Nada disto reserva memoria nin bloquea: vale no fío do audio.
namespace escoita
{
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
