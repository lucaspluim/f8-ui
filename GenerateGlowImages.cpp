#include <juce_graphics/juce_graphics.h>
#include <juce_core/juce_core.h>
#include <iostream>

// Copy the exact same function from XYControlComponent
juce::Image createBlurredGradient(int size, const juce::Colour& color, int blurRadius)
{
    int imageSize = size + blurRadius * 2;
    juce::Image img(juce::Image::ARGB, imageSize, imageSize, true);

    juce::Graphics g(img);

    float centerX = imageSize / 2.0f;
    float centerY = imageSize / 2.0f;
    float radius = size / 2.0f;

    juce::ColourGradient gradient(
        color,
        centerX, centerY,
        color.withAlpha(0.0f),
        centerX + radius, centerY,
        true
    );

    gradient.addColour(0.3, color.withMultipliedAlpha(0.9f));
    gradient.addColour(0.5, color.withMultipliedAlpha(0.6f));
    gradient.addColour(0.7, color.withMultipliedAlpha(0.3f));
    gradient.addColour(0.9, color.withMultipliedAlpha(0.1f));

    g.setGradientFill(gradient);
    g.fillEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);

    if (blurRadius > 0)
    {
        juce::ImageConvolutionKernel blur(blurRadius);
        blur.createGaussianBlur(blurRadius * 0.4f);
        blur.applyToImage(img, img, juce::Rectangle<int>(0, 0, imageSize, imageSize));
    }

    return img;
}

int main(int argc, char* argv[])
{
    juce::initialiseJuce_GUI();

    std::cout << "Generating glow images...\n";

    // Match exactly what's in XYControlComponent
    struct LayerConfig {
        int size;
        juce::Colour color;
        int blurRadius;
        const char* filename;
    };

    LayerConfig layers[] = {
        {120, juce::Colour::fromFloatRGBA(0.0f, 0.55f, 1.0f, 0.95f), 15, "glow_layer_0.png"},
        {180, juce::Colour::fromFloatRGBA(0.0f, 0.57f, 1.0f, 0.75f), 20, "glow_layer_1.png"},
        {260, juce::Colour::fromFloatRGBA(0.04f, 0.59f, 1.0f, 0.60f), 30, "glow_layer_2.png"},
        {360, juce::Colour::fromFloatRGBA(0.12f, 0.63f, 1.0f, 0.45f), 40, "glow_layer_3.png"},
        {480, juce::Colour::fromFloatRGBA(0.20f, 0.69f, 1.0f, 0.35f), 50, "glow_layer_4.png"}
    };

    juce::File resourcesDir = juce::File::getCurrentWorkingDirectory().getChildFile("Resources");
    if (!resourcesDir.exists())
        resourcesDir.createDirectory();

    for (int i = 0; i < 5; ++i)
    {
        auto start = juce::Time::getMillisecondCounter();
        std::cout << "  Creating layer " << i << " (size=" << layers[i].size
                  << ", blur=" << layers[i].blurRadius << ")..." << std::flush;

        juce::Image img = createBlurredGradient(layers[i].size, layers[i].color, layers[i].blurRadius);

        juce::File outputFile = resourcesDir.getChildFile(layers[i].filename);
        juce::FileOutputStream stream(outputFile);

        if (stream.openedOk())
        {
            juce::PNGImageFormat pngFormat;
            pngFormat.writeImageToStream(img, stream);

            auto elapsed = juce::Time::getMillisecondCounter() - start;
            std::cout << " done (" << elapsed << "ms)\n";
        }
        else
        {
            std::cout << " FAILED to write file!\n";
            return 1;
        }
    }

    std::cout << "\nAll images generated successfully in: " << resourcesDir.getFullPathName() << "\n";

    juce::shutdownJuce_GUI();
    return 0;
}
