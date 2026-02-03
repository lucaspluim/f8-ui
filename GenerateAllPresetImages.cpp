#include <juce_graphics/juce_graphics.h>
#include <juce_core/juce_core.h>
#include <iostream>

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

    std::cout << "Generating glow images for all 3 presets...\n\n";

    struct PresetConfig {
        const char* name;
        juce::Colour colors[5];
        int sizes[5];
    };

    // Standard sizes for blue and red presets
    int sizes[] = {120, 180, 260, 360, 480};
    
    // Smaller sizes for black preset (white glow needs to be smaller for visual balance)
    int blackPresetSizes[] = {100, 150, 215, 300, 400};

    PresetConfig presets[] = {
        // Blue preset
        {
            "blue",
            {
                juce::Colour::fromFloatRGBA(0.0f, 0.55f, 1.0f, 0.95f),
                juce::Colour::fromFloatRGBA(0.0f, 0.57f, 1.0f, 0.75f),
                juce::Colour::fromFloatRGBA(0.04f, 0.59f, 1.0f, 0.60f),
                juce::Colour::fromFloatRGBA(0.12f, 0.63f, 1.0f, 0.45f),
                juce::Colour::fromFloatRGBA(0.20f, 0.69f, 1.0f, 0.35f)
            },
            {}  // Will be filled below
        },
        // Red preset
        {
            "red",
            {
                juce::Colour::fromFloatRGBA(1.0f, 0.27f, 0.23f, 0.95f),
                juce::Colour::fromFloatRGBA(1.0f, 0.29f, 0.25f, 0.75f),
                juce::Colour::fromFloatRGBA(1.0f, 0.33f, 0.29f, 0.60f),
                juce::Colour::fromFloatRGBA(1.0f, 0.39f, 0.35f, 0.45f),
                juce::Colour::fromFloatRGBA(1.0f, 0.47f, 0.43f, 0.35f)
            },
            {}  // Will be filled below
        },
        // Black preset (white glow)
        {
            "black",
            {
                juce::Colour::fromFloatRGBA(1.0f, 1.0f, 1.0f, 0.95f),
                juce::Colour::fromFloatRGBA(1.0f, 1.0f, 1.0f, 0.75f),
                juce::Colour::fromFloatRGBA(1.0f, 1.0f, 1.0f, 0.60f),
                juce::Colour::fromFloatRGBA(1.0f, 1.0f, 1.0f, 0.45f),
                juce::Colour::fromFloatRGBA(1.0f, 1.0f, 1.0f, 0.35f)
            },
            {}  // Will be filled below
        }
    };

    // Fill in sizes for each preset
    for (int p = 0; p < 3; ++p) {
        int* currentSizes = (p == 2) ? blackPresetSizes : sizes;  // Use smaller sizes for black preset
        for (int i = 0; i < 5; ++i) {
            presets[p].sizes[i] = currentSizes[i];
        }
    }

    juce::File resourcesDir = juce::File::getCurrentWorkingDirectory().getChildFile("Resources");
    if (!resourcesDir.exists())
        resourcesDir.createDirectory();

    int blurRadii[] = {15, 20, 30, 40, 50};

    for (int p = 0; p < 3; ++p)
    {
        std::cout << "=== Preset: " << presets[p].name << " ===\n";

        for (int i = 0; i < 5; ++i)
        {
            auto start = juce::Time::getMillisecondCounter();
            std::cout << "  Layer " << i << " (size=" << presets[p].sizes[i]
                      << ", blur=" << blurRadii[i] << ")..." << std::flush;

            juce::Image img = createBlurredGradient(presets[p].sizes[i], presets[p].colors[i], blurRadii[i]);

            juce::String filename = juce::String("glow_") + presets[p].name + "_layer_" + juce::String(i) + ".png";
            juce::File outputFile = resourcesDir.getChildFile(filename);
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
        std::cout << "\n";
    }

    std::cout << "All presets generated successfully in: " << resourcesDir.getFullPathName() << "\n";

    juce::shutdownJuce_GUI();
    return 0;
}
