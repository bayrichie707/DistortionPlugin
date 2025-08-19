#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LevelMeter.h"
#include "CustomSlider.h"

class CustomKnobLookAndFeel;

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

    // >>> Add these as PUBLIC so the Look&Feel can call them
    juce::Image getSpriteFor(const juce::Slider& s) const;
    int         getSpriteFramesFor(const juce::Slider& s) const;


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

    // --- Choice knobs rendered as rotary sliders ---
    CustomSlider filterTypeKnob, distortionTypeKnob, oversamplingKnob;
    juce::Label  filterTypeKnobLabel, distortionTypeKnobLabel, oversamplingKnobLabel;
    std::unique_ptr<SliderAttachment> filterTypeKnobAttachment, distortionTypeKnobAttachment, oversamplingKnobAttachment;

    // Shared LookAndFeel for all knobs
    std::unique_ptr<CustomKnobLookAndFeel> knobLNF;

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

    // Sprites & their frame counts
    juce::Image spriteDefault;
    juce::Image spriteFilterType;
    int spriteDefaultFrames = 64; // whatever your main knob strip uses
    int spriteFilterTypeFrames = 3;  // our 3-step strip

    //// Helpers for the L&F
    //juce::Image getSpriteFor(const juce::Slider& s) const;
    //int         getSpriteFramesFor(const juce::Slider& s) const;


    // Add this line for your logo:
    juce::ImageComponent logoComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NaniDistortionAudioProcessorEditor)
};

class CustomKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void CustomKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override
    {
        // Safer than dynamic_cast to parent: walk up to the editor
        if (auto* editor = slider.findParentComponentOfClass<NaniDistortionAudioProcessorEditor>())
        {
            // Ask the editor which sprite and how many frames this slider should use
            auto img = editor->getSpriteFor(slider);
            const int numFrames = editor->getSpriteFramesFor(slider);

            if (img.isValid() && numFrames > 0)
            {
                // JUCE gives us a normalized [0..1] position that respects param skew
                const float normalized = juce::jlimit(0.0f, 1.0f, sliderPos);
                const int frameIndex = juce::jlimit(0, numFrames - 1,
                    (int)std::round(normalized * (numFrames - 1)));

                // Assume a VERTICAL strip (N frames stacked top→bottom). If horizontal, swap logic.
                const int frameH = img.getHeight() / numFrames;
                const int frameW = img.getWidth();

                // Draw square, centered in the bounds
                g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
                const int drawSize = juce::jmin(width, height);
                const int dx = x + (width - drawSize) / 2;
                const int dy = y + (height - drawSize) / 2;

                g.drawImage(img,
                    dx, dy, drawSize, drawSize,     // destination
                    0, frameIndex * frameH,         // source (x, y)
                    frameW, frameH);                // source (w, h)
                return;
            }
        }

        // Fallback if no sprite is available
        juce::LookAndFeel_V4::drawRotarySlider(g, x, y, width, height,
            sliderPos, rotaryStartAngle, rotaryEndAngle, slider);
    }
};
