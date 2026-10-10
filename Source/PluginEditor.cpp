#include "PluginEditor.h"

namespace
{
    constexpr float baseW = 560.f, baseH = 426.f;

    float textWidth (const juce::Font& f, const juce::String& t)
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText (f, t, 0.f, 0.f);
        return ga.getBoundingBox (0, -1, true).getWidth();
    }

    juce::Font mk (float size, bool bold = false, float kerning = 0.f)
    {
        auto o = juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain);
        if (kerning != 0.f) o = o.withKerningFactor (kerning);
        return juce::Font (o);
    }

    // Orthographe usuelle des tonalités (Eb plutôt que D#, etc.)
    juce::String spell (int pc, bool minor)
    {
        static const char* maj[12] = { "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
        static const char* min[12] = { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "Bb", "B" };
        pc = ((pc % 12) + 12) % 12;
        return juce::String (minor ? min[pc] : maj[pc]);
    }

    juce::String spellFr (int pc, bool minor)
    {
        static const char* maj[12] = { "Do", "Réb", "Ré", "Mib", "Mi", "Fa", "Fa#", "Sol", "Lab", "La", "Sib", "Si" };
        static const char* min[12] = { "Do", "Do#", "Ré", "Mib", "Mi", "Fa", "Fa#", "Sol", "Sol#", "La", "Sib", "Si" };
        pc = ((pc % 12) + 12) % 12;
        return juce::String::fromUTF8 (minor ? min[pc] : maj[pc]);
    }

    bool inScale (int pc, int tonic, bool minor)
    {
        static const bool maj[12] = { 1,0,1,0,1,1,0,1,0,1,0,1 };
        static const bool min[12] = { 1,0,1,1,0,1,0,1,1,0,1,0 };
        const int d = ((pc - tonic) % 12 + 12) % 12;
        return minor ? min[d] : maj[d];
    }
}

