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

}

NaniDistortionAudioProcessorEditor::~NaniDistortionAudioProcessorEditor() 
{
    stopTimer();
}

void NaniDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(35, 35, 39));

    g.setColour(juce::Colours::white);
    g.setFont(20.0f);

    auto bounds = getLocalBounds();
    auto titleArea = bounds.removeFromTop(40);
    // Remove space for bypass button from title area
    titleArea.removeFromLeft(100);

    g.drawFittedText("Nani Distortion", titleArea, juce::Justification::centred, 1);

    // Draw separator lines
    g.setColour(juce::Colours::darkgrey);

    // First separator (after filter knobs)
    auto tempBounds = bounds;
    tempBounds.removeFromTop(150);
    g.fillRect(tempBounds.removeFromTop(2).toFloat().reduced(20, 0));

    // Second separator (after sliders)
    tempBounds.removeFromTop(180);
    g.fillRect(tempBounds.removeFromTop(2).toFloat().reduced(20, 0));

    // Third separator (after preset controls, before limiter)
    tempBounds.removeFromTop(80); // Adjust this value based on your layout
    g.fillRect(tempBounds.removeFromTop(2).toFloat().reduced(20, 0));

    // Add a section title for the gain controls
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);

   // auto tempBounds = getLocalBounds();
    tempBounds.removeFromTop(50); // Position after the main title
    auto gainTitleArea = tempBounds.removeFromTop(30);
    g.drawText("Gain Controls", gainTitleArea, juce::Justification::centred, true);
}

void NaniDistortionAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(50); // Space for title

    // Position the bypass button in the top left
    auto topArea = bounds.removeFromTop(50); // Space for title and bypass button
    auto bypassArea = topArea.removeFromLeft(100).reduced(10);
    bypassButton.setBounds(bypassArea);

    // Reserve space for meters on left and right
    const int meterWidth = 20;
    const int meterSpacing = 5;
    const int meterLabelHeight = 20;

    // Left side meters (input)
    auto leftMeterArea = bounds.removeFromLeft(meterWidth * 2 + meterSpacing + 10);
    leftMeterArea.removeFromTop(50); // Space for title

    auto inputLabelArea = leftMeterArea.removeFromTop(meterLabelHeight);
    inputMeterLabel.setBounds(inputLabelArea);

    auto inputMeterAreaL = leftMeterArea.removeFromLeft(meterWidth);
    inputLevelMeterL.setBounds(inputMeterAreaL);

    leftMeterArea.removeFromLeft(meterSpacing);

    auto inputMeterAreaR = leftMeterArea.removeFromLeft(meterWidth);
    inputLevelMeterR.setBounds(inputMeterAreaR);

    // Right side meters (output)
    auto rightMeterArea = bounds.removeFromRight(meterWidth * 2 + meterSpacing + 10);
    rightMeterArea.removeFromTop(50); // Space for title

    auto outputLabelArea = rightMeterArea.removeFromTop(meterLabelHeight);
    outputMeterLabel.setBounds(outputLabelArea);

    auto outputMeterAreaL = rightMeterArea.removeFromLeft(meterWidth);
    outputLevelMeterL.setBounds(outputMeterAreaL);

    rightMeterArea.removeFromLeft(meterSpacing);

    auto outputMeterAreaR = rightMeterArea.removeFromLeft(meterWidth);
    outputLevelMeterR.setBounds(outputMeterAreaR);

    // Position the reset clip button
    auto resetButtonArea = bounds.removeFromTop(30);
    int buttonWidth = 100;
    int buttonHeight = 25;
    int buttonX = resetButtonArea.getCentreX() - buttonWidth / 2;
    int buttonY = resetButtonArea.getCentreY() - buttonHeight / 2;
    resetClipButton.setBounds(buttonX, buttonY, buttonWidth, buttonHeight);

    // Define the createSliderLayout lambda once at the beginning
    const int labelWidth = 100;
    const int sliderHeight = 40;
    auto createSliderLayout = [&](juce::Slider& slider, juce::Label& label)
        {
            auto sliderArea = bounds.removeFromTop(sliderHeight);
            label.setBounds(sliderArea.removeFromLeft(labelWidth).reduced(5, 0));
            slider.setBounds(sliderArea.reduced(5, 0));
        };

    // Input and Output Gain controls (at the top of the UI)
    createSliderLayout(inputGainSlider, inputGainLabel);
    createSliderLayout(outputGainSlider, outputGainLabel);

    // Add a spacer before the filter knobs
    bounds.removeFromTop(10);

    // Top area for the big filter knobs
    auto filterKnobArea = bounds.removeFromTop(140);
    filterCutoffSlider.setBounds(filterKnobArea.removeFromLeft(getWidth() / 2).reduced(20));
    filterResonanceSlider.setBounds(filterKnobArea.reduced(20));

    bounds.removeFromTop(20); // Spacer before sliders

    // Middle area for the four linear sliders
    createSliderLayout(driveSlider, driveLabel);
    createSliderLayout(bitDepthSlider, bitDepthLabel);
    createSliderLayout(sampleRateSlider, sampleRateLabel);
    createSliderLayout(mixSlider, mixLabel);

    bounds.removeFromTop(20); // Spacer before combo boxes

    // ComboBoxes layout
    const auto comboBoxHeight = 25;
    const auto comboBoxMargin = 10;

    auto createComboBoxLayout = [&](juce::ComboBox& comboBox)
        {
            comboBox.setBounds(bounds.removeFromTop(comboBoxHeight).reduced(getWidth() * 0.15, 0));
            bounds.removeFromTop(comboBoxMargin);
        };

    createComboBoxLayout(distortionTypeComboBox);
    createComboBoxLayout(filterTypeComboBox);
    createComboBoxLayout(filterRoutingComboBox);
    createComboBoxLayout(oversamplingComboBox);

    // Preset controls
    bounds.removeFromTop(20); // Add more space before preset controls

    const int presetControlHeight = 25;
    const int presetControlSpacing = 5;

    auto presetArea = bounds.removeFromTop(presetControlHeight + presetControlSpacing * 2);
    presetArea.reduce(10, 0);

    auto presetComboBoxArea = presetArea.removeFromLeft(presetArea.getWidth() * 0.6f);
    presetComboBox.setBounds(presetComboBoxArea.reduced(0, presetControlSpacing));

    auto presetNameEditorArea = presetArea.removeFromLeft(presetArea.getWidth() * 0.5f);
    presetNameEditor.setBounds(presetNameEditorArea.reduced(presetControlSpacing, presetControlSpacing));

    auto buttonsArea = presetArea;
    savePresetButton.setBounds(buttonsArea.removeFromLeft(buttonsArea.getWidth() * 0.5f).reduced(presetControlSpacing));
    deletePresetButton.setBounds(buttonsArea.reduced(presetControlSpacing));

    // Add more space after preset controls (before the separator)
    bounds.removeFromTop(20);

    // Limiter section (after the separator)
    bounds.removeFromTop(20); // Add space after the separator, before limiter controls

    // Limiter toggle button
    auto limiterHeaderArea = bounds.removeFromTop(30);
    limiterEnabledButton.setBounds(limiterHeaderArea.reduced(10, 0));

    // Limiter sliders
    createSliderLayout(limiterThresholdSlider, limiterThresholdLabel);
    createSliderLayout(limiterReleaseSlider, limiterReleaseLabel);
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

void NaniDistortionAudioProcessorEditor::timerCallback()
{
    // Update the level meters
    inputLevelMeterL.setLevel(processor.getInputLevel(0));
    inputLevelMeterR.setLevel(processor.getInputLevel(1));
    outputLevelMeterL.setLevel(processor.getOutputLevel(0));
    outputLevelMeterR.setLevel(processor.getOutputLevel(1));
}