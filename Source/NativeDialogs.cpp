#include "NativeDialogs.h"
#include <juce_gui_basics/juce_gui_basics.h>

juce::File NativeDialogs::getPresetsFolder()
{
    auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                     .getChildFile("XYControl Presets");

    if (!folder.exists())
        folder.createDirectory();

    return folder;
}

void NativeDialogs::showSaveDialog(const juce::File& presetsFolder, std::function<void(juce::File)> callback)
{
    auto chooser = std::make_shared<juce::FileChooser>("Save Preset", presetsFolder, "*.json");

    auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(flags, [callback, chooser](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file != juce::File{})
        {
            // Ensure .json extension
            if (!file.hasFileExtension(".json"))
                file = file.withFileExtension(".json");
            callback(file);
        }
        else
        {
            callback(juce::File{});
        }
    });
}

void NativeDialogs::showPresetBrowser(const juce::File& presetsFolder, std::function<void(juce::File)> onLoad)
{
    auto chooser = std::make_shared<juce::FileChooser>("Load Preset", presetsFolder, "*.json");

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(flags, [onLoad, chooser](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        onLoad(file);
    });
}

void NativeDialogs::showConfirmation(const juce::String& title, const juce::String& message, std::function<void()> callback)
{
    juce::NativeMessageBox::showMessageBoxAsync(
        juce::MessageBoxIconType::InfoIcon,
        title,
        message,
        nullptr,
        juce::ModalCallbackFunction::create([callback](int)
        {
            if (callback)
                callback();
        })
    );
}

void NativeDialogs::createNewFolder(const juce::File& parentFolder, std::function<void(bool)> callback)
{
    // Not implemented for cross-platform version
    callback(false);
}

void NativeDialogs::showPresetMenu(std::function<void(int)> callback)
{
    auto* menu = new juce::PopupMenu();
    menu->addItem(1, "Save Preset");
    menu->addItem(2, "Load Preset");

    menu->showMenuAsync(juce::PopupMenu::Options(), [callback](int result)
    {
        callback(result);
    });
}
