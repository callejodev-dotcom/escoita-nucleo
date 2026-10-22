#pragma once

// Presets propios, A/B e desfacer, para calquera plugin con AudioProcessorValueTreeState.
//
// A «foto» dun estado son os valores de todos os parámetros (agás os excluídos, coma o bypass)
// e, se o plugin ten estado que non son parámetros (os sons editados dun sinte, a memoria
// interna…), o que devolva Opcions::extra. Unha foto é un ValueTree:
//   <PARAMS version=.. plugin=..> <P id=".." v=".."/> … [<EXTRA>…</EXTRA>] </PARAMS>
// (o mesmo formato que xa gardaba Barullo Tune nos .btpreset).
//
// Desfacer: o UndoManager é o do APVTS (os cambios de parámetros rexístranse sós). Para o
// estado que non é parámetro, o plugin fai os cambios con cambio(): queda no mesmo paso.
// Un paso novo empeza con novoPaso() (por exemplo, en cada clic na xanela).
namespace escoita
{
    class Estado
    {
    public:
        struct Opcions
        {
            juce::String extension { "preset" };   // dos ficheiros dos presets, sen punto
            juce::StringArray excluir;             // parámetros que non van nas fotos (bypass, comparar…)
            int version = 1;                       // versión do formato do plugin (vai en cada preset)
            // o estado que non é parámetro (opcional): sacalo e poñelo
            std::function<juce::ValueTree()> extra;
            std::function<void (const juce::ValueTree&)> aplicarExtra;
            // presets doutras versións (opcional): tradúcese a foto antes de aplicala
            std::function<void (juce::ValueTree&, int desdeVersion)> migrar;
        };

        Estado (juce::AudioProcessorValueTreeState&, juce::UndoManager*, Opcions);

        // ---- fotos
        juce::ValueTree foto() const;
        void aplicar (const juce::ValueTree&);   // nun paso de desfacer propio

        // ---- A/B: dous xogos de axustes para comparar
        int lado() const { return ladoActual; }  // 0 = A, 1 = B
        void cambiarLado();                      // garda o actual no seu lado e carga o outro (baleiro: copia o actual)
        void copiarAoOutro();                    // o actual, ao outro lado

        // ---- presets propios: Documentos/<produto>/Presets/*.<extension>
        juce::File carpeta() const;
        void setCarpeta (const juce::File& f) { carpetaPropia = f; }
        juce::Array<juce::File> presets() const;
        bool gardar (const juce::String& nome);
        bool cargar (const juce::File&);

        // ---- desfacer
        juce::UndoManager* desfacer() const { return undo; }
        void novoPaso() { if (undo != nullptr) undo->beginNewTransaction(); }
        // un cambio do estado que non é parámetro, que se pode desfacer
        void cambio (const juce::String& nome, std::function<void()> facer, std::function<void()> desfacerCambio);

        // ---- na interface: as entradas dos presets propios e do A/B nun menú (cargar, gardar
        // como…, abrir a carpeta, copiar A en B). «feito» chámase despois de cada acción.
        void engadirAoMenu (juce::PopupMenu&, std::function<void()> feito = {});
        // pedir un nome e gardar (unha xanela do sistema); «feito» co resultado
        void gardarComo (std::function<void (bool gardado)> feito = {});

    private:
        juce::AudioProcessorValueTreeState& params;
        juce::UndoManager* undo;
        Opcions op;
        int ladoActual = 0;
        juce::ValueTree lados[2];
        juce::File carpetaPropia;

        JUCE_DECLARE_NON_COPYABLE (Estado)
    };
}
