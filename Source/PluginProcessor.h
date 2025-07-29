#pragma once

#include <JuceHeader.h>


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
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    
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
    
    // ADD THIS
    //juce::dsp::Oversampling<float> oversampling;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NaniDistortionAudioProcessor)
};