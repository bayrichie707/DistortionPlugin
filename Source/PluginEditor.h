#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LevelMeter.h"

// A handy alias for the long attachment class names to keep code clean
using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

class NaniDistortionAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit NaniDistortionAudioProcessorEditor(NaniDistortionAudioProcessor&);
    ~NaniDistortionAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Add this declaration for the timer callback
    void timerCallback() override;


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

    // Distortion Dropdown Menu
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

    // Oversampling Components
    juce::ComboBox oversamplingComboBox;
    juce::Label oversamplingLabel;
    std::unique_ptr<ComboBoxAttachment> oversamplingAttachment;

    // Preset management components
    juce::ComboBox presetComboBox;
    juce::TextButton savePresetButton;
    juce::TextButton deletePresetButton;
    juce::TextEditor presetNameEditor;

    // Preset management methods
    void updatePresetComboBox();
    void showSavePresetDialog();
    void showDeletePresetConfirmation();

    // Limiter components
    juce::Slider limiterThresholdSlider;
    juce::Slider limiterReleaseSlider;
    juce::ToggleButton limiterEnabledButton;

    juce::Label limiterThresholdLabel;
    juce::Label limiterReleaseLabel;

    std::unique_ptr<SliderAttachment> limiterThresholdAttachment;
    std::unique_ptr<SliderAttachment> limiterReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limiterEnabledAttachment;

    // Gain controls
    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;

    juce::Label inputGainLabel;
    juce::Label outputGainLabel;

    std::unique_ptr<SliderAttachment> inputGainAttachment;
    std::unique_ptr<SliderAttachment> outputGainAttachment;

    // Level meters
    LevelMeter inputLevelMeterL;
    LevelMeter inputLevelMeterR;
    LevelMeter outputLevelMeterL;
    LevelMeter outputLevelMeterR;

    juce::Label inputMeterLabel;
    juce::Label outputMeterLabel;

	// 
    juce::TextButton resetClipButton;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NaniDistortionAudioProcessorEditor)
};