#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// A handy alias for the long attachment class names to keep code clean
using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

class NaniDistortionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit NaniDistortionAudioProcessorEditor(NaniDistortionAudioProcessor&);
    ~NaniDistortionAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    NaniDistortionAudioProcessor& processor;

    // Distortion Components
    juce::Slider driveSlider;
    juce::Slider bitDepthSlider;
    juce::Slider sampleRateSlider;
    juce::Slider mixSlider;
    
    juce::Label driveLabel;
    juce::Label bitDepthLabel;
    juce::Label sampleRateLabel;
    juce::Label mixLabel;

    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> bitDepthAttachment;
    std::unique_ptr<SliderAttachment> sampleRateAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    
    // <<< ADD THESE Distortion Dropdown Menu
    juce::ComboBox distortionTypeComboBox;
    juce::Label distortionTypeLabel;
    std::unique_ptr<ComboBoxAttachment> distortionTypeAttachment;

    // Filter Components
    juce::Slider filterCutoffSlider;
    juce::Slider filterResonanceSlider;
    juce::ComboBox filterTypeComboBox;
    juce::ComboBox filterRoutingComboBox;

    juce::Label filterCutoffLabel;
    juce::Label filterResonanceLabel;
    juce::Label filterTypeLabel;
    juce::Label filterRoutingLabel;

    std::unique_ptr<SliderAttachment> filterCutoffAttachment;
    std::unique_ptr<SliderAttachment> filterResonanceAttachment;
    std::unique_ptr<ComboBoxAttachment> filterTypeAttachment;
    std::unique_ptr<ComboBoxAttachment> filterRoutingAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NaniDistortionAudioProcessorEditor)
};