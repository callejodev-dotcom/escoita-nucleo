#pragma once
// O pedal de sostemento e o reparto de voces, sen JUCE: os motores que non usan JUCE (as
// probas dos motores, por exemplo) poden incluír só este ficheiro.
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <vector>

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

    // ---- pedal
    inline void Sostido::noteOn (int n, int vel)
    {
        if (n < 0 || n > 127) return;
        if (sostidas[(size_t) n]) { sostidas[(size_t) n] = false; if (acabar) acabar (n); }
        premidas[(size_t) n] = true;
        if (empezar) empezar (n, vel);
    }

    inline void Sostido::noteOff (int n)
    {
        if (n < 0 || n > 127) return;
        premidas[(size_t) n] = false;
        if (pedalOn) sostidas[(size_t) n] = true;
        else if (acabar) acabar (n);
    }

    inline void Sostido::pedal (bool abaixo)
    {
        pedalOn = abaixo;
        if (abaixo) return;
        for (int n = 0; n < 128; ++n)
            if (sostidas[(size_t) n])
            {
                sostidas[(size_t) n] = false;
                if (! premidas[(size_t) n] && acabar) acabar (n);
            }
    }

    // ---- voces
    inline int escollerVoz (int voces, const std::function<EstadoVoz (int)>& estado, const std::function<bool (int)>& excluida)
    {
        int mellor = -1;
        EstadoVoz m {};
        for (int v = 0; v < voces; ++v)
        {
            if (excluida && excluida (v)) continue;
            const auto e = estado (v);
            if (mellor < 0) { mellor = v; m = e; continue; }
            if (e.libre != m.libre) { if (e.libre) { mellor = v; m = e; } }
            else if (e.cambio < m.cambio) { mellor = v; m = e; }
        }
        return mellor;
    }

    inline int Voces::escoller() const
    {
        return escollerVoz (numero(), [this] (int v) { return EstadoVoz { notas[(size_t) v] < 0, cambios[(size_t) v] }; });
    }

    inline int Voces::escollerRotando()
    {
        const int n = numero();
        for (int k = 1; k <= n; ++k)
        {
            const int v = (ultima + k + n) % n;
            if (notas[(size_t) v] < 0) return v;
        }
        return escoller();   // todas ocupadas: a que leva máis tempo
    }

    inline int Voces::vozDaNota (int nota) const
    {
        for (size_t v = 0; v < notas.size(); ++v) if (notas[v] == nota) return (int) v;
        return -1;
    }
}
