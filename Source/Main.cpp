#include <juce_gui_extra/juce_gui_extra.h>
#include "MainComponent.h"

class XYControlApplication : public juce::JUCEApplication
{
public:
    XYControlApplication() {}

    const juce::String getApplicationName() override { return "XY Control"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        auto startTime = juce::Time::getMillisecondCounter();

        mainWindow.reset(new MainWindow(getApplicationName()));

        auto elapsedMs = juce::Time::getMillisecondCounter() - startTime;

        // Log to file
        juce::File timingFile = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
            .getChildFile("xycontrol_startup_timing.txt");
        timingFile.replaceWithText("App window created in " + juce::String(elapsedMs) + "ms\n");
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(name,
                           juce::Colours::lightgrey,
                           DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);

            #if JUCE_IOS || JUCE_ANDROID
                setFullScreen(true);
            #else
                setResizable(true, true);
                centreWithSize(getWidth(), getHeight());
            #endif

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(XYControlApplication)
