#include "PluginProcessor.h"
#include "PluginEditor.h"

NaniDistortionAudioProcessorEditor::NaniDistortionAudioProcessorEditor(NaniDistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::Background1_jpg, BinaryData::Background1_jpgSize);

	// Load the sprite strip for knobs
    knobSpriteStrip = juce::ImageCache::getFromMemory(BinaryData::Knob1_png,
        BinaryData::Knob1_pngSize);


    // A shorter alias for the Value Tree State
    auto& vts = processor.getValueTreeState();

    // --- Helper lambda for creating and attaching linear sliders ---
    // <<< THIS IS SIMPLIFIED. We no longer attach the label here.
    auto setupLinearSlider = [&](juce::Slider& slider, juce::Label& label, const std::string& paramID, const std::string& labelText, std::unique_ptr<SliderAttachment>& attachment)
    {
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
        attachment = std::make_unique<SliderAttachment>(vts, paramID, slider);

        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredLeft); // Align text nicely
    };

    // --- Setup all linear sliders using the helper ---
    setupLinearSlider(driveSlider, driveLabel, "drive", "Drive", driveAttachment);
    setupLinearSlider(bitDepthSlider, bitDepthLabel, "bitdepth", "Bit Depth", bitDepthAttachment);
    setupLinearSlider(sampleRateSlider, sampleRateLabel, "samplerate", "Sample Rate", sampleRateAttachment);
    setupLinearSlider(mixSlider, mixLabel, "mix", "Mix", mixAttachment);

    //-----------------------------------------------------------------------------------------------------------------------//
    // In your constructor after creating the filterCutoffSlider:
    addAndMakeVisible(filterCutoffSlider);
    filterCutoffSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterCutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    filterCutoffAttachment = std::make_unique<SliderAttachment>(vts, "filterCutoff", filterCutoffSlider);

    // Set the rotary parameters to match your knob's rotation range
    filterCutoffSlider.setRotaryParameters(
        juce::MathConstants<float>::pi * 1.5f,  // start angle (270 degrees)
        juce::MathConstants<float>::pi * 2.5f,  // end angle (450 degrees)
        true);  // stop at end

    // Set a fixed size for the knob (128x128 is ideal)
    filterCutoffSlider.setSize(80, 80);

    // Set a custom LookAndFeel for the knob
    auto customKnobLookAndFeel = std::make_unique<CustomKnobLookAndFeel>();
    filterCutoffSlider.setLookAndFeel(customKnobLookAndFeel.get());
    filterCutoffSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    customKnobLookAndFeel.release();
    //-----------------------------------------------------------------------------------------------------------------------//

    
    // --- Filter Components (Rotary) ---
    //addAndMakeVisible(filterCutoffSlider);
    //filterCutoffSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //filterCutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    //filterCutoffAttachment = std::make_unique<SliderAttachment>(vts, "filterCutoff", filterCutoffSlider);

    addAndMakeVisible(filterCutoffLabel);
    filterCutoffLabel.setText("Cutoff", juce::dontSendNotification);
    filterCutoffLabel.setJustificationType(juce::Justification::centred);
    filterCutoffLabel.attachToComponent(&filterCutoffSlider, false);

    //addAndMakeVisible(filterResonanceSlider);
    //filterResonanceSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    //filterResonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    //filterResonanceAttachment = std::make_unique<SliderAttachment>(vts, "filterResonance", filterResonanceSlider);
    // Resonance Knob (sprite-strip L&F)
    addAndMakeVisible(filterResonanceSlider);
    filterResonanceSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterResonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    filterResonanceSlider.setRotaryParameters(
        juce::MathConstants<float>::pi * 1.5f,
        juce::MathConstants<float>::pi * 2.5f,
        true
        );
    filterResonanceSlider.setSize(96, 96);
    filterResonanceAttachment = std::make_unique<SliderAttachment>(vts, "filterResonance", filterResonanceSlider);
        {
        auto lnf = std::make_unique<CustomKnobLookAndFeel>();
        filterResonanceSlider.setLookAndFeel(lnf.get());
        // we release ownership so JUCE will delete it when the slider is destroyed
            lnf.release();
        }

    addAndMakeVisible(filterResonanceLabel);
    filterResonanceLabel.setText("Resonance", juce::dontSendNotification);
    filterResonanceLabel.setJustificationType(juce::Justification::centred);
    filterResonanceLabel.attachToComponent(&filterResonanceSlider, false);

    // Drive Knob (rotary with sprite strip)
    addAndMakeVisible(driveSlider);
    driveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    driveSlider.setRotaryParameters(
        juce::MathConstants<float>::pi * 1.5f,   // start angle 270°
        juce::MathConstants<float>::pi * 2.5f,   // end angle 450°
        true
         );
    driveSlider.setSize(80, 80);
    driveAttachment = std::make_unique<SliderAttachment>(vts, "drive", driveSlider);
        // apply the same custom sprite‐knob L&F
        {
        auto lnf = std::make_unique<CustomKnobLookAndFeel>();
        driveSlider.setLookAndFeel(lnf.get());
        lnf.release();
        }
    addAndMakeVisible(driveLabel);
    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    driveLabel.attachToComponent(&driveSlider, false);

    // Mix Knob (rotary with sprite strip)
    addAndMakeVisible(mixSlider);
    mixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    mixSlider.setRotaryParameters(
        juce::MathConstants<float>::pi * 1.5f,
        juce::MathConstants<float>::pi * 2.5f,
        true
         );
    mixSlider.setSize(80, 80);
    mixAttachment = std::make_unique<SliderAttachment>(vts, "mix", mixSlider);
    {
        auto lnf = std::make_unique<CustomKnobLookAndFeel>();
        mixSlider.setLookAndFeel(lnf.get());
        lnf.release();
        }
    addAndMakeVisible(mixLabel);
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    mixLabel.attachToComponent(&mixSlider, false);

    // Bit Depth Knob
    addAndMakeVisible(bitDepthSlider);
    bitDepthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    bitDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    bitDepthSlider.setRotaryParameters(
        juce::MathConstants<float>::pi * 1.5f,
        juce::MathConstants<float>::pi * 2.5f,
        true
        );
    bitDepthSlider.setSize(80, 80);
    bitDepthAttachment = std::make_unique<SliderAttachment>(vts, "bitDepth", bitDepthSlider);
    {
        auto lnf = std::make_unique<CustomKnobLookAndFeel>();
        bitDepthSlider.setLookAndFeel(lnf.get());
        lnf.release();
        }
    addAndMakeVisible(bitDepthLabel);
    bitDepthLabel.setText("Bit Depth", juce::dontSendNotification);
    bitDepthLabel.setJustificationType(juce::Justification::centred);
    bitDepthLabel.attachToComponent(&bitDepthSlider, false);

    // Sample Rate Knob
    addAndMakeVisible(sampleRateSlider);
    sampleRateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    sampleRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    sampleRateSlider.setRotaryParameters(
        juce::MathConstants<float>::pi * 1.5f,
        juce::MathConstants<float>::pi * 2.5f,
        true
        );
    sampleRateSlider.setSize(80, 80);
    sampleRateAttachment = std::make_unique<SliderAttachment>(vts, "sampleRate", sampleRateSlider);
    {
        auto lnf = std::make_unique<CustomKnobLookAndFeel>();
        sampleRateSlider.setLookAndFeel(lnf.get());
        lnf.release();
        }
    addAndMakeVisible(sampleRateLabel);
    sampleRateLabel.setText("Sample Rate", juce::dontSendNotification);
    sampleRateLabel.setJustificationType(juce::Justification::centred);
    sampleRateLabel.attachToComponent(&sampleRateSlider, false);

    // --- ComboBoxes ---
    addAndMakeVisible(filterTypeComboBox);
    filterTypeComboBox.addItemList({ "Low-Pass", "High-Pass", "Band-Pass" }, 1);
    filterTypeAttachment = std::make_unique<ComboBoxAttachment>(vts, "filterType", filterTypeComboBox);
    addAndMakeVisible(filterTypeLabel);
    filterTypeLabel.setText("Filter Type", juce::dontSendNotification);
    filterTypeLabel.attachToComponent(&filterTypeComboBox, true);

    addAndMakeVisible(filterRoutingComboBox);
    filterRoutingComboBox.addItemList({ "Pre-Distortion", "Post-Distortion" }, 1);
    filterRoutingAttachment = std::make_unique<ComboBoxAttachment>(vts, "filterRouting", filterRoutingComboBox);
    addAndMakeVisible(filterRoutingLabel);
    filterRoutingLabel.setText("Filter Routing", juce::dontSendNotification);
    filterRoutingLabel.attachToComponent(&filterRoutingComboBox, true);
    
    
    
    // <<< ADD THE NEW DISTORTION TYPE COMBOBOX
    addAndMakeVisible(distortionTypeComboBox);
    distortionTypeComboBox.addItemList({ "Soft Clip", "Hard Clip", "Foldback", "Bit Glitch" }, 1);
    distortionTypeAttachment = std::make_unique<ComboBoxAttachment>(vts, "distortionType", distortionTypeComboBox);
    addAndMakeVisible(distortionTypeLabel);
    distortionTypeLabel.setText("Distortion Mode", juce::dontSendNotification);
    distortionTypeLabel.attachToComponent(&distortionTypeComboBox, true);


    // Add this to your constructor in PluginEditor.cpp:
    addAndMakeVisible(oversamplingComboBox);
    oversamplingComboBox.addItemList({ "Off", "2x", "4x", "8x", "16x" }, 1);
    oversamplingAttachment = std::make_unique<ComboBoxAttachment>(vts, "oversamplingFactor", oversamplingComboBox);
    addAndMakeVisible(oversamplingLabel);
    oversamplingLabel.setText("Oversampling", juce::dontSendNotification);
    oversamplingLabel.attachToComponent(&oversamplingComboBox, true);


    // Preset ComboBox
    addAndMakeVisible(presetComboBox);
    presetComboBox.setTextWhenNothingSelected("Select Preset");
    presetComboBox.onChange = [this] {
        if (presetComboBox.getSelectedItemIndex() >= 0)
            processor.loadPreset(presetComboBox.getText());
        };
    updatePresetComboBox();

    // Save Preset Button
    addAndMakeVisible(savePresetButton);
    savePresetButton.setButtonText("Save");
    savePresetButton.onClick = [this] { showSavePresetDialog(); };

    // Delete Preset Button
    addAndMakeVisible(deletePresetButton);
    deletePresetButton.setButtonText("Delete");
    deletePresetButton.onClick = [this] { showDeletePresetConfirmation(); };

    // Preset Name Editor
    addAndMakeVisible(presetNameEditor);
    presetNameEditor.setMultiLine(false);
    presetNameEditor.setJustification(juce::Justification::centred);
    presetNameEditor.setTextToShowWhenEmpty("New Preset Name", juce::Colours::grey.withAlpha(0.5f));



    // Limiter section title
    addAndMakeVisible(limiterEnabledButton);
    limiterEnabledButton.setButtonText("Limiter");
    limiterEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.getValueTreeState(), "limiterEnabled", limiterEnabledButton);

    // Limiter Threshold
    addAndMakeVisible(limiterThresholdSlider);
    limiterThresholdSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    limiterThresholdSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    limiterThresholdAttachment = std::make_unique<SliderAttachment>(
        processor.getValueTreeState(), "limiterThreshold", limiterThresholdSlider);

    addAndMakeVisible(limiterThresholdLabel);
    limiterThresholdLabel.setText("Threshold", juce::dontSendNotification);
    limiterThresholdLabel.setJustificationType(juce::Justification::centredLeft);

    // Limiter Release
    addAndMakeVisible(limiterReleaseSlider);
    limiterReleaseSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    limiterReleaseSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    limiterReleaseAttachment = std::make_unique<SliderAttachment>(
        processor.getValueTreeState(), "limiterRelease", limiterReleaseSlider);

    addAndMakeVisible(limiterReleaseLabel);
    limiterReleaseLabel.setText("Release", juce::dontSendNotification);
    limiterReleaseLabel.setJustificationType(juce::Justification::centredLeft);

  

	// --- Gain Controls ---
    // Input Gain
    addAndMakeVisible(inputGainSlider);
    inputGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    inputGainAttachment = std::make_unique<SliderAttachment>(
        processor.getValueTreeState(), "inputGain", inputGainSlider);

    addAndMakeVisible(inputGainLabel);
    inputGainLabel.setText("Input Gain", juce::dontSendNotification);
    inputGainLabel.setJustificationType(juce::Justification::centredLeft);

    // Output Gain
    addAndMakeVisible(outputGainSlider);
    outputGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    outputGainAttachment = std::make_unique<SliderAttachment>(
        processor.getValueTreeState(), "outputGain", outputGainSlider);

    addAndMakeVisible(outputGainLabel);
    outputGainLabel.setText("Output Gain", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centredLeft);

  

    // Level meter labels
    addAndMakeVisible(inputMeterLabel);
    inputMeterLabel.setText("Input", juce::dontSendNotification);
    inputMeterLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(outputMeterLabel);
    outputMeterLabel.setText("Output", juce::dontSendNotification);
    outputMeterLabel.setJustificationType(juce::Justification::centred);

    // Level meters
    addAndMakeVisible(inputLevelMeterL);
    addAndMakeVisible(inputLevelMeterR);
    addAndMakeVisible(outputLevelMeterL);
    addAndMakeVisible(outputLevelMeterR);


    //// Start a timer to update the meters
    //startTimerHz(30); // 30 fps is smooth enough for meters

  

    // Start a timer to update the meters
    startTimerHz(30); // 30 fps is smooth enough for meters (Increase for reactive meters)

	// Reset Clip Button
    addAndMakeVisible(resetClipButton);
    resetClipButton.setButtonText("Reset Clip");
    resetClipButton.onClick = [this]() {
        inputLevelMeterL.resetClipping();
        inputLevelMeterR.resetClipping();
        outputLevelMeterL.resetClipping();
        outputLevelMeterR.resetClipping();
        };

    // Bypass button
    addAndMakeVisible(bypassButton);
    bypassButton.setButtonText("Bypass");
    bypassButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::red);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.getValueTreeState(), "bypass", bypassButton);

    // Stereo width control
    addAndMakeVisible(stereoWidthSlider);
    stereoWidthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    stereoWidthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    stereoWidthAttachment = std::make_unique<SliderAttachment>(
        processor.getValueTreeState(), "stereoWidth", stereoWidthSlider);

    addAndMakeVisible(stereoWidthLabel);
    stereoWidthLabel.setText("Stereo Width", juce::dontSendNotification);
    stereoWidthLabel.setJustificationType(juce::Justification::centred);
    stereoWidthLabel.attachToComponent(&stereoWidthSlider, false);

    // Input and Output Gain
    inputGainSlider.setValueDisplayMode(CustomSlider::Decibels);
    outputGainSlider.setValueDisplayMode(CustomSlider::Decibels);

    // Drive
    driveSlider.setValueDisplayMode(CustomSlider::Times);

    // Bit Depth
    bitDepthSlider.setValueDisplayMode(CustomSlider::Samples);

    // Sample Rate Reduction
    sampleRateSlider.setValueDisplayMode(CustomSlider::Percentage);

    // Mix
    mixSlider.setValueDisplayMode(CustomSlider::Percentage);

    // Filter Cutoff
    filterCutoffSlider.setValueDisplayMode(CustomSlider::Hertz);

    // Filter Resonance
    filterResonanceSlider.setValueDisplayMode(CustomSlider::Ratio);

    // Limiter Threshold
    limiterThresholdSlider.setValueDisplayMode(CustomSlider::Decibels);

    // Limiter Release
    limiterReleaseSlider.setValueDisplayMode(CustomSlider::Milliseconds);

    // Stereo Width
    stereoWidthSlider.setValueDisplayMode(CustomSlider::Ratio);

    // At the end of your constructor
    updateAllSliderDisplays();

    // Adjust window size to accommodate meters
    setSize(600, 600); 
}

