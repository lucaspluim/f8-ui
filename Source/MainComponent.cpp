#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(700, 700);
    addAndMakeVisible(xyControl);

    // Initialize with placeholder presets to demonstrate scrolling
    savedPresets.push_back({"preset_1", "", false});
    savedPresets.push_back({"preset_2", "", false});
    savedPresets.push_back({"preset_3", "", true});  // Favorited
    savedPresets.push_back({"preset_4", "", false});
    savedPresets.push_back({"preset_5", "", true});  // Favorited
    savedPresets.push_back({"preset_6", "", false});
    savedPresets.push_back({"preset_7", "", false});
    savedPresets.push_back({"preset_8", "", false});
    savedPresets.push_back({"preset_9", "", false});
    savedPresets.push_back({"preset_10", "", true});  // Favorited
    savedPresets.push_back({"preset_11", "", false});
    savedPresets.push_back({"preset_12", "", false});
    savedPresets.push_back({"preset_13", "", false});
    savedPresets.push_back({"preset_14", "", false});
    savedPresets.push_back({"preset_15", "", true});  // Favorited
    savedPresets.push_back({"preset_16", "", false});
    savedPresets.push_back({"preset_17", "", false});
    savedPresets.push_back({"preset_18", "", false});
    savedPresets.push_back({"preset_19", "", false});
    savedPresets.push_back({"preset_20", "", true});  // Favorited
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colour(0xfff5f5f7));

    // Draw drop shadow for XY control with appropriate color for preset
    auto controlBounds = xyControl.getBounds();
    juce::Path shadowPath;
    shadowPath.addRoundedRectangle(controlBounds.toFloat(), 24.0f);

    // Use lighter shadow for dark presets, darker shadow for light presets
    auto preset = xyControl.getCurrentPreset();
    juce::Colour shadowColor;

    if (preset == XYControlComponent::Preset::Blue)
        shadowColor = juce::Colour(0x14000000);  // Dark shadow for white background
    else if (preset == XYControlComponent::Preset::Red)
        shadowColor = juce::Colour(0x30000000);  // Darker shadow for red
    else // Black
        shadowColor = juce::Colour(0x40000000);  // Even darker shadow for black

    juce::DropShadow shadow(shadowColor, 16, juce::Point<int>(0, 4));
    shadow.drawForPath(g, shadowPath);
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();

    // Make it 500x500 like the HTML version
    xyControl.setBounds(bounds.withSizeKeepingCentre(500, 500));
}

void MainComponent::mouseDown(const juce::MouseEvent& event)
{
    // Check if click is outside the XY control area
    if (!xyControl.getBounds().contains(event.getPosition()))
    {
        isHoldingOutside = true;
        holdStartTime = juce::Time::currentTimeMillis();
        menuShown = false;
        startTimer(100);  // Check every 100ms
    }
}

void MainComponent::mouseUp(const juce::MouseEvent& event)
{
    isHoldingOutside = false;
    stopTimer();
}

void MainComponent::mouseDoubleClick(const juce::MouseEvent& event)
{
    // Check if double-click is outside the XY control area
    if (!xyControl.getBounds().contains(event.getPosition()))
    {
        // Cycle to next preset
        auto currentPreset = static_cast<int>(xyControl.getCurrentPreset());
        currentPreset = (currentPreset + 1) % 3;  // 0->1->2->0
        xyControl.setPreset(static_cast<XYControlComponent::Preset>(currentPreset));
    }
}

void MainComponent::timerCallback()
{
    if (isHoldingOutside && !menuShown)
    {
        int64_t currentTime = juce::Time::currentTimeMillis();
        int64_t holdDuration = currentTime - holdStartTime;

        if (holdDuration >= 3000)  // 3 seconds
        {
            menuShown = true;
            stopTimer();
            showPresetMenu();
        }
    }
}

