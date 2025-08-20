#include "PluginProcessor.h"
#include "PluginEditor.h"

NaniDistortionAudioProcessorEditor::NaniDistortionAudioProcessorEditor(NaniDistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    // === Load assets ===
    /*backgroundImage = juce::ImageCache::getFromMemory(BinaryData::Background2_jpg, BinaryData::Background2_jpgSize);*/
	backgroundImage = juce::ImageCache::getFromMemory(BinaryData::Background3_png, BinaryData::Background3_pngSize);
    //knobSpriteStrip = juce::ImageCache::getFromMemory(BinaryData::Knob1_png, BinaryData::Knob1_pngSize);



    // 2a) Load sprites from BinaryData
    spriteDefault = juce::ImageCache::getFromMemory(BinaryData::cutoffKnob1_png,
        BinaryData::cutoffKnob1_pngSize);   // your existing strip
    spriteDefaultFrames = 64; // set to the real number for your default strip

    // 3-frame strip for the Filter Type knob (frames ordered: 180°, 90°, 0°)
    spriteFilterType = juce::ImageCache::getFromMemory(BinaryData::filterTypeKnob1_png,
        BinaryData::filterTypeKnob1_pngSize);
    spriteFilterTypeFrames = 3;

    spriteOversampling = juce::ImageCache::getFromMemory(
        BinaryData::oversamplingKnob1_png,
        BinaryData::oversamplingKnob1_pngSize);
    spriteOversamplingFrames = 5;

    // identify the slider so L&F can pick the right strip
    oversamplingKnob.setComponentID("oversampling");
    // 5 steps: 0..4 (e.g., 1x, 2x, 4x, 8x, 16x)
    oversamplingKnob.setRange(0.0, 4.0, 1.0);
    oversamplingKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    // 2b) Give component IDs so we can distinguish them in L&F
    filterTypeKnob.setComponentID("filterType");    // identify this slider
    filterTypeKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    filterTypeKnob.setRange(0.0, 2.0, 1.0);         // 3 steps: 0,1,2
    // (Optional) tag others too if you want per-knob sprites later

   /* knobSpriteStrip = juce::ImageCache::getFromMemory(BinaryData::cutoffKnob1_png, BinaryData::cutoffKnob1_pngSize);*/


    //auto logoImg = juce::ImageCache::getFromMemory(BinaryData::swordfishLogo_png, BinaryData::swordfishLogo_pngSize);
    //logoComponent.setImage(logoImg, juce::RectanglePlacement::fillDestination);
    //addAndMakeVisible(logoComponent);

    // at the top of the constructor, before creating knobs
    knobLNF = std::make_unique<CustomKnobLookAndFeel>();

    // === ValueTree alias ===
    auto& vts = processor.getValueTreeState();

    // === Helper to create rotary knob ===
    auto setupRotaryKnob = [&](CustomSlider& slider, juce::Label& label, const std::string& paramID,
        const std::string& labelText, std::unique_ptr<SliderAttachment>& attachment,
        int size = 80)
        {
            addAndMakeVisible(slider);
            slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
            slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.5f,
                juce::MathConstants<float>::pi * 2.5f, true);
            slider.setSize(size, size);
            attachment = std::make_unique<SliderAttachment>(vts, paramID, slider);
            // AFTER
            slider.setLookAndFeel(knobLNF.get());

            addAndMakeVisible(label);
            label.setText(labelText, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centred);
            label.attachToComponent(&slider, false);
        };

    // === Helper to create linear slider ===
    auto setupLinearSlider = [&](CustomSlider& slider, juce::Label& label, const std::string& paramID,
        const std::string& labelText, std::unique_ptr<SliderAttachment>& attachment)
        {
            addAndMakeVisible(slider);
            slider.setSliderStyle(juce::Slider::LinearHorizontal);
            slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
            attachment = std::make_unique<SliderAttachment>(vts, paramID, slider);

            addAndMakeVisible(label);
            label.setText(labelText, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centredLeft);
        };

    // === Rotary Knobs (existing) ===
    setupRotaryKnob(driveSlider, driveLabel, "drive", "Drive", driveAttachment);
    setupRotaryKnob(mixSlider, mixLabel, "mix", "Mix", mixAttachment);
    setupRotaryKnob(bitDepthSlider, bitDepthLabel, "bitdepth", "Bit Depth", bitDepthAttachment, 80);
    setupRotaryKnob(sampleRateSlider, sampleRateLabel, "samplerate", "Sample Rate", sampleRateAttachment, 80);
    setupRotaryKnob(filterCutoffSlider, filterCutoffLabel, "filterCutoff", "Cutoff", filterCutoffAttachment);
    setupRotaryKnob(filterResonanceSlider, filterResonanceLabel, "filterResonance", "Resonance", filterResonanceAttachment, 72);
    setupRotaryKnob(stereoWidthSlider, stereoWidthLabel, "stereoWidth", "Stereo Width", stereoWidthAttachment);

    // NEW: make these rotary (were linear)
    setupRotaryKnob(inputGainSlider, inputGainLabel, "inputGain", "Input", inputGainAttachment);
    setupRotaryKnob(outputGainSlider, outputGainLabel, "outputGain", "Output", outputGainAttachment);
    setupRotaryKnob(limiterThresholdSlider, limiterThresholdLabel, "limiterThreshold", "Threshold", limiterThresholdAttachment);
    setupRotaryKnob(limiterReleaseSlider, limiterReleaseLabel, "limiterRelease", "Release", limiterReleaseAttachment);

    // NEW: stepped “choice” knobs for choice parameters
    auto setupChoiceKnob = [&](CustomSlider& s, juce::Label& l, const char* id,
        const char* label, std::unique_ptr<SliderAttachment>& att, int choices)
        {
            addAndMakeVisible(s);
            s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
            s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            s.setRotaryParameters(juce::MathConstants<float>::pi * 1.5f,
                juce::MathConstants<float>::pi * 2.5f, true);
            s.setRange(0.0, choices - 1.0, 1.0); // snap to steps
            s.setLookAndFeel(knobLNF.get());
            att = std::make_unique<SliderAttachment>(processor.getValueTreeState(), id, s);

            addAndMakeVisible(l);
            l.setText(label, juce::dontSendNotification);
            l.setJustificationType(juce::Justification::centred);
            l.setInterceptsMouseClicks(false, false);
        };

    setupChoiceKnob(filterTypeKnob, filterTypeKnobLabel, "filterType", "Filter Type", filterTypeKnobAttachment, 3);
    setupChoiceKnob(distortionTypeKnob, distortionTypeKnobLabel, "distortionType", "Type", distortionTypeKnobAttachment, 4);
    setupChoiceKnob(oversamplingKnob, oversamplingKnobLabel, "oversamplingFactor", "Oversampling", oversamplingKnobAttachment, 5);

    // Use the shared L&F for all existing rotary knobs too
    for (auto* s : { &driveSlider, &mixSlider, &bitDepthSlider, &sampleRateSlider,
                     &filterCutoffSlider, &filterResonanceSlider,
                     &stereoWidthSlider, &inputGainSlider, &outputGainSlider,
                     &limiterThresholdSlider, &limiterReleaseSlider })
	{
        s->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s->setLookAndFeel(knobLNF.get());
	}


    // === Buttons ===
    addAndMakeVisible(bypassButton);
    bypassButton.setButtonText("Bypass");
    bypassButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::red);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "bypass", bypassButton);

    addAndMakeVisible(limiterEnabledButton);
    limiterEnabledButton.setButtonText("Limiter");
    limiterEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(vts, "limiterEnabled", limiterEnabledButton);

    // === ComboBoxes ===
    auto setupComboBox = [&](juce::ComboBox& box, juce::Label& label, const juce::StringArray& items,
        const std::string& paramID, const std::string& labelText, std::unique_ptr<ComboBoxAttachment>& attachment)
        {
            addAndMakeVisible(box);
            box.addItemList(items, 1);
            attachment = std::make_unique<ComboBoxAttachment>(vts, paramID, box);

            addAndMakeVisible(label);
            label.setText(labelText, juce::dontSendNotification);
            label.attachToComponent(&box, true);
        };

    setupComboBox(filterTypeComboBox, filterTypeLabel, { "Low-Pass", "High-Pass", "Band-Pass" }, "filterType", "Filter Type", filterTypeAttachment);
    setupComboBox(filterRoutingComboBox, filterRoutingLabel, { "Pre-Distortion", "Post-Distortion" }, "filterRouting", "Filter Routing", filterRoutingAttachment);
    setupComboBox(distortionTypeComboBox, distortionTypeLabel, { "Soft Clip", "Hard Clip", "Foldback", "Bit Glitch" }, "distortionType", "Distortion Mode", distortionTypeAttachment);
    setupComboBox(oversamplingComboBox, oversamplingLabel, { "Off", "2x", "4x", "8x", "16x" }, "oversamplingFactor", "Oversampling", oversamplingAttachment);


    // Hide the combo boxes this layout replaces
    filterTypeComboBox.setVisible(false);
    filterRoutingComboBox.setVisible(false);
    distortionTypeComboBox.setVisible(false);
    oversamplingComboBox.setVisible(false);

    // === Preset UI ===
    addAndMakeVisible(presetComboBox);
    presetComboBox.setTextWhenNothingSelected("Select Preset");
    presetComboBox.onChange = [this] {
        if (presetComboBox.getSelectedItemIndex() >= 0)
            processor.loadPreset(presetComboBox.getText());
        };

    addAndMakeVisible(savePresetButton);
    savePresetButton.setButtonText("Save");
    savePresetButton.onClick = [this] { showSavePresetDialog(); };

    addAndMakeVisible(deletePresetButton);
    deletePresetButton.setButtonText("Delete");
    deletePresetButton.onClick = [this] { showDeletePresetConfirmation(); };

    addAndMakeVisible(presetNameEditor);
    presetNameEditor.setMultiLine(false);
    presetNameEditor.setJustification(juce::Justification::centred);
    presetNameEditor.setTextToShowWhenEmpty("New Preset Name", juce::Colours::grey.withAlpha(0.5f));
    updatePresetComboBox();

    // === Level Meters ===
    addAndMakeVisible(inputMeterLabel);
    inputMeterLabel.setText("Input", juce::dontSendNotification);
    inputMeterLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(outputMeterLabel);
    outputMeterLabel.setText("Output", juce::dontSendNotification);
    outputMeterLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(inputLevelMeterL);
    addAndMakeVisible(inputLevelMeterR);
    addAndMakeVisible(outputLevelMeterL);
    addAndMakeVisible(outputLevelMeterR);

    // 13 LEDs → 14 frames (frame 0 = all off, frame 13 = all on)
    // Replace the BinaryData symbol with your actual asset name.
    auto meterStrip = juce::ImageCache::getFromMemory(
        BinaryData::outputMeterRight_png,  // <-- your PNG name
        BinaryData::outputMeterRight_pngSize);

    for (auto* m : { &inputLevelMeterL, &inputLevelMeterR, &outputLevelMeterL, &outputLevelMeterR })
        m->setSpriteStrip(meterStrip, /*frames*/14, /*frame0AtTop*/ true, /*numLights*/ 13);

    // === Reset Clip Button ===
    addAndMakeVisible(resetClipButton);
    resetClipButton.setButtonText("Reset Clip");
    resetClipButton.onClick = [this]() {
        inputLevelMeterL.resetClipping();
        inputLevelMeterR.resetClipping();
        outputLevelMeterL.resetClipping();
        outputLevelMeterR.resetClipping();
        };

    // === Value Display Modes ===
    inputGainSlider.setValueDisplayMode(CustomSlider::Decibels);
    outputGainSlider.setValueDisplayMode(CustomSlider::Decibels);
    driveSlider.setValueDisplayMode(CustomSlider::Times);
    bitDepthSlider.setValueDisplayMode(CustomSlider::Samples);
    sampleRateSlider.setValueDisplayMode(CustomSlider::Percentage);
    mixSlider.setValueDisplayMode(CustomSlider::Percentage);
    filterCutoffSlider.setValueDisplayMode(CustomSlider::Hertz);
    filterResonanceSlider.setValueDisplayMode(CustomSlider::Ratio);
    limiterThresholdSlider.setValueDisplayMode(CustomSlider::Decibels);
    limiterReleaseSlider.setValueDisplayMode(CustomSlider::Milliseconds);
    stereoWidthSlider.setValueDisplayMode(CustomSlider::Ratio);

    //// Optional: hide the preset bar to match art
    //presetComboBox.setVisible(false);
    //presetNameEditor.setVisible(false);
    //savePresetButton.setVisible(false);
    //deletePresetButton.setVisible(false);

    // The artwork already contains text under knobs; hide extra labels
    for (auto* lbl : { &driveLabel,&mixLabel,&bitDepthLabel,&sampleRateLabel,
                       &filterCutoffLabel,&filterResonanceLabel,
                       &stereoWidthLabel,&inputGainLabel,&outputGainLabel,
                       &limiterThresholdLabel,&limiterReleaseLabel })
        lbl->setVisible(false);

    // Finalize
    updateAllSliderDisplays();
    startTimerHz(30); // Smooth meter updates
    setSize(600, 600);

}