NaniDistortionAudioProcessorEditor::~NaniDistortionAudioProcessorEditor() 
{
    filterCutoffSlider.setLookAndFeel(nullptr); // Important to avoid memory leaks
    stopTimer();
}

void NaniDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    // === BACKGROUND ===
    if (backgroundImage.isValid())
    {
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    }
    else
    {
        g.fillAll(juce::Colour::fromRGB(35, 35, 39));
    }

    // === TITLE ===
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    auto header = getLocalBounds().removeFromTop(60);
    auto titleArea = header.withTrimmedLeft(100).withTrimmedRight(120); // between bypass and stereo width
    g.drawFittedText("Nani Distortion", titleArea, juce::Justification::centred, 1);

    // === SECTION DIVIDERS ===
    g.setColour(juce::Colours::darkgrey);
    g.setFont(14.0f);

    auto drawDivider = [&](int y, const juce::String& text)
        {
            auto area = getLocalBounds();
            g.fillRect(area.withY(y).withHeight(1).reduced(10, 0));
            g.setColour(juce::Colours::white);
            g.drawText(text, area.withY(y - 15).withHeight(20), juce::Justification::centred);
            g.setColour(juce::Colours::darkgrey);
        };

    // Update divider Y positions to match compact layout
    drawDivider(100, "Gain");
    drawDivider(150, "Filter");
    drawDivider(270, "Distortion");
    drawDivider(380, "Processing Mode");
	//drawDivider(390, "Presets"); Presets are now at the top
    drawDivider(500, "Limiter");
}


void NaniDistortionAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    const int headerHeight = 60;
    const int meterWidth = 20;
    const int meterSpacing = 4;
    const int labelWidth = 90;
    const int controlHeight = 25;
    const int controlSpacing = 8;
    const int comboBoxHeight = 28;
    const int rotaryKnobSize = 80;

    // === PRESET CONTROLS ===
    auto presetRow = bounds.removeFromTop(28);
    auto w = presetRow.getWidth();
    presetComboBox.setBounds(presetRow.removeFromLeft(w * 0.25f).reduced(2));
    presetNameEditor.setBounds(presetRow.removeFromLeft(w * 0.35f).reduced(2));
    savePresetButton.setBounds(presetRow.removeFromLeft(w * 0.2f).reduced(2));
    deletePresetButton.setBounds(presetRow.reduced(2));

    bounds.removeFromTop(6);

    // ===== HEADER =====
    auto header = bounds.removeFromTop(headerHeight);
    bypassButton.setBounds(header.removeFromLeft(100).reduced(5));
    stereoWidthLabel.setBounds(header.removeFromRight(120).removeFromTop(15));
    stereoWidthSlider.setBounds(header.removeFromTop(rotaryKnobSize));

    // ===== METERS LEFT/RIGHT =====
    auto leftMeters = bounds.removeFromLeft(meterWidth * 2 + meterSpacing + 5);
    inputMeterLabel.setBounds(leftMeters.removeFromTop(15));
    inputLevelMeterL.setBounds(leftMeters.removeFromLeft(meterWidth));
    leftMeters.removeFromLeft(meterSpacing);
    inputLevelMeterR.setBounds(leftMeters.removeFromLeft(meterWidth));

    auto rightMeters = bounds.removeFromRight(meterWidth * 2 + meterSpacing + 5);
    outputMeterLabel.setBounds(rightMeters.removeFromTop(15));
    outputLevelMeterL.setBounds(rightMeters.removeFromLeft(meterWidth));
    rightMeters.removeFromLeft(meterSpacing);
    outputLevelMeterR.setBounds(rightMeters.removeFromLeft(meterWidth));

    // ===== MAIN AREA =====
    auto area = bounds.reduced(6);

    auto layoutRow = [&](CustomSlider& s1, juce::Label& l1, CustomSlider& s2, juce::Label& l2)
    {
        auto row = area.removeFromTop(controlHeight);
        auto half = row.removeFromLeft(row.getWidth() / 2);

        l1.setBounds(half.removeFromLeft(labelWidth));
        s1.setBounds(half.reduced(2, 0));

        l2.setBounds(row.removeFromLeft(labelWidth));
        s2.setBounds(row.reduced(2, 0));

        area.removeFromTop(controlSpacing);
    };

    // === Gain ===
    layoutRow(inputGainSlider, inputGainLabel, outputGainSlider, outputGainLabel);

    // === Filter knobs ===
    auto filterArea = area.removeFromTop(rotaryKnobSize + 10);
    filterCutoffSlider.setBounds(filterArea.removeFromLeft(filterArea.getWidth() / 2).reduced(8));
    filterResonanceSlider.setBounds(filterArea.reduced(8));
    area.removeFromTop(controlSpacing);

    // === Distortion Sliders ===
    //layoutRow(driveSlider, driveLabel, bitDepthSlider, bitDepthLabel);
    //layoutRow(sampleRateSlider, sampleRateLabel, mixSlider, mixLabel);
    //// Place Drive and Mix as big rotary knobs
    //auto knobRow = area.removeFromTop(rotaryKnobSize + 10);
    //// Drive
    //driveSlider.setBounds(knobRow.removeFromLeft(knobRow.getWidth() / 2).reduced(8));
    //// Bit Depth / Sample Rate remain linear in the next rows
    //auto nextRow = area.removeFromTop(controlHeight);
    //bitDepthLabel.setBounds(nextRow.removeFromLeft(labelWidth));
    //bitDepthSlider.setBounds(nextRow.reduced(2, 0));
    //area.removeFromTop(controlSpacing);
    //nextRow = area.removeFromTop(controlHeight);
    //sampleRateLabel.setBounds(nextRow.removeFromLeft(labelWidth));
    //sampleRateSlider.setBounds(nextRow.reduced(2, 0));
    //area.removeFromTop(controlSpacing);
    //// Mix
    //auto mixRow = area.removeFromTop(rotaryKnobSize + 10);
    //mixSlider.setBounds(mixRow.removeFromLeft(mixRow.getWidth() / 2).reduced(8));
    // find the tallest of the four knobs
    int knobHeight = juce::jmax(
        driveSlider.getHeight(),
        bitDepthSlider.getHeight(),
        sampleRateSlider.getHeight(),
        mixSlider.getHeight()
    );

    // allocate that height plus a bit of padding
    auto knobArea = area.removeFromTop(knobHeight + 5);

    // split into four equal columns
    int colW = knobArea.getWidth() / 4;

    // Drive
    auto r = knobArea.removeFromLeft(colW).reduced(8);
    driveSlider.setBounds(r);

    // Bit Depth
    r = knobArea.removeFromLeft(colW).reduced(8);
    bitDepthSlider.setBounds(r);

    // Sample Rate
    r = knobArea.removeFromLeft(colW).reduced(8);
    sampleRateSlider.setBounds(r);

    // Mix
    r = knobArea.removeFromLeft(colW).reduced(8);
    mixSlider.setBounds(r);

    // === ComboBoxes ===
    auto comboRow = [&](juce::ComboBox& cb)
    {
        cb.setBounds(area.removeFromTop(comboBoxHeight).reduced(40, 0));
        area.removeFromTop(4);
    };

    comboRow(distortionTypeComboBox);
    comboRow(filterTypeComboBox);
    comboRow(filterRoutingComboBox);
    comboRow(oversamplingComboBox);

    //// === Presets Row ===
    //auto presetRow = area.removeFromTop(28);
    //auto w = presetRow.getWidth();
    //presetComboBox.setBounds(presetRow.removeFromLeft(w * 0.25f).reduced(2));
    //presetNameEditor.setBounds(presetRow.removeFromLeft(w * 0.35f).reduced(2));
    //savePresetButton.setBounds(presetRow.removeFromLeft(w * 0.2f).reduced(2));
    //deletePresetButton.setBounds(presetRow.reduced(2));

    //area.removeFromTop(6);

    // === Limiter ===
    limiterEnabledButton.setBounds(area.removeFromTop(22).reduced(4));

    auto limiterRow = area.removeFromTop(controlHeight);
    auto half = limiterRow.removeFromLeft(limiterRow.getWidth() / 2);
    limiterThresholdLabel.setBounds(half.removeFromLeft(labelWidth));
    limiterThresholdSlider.setBounds(half.reduced(2, 0));

    limiterReleaseLabel.setBounds(limiterRow.removeFromLeft(labelWidth));
    limiterReleaseSlider.setBounds(limiterRow.reduced(2, 0));

    // === Reset Clip ===
    // Move reset button to the bottom center
    auto resetArea = getLocalBounds().reduced(10);
    resetClipButton.setBounds(resetArea.removeFromBottom(40).withSizeKeepingCentre(100, 24));
   
}



