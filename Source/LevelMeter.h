// LevelMeter.h
#pragma once

#include <JuceHeader.h>

class LevelMeter : public juce::Component, private juce::Timer
{
public:
    LevelMeter() : juce::Component()
    {
        startTimerHz(60); // Update at 60 fps
    }

    ~LevelMeter() override
    {
        stopTimer();
    }

    void setLevel(float newLevel)
    {
        // Check for clipping
        if (newLevel >= 1.0f)
        {
            isClipping = true;
            clipTimer = clipHoldTime;
        }

        // Convert to dB for better visual representation
        targetLevel = juce::Decibels::gainToDecibels(newLevel, -60.0f);
        // Normalize to 0.0 - 1.0 range
        targetLevel = juce::jmap(targetLevel, -60.0f, 6.0f, 0.0f, 1.0f);
        targetLevel = juce::jlimit(0.0f, 1.0f, targetLevel);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillRect(bounds);

        // Calculate meter height based on level
        float meterHeight = bounds.getHeight() * level;

        // Draw the level meter
        juce::ColourGradient gradient(
            juce::Colours::green,
            bounds.getBottomLeft(),
            juce::Colours::red,
            bounds.getTopLeft(),
            false
        );
        gradient.addColour(0.7, juce::Colours::yellow);

        g.setGradientFill(gradient);
        g.fillRect(bounds.withTrimmedTop(bounds.getHeight() - meterHeight));

        // Draw tick marks
        g.setColour(juce::Colours::white.withAlpha(0.5f));

        // 0 dB mark (near the top)
        float zeroDbY = juce::jmap(juce::Decibels::gainToDecibels(1.0f, -60.0f),
            -60.0f, 6.0f,
            bounds.getBottom(), bounds.getY());
        g.drawLine(bounds.getX(), zeroDbY, bounds.getRight(), zeroDbY, 1.0f);

        // -12 dB mark
        float minus12DbY = juce::jmap(juce::Decibels::gainToDecibels(0.25f, -60.0f),
            -60.0f, 6.0f,
            bounds.getBottom(), bounds.getY());
        g.drawLine(bounds.getX(), minus12DbY, bounds.getRight(), minus12DbY, 1.0f);

        // -24 dB mark
        float minus24DbY = juce::jmap(juce::Decibels::gainToDecibels(0.0625f, -60.0f),
            -60.0f, 6.0f,
            bounds.getBottom(), bounds.getY());
        g.drawLine(bounds.getX(), minus24DbY, bounds.getRight(), minus24DbY, 1.0f);

        // Draw clipping indicator at the bottom of the meter
        auto clipRect = bounds.removeFromBottom(clipIndicatorHeight);

        if (isClipping)
            g.setColour(juce::Colours::red);
        else
            g.setColour(juce::Colours::darkgrey);

        g.fillRect(clipRect);

        // Draw border
        g.setColour(juce::Colours::white);
        g.drawRect(bounds.withHeight(bounds.getHeight() + clipIndicatorHeight), 1.0f);
    }

    void resized() override
    {
        // Nothing to do here
    }

    void resetClipping()
    {
        isClipping = false;
    }

private:
    float level = 0.0f;
    float targetLevel = 0.0f;
    float attackRate = 0.9f;    // How quickly the meter rises (smaller = faster)
    float releaseRate = 0.75f;  // How quickly the meter falls (smaller = faster)

    // Clipping indicator
    bool isClipping = false;
    int clipTimer = 0;
    const int clipHoldTime = 120; // Hold the clip indicator for 2 seconds (at 60fps)
    const int clipIndicatorHeight = 10; // Height of the clip indicator in pixels

    void timerCallback() override
    {
        // Fast attack, slower release for natural meter behavior
        if (targetLevel > level)
            level = level * attackRate + targetLevel * (1.0f - attackRate); // Rise quickly
        else
            level = level * releaseRate + targetLevel * (1.0f - releaseRate); // Fall more slowly

        // Update clip timer
        if (isClipping && clipTimer > 0)
        {
            clipTimer--;
            if (clipTimer <= 0)
                isClipping = false;
        }

        repaint();
    }
};