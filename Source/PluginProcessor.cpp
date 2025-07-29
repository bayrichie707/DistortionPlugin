#include "PluginProcessor.h"
#include "PluginEditor.h"

// The definition of the function is placed here, outside of the constructor.
// It is correctly namespaced to the class.
juce::AudioProcessorValueTreeState::ParameterLayout NaniDistortionAudioProcessor::createParameterLayout()
{

    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "drive", 1 },
        "Drive",
        0.0f,
        2.0f,
        1.0f
    ));
    
    // OLD, PROBLEMATIC BLOCK:
    /*
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "bitdepth", 1 },
        "Bit Depth",
        1.0f,
        16.0f,
        16.0f
    ));
    */
    
    // NEW, CORRECTED LINE:
    // We use AudioParameterInt to enforce whole numbers and set the range from 1 to 16.
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{ "bitdepth", 1 }, // ID
        "Bit Depth",                       // Name
        1,                                 // Minimum Value
        16,                                // Maximum Value
        16                                 // Default Value
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "samplerate", 1 },
        "Sample Rate Reduction",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "mix", 1 },
        "Mix",
        0.0f,
        1.0f,
        1.0f
    ));
    
    // <<< ADD THE NEW DISTORTION TYPE PARAMETER
    juce::StringArray distortionTypeChoices;
    distortionTypeChoices.add("Soft Clip");
    distortionTypeChoices.add("Hard Clip");
    distortionTypeChoices.add("Foldback");
    distortionTypeChoices.add("Bit Glitch");

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "distortionType", 1 }, "Distortion Type", distortionTypeChoices, 0)); // Default to Soft Clip
    
    // <<< ADD THE NEW FILTER PARAMETERS

    // Cutoff: Logarithmic scale for a natural feel
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "filterCutoff", 1 }, "Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 20000.0f));

    // Resonance (Q)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "filterResonance", 1 }, "Filter Resonance", 1.0f, 10.0f, 1.0f));

    // Filter Type (LowPass, HighPass, BandPass)
    juce::StringArray filterTypeChoices;
    filterTypeChoices.add("Low-Pass");
    filterTypeChoices.add("High-Pass");
    filterTypeChoices.add("Band-Pass");
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "filterType", 1 }, "Filter Type", filterTypeChoices, 0));

    // Filter Routing (Pre/Post Distortion)
    juce::StringArray filterRoutingChoices;
    filterRoutingChoices.add("Pre-Distortion");
    filterRoutingChoices.add("Post-Distortion");
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "filterRouting", 1 }, "Filter Routing", filterRoutingChoices, 1)); // Default to Post

	// <<< ADD THE NEW OVERSAMPLING PARAMETERS
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ "oversamplingFactor", 1 },
        "Oversampling",
        juce::StringArray("Off", "2x", "4x", "8x", "16x"),
        1)); // Default to 2x

	// <<< ADD THE NEW LIMITER PARAMETERS
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "limiterThreshold", 1 },
        "Limiter Threshold",
        juce::NormalisableRange<float>(-30.0f, 0.0f, 0.1f),
        -0.5f)); // Default to -0.5 dB

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "limiterRelease", 1 },
        "Limiter Release",
        juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f),
        100.0f)); // Default to 100 ms

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "limiterEnabled", 1 },
        "Limiter Enabled",
        true)); // Default to enabled

	// <<< ADD THE NEW INPUT AND OUTPUT GAIN PARAMETERS
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "inputGain", 1 },
        "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f)); // Default to 0 dB (unity gain)

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "outputGain", 1 },
        "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f)); // Default to 0 dB (unity gain)

	// Bypass parameter
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ "bypass", 1 },
        "Bypass",
        false)); // Default to not bypassed

	// Stereo Width parameter
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "stereoWidth", 1 },
        "Stereo Width",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
        1.0f)); // Default to 1.0 (normal stereo)


    return { params.begin(), params.end() };
}

// The constructor now correctly calls the static function.
NaniDistortionAudioProcessor::NaniDistortionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      treeState(*this, nullptr, "PARAMETERS", NaniDistortionAudioProcessor::createParameterLayout())
#endif
{
}

NaniDistortionAudioProcessor::~NaniDistortionAudioProcessor() {}