void NaniDistortionAudioProcessorEditor::updatePresetComboBox()
{
    // Clear the combo box
    presetComboBox.clear();

    // Get the preset list from the processor
    juce::StringArray presetList = processor.getPresetList();

    // Add the presets to the combo box
    presetComboBox.addItemList(presetList, 1);

    // Select the current preset if there is one
    juce::String currentPreset = processor.getCurrentPresetName();
    if (currentPreset.isNotEmpty())
    {
        presetComboBox.setText(currentPreset, juce::dontSendNotification);
    }
}

void NaniDistortionAudioProcessorEditor::showSavePresetDialog()
{
    // Get the current text from the preset name editor
    juce::String presetName = presetNameEditor.getText();

    // If the preset name is empty, show an alert
    if (presetName.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Save Preset",
            "Please enter a preset name.");
        return;
    }

    // Check if the preset already exists
    juce::StringArray presetList = processor.getPresetList();
    if (presetList.contains(presetName))
    {
        // Ask for confirmation to overwrite
        juce::AlertWindow::showOkCancelBox(juce::AlertWindow::QuestionIcon,
            "Overwrite Preset",
            "A preset with this name already exists. Do you want to overwrite it?",
            "Yes",
            "No",
            this,
            juce::ModalCallbackFunction::create([this, presetName](int result) {
                if (result == 1) // User clicked "Yes"
                {
                    // Save the preset
                    processor.savePreset(presetName);

                    // Update the combo box
                    updatePresetComboBox();

                    // Clear the preset name editor
                    presetNameEditor.clear();
                }
                }));
    }
    else
    {
        // Save the preset
        processor.savePreset(presetName);

        // Update the combo box
        updatePresetComboBox();

        // Clear the preset name editor
        presetNameEditor.clear();
    }
}

