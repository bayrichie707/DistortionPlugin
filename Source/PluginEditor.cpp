#include "PluginProcessor.h"
#include "PluginEditor.h"

NaniDistortionAudioProcessorEditor::NaniDistortionAudioProcessorEditor(NaniDistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
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
    
    // --- Filter Components (Rotary) ---
    addAndMakeVisible(filterCutoffSlider);
    filterCutoffSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterCutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    filterCutoffAttachment = std::make_unique<SliderAttachment>(vts, "filterCutoff", filterCutoffSlider);

    addAndMakeVisible(filterCutoffLabel);
    filterCutoffLabel.setText("Cutoff", juce::dontSendNotification);
    filterCutoffLabel.setJustificationType(juce::Justification::centred);
    filterCutoffLabel.attachToComponent(&filterCutoffSlider, false);

    addAndMakeVisible(filterResonanceSlider);
    filterResonanceSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterResonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    filterResonanceAttachment = std::make_unique<SliderAttachment>(vts, "filterResonance", filterResonanceSlider);

    addAndMakeVisible(filterResonanceLabel);
    filterResonanceLabel.setText("Resonance", juce::dontSendNotification);
    filterResonanceLabel.setJustificationType(juce::Justification::centred);
    filterResonanceLabel.attachToComponent(&filterResonanceSlider, false);

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
    
    setSize(500, 420);
    
    // <<< ADD THE NEW DISTORTION TYPE COMBOBOX
    addAndMakeVisible(distortionTypeComboBox);
    distortionTypeComboBox.addItemList({ "Soft Clip", "Hard Clip", "Foldback", "Bit Glitch" }, 1);
    distortionTypeAttachment = std::make_unique<ComboBoxAttachment>(vts, "distortionType", distortionTypeComboBox);
    addAndMakeVisible(distortionTypeLabel);
    distortionTypeLabel.setText("Distortion Mode", juce::dontSendNotification);
    distortionTypeLabel.attachToComponent(&distortionTypeComboBox, true);

    setSize(500, 500); // <<< Increase height slightly for the new control

    // Add this to your constructor in PluginEditor.cpp:
    addAndMakeVisible(oversamplingComboBox);
    oversamplingComboBox.addItemList({ "Off", "2x", "4x", "8x", "16x" }, 1);
    oversamplingAttachment = std::make_unique<ComboBoxAttachment>(vts, "oversamplingFactor", oversamplingComboBox);
    addAndMakeVisible(oversamplingLabel);
    oversamplingLabel.setText("Oversampling", juce::dontSendNotification);
    oversamplingLabel.attachToComponent(&oversamplingComboBox, true);

    // Increase the window size to accommodate the new control
    setSize(500, 530);

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

    // In your constructor, update the size:
    setSize(500, 560); // Increase height for the preset controls

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

    // Increase window size to accommodate limiter controls
    setSize(500, 700);

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

    // Increase window size to accommodate gain controls
    setSize(500, 820);

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

    // Adjust window size to accommodate meters
    setSize(600, 820); // Wider to fit meters on sides

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
    setSize(600, 760); 
}

NaniDistortionAudioProcessorEditor::~NaniDistortionAudioProcessorEditor() 
{
    stopTimer();
}

void NaniDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(35, 35, 39));

    // Draw title
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);

    auto bounds = getLocalBounds();
    auto headerSection = bounds.removeFromTop(70); // Match the increased height
    headerSection.removeFromLeft(120); // Space for bypass button
    auto titleArea = headerSection.removeFromLeft(headerSection.getWidth() - 140); // Adjusted width
    g.drawFittedText("Nani Distortion", titleArea, juce::Justification::centred, 1);

    // Rest of your paint code...
    // Draw section dividers and titles
    g.setColour(juce::Colours::darkgrey);

    // Helper function to draw a section divider with title
    auto drawSectionDivider = [&](int yPosition, const juce::String& title) {
        auto dividerBounds = getLocalBounds().withY(yPosition).withHeight(2);
        g.fillRect(dividerBounds.toFloat().reduced(20, 0));

        if (title.isNotEmpty())
        {
            g.setColour(juce::Colours::white);
            g.setFont(16.0f);
            auto titleBounds = getLocalBounds().withY(yPosition - 20).withHeight(20);
            g.drawText(title, titleBounds, juce::Justification::centred, false);
            g.setColour(juce::Colours::darkgrey);
        }
        };

    // Draw dividers at key positions (adjusted for the taller header)
    drawSectionDivider(130, "Gain");
    drawSectionDivider(250, "Filter");
    drawSectionDivider(370, "Distortion");
    drawSectionDivider(510, "Presets");
    drawSectionDivider(570, "");
    drawSectionDivider(660, "Limiter");
}

void NaniDistortionAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Constants for layout
    const int labelWidth = 100;
    const int sliderHeight = 40;
    const int sectionSpacing = 15;
    const int meterWidth = 20;
    const int meterSpacing = 5;

    // ===== HEADER SECTION =====
    auto headerSection = bounds.removeFromTop(70); // Increased height for header

    // Bypass button (top left)
    auto bypassArea = headerSection.removeFromLeft(120).reduced(10);
    bypassButton.setBounds(bypassArea);

    // Title (center)
    auto titleArea = headerSection.removeFromLeft(headerSection.getWidth() - 140); // Adjusted width

    // Stereo width (top right) - increased area and adjusted position
    auto stereoWidthArea = headerSection.removeFromRight(140); // Increased width

    // Position the stereo width control with more space
    stereoWidthSlider.setBounds(stereoWidthArea.reduced(10));

    // Make sure the label is positioned correctly
    stereoWidthLabel.setBounds(stereoWidthArea.removeFromTop(20));

    // Rest of your layout code remains the same...
    // ===== METERS SECTION =====
    // We'll place meters on the sides of the entire remaining area
    auto mainContentWithMeters = bounds;

    // Left side meters (input)
    auto leftMeterArea = mainContentWithMeters.removeFromLeft(meterWidth * 2 + meterSpacing + 10);
    auto inputLabelArea = leftMeterArea.removeFromTop(20);
    inputMeterLabel.setBounds(inputLabelArea);

    auto inputMeterAreaL = leftMeterArea.removeFromLeft(meterWidth);
    inputLevelMeterL.setBounds(inputMeterAreaL);

    leftMeterArea.removeFromLeft(meterSpacing);

    auto inputMeterAreaR = leftMeterArea.removeFromLeft(meterWidth);
    inputLevelMeterR.setBounds(inputMeterAreaR);

    // Right side meters (output)
    auto rightMeterArea = mainContentWithMeters.removeFromRight(meterWidth * 2 + meterSpacing + 10);
    auto outputLabelArea = rightMeterArea.removeFromTop(20);
    outputMeterLabel.setBounds(outputLabelArea);

    auto outputMeterAreaL = rightMeterArea.removeFromLeft(meterWidth);
    outputLevelMeterL.setBounds(outputMeterAreaL);

    rightMeterArea.removeFromLeft(meterSpacing);

    auto outputMeterAreaR = rightMeterArea.removeFromLeft(meterWidth);
    outputLevelMeterR.setBounds(outputMeterAreaR);

    // Main content area (between meters)
    auto mainContent = mainContentWithMeters;

    // Helper lambda for creating slider layouts
    auto createSliderLayout = [&](CustomSlider& slider, juce::Label& label)
        {
            auto sliderArea = mainContent.removeFromTop(sliderHeight);
            label.setBounds(sliderArea.removeFromLeft(labelWidth).reduced(5, 0));
            slider.setBounds(sliderArea.reduced(5, 0));
        };

    // ===== GAIN SECTION =====
    mainContent.removeFromTop(sectionSpacing);
    createSliderLayout(inputGainSlider, inputGainLabel);
    createSliderLayout(outputGainSlider, outputGainLabel);

    // ===== FILTER SECTION =====
    mainContent.removeFromTop(sectionSpacing);
    auto filterKnobArea = mainContent.removeFromTop(100);
    filterCutoffSlider.setBounds(filterKnobArea.removeFromLeft(filterKnobArea.getWidth() / 2).reduced(15));
    filterResonanceSlider.setBounds(filterKnobArea.reduced(15));

    // ===== DISTORTION SECTION =====
    mainContent.removeFromTop(sectionSpacing);
    createSliderLayout(driveSlider, driveLabel);
    createSliderLayout(bitDepthSlider, bitDepthLabel);
    createSliderLayout(sampleRateSlider, sampleRateLabel);
    createSliderLayout(mixSlider, mixLabel);

    // ===== COMBO BOXES SECTION =====
    mainContent.removeFromTop(sectionSpacing);
    const auto comboBoxHeight = 25;
    const auto comboBoxMargin = 5;

    auto createComboBoxLayout = [&](juce::ComboBox& comboBox)
        {
            comboBox.setBounds(mainContent.removeFromTop(comboBoxHeight).reduced(mainContent.getWidth() * 0.15, 0));
            mainContent.removeFromTop(comboBoxMargin);
        };

    createComboBoxLayout(distortionTypeComboBox);
    createComboBoxLayout(filterTypeComboBox);
    createComboBoxLayout(filterRoutingComboBox);
    createComboBoxLayout(oversamplingComboBox);

    // ===== PRESET SECTION =====
    mainContent.removeFromTop(sectionSpacing);
    const int presetControlHeight = 25;
    const int presetControlSpacing = 5;

    // Make preset controls more compact by placing them side by side
    auto presetArea = mainContent.removeFromTop(presetControlHeight + presetControlSpacing);
    presetArea.reduce(10, 0);

    // Preset combo box (left)
    auto presetComboBoxArea = presetArea.removeFromLeft(presetArea.getWidth() * 0.4f);
    presetComboBox.setBounds(presetComboBoxArea.reduced(presetControlSpacing));

    // Preset name editor (center)
    auto presetNameEditorArea = presetArea.removeFromLeft(presetArea.getWidth() * 0.4f);
    presetNameEditor.setBounds(presetNameEditorArea.reduced(presetControlSpacing));

    // Save and delete buttons (right)
    auto saveButtonArea = presetArea.removeFromLeft(presetArea.getWidth() * 0.5f);
    savePresetButton.setBounds(saveButtonArea.reduced(presetControlSpacing));
    deletePresetButton.setBounds(presetArea.reduced(presetControlSpacing));

    // ===== RESET CLIP BUTTON =====
    mainContent.removeFromTop(sectionSpacing);
    auto resetButtonArea = mainContent.removeFromTop(30);
    int buttonWidth = 100;
    int buttonHeight = 25;
    int buttonX = resetButtonArea.getCentreX() - buttonWidth / 2;
    int buttonY = resetButtonArea.getCentreY() - buttonHeight / 2;
    resetClipButton.setBounds(buttonX, buttonY, buttonWidth, buttonHeight);

    // ===== LIMITER SECTION =====
    mainContent.removeFromTop(sectionSpacing);

    // Limiter toggle
    auto limiterHeaderArea = mainContent.removeFromTop(30);
    limiterEnabledButton.setBounds(limiterHeaderArea.reduced(10, 0));

    // Place limiter sliders side by side to save vertical space
    auto limiterSlidersArea = mainContent.removeFromTop(sliderHeight);
    auto limiterThresholdArea = limiterSlidersArea.removeFromLeft(limiterSlidersArea.getWidth() / 2);
    limiterThresholdLabel.setBounds(limiterThresholdArea.removeFromLeft(labelWidth).reduced(5, 0));
    limiterThresholdSlider.setBounds(limiterThresholdArea.reduced(5, 0));

    auto limiterReleaseArea = limiterSlidersArea;
    limiterReleaseLabel.setBounds(limiterReleaseArea.removeFromLeft(labelWidth).reduced(5, 0));
    limiterReleaseSlider.setBounds(limiterReleaseArea.reduced(5, 0));

    // Add some padding at the bottom
    mainContent.removeFromTop(20);
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