void MainComponent::showPresetMenu()
{
    // Custom paint for dim overlay
    class DimComponent : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0x40000000));
        }
    };
    dimOverlay.reset(new DimComponent());
    dimOverlay->setBounds(getLocalBounds());
    addAndMakeVisible(dimOverlay.get());

    // Create centered preset menu
    presetMenu = std::make_unique<PresetMenuOverlay>(&savedPresets, [this](int result, const juce::String& action)
    {
        // Handle favorite toggle without closing menu
        if (action == "favorite")
        {
            toggleFavorite(result - 200);
            return;
        }

        // Remove overlays for other actions
        if (dimOverlay != nullptr)
        {
            removeChildComponent(dimOverlay.get());
            dimOverlay.reset();
        }
        if (presetMenu != nullptr)
        {
            removeChildComponent(presetMenu.get());
            presetMenu.reset();
        }

        handleMenuResult(result, action);
    });

    addAndMakeVisible(presetMenu.get());

    // Center the menu
    auto bounds = getLocalBounds();
    int menuWidth = presetMenu->getWidth();
    int menuHeight = presetMenu->getHeight();
    presetMenu->setBounds((bounds.getWidth() - menuWidth) / 2,
                          (bounds.getHeight() - menuHeight) / 2,
                          menuWidth,
                          menuHeight);
}

void MainComponent::showSaveDialog()
{
    // Custom paint for dim overlay
    class DimComponent : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0x40000000));
        }
    };
    dimOverlay.reset(new DimComponent());
    dimOverlay->setBounds(getLocalBounds());
    addAndMakeVisible(dimOverlay.get());

    // Create centered save dialog
    saveDialog = std::make_unique<SavePresetDialog>([this](juce::String name)
    {
        // Remove overlays
        if (dimOverlay != nullptr)
        {
            removeChildComponent(dimOverlay.get());
            dimOverlay.reset();
        }
        if (saveDialog != nullptr)
        {
            removeChildComponent(saveDialog.get());
            saveDialog.reset();
        }

        if (name.isNotEmpty())
        {
            savePreset(name);
        }
    });

    addAndMakeVisible(saveDialog.get());

    // Center the dialog
    auto bounds = getLocalBounds();
    int dialogWidth = saveDialog->getWidth();
    int dialogHeight = saveDialog->getHeight();
    saveDialog->setBounds((bounds.getWidth() - dialogWidth) / 2,
                          (bounds.getHeight() - dialogHeight) / 2,
                          dialogWidth,
                          dialogHeight);
}

void MainComponent::handleMenuResult(int result, const juce::String& action)
{
    if (result == 1)
    {
        // Save preset - show save dialog
        showSaveDialog();
    }
    else if (result >= 100 && result < 100 + (int)savedPresets.size())
    {
        // Load preset
        int presetIndex = result - 100;
        loadPreset(savedPresets[presetIndex].name);
    }

    isHoldingOutside = false;
}

void MainComponent::toggleFavorite(int index)
{
    if (index >= 0 && index < (int)savedPresets.size())
    {
        savedPresets[index].isFavorite = !savedPresets[index].isFavorite;

        // Rebuild the menu to show updated favorite status
        if (presetMenu != nullptr)
        {
            presetMenu->rebuildPresetList();
        }
    }
}

void MainComponent::savePreset(const juce::String& name)
{
    // Add to saved presets list (placeholder for now)
    savedPresets.push_back({name, "", false});

    // Show confirmation
    showConfirmation("Preset Saved", "\"" + name + "\" saved successfully!");
}

void MainComponent::loadPreset(const juce::String& name)
{
    // Placeholder - just show which preset was loaded
    showConfirmation("Preset Loaded", "Loaded \"" + name + "\"");
}

void MainComponent::showConfirmation(const juce::String& title, const juce::String& message)
{
    // Custom paint for dim overlay
    class DimComponent : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0x40000000));
        }
    };
    dimOverlay.reset(new DimComponent());
    dimOverlay->setBounds(getLocalBounds());
    addAndMakeVisible(dimOverlay.get());

    // Create centered confirmation dialog
    confirmDialog = std::make_unique<ConfirmationDialog>(title, message, [this]()
    {
        // Remove overlays
        if (dimOverlay != nullptr)
        {
            removeChildComponent(dimOverlay.get());
            dimOverlay.reset();
        }
        if (confirmDialog != nullptr)
        {
            removeChildComponent(confirmDialog.get());
            confirmDialog.reset();
        }
    });

    addAndMakeVisible(confirmDialog.get());

    // Center the dialog
    auto bounds = getLocalBounds();
    int dialogWidth = confirmDialog->getWidth();
    int dialogHeight = confirmDialog->getHeight();
    confirmDialog->setBounds((bounds.getWidth() - dialogWidth) / 2,
                             (bounds.getHeight() - dialogHeight) / 2,
                             dialogWidth,
                             dialogHeight);
}