NaniDistortionAudioProcessorEditor::~NaniDistortionAudioProcessorEditor() 
{
    for (auto* s : {
        &driveSlider, &bitDepthSlider, &sampleRateSlider, &mixSlider,
        &filterCutoffSlider, &filterResonanceSlider, &stereoWidthSlider,
        &inputGainSlider, &outputGainSlider,
        &limiterThresholdSlider, &limiterReleaseSlider,
        &filterTypeKnob, &distortionTypeKnob, &oversamplingKnob // ← add these
        })
        s->setLookAndFeel(nullptr);

    knobLNF.reset();
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

    //// === TITLE ===
    //g.setColour(juce::Colours::white);
    //g.setFont(20.0f);
    //auto header = getLocalBounds().removeFromTop(100);
    //auto titleArea = header.withTrimmedLeft(100).withTrimmedRight(120); // between bypass and stereo width
    //g.drawFittedText("Nani Distortion", titleArea, juce::Justification::centred, 1);

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

 //   // Update divider Y positions to match compact layout
 //   drawDivider(100, "Gain");
 //   drawDivider(150, "Filter");
 //   drawDivider(270, "Distortion");
 //   drawDivider(380, "Processing Mode");
	////drawDivider(390, "Presets"); Presets are now at the top
 //   drawDivider(500, "Limiter");
}


