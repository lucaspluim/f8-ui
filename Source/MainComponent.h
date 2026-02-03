#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "XYControlComponent.h"
#include "NativeDialogs.h"

// Apple-style confirmation dialog (dark mode)
class ConfirmationDialog : public juce::Component
{
public:
    ConfirmationDialog(const juce::String& title, const juce::String& message, std::function<void()> callback)
        : resultCallback(callback)
    {
        titleText = title;
        messageText = message;

        // OK button
        okButton.setButtonText("OK");
        okButton.onClick = [this]() { resultCallback(); };
        okButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff007aff));
        okButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff0066dd));
        okButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        okButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(okButton);

        setSize(320, 140);
    }

    void paint(juce::Graphics& g) override
    {
        // Drop shadow
        juce::Path shadowPath;
        shadowPath.addRoundedRectangle(getLocalBounds().toFloat(), 12.0f);
        juce::DropShadow shadow(juce::Colour(0x60000000), 20, juce::Point<int>(0, 4));
        shadow.drawForPath(g, shadowPath);

        // Dark background
        g.setColour(juce::Colour(0xff1c1c1e));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 12.0f);

        // Border
        g.setColour(juce::Colour(0x20ffffff));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 12.0f, 1.0f);

        // Title
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(17.0f, juce::Font::bold));
        g.drawText(titleText, 20, 16, getWidth() - 40, 24, juce::Justification::centred);

        // Message
        g.setColour(juce::Colour(0xffcccccc));
        g.setFont(juce::Font(14.0f));
        g.drawText(messageText, 20, 48, getWidth() - 40, 40, juce::Justification::centred);
    }

    void resized() override
    {
        okButton.setBounds((getWidth() - 140) / 2, getHeight() - 48, 140, 36);
    }

private:
    juce::TextButton okButton;
    juce::String titleText;
    juce::String messageText;
    std::function<void()> resultCallback;
};

// Apple-style save dialog (dark mode)
class SavePresetDialog : public juce::Component
{
public:
    SavePresetDialog(std::function<void(juce::String)> callback)
        : resultCallback(callback)
    {
        // Text input
        nameEditor.setText("my_preset");
        nameEditor.setFont(juce::Font(16.0f));
        nameEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2c2c2e));
        nameEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
        nameEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0x30ffffff));
        nameEditor.setBorder(juce::BorderSize<int>(8));
        addAndMakeVisible(nameEditor);

        // Save button
        saveButton.setButtonText("Save");
        saveButton.onClick = [this]()
        {
            juce::String name = nameEditor.getText().trim();
            if (name.isNotEmpty())
                resultCallback(name);
        };
        saveButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff007aff));
        saveButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff0066dd));
        saveButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        saveButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(saveButton);

        // Cancel button
        cancelButton.setButtonText("Cancel");
        cancelButton.onClick = [this]() { resultCallback(""); };
        cancelButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2c2c2e));
        cancelButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff3a3a3c));
        cancelButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        cancelButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(cancelButton);

        setSize(320, 160);
    }

    void paint(juce::Graphics& g) override
    {
        // Drop shadow
        juce::Path shadowPath;
        shadowPath.addRoundedRectangle(getLocalBounds().toFloat(), 12.0f);
        juce::DropShadow shadow(juce::Colour(0x60000000), 20, juce::Point<int>(0, 4));
        shadow.drawForPath(g, shadowPath);

        // Dark background
        g.setColour(juce::Colour(0xff1c1c1e));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 12.0f);

        // Border
        g.setColour(juce::Colour(0x20ffffff));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 12.0f, 1.0f);

        // Title
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(17.0f, juce::Font::bold));
        g.drawText("Save Preset", 20, 16, getWidth() - 40, 24, juce::Justification::centred);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(20);
        bounds.removeFromTop(44);

        nameEditor.setBounds(bounds.removeFromTop(40));
        bounds.removeFromTop(12);

        auto buttonArea = bounds.removeFromTop(36);
        cancelButton.setBounds(buttonArea.removeFromLeft(140));
        buttonArea.removeFromLeft(8);
        saveButton.setBounds(buttonArea);
    }

private:
    juce::TextEditor nameEditor;
    juce::TextButton saveButton;
    juce::TextButton cancelButton;
    std::function<void(juce::String)> resultCallback;
};

// Preset item component
struct PresetItem
{
    juce::String name;
    juce::String folder;
    bool isFavorite = false;
};

