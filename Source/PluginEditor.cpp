#include "PluginProcessor.h"
#include "PluginEditor.h"

XYControlAudioProcessorEditor::XYControlAudioProcessorEditor(XYControlAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(368, 368);  // Match standalone app size
    addAndMakeVisible(xyControl);

    presetsFolder = NativeDialogs::getPresetsFolder();

    // Set initial position from parameters
    xyControl.setPosition(*audioProcessor.xParam, *audioProcessor.yParam);
    xyControl.setPreset(static_cast<XYControlComponent::Preset>((int)*audioProcessor.presetParam));

    startTimerHz(30);  // Update parameters regularly
}

XYControlAudioProcessorEditor::~XYControlAudioProcessorEditor()
{
}

void XYControlAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto preset = xyControl.getCurrentPreset();

    // Fill background with color matching the preset
    juce::Colour backgroundColor;
    juce::Colour borderColor;

    if (preset == XYControlComponent::Preset::Blue)
    {
        backgroundColor = juce::Colours::white;
        borderColor = juce::Colours::black;
    }
    else if (preset == XYControlComponent::Preset::Red)
    {
        backgroundColor = juce::Colour(0xFFFF0000);  // Red
        borderColor = juce::Colours::black;
    }
    else // Black
    {
        backgroundColor = juce::Colour(0xFF0A0A0A);  // Very dark gray instead of pure black
        borderColor = juce::Colours::white;
    }

    g.fillAll(backgroundColor);

    // Subtle drop shadow for depth (Apple-style)
    auto controlBounds = xyControl.getBounds().toFloat();
    float cornerRadius = 24.0f;

    juce::Path shadowPath;
    shadowPath.addRoundedRectangle(controlBounds, cornerRadius);

    // Use appropriate shadow based on preset - all use dark shadows for uniformity
    juce::Colour shadowColor;
    if (preset == XYControlComponent::Preset::Blue)
        shadowColor = juce::Colour(0x14000000);  // Subtle dark on white
    else if (preset == XYControlComponent::Preset::Red)
        shadowColor = juce::Colour(0x30000000);  // Darker on red
    else // Black
        shadowColor = juce::Colour(0x40000000);  // Dark shadow on very dark gray (barely visible but maintains uniformity)

    juce::DropShadow shadow(shadowColor, 18, juce::Point<int>(0, 4));
    shadow.drawForPath(g, shadowPath);

    // Draw blue progress ring during hold (stays glued to border)
    // Only show after brief delay to avoid flashing on quick double-clicks
    if (holdProgress > 0.07f)  // ~200ms delay before becoming visible
    {
        // Adjust progress to start from 0 after the delay
        float adjustedProgress = (holdProgress - 0.07f) / 0.93f;

        // Blue ring color with opacity based on progress
        g.setColour(juce::Colour(0xff007aff).withAlpha(0.3f + adjustedProgress * 0.7f));

        // Stroke width grows with progress - ring gets thicker but stays attached
        float strokeWidth = 2.0f + adjustedProgress * 10.0f;

        // Draw ring that stays glued to the XY border (no expansion)
        g.drawRoundedRectangle(controlBounds, cornerRadius, strokeWidth);
    }
}

void XYControlAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Add padding (26px on each side) for clickable area
    auto xySize = bounds.getWidth() - 52;  // 26px padding on each side
    xyControl.setBounds(bounds.withSizeKeepingCentre(xySize, xySize));
}

void XYControlAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    if (!xyControl.getBounds().contains(event.getPosition()))
    {
        isHoldingOutside = true;
        holdStartTime = juce::Time::currentTimeMillis();
        holdProgress = 0.0f;
        menuShown = false;
        startTimer(16);
    }
}

void XYControlAudioProcessorEditor::mouseUp(const juce::MouseEvent& event)
{
    isHoldingOutside = false;
    holdProgress = 0.0f;
    stopTimer();
    repaint();
}

void XYControlAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)
{
    if (!xyControl.getBounds().contains(event.getPosition()))
    {
        auto currentPreset = static_cast<int>(xyControl.getCurrentPreset());
        currentPreset = (currentPreset + 1) % 3;
        xyControl.setPreset(static_cast<XYControlComponent::Preset>(currentPreset));
        *audioProcessor.presetParam = currentPreset;
    }
}

void XYControlAudioProcessorEditor::timerCallback()
{
    // Update parameters from XY control
    updateParametersFromXY();

    // Handle hold progress
    if (isHoldingOutside && !menuShown)
    {
        int64_t currentTime = juce::Time::currentTimeMillis();
        int64_t holdDuration = currentTime - holdStartTime;

        holdProgress = juce::jmin(1.0f, holdDuration / 3000.0f);
        repaint();

        if (holdDuration >= 3000)
        {
            menuShown = true;
            holdProgress = 0.0f;
            stopTimer();
            repaint();
            showPresetOptions();
        }
    }
}

void XYControlAudioProcessorEditor::updateParametersFromXY()
{
    auto position = xyControl.getPosition();
    *audioProcessor.xParam = position.x;
    *audioProcessor.yParam = position.y;
    *audioProcessor.presetParam = static_cast<int>(xyControl.getCurrentPreset());
}

void XYControlAudioProcessorEditor::showPresetOptions()
{
    NativeDialogs::showPresetMenu([this](int result)
    {
        if (result == 1)
        {
            NativeDialogs::showSaveDialog(presetsFolder, [this](juce::File file)
            {
                if (file != juce::File())
                {
                    savePresetToFile(file);
                }
            });
        }
        else if (result == 2)
        {
            NativeDialogs::showPresetBrowser(presetsFolder, [this](juce::File file)
            {
                if (file != juce::File())
                {
                    loadPresetFromFile(file);
                }
            });
        }

        isHoldingOutside = false;
    });
}

void XYControlAudioProcessorEditor::savePresetToFile(const juce::File& file)
{
    auto position = xyControl.getPosition();
    int presetIndex = static_cast<int>(xyControl.getCurrentPreset());

    juce::var presetData(new juce::DynamicObject());
    auto* obj = presetData.getDynamicObject();
    obj->setProperty("x", position.x);
    obj->setProperty("y", position.y);
    obj->setProperty("preset", presetIndex);

    juce::String jsonString = juce::JSON::toString(presetData, true);
    file.replaceWithText(jsonString);

    NativeDialogs::showConfirmation("Preset Saved",
        "Preset saved to " + file.getFileName(), [](){});
}

void XYControlAudioProcessorEditor::loadPresetFromFile(const juce::File& file)
{
    juce::String jsonString = file.loadFileAsString();
    juce::var presetData = juce::JSON::parse(jsonString);

    if (presetData.isObject())
    {
        auto* obj = presetData.getDynamicObject();

        float x = obj->getProperty("x");
        float y = obj->getProperty("y");
        int presetIndex = obj->getProperty("preset");

        xyControl.setPreset(static_cast<XYControlComponent::Preset>(presetIndex));
        xyControl.setPosition(x, y);

        *audioProcessor.xParam = x;
        *audioProcessor.yParam = y;
        *audioProcessor.presetParam = presetIndex;

        NativeDialogs::showConfirmation("Preset Loaded",
            "Loaded preset from " + file.getFileName(), [](){});
    }
}
