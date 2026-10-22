#pragma once

// Limitador de seguridade: a saída nunca pasa do teito. Ataque instantáneo (o pico de agora
// xa se limita) e sen anticipación (non engade latencia); solta en «soltar» segundos.
namespace escoita
{
    class Teito
    {
    public:
        // teito en lineal (0,891 ≈ −1 dBFS)
        void preparar (double frecuencia, double soltar = 0.080, double teitoLineal = 0.891)
        {
            coef = std::exp (-1.0 / (soltar * frecuencia));
            teito = teitoLineal;
            env = 0;
        }
        void reiniciar() { env = 0; }

        double procesar (double x)   // mono
        {
            env = std::max (std::abs (x), env * coef);
            return env > teito ? x * (teito / env) : x;
        }
        void procesar (double& l, double& r)   // estéreo, coa mesma ganancia nas dúas canles
        {
            env = std::max (std::max (std::abs (l), std::abs (r)), env * coef);
            if (env > teito) { const double g = teito / env; l *= g; r *= g; }
        }

    private:
        double coef = 0.999, teito = 0.891, env = 0;
    };
}
