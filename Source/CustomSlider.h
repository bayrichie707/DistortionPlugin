// CustomSlider.h
#pragma once

#include <JuceHeader.h>

class CustomSlider : public juce::Slider
{
public:
    void updateTextDisplay()
    {
        // Force a complete refresh of the text display
        setTextValueSuffix(""); // Clear any existing suffix
        updateText();           // Update the internal text
        repaint();              // Force a repaint
    }

    enum ValueDisplayMode
    {
        Default,
        Decibels,
        Percentage,
        Ratio,
        Hertz,
        Milliseconds,
        Samples,
        Times,
        Degrees
    };

    CustomSlider() : juce::Slider()
    {
        // Default settings
        setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    }

    void setValueDisplayMode(ValueDisplayMode newMode)
    {
        displayMode = newMode;
    }

    juce::String getTextFromValue(double value) override
    {
        switch (displayMode)
        {
        case Decibels:
            return juce::String(value, 1) + " dB";

        case Percentage:
            return juce::String(value * 100.0, 1) + " %";

        case Ratio:
            if (value >= 10.0)
                return juce::String(value, 1) + ":1";
            else
                return juce::String(value, 2) + ":1";

        case Hertz:
            if (value >= 1000.0)
                return juce::String(value / 1000.0, 2) + " kHz";
            else
                return juce::String(value, 0) + " Hz";

        case Milliseconds:
            return juce::String(value, 1) + " ms";

        case Samples:
            return juce::String((int)value) + " smp";

        case Times:
            return juce::String(value, 2) + "x";

        case Degrees:
            return juce::String(value, 0) + "°";

        case Default:
        default:
            return juce::Slider::getTextFromValue(value);
        }


    }

    double getValueFromText(const juce::String& text) override
    {
        // Strip any units from the text
        juce::String t = text.trim();

        if (t.endsWith(" dB") || t.endsWith("dB"))
            t = t.upToFirstOccurrenceOf(" dB", false, true).trim();
        else if (t.endsWith(" %") || t.endsWith("%"))
            t = t.upToFirstOccurrenceOf(" %", false, true).trim();
        else if (t.endsWith(":1"))
            t = t.upToFirstOccurrenceOf(":", false, true).trim();
        else if (t.endsWith(" kHz") || t.endsWith("kHz"))
            return t.upToFirstOccurrenceOf(" kHz", false, true).trim().getDoubleValue() * 1000.0;
        else if (t.endsWith(" Hz") || t.endsWith("Hz"))
            t = t.upToFirstOccurrenceOf(" Hz", false, true).trim();
        else if (t.endsWith(" ms") || t.endsWith("ms"))
            t = t.upToFirstOccurrenceOf(" ms", false, true).trim();
        else if (t.endsWith(" smp") || t.endsWith("smp"))
            t = t.upToFirstOccurrenceOf(" smp", false, true).trim();
        else if (t.endsWith("x"))
            t = t.upToFirstOccurrenceOf("x", false, true).trim();
        else if (t.endsWith("°"))
            t = t.upToFirstOccurrenceOf("°", false, true).trim();

        return t.getDoubleValue();
    }

private:
    ValueDisplayMode displayMode = Default;
};