const juce::String NaniDistortionAudioProcessor::getName() const { return JucePlugin_Name; }
bool NaniDistortionAudioProcessor::acceptsMidi() const { return false; }
bool NaniDistortionAudioProcessor::producesMidi() const { return false; }
double NaniDistortionAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int NaniDistortionAudioProcessor::getNumPrograms() { return 1; }
int NaniDistortionAudioProcessor::getCurrentProgram() { return 0; }
void NaniDistortionAudioProcessor::setCurrentProgram(int index) {}
const juce::String NaniDistortionAudioProcessor::getProgramName(int index) { return {}; }
void NaniDistortionAudioProcessor::changeProgramName(int index, const juce::String& newName) {}


juce::AudioProcessorValueTreeState& NaniDistortionAudioProcessor::getValueTreeState()
{
    return treeState;
}

void NaniDistortionAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Create all possible oversampling objects
    oversamplers.clear();

    // No oversampling (1x)
    oversamplers.push_back(nullptr);

    // 2x oversampling
    oversamplers.push_back(std::make_unique<juce::dsp::Oversampling<float>>(
        getTotalNumOutputChannels(),
        1, // 2^1 = 2x
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
        true
    ));

    // 4x oversampling
    oversamplers.push_back(std::make_unique<juce::dsp::Oversampling<float>>(
        getTotalNumOutputChannels(),
        2, // 2^2 = 4x
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
        true
    ));

    // 8x oversampling
    oversamplers.push_back(std::make_unique<juce::dsp::Oversampling<float>>(
        getTotalNumOutputChannels(),
        3, // 2^3 = 8x
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
        true
    ));

    // 16x oversampling
    oversamplers.push_back(std::make_unique<juce::dsp::Oversampling<float>>(
        getTotalNumOutputChannels(),
        4, // 2^4 = 16x
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
        true
    ));

    // Initialize all oversamplers
    for (auto i = 1; i < oversamplers.size(); ++i) {
        if (oversamplers[i]) {
            oversamplers[i]->initProcessing(samplesPerBlock);
            oversamplers[i]->reset();
        }
    }

    // Get parameter pointers - this is the correct way to get atomic parameters
    limiterThresholdParam = treeState.getRawParameterValue("limiterThreshold");
    limiterReleaseParam = treeState.getRawParameterValue("limiterRelease");
    limiterEnabledParam = treeState.getRawParameterValue("limiterEnabled");
    inputGainParam = treeState.getRawParameterValue("inputGain");
    outputGainParam = treeState.getRawParameterValue("outputGain");

    // Get bypass parameter
    bypassParam = treeState.getRawParameterValue("bypass");

    // Get stereo width parameter
    stereoWidthParam = treeState.getRawParameterValue("stereoWidth");

    // Prepare filter for the highest possible oversampling rate
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate * 16; // Maximum oversampling
    spec.maximumBlockSize = samplesPerBlock * 16;
    spec.numChannels = getTotalNumOutputChannels();

    filter.prepare(spec);
    filter.reset();

    // Prepare the limiter
    juce::dsp::ProcessSpec limiterSpec;
    limiterSpec.sampleRate = sampleRate;
    limiterSpec.maximumBlockSize = samplesPerBlock;
    limiterSpec.numChannels = getTotalNumOutputChannels();

    limiter.prepare(limiterSpec);
    limiter.reset();
}

void NaniDistortionAudioProcessor::releaseResources() 
{
    for (auto& oversampler : oversamplers) {
        if (oversampler) {
            oversampler->reset();
        }
    }
    oversamplers.clear();
    filter.reset();
}

/*
void NaniDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Get parameter values from the treeState atomically.
    const float drive = treeState.getRawParameterValue("drive")->load();
    const float bitDepth = treeState.getRawParameterValue("bitdepth")->load();
    const float sampleRateReduction = treeState.getRawParameterValue("samplerate")->load();
    const float mix = treeState.getRawParameterValue("mix")->load();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float drySample = channelData[sample];
            float wetSample = drySample;

            wetSample = bitCrush(wetSample, (int)bitDepth);

            float downsampleFactor = 1.0f + (sampleRateReduction * 15.0f);
            wetSample = downsample(wetSample, downsampleFactor);
            
            wetSample = waveshaper(wetSample, drive);
            
            channelData[sample] = (wetSample * mix) + (drySample * (1.0f - mix));
        }
    }
}
*/

