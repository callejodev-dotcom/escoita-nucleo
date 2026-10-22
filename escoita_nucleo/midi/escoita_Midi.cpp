namespace escoita
{
// ---- pedal
void Sostido::noteOn (int n, int vel)
{
    if (n < 0 || n > 127) return;
    if (sostidas[(size_t) n]) { sostidas[(size_t) n] = false; if (acabar) acabar (n); }
    premidas[(size_t) n] = true;
    if (empezar) empezar (n, vel);
}

void Sostido::noteOff (int n)
{
    if (n < 0 || n > 127) return;
    premidas[(size_t) n] = false;
    if (pedalOn) sostidas[(size_t) n] = true;
    else if (acabar) acabar (n);
}

void Sostido::pedal (bool abaixo)
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
int escollerVoz (int voces, const std::function<EstadoVoz (int)>& estado, const std::function<bool (int)>& excluida)
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

int Voces::escoller() const
{
    return escollerVoz (numero(), [this] (int v) { return EstadoVoz { notas[(size_t) v] < 0, cambios[(size_t) v] }; });
}

int Voces::escollerRotando()
{
    const int n = numero();
    for (int k = 1; k <= n; ++k)
    {
        const int v = (ultima + k + n) % n;
        if (notas[(size_t) v] < 0) return v;
    }
    return escoller();   // todas ocupadas: a que leva máis tempo
}

int Voces::vozDaNota (int nota) const
{
    for (size_t v = 0; v < notas.size(); ++v) if (notas[v] == nota) return (int) v;
    return -1;
}

// ---- MPE
void MPE::reiniciar()
{
    for (int c = 0; c <= 16; ++c)
    {
        bendCanle[c] = 0; presionCanle[c] = 0; timbreCanle[c] = 0.5;
        rango[c] = 2; rpnMsb[c] = rpnLsb[c] = 127;
    }
    membros = 0;
}

void MPE::procesar (const juce::MidiMessage& m)
{
    const int c = canle (m.getChannel());
    if (m.isPitchWheel()) bendCanle[c] = (m.getPitchWheelValue() - 8192) / 8192.0;
    else if (m.isChannelPressure()) presionCanle[c] = m.getChannelPressureValue() / 127.0;
    else if (m.isController())
    {
        const int cc = m.getControllerNumber(), v = m.getControllerValue();
        if (cc == 74) timbreCanle[c] = v / 127.0;
        else if (cc == 101) rpnMsb[c] = v;
        else if (cc == 100) rpnLsb[c] = v;
        else if (cc == 6 && rpnMsb[c] == 0 && rpnLsb[c] == 0) rango[c] = v;                     // rango do bend
        else if (cc == 6 && rpnMsb[c] == 0 && rpnLsb[c] == 6 && c == 1)                         // configuración MPE
        {
            membros = juce::jlimit (0, 15, v);
            for (int k = 2; k <= 16; ++k) rango[k] = membros > 0 ? 48 : 2;                      // o rango por defecto das notas en MPE
        }
    }
}

double MPE::bendSemitonos (int c) const
{
    const int k = canle (c);
    return bendCanle[k] * rango[k] + (k != 1 ? bendCanle[1] * rango[1] : 0.0);
}
}
