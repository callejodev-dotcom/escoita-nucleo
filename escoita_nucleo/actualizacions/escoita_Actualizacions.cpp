namespace escoita
{
bool versionNova (const juce::String& candidata, const juce::String& actual)
{
    auto partes = [] (const juce::String& s)
    {
        juce::StringArray a;
        a.addTokens (s.trim().trimCharactersAtStart ("vV"), ".", {});
        std::array<int, 3> n {};
        for (int i = 0; i < 3 && i < a.size(); ++i) n[(size_t) i] = a[i].getIntValue();
        return n;
    };
    return partes (candidata) > partes (actual);
}

juce::String Actualizacions::downloadsUrl() { return "https://github.com/" ESCOITA_REPO "/releases/latest"; }

Actualizacions::Actualizacions() : juce::Thread (ESCOITA_PRODUTO ": versions")
{
   #if defined (ESCOITA_SEN_REDE) && ESCOITA_SEN_REDE
    // nas probas non se sae á rede nin se tocan os axustes; ESCOITA_NOVA_VERSION finxe unha versión nova
    latest = juce::SystemStats::getEnvironmentVariable ("ESCOITA_NOVA_VERSION", {});
   #else
    juce::PropertiesFile::Options o;
    o.applicationName = ESCOITA_PRODUTO;
    o.folderName = "Escoita";
    o.filenameSuffix = ".versions";
    o.osxLibrarySubFolder = "Application Support";
    o.millisecondsBeforeSaving = -1;   // gárdase á man (tamén desde o fío)
    settings = std::make_unique<juce::PropertiesFile> (o);
    startIfDue();
   #endif
}

Actualizacions::~Actualizacions()
{
    signalThreadShouldExit();
    {
        const juce::ScopedLock sl (lock);
        if (stream != nullptr) stream->cancel();
    }
    stopThread (10000);
}

bool Actualizacions::enabled() const { return settings == nullptr || settings->getBoolValue ("avisarVersions", true); }

void Actualizacions::setEnabled (bool on)
{
    if (settings == nullptr) return;
    settings->setValue ("avisarVersions", on);
    settings->saveIfNeeded();
    if (on) startIfDue();
}

juce::String Actualizacions::newVersion() const
{
    if (! enabled()) return {};
    const juce::ScopedLock sl (lock);
    return versionNova (latest, ESCOITA_VERSION) ? latest : juce::String();
}

// o que se soubo hoxe vale; se pasou un día (ou nunca se preguntou), pregúntase
void Actualizacions::startIfDue()
{
    if (settings == nullptr || ! enabled()) return;
    {
        const juce::ScopedLock sl (lock);
        latest = settings->getValue ("ultimaVersion");
    }
    const auto now = juce::Time::currentTimeMillis();
    const auto last = settings->getValue ("ultimaComprobacion").getLargeIntValue();
    if (now >= last && now - last < 24 * 3600 * 1000LL) return;
    if (! isThreadRunning()) startThread (juce::Thread::Priority::low);
}

void Actualizacions::run()
{
    {
        const juce::ScopedLock sl (lock);
        if (threadShouldExit()) return;
        stream = std::make_unique<juce::WebInputStream> (juce::URL ("https://api.github.com/repos/" ESCOITA_REPO "/releases/latest"), false);
        stream->withExtraHeaders (juce::String ("Accept: application/vnd.github+json\r\nUser-Agent: ")
                                  + juce::String (ESCOITA_PRODUTO).removeCharacters (" ") + "/" ESCOITA_VERSION)
               .withConnectionTimeout (8000);
    }
    juce::String text;
    if (stream->connect (nullptr) && stream->getStatusCode() == 200 && ! threadShouldExit())
        text = stream->readEntireStreamAsString();
    {
        const juce::ScopedLock sl (lock);
        stream.reset();
    }
    // sen rede ou sen resposta: nada (volverase preguntar a próxima vez)
    const auto tag = juce::JSON::parse (text).getProperty ("tag_name", {}).toString().trim().trimCharactersAtStart ("vV");
    if (threadShouldExit() || tag.isEmpty()) return;
    {
        const juce::ScopedLock sl (lock);
        latest = tag;
    }
    settings->setValue ("ultimaVersion", tag);
    settings->setValue ("ultimaComprobacion", juce::String (juce::Time::currentTimeMillis()));
    settings->saveIfNeeded();
}
}