/*
void NaniDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // 1. GET PARAMETERS
    // It's efficient to get these once per block.
    const float drive = treeState.getRawParameterValue("drive")->load();
    const float bitDepth = treeState.getRawParameterValue("bitdepth")->load();
    const float sampleRateReduction = treeState.getRawParameterValue("samplerate")->load();
    const float mix = treeState.getRawParameterValue("mix")->load();

    // 2. CREATE A COPY OF THE DRY SIGNAL FOR THE MIX KNOB
    // We need this because the main 'buffer' will be overwritten with the wet signal.
    juce::AudioBuffer<float> dryBuffer;
    if (mix < 1.0f) // Only copy if we actually need the dry signal
    {
        dryBuffer.makeCopyOf(buffer);
    }

    // 3. WRAP THE BUFFER IN A DSP AUDIOBLOCK
    // The oversampling object works with AudioBlocks.
    juce::dsp::AudioBlock<float> block(buffer);
    
    // 4. UPSAMPLE THE AUDIO
    // This returns a NEW AudioBlock that is N times larger and at N times the sample rate.
    juce::dsp::AudioBlock<float> oversampledBlock = oversampling.processSamplesUp(block);

    // 5. PROCESS THE OVERSAMPLED BLOCK
    // All our distortion logic now runs on the high-resolution audio.
    for (int channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        auto* channelData = oversampledBlock.getChannelPointer(channel);

        // IMPORTANT: Loop over the number of samples in the oversampled block.
        for (int sample = 0; sample < oversampledBlock.getNumSamples(); ++sample)
        {
            float wetSample = channelData[sample]; // Start with the current sample

            // Apply effects in order
            wetSample = bitCrush(wetSample, (int)bitDepth);

            float downsampleFactor = 1.0f + (sampleRateReduction * 15.0f);
            wetSample = downsample(wetSample, downsampleFactor);
            
            wetSample = waveshaper(wetSample, drive);
            
            // Write the processed sample back to the oversampled block
            channelData[sample] = wetSample;
        }
    }

    // 6. DOWNSAMPLE THE AUDIO
    // This applies an anti-aliasing filter and writes the result back to the original 'block' (and thus, 'buffer').
    oversampling.processSamplesDown(block);
    
    // 7. APPLY THE DRY/WET MIX
    // Now that the 'buffer' contains the 100% wet signal, we can mix it with our saved dry signal.
    if (mix < 1.0f)
    {
        buffer.addFrom(0, 0, dryBuffer, 0, 0, buffer.getNumSamples(), (1.0f - mix));
        buffer.applyGain(0, 0, buffer.getNumSamples(), mix);

        if (buffer.getNumChannels() > 1)
        {
            buffer.addFrom(1, 0, dryBuffer, 1, 0, buffer.getNumSamples(), (1.0f - mix));
            buffer.applyGain(1, 0, buffer.getNumSamples(), mix);
        }
    }
}
*/
// Source/Plugin-processor.cpp

void NaniDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Check if bypassed
    bool shouldBypass = bypassParam->load() > 0.5f;

    // Calculate input levels (before any processing)
    for (int channel = 0; channel < buffer.getNumChannels() && channel < 2; ++channel)
    {
        // Apply level decay
        inputLevels[channel] *= levelDecayRate;

        // Find the peak level in this buffer
        float currentPeak = 0.0f;

        // Check each sample and find the peak
        auto* channelData = buffer.getReadPointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float sample = std::abs(channelData[i]);
            if (sample > currentPeak)
                currentPeak = sample;
        }

        // Update the level if the new peak is higher
        if (currentPeak > inputLevels[channel])
            inputLevels[channel] = currentPeak;
    }

    // If bypassed, skip all processing and just update output levels
    if (shouldBypass)
    {
        // When bypassed, output levels are the same as input levels
        for (int channel = 0; channel < buffer.getNumChannels() && channel < 2; ++channel)
        {
            outputLevels[channel] = inputLevels[channel];
        }

        return;
    }

    // Get parameters
    const int oversamplingIndex = static_cast<int>(treeState.getRawParameterValue("oversamplingFactor")->load());
    const float drive = treeState.getRawParameterValue("drive")->load();
    const float bitDepth = treeState.getRawParameterValue("bitdepth")->load();
    const float sampleRateReduction = treeState.getRawParameterValue("samplerate")->load();
    const float mix = treeState.getRawParameterValue("mix")->load();
    const float cutoff = treeState.getRawParameterValue("filterCutoff")->load();
    const float resonance = treeState.getRawParameterValue("filterResonance")->load();
    const auto filterType = static_cast<FilterType>(static_cast<int>(treeState.getRawParameterValue("filterType")->load()));
    const auto filterRouting = static_cast<FilterRouting>(static_cast<int>(treeState.getRawParameterValue("filterRouting")->load()));
    const auto distortionType = static_cast<DistortionType>(static_cast<int>(treeState.getRawParameterValue("distortionType")->load()));

    // Get gain parameters
    const float inputGain = juce::Decibels::decibelsToGain(inputGainParam->load());
    const float outputGain = juce::Decibels::decibelsToGain(outputGainParam->load());

    // Get stereo width parameter
    const float stereoWidth = stereoWidthParam->load();

    // Create dry buffer for mix
    juce::AudioBuffer<float> dryBuffer;
    if (mix < 1.0f) dryBuffer.makeCopyOf(buffer);

    // Apply input gain
    buffer.applyGain(inputGain);

    // Apply stereo width before processing (if stereo)
    if (buffer.getNumChannels() > 1 && stereoWidth != 1.0f)
    {
        applyStereoWidth(buffer, stereoWidth);
    }

    // Process with or without oversampling
    if (oversamplingIndex == 0 || oversamplers.size() <= oversamplingIndex || !oversamplers[oversamplingIndex]) {
        // No oversampling - process directly
        processAudio(buffer, drive, bitDepth, sampleRateReduction,
            filterType, filterRouting, cutoff, resonance, distortionType);
    }
    else {
        // With oversampling
        juce::dsp::AudioBlock<float> block(buffer);
        auto& oversampler = *oversamplers[oversamplingIndex];

        // Upsample
        auto oversampledBlock = oversampler.processSamplesUp(block);

        // Process the oversampled audio
        processOversampledBlock(oversampledBlock, drive, bitDepth, sampleRateReduction,
            filterType, filterRouting, cutoff, resonance, distortionType);

        // Downsample
        oversampler.processSamplesDown(block);
    }

    // Apply mix
    applyMix(buffer, dryBuffer, mix);

    // Apply output gain
    buffer.applyGain(outputGain);

    // Apply limiter (if enabled)
    if (limiterEnabledParam->load() > 0.5f)
    {
        limiter.setThreshold(limiterThresholdParam->load());
        limiter.setRelease(limiterReleaseParam->load() / 1000.0f);

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        limiter.process(context);
    }

    // Calculate output levels (after all processing)
    for (int channel = 0; channel < buffer.getNumChannels() && channel < 2; ++channel)
    {
        // Apply level decay
        outputLevels[channel] *= levelDecayRate;

        // Find the peak level in this buffer
        float currentPeak = 0.0f;

        // Check each sample for clipping and find the peak
        auto* channelData = buffer.getReadPointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float sample = std::abs(channelData[i]);
            if (sample > currentPeak)
                currentPeak = sample;
        }

        // Update the level if the new peak is higher
        if (currentPeak > outputLevels[channel])
            outputLevels[channel] = currentPeak;
    }
}

// Helper method to process audio without oversampling
void NaniDistortionAudioProcessor::processAudio(juce::AudioBuffer<float>& buffer,
    float drive, float bitDepth, float sampleRateReduction,
    FilterType filterType, FilterRouting filterRouting,
    float cutoff, float resonance, DistortionType distortionType)
{
    // Update filter settings
    switch (filterType) {
    case LowPass:  filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);  break;
    case HighPass: filter.setType(juce::dsp::StateVariableTPTFilterType::highpass); break;
    case BandPass: filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass); break;
    }
    filter.setCutoffFrequency(cutoff);
    filter.setResonance(resonance);

    // Create context for filter
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Apply pre-distortion filter if needed
    if (filterRouting == FilterRouting::Pre) {
        filter.process(context);
    }

    // Apply distortion
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            float wetSample = channelData[sample];
            wetSample = bitCrush(wetSample, (int)bitDepth);
            float downsampleFactor = 1.0f + (sampleRateReduction * 15.0f);
            wetSample = downsample(wetSample, downsampleFactor);
            wetSample = waveshaper(wetSample, drive, distortionType);
            channelData[sample] = wetSample;
        }
    }

    // Apply post-distortion filter if needed
    if (filterRouting == FilterRouting::Post) {
        filter.process(context);
    }
}

