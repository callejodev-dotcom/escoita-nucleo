#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

// Limitador de seguridade: a saída nunca pasa do teito. Sen JUCE: pódese incluír só (os motores sen JUCE).
//
// Dous xeitos:
//  - sen anticipación (por defecto): ataque instantáneo, o pico de agora xa se limita e non hai latencia.
//    Cando traballa moito soa áspero (a ganancia cambia de golpe en cada pico, coma un clip);
//  - con anticipación (uns ms): o son sae atrasado ese tempo e a ganancia baixa suave antes de que
//    chegue o pico (o mínimo do que fai falta na xanela, alisado coa media desa xanela), así o pico
//    queda xusto no teito sen esmagalo; a ganancia aguanta 25 ms antes de soltar (máis ca un ciclo de
//    40 Hz: nos graves non ondula dentro de cada ciclo, que é o que soa a distorsión). O plugin declara a latencia (latencia()) e atrasa o mesmo o
//    que non pasa por aquí, coma o bypass (Atraso).
// Os dous soltan en «soltar» segundos e non tocan o que vai por baixo do teito (con anticipación,
// sae tal cal, só que atrasado).
namespace escoita
{
    class Teito
    {
    public:
        // teito en lineal (0,891 ≈ −1 dBFS); anticipación en segundos (0: sen latencia)
        void preparar (double frecuencia, double soltar = 0.080, double teitoLineal = 0.891, double anticipacion = 0.0)
        {
            coef = std::exp (-1.0 / (soltar * frecuencia));
            teito = teitoLineal;
            n = anticipacion > 0 ? std::max (1, (int) std::lround (anticipacion * frecuencia)) : 0;
            aguantar = (int) std::lround (0.025 * frecuencia);
            bufL.assign ((size_t) std::max (1, n), 0.0); bufR.assign ((size_t) std::max (1, n), 0.0);
            preciso.assign ((size_t) n + 1, 1.0); solto.assign ((size_t) std::max (1, n), 1.0);
            reiniciar();
        }
        void reiniciar()
        {
            env = 0; rel = 1; posB = posP = posS = 0; aguanta = 0;
            std::fill (bufL.begin(), bufL.end(), 0.0); std::fill (bufR.begin(), bufR.end(), 0.0);
            std::fill (preciso.begin(), preciso.end(), 1.0); std::fill (solto.begin(), solto.end(), 1.0);
        }
        int latencia() const { return n; }   // mostras

        double procesar (double x)   // mono
        {
            if (n == 0)
            {
                env = std::max (std::abs (x), env * coef);
                return env > teito ? x * (teito / env) : x;
            }
            double r = x;
            anticipar (x, r);
            return x;
        }
        void procesar (double& l, double& r)   // estéreo, coa mesma ganancia nas dúas canles
        {
            if (n == 0)
            {
                env = std::max (std::max (std::abs (l), std::abs (r)), env * coef);
                if (env > teito) { const double g = teito / env; l *= g; r *= g; }
                return;
            }
            anticipar (l, r);
        }

    private:
        // A mostra sae n despois. A ganancia: a que pide cada mostra, o mínimo das últimas n+1 (así cobre
        // o pico desde n antes ata que sae), que aguanta e solta amodo, e a media das últimas n: baixa suave e, cando
        // sae o pico, todas as da media xa o cubrían, así que nunca pasa
        void anticipar (double& l, double& r)
        {
            const double m = std::max (std::abs (l), std::abs (r));
            preciso[(size_t) posP] = m > teito ? teito / m : 1.0;
            if (++posP > n) posP = 0;
            const double minimo = *std::min_element (preciso.begin(), preciso.end());
            if (minimo <= rel) { rel = minimo; aguanta = aguantar; }
            else if (aguanta > 0) --aguanta;
            else { rel += (1.0 - rel) * (1.0 - coef); if (rel > 1.0 - 1.0e-7) rel = 1.0; }
            solto[(size_t) posS] = rel;
            if (++posS >= n) posS = 0;
            double suma = 0;
            for (double v : solto) suma += v;   // (sumado de novo cada vez: sen erro que se acumule; n ones dan 1 xusto)
            const double g = suma / n;

            const double dl = bufL[(size_t) posB], dr = bufR[(size_t) posB];
            bufL[(size_t) posB] = l; bufR[(size_t) posB] = r;
            if (++posB >= n) posB = 0;
            l = std::clamp (dl * g, -teito, teito);   // (o clamp só polo redondeo)
            r = std::clamp (dr * g, -teito, teito);
        }

        double coef = 0.999, teito = 0.891, env = 0, rel = 1;
        int n = 0, posB = 0, posP = 0, posS = 0, aguantar = 1200, aguanta = 0;
        std::vector<double> bufL, bufR, preciso, solto;
    };

    // un atraso de n mostras, para que o que non pasa polo teito (o bypass, o seco) vaia á par
    class Atraso
    {
    public:
        void preparar (int mostras) { n = std::max (0, mostras); buf.assign ((size_t) std::max (1, n), 0.0); pos = 0; }
        void reiniciar() { std::fill (buf.begin(), buf.end(), 0.0); pos = 0; }
        double procesar (double x)
        {
            if (n == 0) return x;
            const double y = buf[(size_t) pos];
            buf[(size_t) pos] = x;
            if (++pos >= n) pos = 0;
            return y;
        }
    private:
        int n = 0, pos = 0;
        std::vector<double> buf;
    };
}
