namespace escoita
{
juce::String maiusculas (const juce::String& t)
{
    juce::String out;
    out.preallocateBytes (t.getNumBytesAsUTF8());
    for (auto p = t.getCharPointer(); ! p.isEmpty();)
    {
        auto c = p.getAndAdvance();
        if (c >= 'a' && c <= 'z') c -= 32;
        else if (c >= 0xe0 && c <= 0xfe && c != 0xf7) c -= 32;   // à…þ (á, é, í, ó, ú, ñ, ü)
        out += c;
    }
    return out;
}

juce::String numero (double valor, int decimais)
{
    return juce::String (valor, decimais).replaceCharacter ('.', ',').replaceCharacter ('-', (juce::juce_wchar) 0x2212);
}

// UTF-8 válido (sen secuencias longas de máis, suplentes nin cortado ao final)? Se non, lese coma Latin-1
static bool utf8Valido (const unsigned char* p, size_t n)
{
    for (size_t i = 0; i < n;)
    {
        const unsigned b = p[i];
        if (b < 0x80) { ++i; continue; }
        const size_t extra = b >= 0xC2 && b <= 0xDF ? 1 : b >= 0xE0 && b <= 0xEF ? 2 : b >= 0xF0 && b <= 0xF4 ? 3 : 0;
        if (extra == 0 || n - i <= extra) return false;   // descoñecido ou cortado ao final
        for (size_t k = 1; k <= extra; ++k) if ((p[i + k] & 0xC0) != 0x80) return false;
        const unsigned c1 = p[i + 1];
        if ((b == 0xE0 && c1 < 0xA0) || (b == 0xED && c1 >= 0xA0) || (b == 0xF0 && c1 < 0x90) || (b == 0xF4 && c1 >= 0x90)) return false;
        i += extra + 1;
    }
    return true;
}

juce::String textoDeBytes (const void* datos, size_t tamano)
{
    auto* p = static_cast<const unsigned char*> (datos);
    if (p == nullptr || tamano == 0) return {};
    if (tamano >= 3 && p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) { p += 3; tamano -= 3; }   // BOM
    size_t n = 0;
    while (n < tamano && p[n] != 0) ++n;   // o texto remata no primeiro cero
    if (utf8Valido (p, n)) return juce::String::fromUTF8 (reinterpret_cast<const char*> (p), (int) n);
    juce::String s;
    s.preallocateBytes (n * 2);
    for (size_t i = 0; i < n; ++i) s += (juce::juce_wchar) p[i];
    return s;
}

juce::String lerTexto (const juce::File& f, juce::int64 maximo)
{
    if (! f.existsAsFile() || f.getSize() > maximo) return {};
    juce::MemoryBlock mb;
    if (! f.loadFileAsData (mb)) return {};
    return textoDeBytes (mb.getData(), mb.getSize());
}

juce::String textoDoDAW (const juce::String& t)
{
    std::string bytes;
    bool alto = false;
    for (auto c : t)
    {
        if (c > 0xff) return t;   // xa son caracteres de verdade
        alto = alto || c >= 0x80;
        bytes.push_back ((char) c);
    }
    return alto ? textoDeBytes (bytes.data(), bytes.size()) : t;
}

juce::AudioParameterChoiceAttributes atributosEleccion (const juce::StringArray& opcions)
{
    return juce::AudioParameterChoiceAttributes().withValueFromStringFunction ([opcions] (const juce::String& t)
    {
        const auto r = textoDoDAW (t).trim();
        for (int i = 0; i < opcions.size(); ++i)
            if (opcions[i].equalsIgnoreCase (r)) return i;
        return juce::jlimit (0, opcions.size() - 1, r.getIntValue());
    });
}
}
