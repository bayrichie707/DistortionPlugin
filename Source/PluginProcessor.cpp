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
    // The oversampling factor. We'll hardcode 4x.
    int factor = 4;

    // Create a new instance of the Oversampling object on the heap.
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        getTotalNumOutputChannels(),
        factor,
        juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple,
        true // Use high-quality (polyphase) FIR filters
    );

    // This is the correct initialization method for this class.
    oversampling->initProcessing(samplesPerBlock);
    
    // <<< PREPARE THE FILTER
    // We need a separate ProcessSpec for the oversampled signal
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate * factor; // Use the oversampled rate!
    spec.maximumBlockSize = samplesPerBlock * factor; // Use the oversampled block size!
    spec.numChannels = getTotalNumOutputChannels();
    
    filter.prepare(spec);
    
    
    // It's good practice to reset it to clear any old state.
    filter.reset();
    oversampling->reset();
}

void NaniDistortionAudioProcessor::releaseResources() 
{
    oversampling.reset();
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

void NaniDistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    if (!oversampling) return;

    // 1. GET ALL PARAMETERS
    const float drive = treeState.getRawParameterValue("drive")->load();
    const float bitDepth = treeState.getRawParameterValue("bitdepth")->load();
    const float sampleRateReduction = treeState.getRawParameterValue("samplerate")->load();
    const float mix = treeState.getRawParameterValue("mix")->load();

    // Get filter parameters, casting choices to our enums for safety
    const float cutoff = treeState.getRawParameterValue("filterCutoff")->load();
    const float resonance = treeState.getRawParameterValue("filterResonance")->load();
    const auto filterType = static_cast<FilterType>(treeState.getRawParameterValue("filterType")->load());
    const auto filterRouting = static_cast<FilterRouting>(treeState.getRawParameterValue("filterRouting")->load());
    // <<< GET THE NEW DISTORTION TYPE PARAMETER
    const auto distortionType = static_cast<DistortionType>(treeState.getRawParameterValue("distortionType")->load());
    
    // 2. PREPARE DRY BUFFER AND DSP BLOCK (same as before)
    juce::AudioBuffer<float> dryBuffer;
    if (mix < 1.0f) dryBuffer.makeCopyOf(buffer);
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> oversampledBlock = oversampling->processSamplesUp(block);

    // 3. UPDATE FILTER SETTINGS
    // This must be done BEFORE the processing loop.
    switch (filterType)
    {
        case LowPass:  filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);  break;
        case HighPass: filter.setType(juce::dsp::StateVariableTPTFilterType::highpass); break;
        case BandPass: filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass); break;
    }
    filter.setCutoffFrequency(cutoff);
    filter.setResonance(resonance);

    // Context for processing the oversampled block
    juce::dsp::ProcessContextReplacing<float> oversampledContext(oversampledBlock);

    // 4. APPLY PRE-DISTORTION FILTER (if selected)
    if (filterRouting == FilterRouting::Pre)
    {
        filter.process(oversampledContext);
    }
    
    // 5. APPLY DISTORTION (your existing logic)
    for (int channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        auto* channelData = oversampledBlock.getChannelPointer(channel);
        for (int sample = 0; sample < oversampledBlock.getNumSamples(); ++sample)
        {
            float wetSample = channelData[sample];
            wetSample = bitCrush(wetSample, (int)bitDepth);
            float downsampleFactor = 1.0f + (sampleRateReduction * 15.0f);
            wetSample = downsample(wetSample, downsampleFactor);
            wetSample = waveshaper(wetSample, drive, distortionType);
            channelData[sample] = wetSample;
        }
    }
    
    // 6. APPLY POST-DISTORTION FILTER (if selected)
    if (filterRouting == FilterRouting::Post)
    {
        filter.process(oversampledContext);
    }

    // 7. DOWNSAMPLE AND MIX (same as before)
    oversampling->processSamplesDown(block);
    
    // At this point, 'buffer' contains the 100% wet signal.
    // 'dryBuffer' contains the 100% dry signal.
    // 8. APPLY THE CORRECT DRY/WET MIX
    if (mix == 0.0f)
    {
        // If mix is 0%, just copy the dry signal back.
        buffer.copyFrom(0, 0, dryBuffer, 0, 0, buffer.getNumSamples());
        if (buffer.getNumChannels() > 1)
            buffer.copyFrom(1, 0, dryBuffer, 1, 0, buffer.getNumSamples());
    }
    else if (mix < 1.0f)
    {
        // For partial mix, apply gain to the wet signal...
        buffer.applyGain(mix);
        // ...and add the dry signal with its gain.
        buffer.addFrom(0, 0, dryBuffer, 0, 0, buffer.getNumSamples(), (1.0f - mix));
        if (buffer.getNumChannels() > 1)
            buffer.addFrom(1, 0, dryBuffer, 1, 0, buffer.getNumSamples(), (1.0f - mix));
    }
    // If mix is 1.0f, we do nothing, because the buffer already contains the 100% wet signal.      
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

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NaniDistortionAudioProcessor();
}