namespace escoita
{
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
