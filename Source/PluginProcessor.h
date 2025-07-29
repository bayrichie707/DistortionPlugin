#pragma once

#include <JuceHeader.h>
// Add these includes at the top of your file if they're not already there
#include <juce_dsp/juce_dsp.h>

// <<< ADD THESE ENUMS for clarity and type safety
enum FilterType { LowPass, HighPass, BandPass };
enum FilterRouting { Pre, Post };

// <<< ADD THIS ENUM FOR OUR NEW DISTORTION TYPES
enum DistortionType { SoftClip, HardClip, Foldback, BitGlitch };

class NaniDistortionAudioProcessor : public juce::AudioProcessor
{
public:
    NaniDistortionAudioProcessor();
    ~NaniDistortionAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Preset management
    void savePreset(const juce::String& name);
    void loadPreset(const juce::String& name);
    void deletePreset(const juce::String& name);
    juce::StringArray getPresetList();
    juce::String getCurrentPresetName() const { return currentPresetName; }
    void setCurrentPresetName(const juce::String& name) { currentPresetName = name; }

    // Level meter methods
    float getInputLevel(int channel) const;
    float getOutputLevel(int channel) const;
    
    // Public access to the state for the editor
    juce::AudioProcessorValueTreeState& getValueTreeState();
    
    // <<< THIS IS THE CRITICAL DECLARATION THAT MUST BE PRESENT
    // This tells the compiler that our class has a static function with this name.
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    // The AudioProcessorValueTreeState must be declared before any parameter pointers
    juce::AudioProcessorValueTreeState treeState;
    
    // <<< CHANGE THIS
    // We must use a pointer because the constructor needs parameters
    // that we only get in prepareToPlay.
    // Replace the single oversampling member with a vector of oversamplers
    std::vector<std::unique_ptr<juce::dsp::Oversampling<float>>> oversamplers;
    
    // <<< ADD THE FILTER OBJECT
    juce::dsp::StateVariableTPTFilter<float> filter;

    // DSP processing chain
    float downsampleCounter = 0.0f;
    float downsamplePhase = 0.0f;

    // Internal processing functions
    float bitCrush(float sample, int bits);
    float downsample(float sample, float factor);
    // <<< UPDATE THE WAVESHAPER FUNCTION SIGNATURE
    float waveshaper(float sample, float drive, DistortionType type);

	// Helper methods for preset management
    juce::File getPresetsDirectory();
    juce::String currentPresetName;
    
    // ADD THIS
    //juce::dsp::Oversampling<float> oversampling;

    // In PluginProcessor.h:
    // Add the new helper methods
    void processAudio(juce::AudioBuffer<float>& buffer,
        float drive, float bitDepth, float sampleRateReduction,
        FilterType filterType, FilterRouting filterRouting,
        float cutoff, float resonance, DistortionType distortionType);

    void processOversampledBlock(juce::dsp::AudioBlock<float>& oversampledBlock,
        float drive, float bitDepth, float sampleRateReduction,
        FilterType filterType, FilterRouting filterRouting,
        float cutoff, float resonance, DistortionType distortionType);

    void applyMix(juce::AudioBuffer<float>& buffer,
        const juce::AudioBuffer<float>& dryBuffer,
        float mix);

    // Limiter components
    juce::dsp::Limiter<float> limiter;
    bool limiterEnabled = true;  // Default to enabled

    // Limiter parameters
    std::atomic<float>* limiterThresholdParam = nullptr;
    std::atomic<float>* limiterReleaseParam = nullptr;
    std::atomic<float>* limiterEnabledParam = nullptr;

    // Gain parameters
    std::atomic<float>* inputGainParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;

    // Level meter variables
    std::array<float, 2> inputLevels = { 0.0f, 0.0f };  // For stereo (left/right)
    std::array<float, 2> outputLevels = { 0.0f, 0.0f }; // For stereo (left/right)
    float levelDecayRate = 0.8f;                   // How quickly the meters fall

    // Bypass parameter
    std::atomic<float>* bypassParam = nullptr;
    bool isBypassed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NaniDistortionAudioProcessor)
};