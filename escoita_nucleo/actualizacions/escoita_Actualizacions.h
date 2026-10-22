#pragma once

// Avisar das versións novas: unha vez ao día, nun fío aparte, pregúntase a GitHub cal é a
// última versión publicada do produto (sen mandar ningún dato nin descargar nada máis). A
// resposta gárdase en Escoita/<produto>.versions (Application Support, AppData ou .config),
// así que as outras instancias e as aperturas do mesmo día non volven preguntar.
//
// Úsase cun juce::SharedResourcePointer<escoita::Actualizacions> no editor: o fío vive
// mentres haxa algunha xanela aberta. Cada plugin debuxa o aviso ao seu xeito e debe deixar
// desactivalo (setEnabled).
//
// En Linux, JUCE vai sen curl (e GitHub só fala https): pregúntase co programa curl do sistema
// (ou wget); se non hai ningún, non se avisa.
namespace escoita
{
    // «candidata» é posterior a «actual»? (números separados por puntos; acepta un «v» diante)
    bool versionNova (const juce::String& candidata, const juce::String& actual);

    class Actualizacions : private juce::Thread
    {
    public:
        Actualizacions();
        ~Actualizacions() override;

        // a versión nova (sen «v»), ou baleiro se non hai, non se sabe ou está desactivado
        juce::String newVersion() const;
        bool enabled() const;
        void setEnabled (bool);

        static juce::String downloadsUrl();   // a páxina da última versión
        static juce::String currentVersion() { return ESCOITA_VERSION; }

    private:
        void run() override;
        void startIfDue();
        std::unique_ptr<juce::PropertiesFile> settings;
        mutable juce::CriticalSection lock;
        juce::String latest;
        std::unique_ptr<juce::WebInputStream> stream;
        std::unique_ptr<juce::ChildProcess> programa;   // Linux: curl ou wget

        JUCE_DECLARE_NON_COPYABLE (Actualizacions)
    };
}