// Apple-style preset menu overlay with folders and favorites (dark mode)
class PresetMenuOverlay : public juce::Component
{
public:
    PresetMenuOverlay(std::vector<PresetItem>* presets, std::function<void(int, const juce::String&)> callback)
        : allPresets(presets), resultCallback(callback)
    {
        // Save button (dark mode)
        saveButton.setButtonText("+ Save Current Preset");
        saveButton.onClick = [this]() { resultCallback(1, ""); };
        saveButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        saveButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0x20ffffff));
        saveButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff0a84ff));
        saveButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(saveButton);

        // Folder buttons (dark mode)
        allButton.setButtonText("All Presets");
        allButton.onClick = [this]() { setCurrentFolder(""); };
        allButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        styleFilterButton(allButton);
        addAndMakeVisible(allButton);

        favoritesButton.setButtonText("Favorites");
        favoritesButton.onClick = [this]() { setCurrentFolder("Favorites"); };
        favoritesButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        styleFilterButton(favoritesButton);
        addAndMakeVisible(favoritesButton);

        // Scrollable preset list
        viewport.setViewedComponent(&presetContainer, false);
        viewport.setScrollBarsShown(true, false);
        viewport.setScrollOnDragMode(juce::Viewport::ScrollOnDragMode::nonHover);
        viewport.setScrollBarThickness(8);
        addAndMakeVisible(viewport);

        // Set up container for smooth scrolling
        presetContainer.setBufferedToImage(true);

        // Close button (dark mode)
        closeButton.setButtonText("X");
        closeButton.onClick = [this]() { resultCallback(0, ""); };
        closeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        closeButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0x20ffffff));
        closeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffaaaaaa));
        closeButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(closeButton);

        setSize(360, 420);
        setCurrentFolder("");
    }

    void styleFilterButton(juce::TextButton& btn)
    {
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2c2c2e));
        btn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff0a84ff));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        btn.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    }

    void setCurrentFolder(const juce::String& folder)
    {
        currentFolder = folder;
        allButton.setToggleState(folder.isEmpty(), juce::dontSendNotification);
        favoritesButton.setToggleState(folder == "Favorites", juce::dontSendNotification);
        rebuildPresetList();
    }

    void rebuildPresetList()
    {
        presetButtons.clear();

        int yPos = 4;
        for (size_t i = 0; i < allPresets->size(); ++i)
        {
            const auto& preset = (*allPresets)[i];

            // Filter by folder
            if (!currentFolder.isEmpty() && currentFolder == "Favorites" && !preset.isFavorite)
                continue;
            if (!currentFolder.isEmpty() && currentFolder != "Favorites" && preset.folder != currentFolder)
                continue;

            // Create preset row (dark mode)
            auto* btn = new juce::TextButton(preset.name);
            btn->onClick = [this, i]() { resultCallback(100 + (int)i, "load"); };
            btn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            btn->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0x20ffffff));
            btn->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            btn->setBounds(4, yPos, 290, 32);

            // Favorite button (dark mode)
            auto* favBtn = new juce::TextButton("*");
            favBtn->onClick = [this, i]() { resultCallback(200 + (int)i, "favorite"); };
            favBtn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            favBtn->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0x20ffffff));
            favBtn->setColour(juce::TextButton::textColourOffId, preset.isFavorite ? juce::Colour(0xffffcc00) : juce::Colour(0xff666666));
            favBtn->setMouseCursor(juce::MouseCursor::PointingHandCursor);
            favBtn->setBounds(298, yPos, 36, 32);

            presetContainer.addAndMakeVisible(btn);
            presetContainer.addAndMakeVisible(favBtn);
            presetButtons.add(btn);
            presetButtons.add(favBtn);

            yPos += 36;
        }

        presetContainer.setSize(340, juce::jmax(yPos, 200));
    }

    void paint(juce::Graphics& g) override
    {
        // Drop shadow
        juce::Path shadowPath;
        shadowPath.addRoundedRectangle(getLocalBounds().toFloat(), 12.0f);
        juce::DropShadow shadow(juce::Colour(0x60000000), 20, juce::Point<int>(0, 4));
        shadow.drawForPath(g, shadowPath);

        // Dark background
        g.setColour(juce::Colour(0xff1c1c1e));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 12.0f);

        // Border
        g.setColour(juce::Colour(0x20ffffff));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 12.0f, 1.0f);

        // Separator lines (dark mode)
        g.setColour(juce::Colour(0x15ffffff));
        g.drawLine(16, 56, getWidth() - 16, 56, 1.0f);  // After save button
        g.drawLine(16, 104, getWidth() - 16, 104, 1.0f);  // After folder buttons
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(12);

        // Close button (top right, doesn't overlap save button text)
        closeButton.setBounds(getWidth() - 40, 12, 28, 28);

        // Save button (with space on right for close button)
        auto saveArea = bounds.removeFromTop(40);
        saveArea.removeFromRight(32);  // Leave space for close button
        saveButton.setBounds(saveArea);
        bounds.removeFromTop(8);

        // Folder filter buttons
        auto filterArea = bounds.removeFromTop(40);
        allButton.setBounds(filterArea.removeFromLeft((filterArea.getWidth() - 4) / 2));
        filterArea.removeFromLeft(4);
        favoritesButton.setBounds(filterArea);
        bounds.removeFromTop(8);

        // Scrollable list
        viewport.setBounds(bounds);
    }

private:
    juce::TextButton saveButton;
    juce::TextButton closeButton;
    juce::TextButton allButton;
    juce::TextButton favoritesButton;
    juce::Viewport viewport;
    juce::Component presetContainer;
    juce::OwnedArray<juce::Button> presetButtons;
    std::vector<PresetItem>* allPresets;
    juce::String currentFolder;
    std::function<void(int, const juce::String&)> resultCallback;
};

class MainComponent : public juce::Component,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;

private:
    void timerCallback() override;
    void savePreset(const juce::String& name);
    void loadPreset(const juce::String& name);
    void showPresetMenu();
    void showSaveDialog();
    void handleMenuResult(int result, const juce::String& action);
    void toggleFavorite(int index);
    void showConfirmation(const juce::String& title, const juce::String& message);

    XYControlComponent xyControl;

    bool isHoldingOutside = false;
    int64_t holdStartTime = 0;
    bool menuShown = false;
    float holdProgress = 0.0f;  // 0.0 to 1.0 for visual feedback

    std::vector<PresetItem> savedPresets;

    std::unique_ptr<juce::Component> dimOverlay;
    std::unique_ptr<PresetMenuOverlay> presetMenu;
    std::unique_ptr<SavePresetDialog> saveDialog;
    std::unique_ptr<ConfirmationDialog> confirmDialog;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
