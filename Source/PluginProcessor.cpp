/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PracticeEQAudioProcessor::PracticeEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

PracticeEQAudioProcessor::~PracticeEQAudioProcessor()
{
}

//==============================================================================
const juce::String PracticeEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PracticeEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PracticeEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PracticeEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PracticeEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PracticeEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PracticeEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PracticeEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PracticeEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void PracticeEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PracticeEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);

    auto chainSettings = getChainSettings(apvts);

    //function below needs a Quality parameter I think
    //auto hiCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.hiFreq, );
}

void PracticeEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PracticeEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PracticeEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool PracticeEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PracticeEQAudioProcessor::createEditor()
{
    //return new PracticeEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void PracticeEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PracticeEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.hiFreq = apvts.getRawParameterValue("HI Freq")->load();
    settings.hiMidFreq = apvts.getRawParameterValue("HI-MID Freq")->load();
    settings.loMidFreq = apvts.getRawParameterValue("LO-MID Freq")->load();
    settings.lowFreq = apvts.getRawParameterValue("LOW Freq")->load();
    settings.hpFreq = apvts.getRawParameterValue("HP Freq")->load();
    settings.lpFreq = apvts.getRawParameterValue("LP Freq")->load();

    settings.hiGain = apvts.getRawParameterValue("HI Gain")->load();
    settings.hiMidGain = apvts.getRawParameterValue("HI-MID Gain")->load();
    settings.loMidGain = apvts.getRawParameterValue("LO-MID Gain")->load();
    settings.lowGain = apvts.getRawParameterValue("LOW Gain")->load();

    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout PracticeEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "HI Freq",
        "HI Freq",
        juce::NormalisableRange<float>(800.f, 16000.f, 1.f, 1.f),
        800.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "HI-MID Freq",
        "HI-MID Freq",
        juce::NormalisableRange<float>(400.f, 8000.f, 1.f, 1.f),
        400.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LO-MID Freq",
        "LO-MID Freq",
        juce::NormalisableRange<float>(200.f, 4000.f, 1.f, 1.f),
        200.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LOW Freq", 
        "LOW Freq", 
        juce::NormalisableRange<float>(40.f, 800.f, 1.f, 1.f), 
        40.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "HP Freq",
        "HP Freq",
        juce::NormalisableRange<float>(25.f, 2750.f, 1.f, 1.f),
        25.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LP Freq",
        "LP Freq",
        juce::NormalisableRange<float>(160.f, 20000.f, 1.f, 1.f),
        160.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "HI Gain",
        "HI Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "HI-MID Gain",
        "HI-MID Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LO-MID Gain",
        "LO-MID Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LOW Gain",
        "LOW Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.f));

    return layout;
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PracticeEQAudioProcessor();
}
