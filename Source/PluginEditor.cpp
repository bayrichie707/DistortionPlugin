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

}

NaniDistortionAudioProcessorEditor::~NaniDistortionAudioProcessorEditor() {}

void NaniDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(35, 35, 39));

    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    
    auto bounds = getLocalBounds();
    auto titleArea = bounds.removeFromTop(40);
    g.drawFittedText("Nani Distortion", titleArea, juce::Justification::centred, 1);
    
    g.setColour(juce::Colours::darkgrey);
    bounds.removeFromTop(150);
    g.fillRect(bounds.removeFromTop(2).toFloat().reduced(20, 0));
    bounds.removeFromTop(180);
    g.fillRect(bounds.removeFromTop(2).toFloat().reduced(20, 0));
}

// <<< THIS FUNCTION HAS THE CRITICAL FIX
void NaniDistortionAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(50); // Space for title

    // Top area for the big filter knobs
    auto filterKnobArea = bounds.removeFromTop(140);
    filterCutoffSlider.setBounds(filterKnobArea.removeFromLeft(getWidth() / 2).reduced(20));
    filterResonanceSlider.setBounds(filterKnobArea.reduced(20));

    bounds.removeFromTop(20); // Spacer

    // Middle area for the four linear sliders
    const int labelWidth = 100;
    const int sliderHeight = 40;
    auto createSliderLayout = [&](juce::Slider& slider, juce::Label& label)
    {
        auto sliderArea = bounds.removeFromTop(sliderHeight);
        label.setBounds(sliderArea.removeFromLeft(labelWidth).reduced(5, 0));
        slider.setBounds(sliderArea.reduced(5, 0));
    };
    createSliderLayout(driveSlider, driveLabel);
    createSliderLayout(bitDepthSlider, bitDepthLabel);
    createSliderLayout(sampleRateSlider, sampleRateLabel);
    createSliderLayout(mixSlider, mixLabel);

    bounds.removeFromTop(20); // Spacer

    // --- CORRECTED LAYOUT FOR COMBOBOXES ---
    const auto comboBoxHeight = 25;
    const auto comboBoxMargin = 10;
    
    // Helper lambda to make the layout clear and reusable
    auto createComboBoxLayout = [&](juce::ComboBox& comboBox)
    {
        comboBox.setBounds(bounds.removeFromTop(comboBoxHeight).reduced(getWidth() * 0.15, 0));
        bounds.removeFromTop(comboBoxMargin); // Apply a consistent spacer
    };
    
    // Position them in the order they should appear visually
    createComboBoxLayout(distortionTypeComboBox);
    createComboBoxLayout(filterTypeComboBox);
    createComboBoxLayout(filterRoutingComboBox);
    // --- End of corrected layout ---
}