// LevelMeter.h
#pragma once

#include <JuceHeader.h>

class LevelMeter : public juce::Component, private juce::Timer
{
public:
    LevelMeter() : juce::Component()
    {
        //startTimerHz(60); // Update at 60 fps
        startTimerHz(30);
    }

    ~LevelMeter() override
    {
        stopTimer();
    }

    // NEW: call this once from the editor to use a filmstrip
    // frames = number of frames in the strip (14 for: off + 13 LEDs)
    // frame0AtTop = true if frame #0 is at the TOP of the image
    // numLights = how many LED steps (13)
    void setSpriteStrip(juce::Image img, int frames, bool frame0AtTop = true, int numLights = 13)
    {
        filmstrip = img;
        filmstripFrames = juce::jmax(0, frames);
        frameZeroAtTop = frame0AtTop;
        lights = juce::jmax(1, numLights);
        repaint();
    }

    // unchanged API used by your editor timer
    void setLevel(float newLevel)
    {
        // clipping latch
        if (newLevel >= 1.0f) { isClipping = true; clipTimer = clipHoldTime; }

        // map linear gain → dB → normalized 0..1 (same as before)
        targetLevel = juce::Decibels::gainToDecibels(newLevel, -60.0f);
        // Normalize -60..0 dB to 0..1
        targetLevel = juce::jmap(targetLevel, -60.0f, 0.0f, 0.0f, 1.0f);
        targetLevel = juce::jlimit(0.0f, 1.0f, targetLevel);
    }

	// Override paint method to draw the sprite strip or gradient bar.
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Split an area for the clip indicator at the bottom
        const auto meterArea = bounds.withTrimmedBottom(clipIndicatorHeight);
        const auto clipRect = bounds.removeFromBottom((float)clipIndicatorHeight);

        // If we have a valid strip, draw that. Otherwise, fall back to your gradient.
        if (filmstrip.isValid() && filmstripFrames > 0)
        {
            // pick a frame from 0..lights (14 frames total: 0=off, 13=all lit)
            const int frameFromLevel = juce::jlimit(0, lights, (int)std::round(level * lights));
            int frameIndex = frameFromLevel; // 0..13
            if (!frameZeroAtTop) frameIndex = (filmstripFrames - 1) - frameIndex;

            const int frameH = filmstrip.getHeight() / filmstripFrames;
            const int frameW = filmstrip.getWidth();

            g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
            g.drawImage(filmstrip,
                (int)meterArea.getX(), (int)meterArea.getY(),
                (int)meterArea.getWidth(), (int)meterArea.getHeight(),
                0, frameIndex * frameH, frameW, frameH);
        }
        else
        {
            // Fallback: your original gradient bar
            g.setColour(juce::Colours::black.withAlpha(0.5f));
            g.fillRect(meterArea);

            const float meterHeight = meterArea.getHeight() * level;

            juce::ColourGradient gradient(juce::Colours::green, meterArea.getBottomLeft(),
                juce::Colours::red, meterArea.getTopLeft(), false);
            gradient.addColour(0.7, juce::Colours::yellow);

            g.setGradientFill(gradient);
            g.fillRect(meterArea.withTrimmedTop(meterArea.getHeight() - meterHeight));

            // Optional: keep your tick marks in fallback mode (omitted here for brevity)
        }

        // Clip indicator
        g.setColour(isClipping ? juce::Colours::red : juce::Colours::darkgrey);
        g.fillRect(clipRect);

        //// thin border (optional)
        //g.setColour(juce::Colours::white.withAlpha(0.8f));
        //g.drawRect(getLocalBounds());
    }

	//// Override paint method to draw the level meter (OLD)
 //   void paint(juce::Graphics& g) override
 //   {
 //       auto bounds = getLocalBounds().toFloat();

 //       // Background
 //       g.setColour(juce::Colours::black.withAlpha(0.5f));
 //       g.fillRect(bounds);

 //       // Calculate meter height based on level
 //       float meterHeight = bounds.getHeight() * level;

 //       // Draw the level meter
 //       juce::ColourGradient gradient(
 //           juce::Colours::green,
 //           bounds.getBottomLeft(),
 //           juce::Colours::red,
 //           bounds.getTopLeft(),
 //           false
 //       );
 //       gradient.addColour(0.7, juce::Colours::yellow);

 //       g.setGradientFill(gradient);
 //       g.fillRect(bounds.withTrimmedTop(bounds.getHeight() - meterHeight));

 //       // Draw tick marks
 //       g.setColour(juce::Colours::white.withAlpha(0.5f));

 //       // 0 dB mark (near the top)
 //       float zeroDbY = juce::jmap(juce::Decibels::gainToDecibels(1.0f, -60.0f),
 //           -60.0f, 6.0f,
 //           bounds.getBottom(), bounds.getY());
 //       g.drawLine(bounds.getX(), zeroDbY, bounds.getRight(), zeroDbY, 1.0f);

 //       // -12 dB mark
 //       float minus12DbY = juce::jmap(juce::Decibels::gainToDecibels(0.25f, -60.0f),
 //           -60.0f, 6.0f,
 //           bounds.getBottom(), bounds.getY());
 //       g.drawLine(bounds.getX(), minus12DbY, bounds.getRight(), minus12DbY, 1.0f);

 //       // -24 dB mark
 //       float minus24DbY = juce::jmap(juce::Decibels::gainToDecibels(0.0625f, -60.0f),
 //           -60.0f, 6.0f,
 //           bounds.getBottom(), bounds.getY());
 //       g.drawLine(bounds.getX(), minus24DbY, bounds.getRight(), minus24DbY, 1.0f);

 //       // Draw clipping indicator at the bottom of the meter
 //       auto clipRect = bounds.removeFromBottom(clipIndicatorHeight);

 //       if (isClipping)
 //           g.setColour(juce::Colours::red);
 //       else
 //           g.setColour(juce::Colours::darkgrey);

 //       g.fillRect(clipRect);

 //       // Draw border
 //       g.setColour(juce::Colours::white);
 //       g.drawRect(bounds.withHeight(bounds.getHeight() + clipIndicatorHeight), 1.0f);
 //   }



    void resized() override
    {
        // Nothing to do here
    }

    void resetClipping()
    {
        isClipping = false;
    }

private:
    // smoothed meter state
    float level = 0.0f;
    float targetLevel = 0.0f;
    float attackRate = 0.9f;    // smaller = faster rise
    float releaseRate = 0.75f;  // smaller = faster fall

    // sprite filmstrip
    juce::Image filmstrip;
    int  filmstripFrames = 0;   // e.g. 14
    int  lights = 13;           // LEDs (frame count - 1)
    bool frameZeroAtTop = true; // adjust if your strip is inverted

    // clipping latch
    bool isClipping = false;
    int  clipTimer = 0;
    const int clipHoldTime = 120;      // ~2s at 60fps
    const int clipIndicatorHeight = 10;

    void timerCallback() override
    {
        // fast attack, slower release
        level = (targetLevel > level)
            ? level * attackRate + targetLevel * (1.0f - attackRate)
            : level * releaseRate + targetLevel * (1.0f - releaseRate);

        if (isClipping && clipTimer > 0)
        {
            if (--clipTimer <= 0) isClipping = false;
        }
        repaint();
    }
};