//void NaniDistortionAudioProcessorEditor::resized()
//{
//    auto bounds = getLocalBounds();
//
//    // ────── Layout Constants ──────
//    const int headerHeight = 60;
//    const int meterWidth = 20;
//    const int meterSpacing = 4;
//    const int labelWidth = 90;
//    const int controlHeight = 25;
//    const int controlSpacing = 8;
//    const int comboBoxHeight = 28;
//    const int rotaryKnobSize = 72;
//    const int margin = 8;
//
//    // ────── Preset Bar ──────
//    auto presetRow = bounds.removeFromTop(28);
//    auto w = presetRow.getWidth();
//    presetComboBox.setBounds(presetRow.removeFromLeft(w * 0.25f).reduced(2));
//    presetNameEditor.setBounds(presetRow.removeFromLeft(w * 0.35f).reduced(2));
//    savePresetButton.setBounds(presetRow.removeFromLeft(w * 0.2f).reduced(2));
//    deletePresetButton.setBounds(presetRow.reduced(2));
//
//    bounds.removeFromTop(6); // spacing
//
//    // ────── Logo Placement (No Layout Shift) ──────
//    constexpr int logoW = 180;
//    constexpr int logoH = 80;
//    logoComponent.setBounds(getWidth() - logoW - margin, margin, logoW, logoH);
//
//    // ────── Layout Area (No shift for logo) ──────
//    bounds.removeFromTop(20); // Optional minimal top margin
//    auto content = bounds;
//
//    // ────── Header Section ──────
//    auto header = content.removeFromTop(headerHeight);
//    bypassButton.setBounds(header.removeFromLeft(100).reduced(5));
//    stereoWidthLabel.setBounds(header.removeFromRight(120).removeFromTop(15));
//    stereoWidthSlider.setBounds(header.removeFromTop(rotaryKnobSize));
//
//    // ────── Meters ──────
//    auto leftMeters = content.removeFromLeft(meterWidth * 2 + meterSpacing + 5);
//    inputMeterLabel.setBounds(leftMeters.removeFromTop(15));
//    inputLevelMeterL.setBounds(leftMeters.removeFromLeft(meterWidth));
//    leftMeters.removeFromLeft(meterSpacing);
//    inputLevelMeterR.setBounds(leftMeters.removeFromLeft(meterWidth));
//
//    auto rightMeters = content.removeFromRight(meterWidth * 2 + meterSpacing + 5);
//    outputMeterLabel.setBounds(rightMeters.removeFromTop(15));
//    outputLevelMeterL.setBounds(rightMeters.removeFromLeft(meterWidth));
//    rightMeters.removeFromLeft(meterSpacing);
//    outputLevelMeterR.setBounds(rightMeters.removeFromLeft(meterWidth));
//
//    // ────── Control Area ──────
//    auto area = content.reduced(6);
//
//    auto layoutRow = [&](CustomSlider& s1, juce::Label& l1, CustomSlider& s2, juce::Label& l2)
//        {
//            auto row = area.removeFromTop(controlHeight);
//            auto half = row.removeFromLeft(row.getWidth() / 2);
//            l1.setBounds(half.removeFromLeft(labelWidth));
//            s1.setBounds(half.reduced(2, 0));
//            l2.setBounds(row.removeFromLeft(labelWidth));
//            s2.setBounds(row.reduced(2, 0));
//            area.removeFromTop(controlSpacing);
//        };
//
//    // Gain
//    layoutRow(inputGainSlider, inputGainLabel, outputGainSlider, outputGainLabel);
//
//    // Filter knobs
//    auto filterArea = area.removeFromTop(rotaryKnobSize + 10);
//    filterCutoffSlider.setBounds(filterArea.removeFromLeft(filterArea.getWidth() / 2).reduced(8));
//    filterResonanceSlider.setBounds(filterArea.reduced(8));
//    area.removeFromTop(controlSpacing);
//
//    // Distortion knobs
//    auto distRow1 = area.removeFromTop(rotaryKnobSize + 10);
//    driveSlider.setBounds(distRow1.removeFromLeft(distRow1.getWidth() / 2).reduced(8));
//    mixSlider.setBounds(distRow1.reduced(8));
//    area.removeFromTop(controlSpacing);
//
//    auto distRow2 = area.removeFromTop(rotaryKnobSize + 10);
//    bitDepthSlider.setBounds(distRow2.removeFromLeft(distRow2.getWidth() / 2).reduced(8));
//    sampleRateSlider.setBounds(distRow2.reduced(8));
//    area.removeFromTop(controlSpacing);
//
//    // Filter dropdowns
//    auto dropdownRow = area.removeFromTop(comboBoxHeight + 4);
//    filterTypeComboBox.setBounds(dropdownRow.removeFromLeft(dropdownRow.getWidth() / 2).reduced(4));
//    filterRoutingComboBox.setBounds(dropdownRow.reduced(4));
//    area.removeFromTop(controlSpacing);
//
//    // Distortion type and oversampling
//    auto dropdownRow2 = area.removeFromTop(comboBoxHeight + 4);
//    distortionTypeComboBox.setBounds(dropdownRow2.removeFromLeft(dropdownRow2.getWidth() / 2).reduced(4));
//    oversamplingComboBox.setBounds(dropdownRow2.reduced(4));
//    area.removeFromTop(controlSpacing);
//
//    // Limiter controls
//    layoutRow(limiterThresholdSlider, limiterThresholdLabel, limiterReleaseSlider, limiterReleaseLabel);
//
//    // Limiter toggle and clip reset
//    auto buttonRow = area.removeFromTop(controlHeight);
//    limiterEnabledButton.setBounds(buttonRow.removeFromLeft(100).reduced(2));
//    resetClipButton.setBounds(buttonRow.removeFromLeft(120).reduced(2));
//}
void NaniDistortionAudioProcessorEditor::resized()
{
    auto b = getLocalBounds();

    // Scale to keep the 600×600 artwork’s aspect ratio
    const int designW = 600, designH = 600;
    float sx = b.getWidth() / (float)designW;
    float sy = b.getHeight() / (float)designH;
    float s = std::min(sx, sy);
    int w = int(std::round(designW * s));
    int h = int(std::round(designH * s));
    juce::Rectangle<int> ia{ (b.getWidth() - w) / 2, (b.getHeight() - h) / 2, w, h };

    auto map = [&](float x, float y, float w, float h)
        {
            int X = ia.getX() + int(std::round(x * s));
            int Y = ia.getY() + int(std::round(y * s));
            int W = int(std::round(w * s));
            int H = int(std::round(h * s));
            return juce::Rectangle<int>(X, Y, W, H);
        };

    // ─── Preset bar (top-left) ───
    // Use unique names so we don't shadow the inner-area x/y/w/h above.
    const int presetX = 12;   // left margin in design px
    const int presetY = 12;   // top margin in design px
    const int presetH = 22;   // control height
    const int presetGap = 6;  // gap between controls

    int cursorX = presetX;

    presetComboBox.setBounds(map(cursorX, presetY, 160, presetH)); cursorX += 160 + presetGap;
    presetNameEditor.setBounds(map(cursorX, presetY, 140, presetH)); cursorX += 140 + presetGap;
    savePresetButton.setBounds(map(cursorX, presetY, 40, presetH)); cursorX += 40 + presetGap;
    deletePresetButton.setBounds(map(cursorX, presetY, 40, presetH));

    const int k = 43; // knob size in the design
  

    // ─── 14 KNOBS (from the PNG) ───
    // TOP ROW
    inputGainSlider.setBounds(map(32, 138, k, k));  // INPUT
    outputGainSlider.setBounds(map(526, 138, k, k));  // OUTPUT

    filterCutoffSlider.setBounds(map(153, 137, k, k));  // CUTOFF
    filterTypeKnob.setBounds(map(279, 138, k, k));  // FILTER TYPE (LP/BP/HP)
    filterResonanceSlider.setBounds(map(405, 138, k, k));  // RESONANCE

    driveSlider.setBounds(map(151, 269, k, k));  // DRIVE
    distortionTypeKnob.setBounds(map(279, 267, k, k));  // TYPE (A–D)
    bitDepthSlider.setBounds(map(403, 268, k, k));  // BIT DEPTH

    sampleRateSlider.setBounds(map(218, 369, k, k));  // SAMPLE RATE
    mixSlider.setBounds(map(339, 369, k, k));  // MIX

    limiterThresholdSlider.setBounds(map(148, 500, k, k));  // THRESHOLD
    limiterReleaseSlider.setBounds(map(280, 500, k, k));  // RELEASE
    oversamplingKnob.setBounds(map(403, 500, k, k));  // OVERSAMPLING
    stereoWidthSlider.setBounds(map(522, 500, k, k));  // STEREOWIDTH

    // ─── METERS ─── (2× vertical bars each side)
    const int meterW = 8, meterH = 147, meterSpacing = 33;
    // Left meters under “INPUT METER”
    inputLevelMeterL.setBounds(map(29, 222, meterW, meterH));
    inputLevelMeterR.setBounds(map(29 + meterW + meterSpacing, 222, meterW, meterH));
    // Right meters under “OUTPUT METER”
    outputLevelMeterL.setBounds(map(523, 222, meterW, meterH));
    outputLevelMeterR.setBounds(map(523 + meterW + meterSpacing, 222, meterW, meterH));

    // Hide the text labels for meters (art already labels them)
    inputMeterLabel.setVisible(false);
    outputMeterLabel.setVisible(false);

    // ─── BYPASS LED/SWITCH (top-left) & LIMITER (bottom-left) ───
    bypassButton.setButtonText({});
    bypassButton.setBounds(map(45, 77, 60, 24));

    limiterEnabledButton.setButtonText({});
    limiterEnabledButton.setBounds(map(45, 523, 54, 40));

    // Clip reset (kept small, off to the side)
    resetClipButton.setBounds(map(28, 581, 80, 22));
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

juce::Image NaniDistortionAudioProcessorEditor::getSpriteFor(const juce::Slider& s) const
{
    // Per-knob overrides
    if (s.getComponentID() == "filterType")   return spriteFilterType;
    if (s.getComponentID() == "oversampling") return spriteOversampling;

    // Default for everyone else
    return spriteDefault.isValid() ? spriteDefault : knobSpriteStrip; // fallback to your original
}

int NaniDistortionAudioProcessorEditor::getSpriteFramesFor(const juce::Slider& s) const
{
    if (s.getComponentID() == "filterType")   return spriteFilterTypeFrames;
    if (s.getComponentID() == "oversampling") return spriteOversamplingFrames;

    return spriteDefault.isValid() ? spriteDefaultFrames : 64; // sensible fallback
}

