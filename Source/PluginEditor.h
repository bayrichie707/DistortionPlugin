#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LevelMeter.h"
#include "CustomSlider.h"

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

	// Method to update all slider displays
    void updateAllSliderDisplays();

    juce::Image getKnobSprite() { return knobSpriteStrip; }


private:
    NaniDistortionAudioProcessor& processor;

    // Distortion Components
    CustomSlider driveSlider;
    CustomSlider bitDepthSlider;
    CustomSlider sampleRateSlider;
    CustomSlider mixSlider;

    // Filter Components
    CustomSlider filterCutoffSlider;
    CustomSlider filterResonanceSlider;

    // Gain controls
    CustomSlider inputGainSlider;
    CustomSlider outputGainSlider;

    // Limiter components
    CustomSlider limiterThresholdSlider;
    CustomSlider limiterReleaseSlider;

    // Stereo width control
    CustomSlider stereoWidthSlider;

    //// Distortion Components
    //juce::Slider driveSlider;
    //juce::Slider bitDepthSlider;
    //juce::Slider sampleRateSlider;
    //juce::Slider mixSlider;

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
    //juce::Slider filterCutoffSlider;
    //juce::Slider filterResonanceSlider;
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
    //juce::Slider limiterThresholdSlider;
    //juce::Slider limiterReleaseSlider;
    juce::ToggleButton limiterEnabledButton;

    juce::Label limiterThresholdLabel;
    juce::Label limiterReleaseLabel;

    std::unique_ptr<SliderAttachment> limiterThresholdAttachment;
    std::unique_ptr<SliderAttachment> limiterReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limiterEnabledAttachment;

    // Gain controls
    //juce::Slider inputGainSlider;
    //juce::Slider outputGainSlider;

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
     
	// Reset Clip Button
    juce::TextButton resetClipButton;

    // Bypass button
    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    // Stereo width control
    //juce::Slider stereoWidthSlider;
    juce::Label stereoWidthLabel;
    std::unique_ptr<SliderAttachment> stereoWidthAttachment;

    juce::Image backgroundImage;

    juce::Image knobSpriteStrip;  // Add this for your sprite strip

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NaniDistortionAudioProcessorEditor)
};

class CustomKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override
    {
        auto image = dynamic_cast<NaniDistortionAudioProcessorEditor*>(slider.getParentComponent())->getKnobSprite();

        if (image.isValid())
        {
            const int numFrames = 24;  // Your sprite strip has 24 frames
            const int frameWidth = 128;
            const int frameHeight = 128;

            // Get the slider's value range
            auto range = slider.getRange();
            float minValue = range.getStart();
            float maxValue = range.getEnd();
            float currentValue = slider.getValue();

            // Calculate the normalized position (0.0 to 1.0)
            float normalizedPos = (currentValue - minValue) / (maxValue - minValue);

            // Calculate which frame to show
            int frameIndex = static_cast<int>(normalizedPos * (numFrames - 1));
            frameIndex = juce::jlimit(0, numFrames - 1, frameIndex);

            // Set high quality resampling
            g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

            // Calculate the correct size to maintain 1:1 aspect ratio
            int drawSize = juce::jmin(width, height);
            int drawX = x + (width - drawSize) / 2;
            int drawY = y + (height - drawSize) / 2;

            // Draw the appropriate frame
            g.drawImage(image,
                drawX, drawY, drawSize, drawSize,  // Destination (square)
                0, frameIndex * frameHeight,       // Source X, Y
                frameWidth, frameHeight);          // Source width, height
        }
        else
        {
            // Fallback drawing
            juce::LookAndFeel_V4::drawRotarySlider(g, x, y, width, height,
                sliderPos, rotaryStartAngle, rotaryEndAngle, slider);
        }
    }
};