void NaniDistortionAudioProcessorEditor::showDeletePresetConfirmation()
{
    // Get the selected preset name
    juce::String presetName = presetComboBox.getText();

    // If no preset is selected, show an alert
    if (presetName.isEmpty() || presetComboBox.getSelectedItemIndex() < 0)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Delete Preset",
            "Please select a preset to delete.");
        return;
    }

    // Ask for confirmation
    juce::AlertWindow::showOkCancelBox(juce::AlertWindow::WarningIcon,
        "Delete Preset",
        "Are you sure you want to delete the preset '" + presetName + "'?",
        "Yes",
        "No",
        this,
        juce::ModalCallbackFunction::create([this, presetName](int result) {
            if (result == 1) // User clicked "Yes"
            {
                // Delete the preset
                processor.deletePreset(presetName);

                // Update the combo box
                updatePresetComboBox();
            }
            }));
}
void NaniDistortionAudioProcessorEditor::updateAllSliderDisplays()
{
    // Update all slider displays
    inputGainSlider.updateTextDisplay();
    outputGainSlider.updateTextDisplay();
    driveSlider.updateTextDisplay();
    bitDepthSlider.updateTextDisplay();
    sampleRateSlider.updateTextDisplay();
    mixSlider.updateTextDisplay();
    filterCutoffSlider.updateTextDisplay();
    filterResonanceSlider.updateTextDisplay();
    limiterThresholdSlider.updateTextDisplay();
    limiterReleaseSlider.updateTextDisplay();
    stereoWidthSlider.updateTextDisplay();
}

void NaniDistortionAudioProcessorEditor::timerCallback()
{
    // Update the level meters
    inputLevelMeterL.setLevel(processor.getInputLevel(0));
    inputLevelMeterR.setLevel(processor.getInputLevel(1));
    outputLevelMeterL.setLevel(processor.getOutputLevel(0));
    outputLevelMeterR.setLevel(processor.getOutputLevel(1));

    // Update slider displays on first timer call
    static bool firstTimerCall = true;
    if (firstTimerCall)
    {
        updateAllSliderDisplays();
        firstTimerCall = false;
    }
}