// Helper method to process oversampled audio block
void NaniDistortionAudioProcessor::processOversampledBlock(juce::dsp::AudioBlock<float>& oversampledBlock,
    float drive, float bitDepth, float sampleRateReduction,
    FilterType filterType, FilterRouting filterRouting,
    float cutoff, float resonance, DistortionType distortionType)
{
    // Update filter settings
    switch (filterType) {
    case LowPass:  filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);  break;
    case HighPass: filter.setType(juce::dsp::StateVariableTPTFilterType::highpass); break;
    case BandPass: filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass); break;
    }
    filter.setCutoffFrequency(cutoff);
    filter.setResonance(resonance);

    // Create context for filter
    juce::dsp::ProcessContextReplacing<float> context(oversampledBlock);

    // Apply pre-distortion filter if needed
    if (filterRouting == FilterRouting::Pre) {
        filter.process(context);
    }

    // Apply distortion
    for (int channel = 0; channel < oversampledBlock.getNumChannels(); ++channel) {
        auto* channelData = oversampledBlock.getChannelPointer(channel);
        for (int sample = 0; sample < oversampledBlock.getNumSamples(); ++sample) {
            float wetSample = channelData[sample];
            wetSample = bitCrush(wetSample, (int)bitDepth);
            float downsampleFactor = 1.0f + (sampleRateReduction * 15.0f);
            wetSample = downsample(wetSample, downsampleFactor);
            wetSample = waveshaper(wetSample, drive, distortionType);
            channelData[sample] = wetSample;
        }
    }

    // Apply post-distortion filter if needed
    if (filterRouting == FilterRouting::Post) {
        filter.process(context);
    }
}

// Helper method to apply mix
void NaniDistortionAudioProcessor::applyMix(juce::AudioBuffer<float>& buffer,
    const juce::AudioBuffer<float>& dryBuffer,
    float mix)
{
    if (mix == 0.0f) {
        // If mix is 0%, just copy the dry signal back
        buffer.copyFrom(0, 0, dryBuffer, 0, 0, buffer.getNumSamples());
        if (buffer.getNumChannels() > 1)
            buffer.copyFrom(1, 0, dryBuffer, 1, 0, buffer.getNumSamples());
    }
    else if (mix < 1.0f) {
        // For partial mix, apply gain to the wet signal
        buffer.applyGain(mix);
        // And add the dry signal with its gain
        buffer.addFrom(0, 0, dryBuffer, 0, 0, buffer.getNumSamples(), (1.0f - mix));
        if (buffer.getNumChannels() > 1)
            buffer.addFrom(1, 0, dryBuffer, 1, 0, buffer.getNumSamples(), (1.0f - mix));
    }
    // If mix is 1.0f, we do nothing
}

float NaniDistortionAudioProcessor::bitCrush(float sample, int bits)
{
    if (bits >= 16) return sample;
    float steps = powf(2.0f, (float)bits);
    return roundf(sample * steps) / steps;
}

float NaniDistortionAudioProcessor::downsample(float sample, float factor)
{
    if (factor <= 1.0f)
    {
        downsampleCounter = 0.0f; // Reset state when effect is off
        downsamplePhase = sample;
        return sample;
    }

    downsampleCounter += 1.0f;
    if (downsampleCounter >= factor)
    {
        downsampleCounter -= factor;
        downsamplePhase = sample;
    }
    return downsamplePhase;
}

// Source/PluginProcessor.cpp

float NaniDistortionAudioProcessor::waveshaper(float sample, float drive, DistortionType type)
{
    // The gain is used differently by each algorithm
    float gain = 1.0f + drive * 9.0f;

    switch (type)
    {
        case SoftClip:
            // Our original smooth tanh curve
            return std::tanh(sample * gain);

        case HardClip:
            // An aggressive, squared-off digital distortion
            return std::clamp(sample * gain, -1.0f, 1.0f);

        case Foldback:
            // Folds the waveform back on itself, creating inharmonic tones
            // The sine function is a great way to do this
            return std::sin(sample * gain);

        case BitGlitch:
            // This is the "Nani" special. It treats the float's memory as an
            // integer and flips some of its bits, creating digital artifacts.
            {
                // Reinterpret the memory address of the float as an integer pointer
                int32_t* p = reinterpret_cast<int32_t*>(&sample);
                
                // Use bitwise XOR with a mask. The drive knob can control the mask.
                // This creates very different glitches at different drive levels.
                int32_t mask = static_cast<int32_t>(drive * 1000.0f) << 12;
                *p ^= mask;
                
                // Make sure we don't return an infinitely large number
                return std::clamp(sample, -1.0f, 1.0f);
            }
        
        default:
            // Fallback to the default in case of an error
            return std::tanh(sample * gain);
    }
}

