#include "PluginProcessor.h"
#include "PluginEditor.h"

KeyDetectorProcessor::KeyDetectorProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout KeyDetectorProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "memory", 1 }, "Memory",
        juce::NormalisableRange<float> (2.0f, 120.0f, 1.0f, 0.5f), 30.0f));
    return layout;
}

bool KeyDetectorProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    if (in != out) return false;
    return in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
}

void KeyDetectorProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    detector.prepare (sampleRate, *apvts.getRawParameterValue ("memory"));
    mono.assign ((size_t) std::max (samplesPerBlock, 1), 0.f);
}

void KeyDetectorProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = getTotalNumInputChannels();
    const int n = buffer.getNumSamples();
    if (numCh == 0 || n == 0) return;

    detector.setMemorySeconds (*apvts.getRawParameterValue ("memory"));

    if ((int) mono.size() < n)           // sécurité si l'hôte dépasse la taille annoncée
        mono.resize ((size_t) n);

    const float g = 1.0f / (float) numCh;
    std::fill (mono.begin(), mono.begin() + n, 0.f);
    for (int ch = 0; ch < numCh; ++ch)
    {
        const float* in = buffer.getReadPointer (ch);
        for (int i = 0; i < n; ++i) mono[(size_t) i] += in[i] * g;
    }

    detector.process (mono.data(), n);
    // L'audio n'est pas modifié : le plugin est transparent.
}

juce::AudioProcessorEditor* KeyDetectorProcessor::createEditor()
{
    return new KeyDetectorEditor (*this);
}

void KeyDetectorProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void KeyDetectorProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KeyDetectorProcessor();
}
