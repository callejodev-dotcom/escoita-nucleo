namespace escoita
{
void MandoEditable::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu() || e.mods.isCommandDown() || e.mods.isCtrlDown()) { escribir(); return; }
    sensibilidade = getMouseDragSensitivity();
    if (e.mods.isShiftDown()) setMouseDragSensitivity (sensibilidade * 5);   // fino
    juce::Slider::mouseDown (e);
}

void MandoEditable::mouseUp (const juce::MouseEvent& e)
{
    juce::Slider::mouseUp (e);
    setMouseDragSensitivity (sensibilidade);
}

bool MandoEditable::keyPressed (const juce::KeyPress& k)
{
    if (k.getKeyCode() == juce::KeyPress::returnKey) { escribir(); return true; }   // intro: escribir o valor
    return juce::Slider::keyPressed (k);
}

void MandoEditable::escribir()
{
    if (param == nullptr || getParentComponent() == nullptr) return;
    recadro = std::make_unique<juce::TextEditor>();
    recadro->setJustification (juce::Justification::centred);
    recadro->setFont (fonte);
    recadro->setText (param->getCurrentValueAsText(), false);
    recadro->selectAll();
    recadro->setTitle (getTitle() + U (": escribir o valor"));
    auto* pai = getParentComponent();
    // (nun mando redondo, enriba do seu valor, debaixo)
    const auto onde = isRotary() ? getBounds().withTop (getBottom() - 22) : getBounds();
    recadro->setBounds (onde.withSizeKeepingCentre (juce::jmax (78, getWidth()), 24));
    pai->addAndMakeVisible (*recadro);
    recadro->grabKeyboardFocus();
    // (o punteiro seguro créase fóra: MSVC le mal o «this» nunha captura dun lambda dentro doutro)
    const juce::Component::SafePointer<MandoEditable> seguro (this);
    auto rematar = [this, seguro] (bool aplicar)
    {
        if (recadro == nullptr) return;
        const auto texto = recadro->getText();
        juce::MessageManager::callAsync ([sp = seguro, texto, aplicar]
        {
            if (sp == nullptr) return;
            if (aplicar) sp->aplicarTexto (texto);
            sp->recadro.reset();
            sp->grabKeyboardFocus();
        });
    };
    recadro->onReturnKey = [rematar] { rematar (true); };
    recadro->onEscapeKey = [rematar] { rematar (false); };
    recadro->onFocusLost = [rematar] { rematar (true); };
}

bool MandoEditable::aplicarTexto (const juce::String& texto)
{
    if (param == nullptr || texto.trim().isEmpty()) return false;
    const auto t = texto.trim().replaceCharacter (',', '.').replace (U ("−"), "-");
    setValue (param->convertFrom0to1 (param->getValueForText (t)), juce::sendNotificationSync);
    return true;
}
}