// Helper methods for preset management
juce::File NaniDistortionAudioProcessor::getPresetsDirectory()
{
    // Get the user's application data directory
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);

    // Create a directory for your plugin's presets
    juce::File presetsDir = appDataDir.getChildFile("NaniDistortion/Presets");

    // Make sure the directory exists
    if (!presetsDir.exists())
        presetsDir.createDirectory();

    return presetsDir;
}

void NaniDistortionAudioProcessor::savePreset(const juce::String& name)
{
    // Get the presets directory
    juce::File presetsDir = getPresetsDirectory();

    // Create a file for the preset
    juce::File presetFile = presetsDir.getChildFile(name + ".preset");

    // Create an XML element to store the preset data
    auto presetXml = treeState.state.createXml();

    // Save the XML to the file
    if (presetXml->writeTo(presetFile))
    {
        currentPresetName = name;
    }
}

void NaniDistortionAudioProcessor::loadPreset(const juce::String& name)
{
    // Get the presets directory
    juce::File presetsDir = getPresetsDirectory();

    // Get the preset file
    juce::File presetFile = presetsDir.getChildFile(name + ".preset");

    // Check if the file exists
    if (presetFile.exists())
    {
        // Load the XML from the file
        std::unique_ptr<juce::XmlElement> presetXml = juce::XmlDocument::parse(presetFile);

        // Check if the XML is valid
        if (presetXml.get() != nullptr)
        {
            // Load the preset data into the value tree state
            treeState.replaceState(juce::ValueTree::fromXml(*presetXml));
            currentPresetName = name;
        }
    }
}

void NaniDistortionAudioProcessor::deletePreset(const juce::String& name)
{
    // Get the presets directory
    juce::File presetsDir = getPresetsDirectory();

    // Get the preset file
    juce::File presetFile = presetsDir.getChildFile(name + ".preset");

    // Delete the file if it exists
    if (presetFile.exists())
    {
        presetFile.deleteFile();

        // If the deleted preset was the current one, clear the current preset name
        if (currentPresetName == name)
            currentPresetName = "";
    }
}

juce::StringArray NaniDistortionAudioProcessor::getPresetList()
{
    // Get the presets directory
    juce::File presetsDir = getPresetsDirectory();

    // Create a string array to store the preset names
    juce::StringArray presetList;

    // Get all files in the directory with the .preset extension
    juce::Array<juce::File> presetFiles = presetsDir.findChildFiles(juce::File::findFiles, false, "*.preset");

    // Add each preset name to the list (without the extension)
    for (auto& file : presetFiles)
    {
        presetList.add(file.getFileNameWithoutExtension());
    }

    return presetList;
}

bool NaniDistortionAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* NaniDistortionAudioProcessor::createEditor()
{
    return new NaniDistortionAudioProcessorEditor(*this);
}

void NaniDistortionAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NaniDistortionAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(treeState.state.getType()))
            treeState.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Add these implementations to your PluginProcessor.cpp file:
float NaniDistortionAudioProcessor::getInputLevel(int channel) const
{
    if (channel >= 0 && channel < 2)
        return inputLevels[channel];
    return 0.0f;
}

float NaniDistortionAudioProcessor::getOutputLevel(int channel) const
{
    if (channel >= 0 && channel < 2)
        return outputLevels[channel];
    return 0.0f;
}

// Apply stereo width to the audio buffer
void NaniDistortionAudioProcessor::applyStereoWidth(juce::AudioBuffer<float>& buffer, float width)
{
    // Only process if we have a stereo buffer
    if (buffer.getNumChannels() < 2)
        return;

    // Get pointers to the left and right channels
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    // Process each sample
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        // Convert to mid/side
        float mid = (leftChannel[i] + rightChannel[i]) * 0.5f;
        float side = (rightChannel[i] - leftChannel[i]) * 0.5f;

        // Apply width to the side signal
        side *= width;

        // Convert back to left/right
        leftChannel[i] = mid - side;
        rightChannel[i] = mid + side;
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NaniDistortionAudioProcessor();
}