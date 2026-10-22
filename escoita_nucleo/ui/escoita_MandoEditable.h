#pragma once

// Un mando (calquera estilo de juce::Slider) ligado a un parámetro, ao que se lle pode escribir
// o valor coma no DAW («35 ms», «−3,5 dB», «E40»): clic dereito, Cmd/Ctrl-clic ou intro. Con
// maiúsculas ao arrastrar, móvese fino. O aspecto é o do LookAndFeel de cada plugin.
namespace escoita
{
    class MandoEditable : public juce::Slider
    {
    public:
        explicit MandoEditable (SliderStyle estilo = LinearBar) : juce::Slider (estilo, NoTextBox) { setWantsKeyboardFocus (true); }

        juce::RangedAudioParameter* param = nullptr;    // o parámetro (para ler e entender o texto)
        juce::Font fonte { juce::FontOptions (14.0f) }; // a do recadro onde se escribe

        void escribir();                                 // abre o recadro co valor de agora
        bool aplicarTexto (const juce::String&);         // o texto coma no DAW; coma decimal e − valen

        void mouseDown (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;
        bool keyPressed (const juce::KeyPress&) override;

    private:
        int sensibilidade = 220;
        std::unique_ptr<juce::TextEditor> recadro;
    };
}
