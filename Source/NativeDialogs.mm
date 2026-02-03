#include "NativeDialogs.h"

#if JUCE_MAC
#import <Cocoa/Cocoa.h>

juce::File NativeDialogs::getPresetsFolder()
{
    // Get user's Documents folder and create XYControl Presets folder
    juce::File documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    juce::File presetsFolder = documentsDir.getChildFile("XYControl Presets");

    if (!presetsFolder.exists())
        presetsFolder.createDirectory();

    return presetsFolder;
}

void NativeDialogs::showSaveDialog(const juce::File& presetsFolder, std::function<void(juce::File)> callback)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSSavePanel *panel = [NSSavePanel savePanel];
        [panel setTitle:@"Save Preset"];
        [panel setPrompt:@"Save"];
        [panel setNameFieldStringValue:@"my_preset.json"];
        [panel setAllowedFileTypes:@[@"json"]];
        [panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:presetsFolder.getFullPathName().toRawUTF8()]]];
        [panel setCanCreateDirectories:YES];
        [panel setShowsTagField:NO];

        NSModalResponse result = [panel runModal];

        if (result == NSModalResponseOK) {
            NSURL *fileURL = [panel URL];
            NSString *path = [fileURL path];
            juce::File file(juce::String::fromUTF8([path UTF8String]));
            callback(file);
        } else {
            callback(juce::File());
        }
    });
}

void NativeDialogs::showPresetBrowser(const juce::File& presetsFolder,
                                      std::function<void(juce::File)> onLoad)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSOpenPanel *panel = [NSOpenPanel openPanel];
        [panel setTitle:@"Load Preset"];
        [panel setPrompt:@"Load"];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];
        [panel setCanCreateDirectories:YES];
        [panel setShowsTagField:NO];
        [panel setAllowedFileTypes:@[@"json"]];
        [panel setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:presetsFolder.getFullPathName().toRawUTF8()]]];

        // Add accessory view with "New Folder" button
        NSView *accessoryView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 300, 30)];
        NSButton *newFolderButton = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 120, 24)];
        [newFolderButton setTitle:@"New Folder..."];
        [newFolderButton setBezelStyle:NSBezelStyleRounded];
        [newFolderButton setTarget:nil];
        [newFolderButton setAction:nil]; // Action handled separately
        [accessoryView addSubview:newFolderButton];
        [panel setAccessoryView:accessoryView];

        NSModalResponse result = [panel runModal];

        if (result == NSModalResponseOK) {
            NSURL *fileURL = [[panel URLs] firstObject];
            NSString *path = [fileURL path];
            juce::File file(juce::String::fromUTF8([path UTF8String]));
            onLoad(file);
        } else {
            onLoad(juce::File());
        }
    });
}

void NativeDialogs::createNewFolder(const juce::File& parentFolder, std::function<void(bool)> callback)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"New Folder"];
        [alert setInformativeText:@"Enter a name for the new folder:"];
        [alert addButtonWithTitle:@"Create"];
        [alert addButtonWithTitle:@"Cancel"];

        NSTextField *input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 260, 24)];
        [input setStringValue:@"New Folder"];
        [alert setAccessoryView:input];

        NSModalResponse returnCode = [alert runModal];

        if (returnCode == NSAlertFirstButtonReturn) {
            NSString *text = [input stringValue];
            juce::String folderName = juce::String::fromUTF8([text UTF8String]);
            juce::File newFolder = parentFolder.getChildFile(folderName);
            bool success = newFolder.createDirectory().wasOk();
            callback(success);
        } else {
            callback(false);
        }
    });
}

void NativeDialogs::showConfirmation(const juce::String& title, const juce::String& message, std::function<void()> callback)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:[NSString stringWithUTF8String:title.toRawUTF8()]];
        [alert setInformativeText:[NSString stringWithUTF8String:message.toRawUTF8()]];
        [alert addButtonWithTitle:@"OK"];

        [alert runModal];
        callback();
    });
}

#else
// Fallback for non-Mac platforms
juce::File NativeDialogs::getPresetsFolder()
{
    juce::File documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    juce::File presetsFolder = documentsDir.getChildFile("XYControl Presets");
    if (!presetsFolder.exists())
        presetsFolder.createDirectory();
    return presetsFolder;
}

void NativeDialogs::showSaveDialog(const juce::File& presetsFolder, std::function<void(juce::File)> callback)
{
    callback(presetsFolder.getChildFile("my_preset.json"));
}

void NativeDialogs::showPresetBrowser(const juce::File& presetsFolder, std::function<void(juce::File)> onLoad)
{
    onLoad(juce::File());
}

void NativeDialogs::showConfirmation(const juce::String& title, const juce::String& message, std::function<void()> callback)
{
    callback();
}

void NativeDialogs::createNewFolder(const juce::File& parentFolder, std::function<void(bool)> callback)
{
    callback(false);
}
#endif
