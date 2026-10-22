namespace escoita
{
Estado::Estado (juce::AudioProcessorValueTreeState& p, juce::UndoManager* u, Opcions o)
    : params (p), undo (u), op (std::move (o))
{
}

juce::ValueTree Estado::foto() const
{
    juce::ValueTree t ("PARAMS");
    for (auto* prm : params.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
        {
            if (op.excluir.contains (rp->paramID)) continue;
            juce::ValueTree c ("P");
            c.setProperty ("id", rp->paramID, nullptr);
            c.setProperty ("v", rp->convertFrom0to1 (rp->getValue()), nullptr);
            t.appendChild (c, nullptr);
        }
    if (op.extra)
    {
        juce::ValueTree e ("EXTRA");
        e.appendChild (op.extra(), nullptr);
        t.appendChild (e, nullptr);
    }
    return t;
}

void Estado::aplicar (const juce::ValueTree& orixinal)
{
    novoPaso();
    auto snap = orixinal;
    const int v = (int) snap.getProperty ("version", op.version);
    if (v < op.version && op.migrar) { snap = orixinal.createCopy(); op.migrar (snap, v); }
    for (auto c : snap)
        if (c.hasType ("P"))
            if (auto* rp = params.getParameter (c.getProperty ("id").toString()))
            {
                rp->beginChangeGesture();
                rp->setValueNotifyingHost (rp->convertTo0to1 ((float) (double) c.getProperty ("v")));
                rp->endChangeGesture();
            }
    const auto e = snap.getChildWithName ("EXTRA");
    if (e.isValid() && e.getNumChildren() > 0 && op.aplicarExtra && op.extra)
    {
        const auto despois = e.getChild (0).createCopy(), antes = op.extra();
        auto aplicarExtra = op.aplicarExtra;
        cambio ("estado", [aplicarExtra, despois] { aplicarExtra (despois); }, [aplicarExtra, antes] { aplicarExtra (antes); });
    }
}

void Estado::cambiarLado()
{
    lados[ladoActual] = foto();
    ladoActual = 1 - ladoActual;
    if (! lados[ladoActual].isValid()) lados[ladoActual] = lados[1 - ladoActual].createCopy();
    aplicar (lados[ladoActual]);
}

void Estado::copiarAoOutro() { lados[1 - ladoActual] = foto(); }

juce::File Estado::carpeta() const
{
    if (carpetaPropia != juce::File()) return carpetaPropia;
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile (ESCOITA_PRODUTO).getChildFile ("Presets");
}

juce::Array<juce::File> Estado::presets() const
{
    auto f = carpeta().findChildFiles (juce::File::findFiles, false, "*." + op.extension);
    f.sort();
    return f;
}

bool Estado::gardar (const juce::String& nome)
{
    const auto limpo = juce::File::createLegalFileName (nome.trim());
    if (limpo.isEmpty()) return false;
    auto dir = carpeta();
    if (! dir.createDirectory()) return false;
    auto snap = foto();
    snap.setProperty ("version", op.version, nullptr);
    snap.setProperty ("plugin", ESCOITA_VERSION, nullptr);
    if (auto xml = snap.createXml()) return xml->writeTo (dir.getChildFile (limpo + "." + op.extension));
    return false;
}

bool Estado::cargar (const juce::File& f)
{
    auto xml = juce::parseXML (lerTexto (f));
    if (xml == nullptr || ! xml->hasTagName ("PARAMS")) return false;
    aplicar (juce::ValueTree::fromXml (*xml));
    return true;
}

namespace
{
    struct Accion : juce::UndoableAction
    {
        Accion (std::function<void()> f, std::function<void()> d) : facer (std::move (f)), desfacer (std::move (d)) {}
        bool perform() override { facer(); return true; }
        bool undo() override { desfacer(); return true; }
        std::function<void()> facer, desfacer;
    };
}

void Estado::cambio (const juce::String& nome, std::function<void()> facer, std::function<void()> desfacerCambio)
{
    if (undo != nullptr && ! undo->isPerformingUndoRedo()) undo->perform (new Accion (std::move (facer), std::move (desfacerCambio)), nome);
    else facer();
}

void Estado::engadirAoMenu (juce::PopupMenu& m, std::function<void()> feito)
{
    auto despois = [feito] { if (feito) feito(); };
    const auto lista = presets();
    if (! lista.isEmpty())
    {
        m.addSectionHeader (U ("Os teus"));
        for (const auto& f : lista)
            m.addItem (f.getFileNameWithoutExtension(), [this, f, despois] { cargar (f); despois(); });
        m.addSeparator();
    }
    m.addItem (U ("Gardar como preset…"), [this, feito] { gardarComo ([feito] (bool) { if (feito) feito(); }); });
    m.addItem (U ("Abrir a carpeta dos presets"), [this] { auto d = carpeta(); d.createDirectory(); d.startAsProcess(); });
    m.addItem (ladoActual == 0 ? U ("Copiar A en B") : U ("Copiar B en A"), [this, despois] { copiarAoOutro(); despois(); });
}

void Estado::gardarComo (std::function<void (bool)> feito)
{
    auto* w = new juce::AlertWindow (U ("Gardar preset"), U ("Nome do preset (garda todos os axustes):"), juce::MessageBoxIconType::NoIcon);
    w->addTextEditor ("nome", "", "");
    w->addButton (U ("Gardar"), 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton (U ("Cancelar"), 0, juce::KeyPress (juce::KeyPress::escapeKey));
    w->enterModalState (true, juce::ModalCallbackFunction::create ([this, w, feito] (int r)
    {
        const bool ok = r == 1 && gardar (w->getTextEditorContents ("nome"));
        if (feito) feito (ok);
    }), true);
}
}
