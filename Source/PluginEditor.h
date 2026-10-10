#pragma once
#include "PluginProcessor.h"

// Palette
namespace Pal
{
    const juce::Colour bg0      { 0xff08090d };
    const juce::Colour bg1      { 0xff10131a };
    const juce::Colour card     { 0xff12151d };
    const juce::Colour stroke   { 0xff1f2430 };
    const juce::Colour text     { 0xfff3f5fa };
    const juce::Colour textDim  { 0xff8b93a7 };
    const juce::Colour textMute { 0xff535b6e };
    const juce::Colour major    { 0xffffb454 };   // ambre
    const juce::Colour minor    { 0xff8b8bff };   // indigo
    const juce::Colour good     { 0xff4ade80 };
    const juce::Colour warn     { 0xfffbbf24 };
    const juce::Colour bad      { 0xfff87171 };
}

// Look & feel : slider fin et bouton « fantôme »
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Colour accent { Pal::minor };

    void drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                           float, float, juce::Slider::SliderStyle, juce::Slider& s) override
    {
        const float cy = (float) y + (float) h * 0.5f;
        const float tr = 4.0f;
        juce::Rectangle<float> track ((float) x + 7.f, cy - tr * 0.5f, (float) w - 14.f, tr);
        g.setColour (Pal::stroke.brighter (0.15f));
        g.fillRoundedRectangle (track, tr * 0.5f);

        auto filled = track.withRight (pos);
        g.setColour (accent);
        g.fillRoundedRectangle (filled, tr * 0.5f);

        const float r = s.isMouseOverOrDragging() ? 8.0f : 7.0f;
        g.setColour (accent.withAlpha (0.25f));
        g.fillEllipse (pos - r - 3.f, cy - r - 3.f, (r + 3.f) * 2.f, (r + 3.f) * 2.f);
        g.setColour (Pal::text);
        g.fillEllipse (pos - r, cy - r, r * 2.f, r * 2.f);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                               bool hover, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (down ? accent.withAlpha (0.30f) : hover ? accent.withAlpha (0.16f) : juce::Colours::transparentBlack);
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (hover ? accent : Pal::stroke.brighter (0.25f));
        g.drawRoundedRectangle (r, r.getHeight() * 0.5f, 1.0f);
    }

    void drawCornerResizer (juce::Graphics& g, int w, int h, bool over, bool down) override
    {
        const auto col = (down || over) ? Pal::textDim : Pal::textMute;
        g.setColour (col.withAlpha (0.8f));
        const float fw = (float) w, fh = (float) h;
        for (int i = 1; i <= 3; ++i)
        {
            const float o = (float) i * 4.5f;
            g.drawLine (fw - o, fh - 2.f, fw - 2.f, fh - o, 1.3f);
        }
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& b, bool hover, bool) override
    {
        g.setColour (hover ? Pal::text : Pal::textDim);
        g.setFont (juce::FontOptions (11.0f * (float) b.getHeight() / 26.0f, juce::Font::bold).withKerningFactor (0.12f));
        g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
    }
};

class KeyDetectorEditor : public juce::AudioProcessorEditor,
                          private juce::Timer
{
public:
    explicit KeyDetectorEditor (KeyDetectorProcessor&);
    ~KeyDetectorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    // dessin
    void drawHeader (juce::Graphics&, juce::Rectangle<float>);
    void drawHero   (juce::Graphics&, juce::Rectangle<float>);
    void drawGauge  (juce::Graphics&, juce::Rectangle<float>);
    void drawChroma (juce::Graphics&, juce::Rectangle<float>);
    float drawChip  (juce::Graphics&, const juce::String&, float x, float y, float h, juce::Colour c, bool filled);

    static juce::String keyName (int tonic, bool minor);
    static juce::String keyNameFr (int tonic, bool minor);
    static juce::String camelot (int tonic, bool minor);

    ModernLookAndFeel lnf;
    KeyDetectorProcessor& proc;
    juce::TextButton resetButton { "RESET" };
    juce::Slider memorySlider;
    juce::AudioProcessorValueTreeState::SliderAttachment memoryAttachment;

    // état affiché (avec hystérésis pour éviter les clignotements)
    KeyDetector::Result result;
    int   shownTonic = -1;
    bool  shownMinor = false;
    int   pendingCount = 0;
    float gauge = 0.f;
    float levelFade = 0.f;     // 0 = en attente, 1 = actif
    juce::Colour accent { Pal::minor };
    std::array<float, 12> shown {};
    std::array<float, 12> chroma {};

    float s = 1.f;             // facteur d'échelle de l'interface
    juce::Rectangle<float> rHeader, rHero, rGauge, rChroma, rFooter, rSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyDetectorEditor)
};