//==============================================================================
KeyDetectorEditor::KeyDetectorEditor (KeyDetectorProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      memoryAttachment (p.apvts, "memory", memorySlider)
{
    setLookAndFeel (&lnf);

    addAndMakeVisible (resetButton);
    resetButton.onClick = [this]
    {
        proc.detector.requestReset();
        shownTonic = -1; pendingCount = 0; gauge = 0.f;
        shown.fill (0.f);
    };

    memorySlider.setSliderStyle (juce::Slider::LinearHorizontal);
    memorySlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    memorySlider.onValueChange = [this] { repaint(); };
    addAndMakeVisible (memorySlider);

    setResizable (true, true);
    setResizeLimits (420, (int) (420 * baseH / baseW), 1120, (int) (1120 * baseH / baseW));
    getConstrainer()->setFixedAspectRatio ((double) baseW / (double) baseH);
    setSize ((int) baseW, (int) baseH);

    startTimerHz (30);
}

KeyDetectorEditor::~KeyDetectorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
juce::String KeyDetectorEditor::keyName (int t, bool minor)
{
    return spell (t, minor) + (minor ? " minor" : " major");
}

juce::String KeyDetectorEditor::keyNameFr (int t, bool minor)
{
    return spellFr (t, minor) + (minor ? " mineur" : " majeur");
}

juce::String KeyDetectorEditor::camelot (int t, bool minor)
{
    return juce::String (KeyDetector::camelotNumber (t, minor)) + (minor ? "A" : "B");
}

//==============================================================================
void KeyDetectorEditor::timerCallback()
{
    chroma = proc.detector.getChroma();
    auto r = KeyDetector::analyse (chroma);
    const bool active = r.tonic >= 0;

    levelFade += ((active ? 1.f : 0.f) - levelFade) * 0.12f;

    if (active)
    {
        if (shownTonic < 0)
        {
            shownTonic = r.tonic; shownMinor = r.minor; pendingCount = 0;
        }
        else if (r.tonic != shownTonic || r.minor != shownMinor)
        {
            // on ne change l'affichage que si un autre candidat domine nettement et durablement
            const float cur = r.scores[(size_t) ((shownMinor ? 12 : 0) + shownTonic)];
            if (r.correlation - cur > 0.03f) { if (++pendingCount > 24) { shownTonic = r.tonic; shownMinor = r.minor; pendingCount = 0; } }
            else pendingCount = 0;
        }
        else pendingCount = 0;
    }
    else
    {
        shownTonic = -1; pendingCount = 0;
    }

    result = r;
    if (shownTonic >= 0)
    {
        result.tonic = shownTonic;
        result.minor = shownMinor;
        result.correlation = r.scores[(size_t) ((shownMinor ? 12 : 0) + shownTonic)];

        float bestAlt = -2.f;
        for (int i = 0; i < 24; ++i)
        {
            if (i == (shownMinor ? 12 : 0) + shownTonic) continue;
            if (r.scores[(size_t) i] > bestAlt) { bestAlt = r.scores[(size_t) i]; result.altTonic = i % 12; result.altMinor = i >= 12; }
        }
        result.altCorrelation = bestAlt;
    }

    float mx = 1e-6f;
    for (float v : chroma) mx = std::max (mx, v);
    for (size_t i = 0; i < 12; ++i)
        shown[i] += 0.22f * ((active ? chroma[i] / mx : 0.f) - shown[i]);

    const float gTarget = shownTonic >= 0 ? juce::jlimit (0.f, 1.f, result.correlation) : 0.f;
    gauge += 0.10f * (gTarget - gauge);

    const auto target = shownTonic < 0 ? Pal::minor : (shownMinor ? Pal::minor : Pal::major);
    accent = accent.interpolatedWith (target, 0.10f);
    lnf.accent = accent;

    repaint();
}

//==============================================================================
void KeyDetectorEditor::resized()
{
    s = (float) getWidth() / baseW;
    const float w = (float) getWidth();
    const float pad = 24.f * s;

    float y = pad;
    rHeader = { pad, y, w - 2 * pad, 28.f * s };           y = rHeader.getBottom() + 14.f * s;
    rHero   = { pad, y, w - 2 * pad, 170.f * s };          y = rHero.getBottom() + 12.f * s;
    rChroma = { pad, y, w - 2 * pad, 112.f * s };          y = rChroma.getBottom() + 14.f * s;
    rFooter = { pad, y, w - 2 * pad, 28.f * s };

    const float gs = 116.f * s;
    rGauge = { rHero.getRight() - gs - 22.f * s, rHero.getCentreY() - gs * 0.5f, gs, gs };

    resetButton.setBounds (juce::Rectangle<float> (rHeader.getRight() - 76.f * s, rHeader.getY() + 1.f * s, 76.f * s, 26.f * s).toNearestInt());
    memorySlider.setBounds (juce::Rectangle<float> (rFooter.getX() + 78.f * s, rFooter.getY(), rFooter.getWidth() - 78.f * s - 56.f * s, rFooter.getHeight()).toNearestInt());
}

//==============================================================================
void KeyDetectorEditor::paint (juce::Graphics& g)
{
    // fond
    g.setGradientFill (juce::ColourGradient::vertical (Pal::bg1, 0.f, Pal::bg0, (float) getHeight()));
    g.fillAll();

    // halo d'ambiance qui suit la couleur de la tonalité
    {
        const float cx = rHero.getRight() - 90.f * s, cy = rHero.getY() + 20.f * s;
        juce::ColourGradient halo (accent.withAlpha (0.16f * (0.35f + 0.65f * levelFade)), cx, cy,
                                   accent.withAlpha (0.f), cx - 340.f * s, cy + 40.f * s, true);
        g.setGradientFill (halo);
        g.fillAll();
    }

    drawHeader (g, rHeader);
    drawHero   (g, rHero);
    drawChroma (g, rChroma);

    // pied : mémoire
    g.setColour (Pal::textMute);
    g.setFont (mk (10.f * s, true, 0.18f));
    g.drawText ("MEMORY", rFooter.withWidth (70.f * s).toNearestInt(), juce::Justification::centredLeft);
    g.setColour (Pal::textDim);
    g.setFont (mk (12.f * s, true));
    g.drawText (juce::String ((int) std::round (memorySlider.getValue())) + " s",
                rFooter.withTrimmedLeft (rFooter.getWidth() - 50.f * s).toNearestInt(), juce::Justification::centredRight);
}

float KeyDetectorEditor::drawChip (juce::Graphics& g, const juce::String& text, float x, float y, float h,
                                   juce::Colour c, bool filled)
{
    const auto f = mk (10.5f * s, true, 0.08f);
    const float w = textWidth (f, text) + 20.f * s;
    juce::Rectangle<float> r (x, y, w, h);
    if (filled) { g.setColour (c.withAlpha (0.16f)); g.fillRoundedRectangle (r, h * 0.5f); }
    g.setColour (c.withAlpha (filled ? 0.55f : 0.30f));
    g.drawRoundedRectangle (r.reduced (0.5f), h * 0.5f, 1.f);
    g.setColour (filled ? c : Pal::textDim);
    g.setFont (f);
    g.drawText (text, r, juce::Justification::centred);
    return w;
}

void KeyDetectorEditor::drawHeader (juce::Graphics& g, juce::Rectangle<float> r)
{
    // pastille + titre
    const float d = 10.f * s;
    g.setColour (accent.withAlpha (0.28f));
    g.fillEllipse (r.getX(), r.getCentreY() - d, d * 2.f, d * 2.f);
    g.setColour (accent);
    g.fillEllipse (r.getX() + d * 0.5f, r.getCentreY() - d * 0.5f, d, d);

    g.setColour (Pal::text);
    g.setFont (mk (12.5f * s, true, 0.22f));
    g.drawText ("KEY DETECTOR", juce::Rectangle<float> (r.getX() + 30.f * s, r.getY(), 220.f * s, r.getHeight()).toNearestInt(),
                juce::Justification::centredLeft);

    // état
    const bool active = shownTonic >= 0;
    g.setColour (active ? Pal::good : Pal::textMute);
    g.setFont (mk (10.f * s, true, 0.16f));
    g.drawText (active ? "ANALYZING" : "WAITING FOR AUDIO",
                juce::Rectangle<float> (r.getRight() - 76.f * s - 220.f * s, r.getY(), 208.f * s, r.getHeight()).toNearestInt(),
                juce::Justification::centredRight);
}

void KeyDetectorEditor::drawHero (juce::Graphics& g, juce::Rectangle<float> r)
{
    const float rad = 20.f * s;
    g.setColour (Pal::card);
    g.fillRoundedRectangle (r, rad);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.045f), r.getX(), r.getY(),
                                             juce::Colours::transparentWhite, r.getX(), r.getY() + 70.f * s, false));
    g.fillRoundedRectangle (r, rad);
    g.setColour (Pal::stroke);
    g.drawRoundedRectangle (r.reduced (0.5f), rad, 1.f);

    drawGauge (g, rGauge);

    const float left = r.getX() + 30.f * s;
    const float baseline = r.getY() + 94.f * s;

    if (shownTonic < 0)
    {
        g.setColour (Pal::text);
        g.setFont (mk (30.f * s, true));
        g.drawSingleLineText ("Play your track", (int) left, (int) (r.getY() + 80.f * s));
        g.setColour (Pal::textDim);
        g.setFont (mk (13.f * s));
        g.drawSingleLineText ("The key appears after a few seconds of audio.", (int) left, (int) (r.getY() + 106.f * s));
        return;
    }

    g.beginTransparencyLayer (juce::jlimit (0.f, 1.f, levelFade));

    // note + mode
    const auto note = spell (result.tonic, result.minor);
    const auto noteFont = mk (88.f * s, true);
    g.setColour (Pal::text);
    g.setFont (noteFont);
    g.drawSingleLineText (note, (int) left, (int) baseline);

    const float nw = textWidth (noteFont, note);
    g.setColour (accent);
    g.setFont (mk (32.f * s, true));
    g.drawSingleLineText (result.minor ? "minor" : "major", (int) (left + nw + 12.f * s), (int) baseline);

    // nom français
    g.setColour (Pal::textDim);
    g.setFont (mk (15.f * s));
    g.drawSingleLineText (keyNameFr (result.tonic, result.minor), (int) (left + 3.f * s), (int) (baseline + 26.f * s));

    // pastilles
    const float chipH = 24.f * s;
    float x = left + 3.f * s;
    const float y = r.getBottom() - chipH - 14.f * s;
    const float xMax = rGauge.getX() - 14.f * s;

    x += drawChip (g, "CAMELOT " + camelot (result.tonic, result.minor), x, y, chipH, accent, true) + 8.f * s;

    const int relT = result.minor ? (result.tonic + 3) % 12 : (result.tonic + 9) % 12;
    const auto relTxt = "REL  " + spell (relT, ! result.minor) + (result.minor ? " major" : " minor");
    if (x + textWidth (mk (10.5f * s, true, 0.08f), relTxt) + 20.f * s < xMax)
        x += drawChip (g, relTxt, x, y, chipH, Pal::textDim, false) + 8.f * s;

    if (result.altTonic >= 0)
    {
        const auto altTxt = "ALT  " + keyName (result.altTonic, result.altMinor);
        if (x + textWidth (mk (10.5f * s, true, 0.08f), altTxt) + 20.f * s < xMax)
            drawChip (g, altTxt, x, y, chipH, Pal::textDim, false);
    }

    g.endTransparencyLayer();
}

void KeyDetectorEditor::drawGauge (juce::Graphics& g, juce::Rectangle<float> r)
{
    const float thick = 8.f * s;
    const float R = r.getWidth() * 0.5f - thick;
    const auto c = r.getCentre();
    const float a0 = -0.75f * juce::MathConstants<float>::pi;
    const float a1 =  0.75f * juce::MathConstants<float>::pi;

    juce::Path track;
    track.addCentredArc (c.x, c.y, R, R, 0.f, a0, a1, true);
    g.setColour (Pal::stroke.brighter (0.1f));
    g.strokePath (track, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // couleur selon la confiance : rouge -> ambre -> vert
    const juce::Colour col = gauge < 0.62f ? Pal::bad.interpolatedWith (Pal::warn, juce::jlimit (0.f, 1.f, gauge / 0.62f))
                                           : Pal::warn.interpolatedWith (Pal::good, juce::jlimit (0.f, 1.f, (gauge - 0.62f) / 0.2f));

    if (gauge > 0.01f)
    {
        juce::Path fill;
        fill.addCentredArc (c.x, c.y, R, R, 0.f, a0, a0 + (a1 - a0) * gauge, true);
        g.setColour (col.withAlpha (0.22f));
        g.strokePath (fill, juce::PathStrokeType (thick * 2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (col);
        g.strokePath (fill, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    const bool active = shownTonic >= 0;
    g.setColour (active ? Pal::text : Pal::textMute);
    g.setFont (mk (30.f * s, true));
    g.drawText (active ? juce::String (juce::roundToInt (gauge * 100.f)) : juce::String ("--"),
                juce::Rectangle<float> (r.getX(), c.y - 24.f * s, r.getWidth(), 36.f * s).toNearestInt(), juce::Justification::centred);
    g.setColour (Pal::textMute);
    g.setFont (mk (9.5f * s, true, 0.2f));
    g.drawText ("MATCH %", juce::Rectangle<float> (r.getX(), c.y + 12.f * s, r.getWidth(), 14.f * s).toNearestInt(), juce::Justification::centred);
}

void KeyDetectorEditor::drawChroma (juce::Graphics& g, juce::Rectangle<float> r)
{
    const float rad = 20.f * s;
    g.setColour (Pal::card);
    g.fillRoundedRectangle (r, rad);
    g.setColour (Pal::stroke);
    g.drawRoundedRectangle (r.reduced (0.5f), rad, 1.f);

    g.setColour (Pal::textMute);
    g.setFont (mk (9.5f * s, true, 0.2f));
    g.drawText ("PITCH CLASSES", juce::Rectangle<float> (r.getX() + 24.f * s, r.getY() + 11.f * s, 200.f * s, 14.f * s).toNearestInt(),
                juce::Justification::centredLeft);

    auto inner = r.reduced (24.f * s, 0.f);
    inner.removeFromTop (32.f * s);
    auto labels = inner.removeFromBottom (20.f * s);
    inner.removeFromBottom (4.f * s);

    const float slot = inner.getWidth() / 12.f;
    const float bw = slot * 0.46f;
    const bool active = shownTonic >= 0;

    for (int i = 0; i < 12; ++i)
    {
        const float cx = inner.getX() + slot * ((float) i + 0.5f);
        const bool isTonic = active && i == result.tonic;
        const bool isScale = active && inScale (i, result.tonic, result.minor);

        juce::Rectangle<float> slotR (cx - bw * 0.5f, inner.getY(), bw, inner.getHeight());
        g.setColour (Pal::stroke.withAlpha (0.65f));
        g.fillRoundedRectangle (slotR, bw * 0.5f);

        const float h = std::max (bw, shown[(size_t) i] * inner.getHeight());
        juce::Rectangle<float> bar (slotR.getX(), slotR.getBottom() - h, bw, h);

        if (isTonic)
        {
            g.setColour (accent.withAlpha (0.22f));
            g.fillRoundedRectangle (bar.expanded (4.f * s, 3.f * s), bw * 0.5f + 3.f * s);
            g.setGradientFill (juce::ColourGradient (accent.brighter (0.35f), bar.getX(), bar.getY(),
                                                     accent, bar.getX(), bar.getBottom(), false));
        }
        else
            g.setColour (isScale ? Pal::textDim.withAlpha (0.85f) : Pal::textMute.withAlpha (0.65f));
        g.fillRoundedRectangle (bar, bw * 0.5f);

        g.setColour (isTonic ? accent : isScale ? Pal::textDim : Pal::textMute);
        g.setFont (mk (10.5f * s, isTonic));
        g.drawText (isTonic ? spell (i, result.minor) : juce::String (KeyDetector::noteName (i)),
                    juce::Rectangle<float> (cx - slot * 0.5f, labels.getY(), slot, labels.getHeight()).toNearestInt(),
                    juce::Justification::centred);
    }
}
