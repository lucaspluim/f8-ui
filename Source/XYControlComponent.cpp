#include "XYControlComponent.h"
#include "BinaryData.h"

XYControlComponent::XYControlComponent()
    : springLayers{{
        SpringLayer(0.20f, 1.13f, 1.6f),    // cursor - overdamped, zero bounce
        SpringLayer(0.09f, 0.88f, 3.8f),    // inner
        SpringLayer(0.07f, 0.85f, 5.2f),    // mid
        SpringLayer(0.05f, 0.82f, 6.8f),    // outer
        SpringLayer(0.04f, 0.78f, 8.5f),    // ambient
        SpringLayer(0.03f, 0.75f, 10.5f)    // atmosphere
    }},
    glowLayers{{
        {120, 0.95f, juce::Colours::black, {}},  // Will be set by preset
        {180, 0.75f, juce::Colours::black, {}},
        {260, 0.60f, juce::Colours::black, {}},
        {360, 0.45f, juce::Colours::black, {}},
        {480, 0.35f, juce::Colours::black, {}}
    }}
{
    lastFrameTime = juce::Time::currentTimeMillis();
    startTimerHz(60);

    updateColorsForPreset();
    loadGlowImagesFromBinaryData();
}

XYControlComponent::~XYControlComponent()
{
}

void XYControlComponent::setPreset(Preset preset)
{
    currentPreset = preset;
    updateColorsForPreset();
    loadGlowImagesFromBinaryData();
    repaint();
}

void XYControlComponent::updateColorsForPreset()
{
    switch (currentPreset)
    {
        case Preset::Blue:
            backgroundColor = juce::Colours::white;
            cursorColor = juce::Colours::white;
            break;

        case Preset::Red:
            backgroundColor = juce::Colour(0xFFFF0000);  // Red
            cursorColor = juce::Colour(0xFFFF0000);      // Red
            break;

        case Preset::Black:
            backgroundColor = juce::Colours::black;
            cursorColor = juce::Colours::black;
            break;
    }
}

void XYControlComponent::loadGlowImagesFromBinaryData()
{
    // Determine preset prefix and sizes
    const char* presetName;
    int sizes[5];

    switch (currentPreset)
    {
        case Preset::Blue:
            presetName = "blue";
            sizes[0] = 120; sizes[1] = 180; sizes[2] = 260; sizes[3] = 360; sizes[4] = 480;
            break;
        case Preset::Red:
            presetName = "red";
            sizes[0] = 120; sizes[1] = 180; sizes[2] = 260; sizes[3] = 360; sizes[4] = 480;
            break;
        case Preset::Black:
            presetName = "black";
            // Smaller sizes for white glow to compensate for visual contrast
            sizes[0] = 100; sizes[1] = 150; sizes[2] = 215; sizes[3] = 300; sizes[4] = 400;
            break;
        default:
            presetName = "blue";
            sizes[0] = 120; sizes[1] = 180; sizes[2] = 260; sizes[3] = 360; sizes[4] = 480;
            break;
    }

    for (size_t i = 0; i < glowLayers.size(); ++i)
    {
        // Update layer size for current preset
        glowLayers[i].size = sizes[i];

        // Construct resource name: "glow_blue_layer_0_png"
        juce::String resourceName = juce::String("glow_") + presetName + "_layer_" + juce::String((int)i) + "_png";

        // Get the binary data for this layer
        int dataSize = 0;
        const char* data = BinaryData::getNamedResource(resourceName.toRawUTF8(), dataSize);

        if (data != nullptr && dataSize > 0)
        {
            // Load PNG from memory
            glowLayers[i].cachedImage = juce::ImageFileFormat::loadFrom(data, (size_t)dataSize);
        }
        else
        {
            // Fallback: create a simple colored circle if resource missing
            juce::Image fallback(juce::Image::ARGB, glowLayers[i].size, glowLayers[i].size, true);
            juce::Graphics g(fallback);
            g.setColour(glowLayers[i].color);
            g.fillEllipse(0, 0, (float)glowLayers[i].size, (float)glowLayers[i].size);
            glowLayers[i].cachedImage = fallback;
        }
    }
}

void XYControlComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Draw rounded rectangle background with preset color
    g.setColour(backgroundColor);
    g.fillRoundedRectangle(bounds.toFloat(), 24.0f);

    // Enable high quality rendering
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

    // Clip to rounded rectangle
    juce::Path clipPath;
    clipPath.addRoundedRectangle(bounds.toFloat(), 24.0f);
    g.reduceClipRegion(clipPath);

    // Draw glow layers from back to front
    for (int i = 4; i >= 0; --i)
    {
        auto& spring = springLayers[i + 1];
        auto& layer = glowLayers[i];

        float pixelX = spring.x * bounds.getWidth();
        float pixelY = spring.y * bounds.getHeight();

        float scaleX = 1.0f;
        float scaleY = 1.0f;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        float rotation = 0.0f;
        float opacity = layer.opacity;

        // Calculate velocity magnitude and direction (always, for smooth blending)
        float speed = std::sqrt(spring.vx * spring.vx + spring.vy * spring.vy);

        // Motion-based deformation
        if (speed > 0.0001f)
        {
            // Angle of movement
            rotation = std::atan2(spring.vy, spring.vx);

            // Speed-based stretching factor with smooth falloff
            float speedFactor = 1.0f - std::exp(-speed * 8.0f);

            // Create comet tail effect:
            // - Stretch along direction of movement (scaleX)
            // - Squash perpendicular (scaleY)
            // - More dramatic on outer layers
            float stretchMultiplier = 1.0f + i * 0.3f;
            scaleX = 1.0f + speedFactor * (1.2f + stretchMultiplier);  // Stretch behind
            scaleY = 1.0f / (1.0f + speedFactor * (0.5f + i * 0.1f)); // Squash sides

            // Offset layers backward along movement vector for tail effect
            float offsetAmount = speedFactor * (15.0f + i * 8.0f);
            offsetX = -std::cos(rotation) * offsetAmount;
            offsetY = -std::sin(rotation) * offsetAmount;
        }

        // Blend in breathing animation when idle
        if (isBreathing && breatheBlend > 0.0f)
        {
            // Breathing animation with slightly different timing for each layer
            float breathePhase = breatheTime + i * 0.3f;
            float breatheScale = 1.0f + 0.08f * std::sin(breathePhase);
            float breatheOpacity = 0.85f + 0.15f * (0.5f + 0.5f * std::sin(breathePhase));

            // Smoothly blend from motion state to breathing state
            scaleX = scaleX * (1.0f - breatheBlend) + breatheScale * breatheBlend;
            scaleY = scaleY * (1.0f - breatheBlend) + breatheScale * breatheBlend;
            opacity = opacity * (1.0f - breatheBlend) + (layer.opacity * breatheOpacity) * breatheBlend;

            // Fade out motion-based rotation and offset
            rotation *= (1.0f - breatheBlend);
            offsetX *= (1.0f - breatheBlend);
            offsetY *= (1.0f - breatheBlend);
        }

        // Draw the cached blurred image with comet transformation
        g.setOpacity(opacity);

        // Create proper directional stretch transform
        float centerX = layer.cachedImage.getWidth() / 2.0f;
        float centerY = layer.cachedImage.getHeight() / 2.0f;

        juce::AffineTransform transform = juce::AffineTransform()
            .translated(-centerX, -centerY)                    // Center at origin
            .scaled(scaleX, scaleY)                            // Apply scale
            .followedBy(juce::AffineTransform::rotation(rotation)) // Rotate
            .translated(pixelX + offsetX, pixelY + offsetY);   // Move to position

        g.drawImageTransformed(layer.cachedImage, transform, false);
    }

    // Draw solid cursor with preset color
    g.setOpacity(1.0f);

    float cursorX = springLayers[0].x * bounds.getWidth();
    float cursorY = springLayers[0].y * bounds.getHeight();
    float cursorRadius = isDragging ? 8.0f : 9.0f;

    // Solid cursor circle
    g.setColour(cursorColor);
    g.fillEllipse(cursorX - cursorRadius, cursorY - cursorRadius,
                  cursorRadius * 2, cursorRadius * 2);
}

void XYControlComponent::resized()
{
}

void XYControlComponent::mouseDown(const juce::MouseEvent& event)
{
    // Immediately stop breathing to prevent jitter
    isBreathing = false;
    breatheBlend = 0.0f;
    idleTimer = 0.0f;
    isDragging = true;

    auto bounds = getLocalBounds().toFloat();
    float newX = event.position.x;
    float newY = event.position.y;

    // Constrain to rounded rectangle
    constrainToRoundedBounds(newX, newY, bounds.getWidth(), bounds.getHeight(), 24.0f);

    targetX = newX / bounds.getWidth();
    targetY = newY / bounds.getHeight();

    repaint();
}

void XYControlComponent::mouseDrag(const juce::MouseEvent& event)
{
    // Ensure breathing is stopped during any drag
    isBreathing = false;
    breatheBlend = 0.0f;
    idleTimer = 0.0f;

    auto bounds = getLocalBounds().toFloat();
    float newX = event.position.x;
    float newY = event.position.y;

    // Constrain to rounded rectangle
    constrainToRoundedBounds(newX, newY, bounds.getWidth(), bounds.getHeight(), 24.0f);

    targetX = newX / bounds.getWidth();
    targetY = newY / bounds.getHeight();

    repaint();
}

void XYControlComponent::mouseUp(const juce::MouseEvent&)
{
    // Don't immediately set isDragging to false - let motion settle first
    // This prevents sudden changes when releasing
    isDragging = false;

    // Don't reset idle timer - let it accumulate naturally
    // idleTimer will start when velocity drops below threshold

    repaint();
}

void XYControlComponent::mouseDoubleClick(const juce::MouseEvent& event)
{
    // Trigger disperse effect
    isDispersing = true;
    disperseTime = 0.0f;
    isBreathing = false;
    breatheBlend = 0.0f;

    // Add radial outward velocity to all glow layers
    // Use golden angle for better distribution
    const float goldenAngle = 2.39996f; // Golden angle in radians
    float baseAngle = juce::Random::getSystemRandom().nextFloat() * 6.28318f;

    for (size_t i = 1; i < springLayers.size(); ++i)
    {
        // Use golden angle spiral for natural, even distribution
        float angle = baseAngle + (i - 1) * goldenAngle;
        float dx = std::cos(angle);
        float dy = std::sin(angle);

        // Apply outward impulse - stronger for outer layers
        float impulse = 0.08f + i * 0.025f;
        springLayers[i].vx += dx * impulse;
        springLayers[i].vy += dy * impulse;
    }

    repaint();
}

void XYControlComponent::timerCallback()
{
    int64_t currentTime = juce::Time::currentTimeMillis();
    float dt = juce::jmin((currentTime - lastFrameTime) / 16.67f, 2.0f);
    lastFrameTime = currentTime;

    // Update all spring layers
    springLayers[0].update(targetX, targetY, dt);

    for (size_t i = 1; i < springLayers.size(); ++i)
    {
        springLayers[i].update(springLayers[0].x, springLayers[0].y, dt);

        // Very gradual velocity decay for smoothest settling
        // Only decay when extremely small to prevent any snapping
        if (std::abs(springLayers[i].vx) < 0.0005f)
            springLayers[i].vx *= 0.98f;  // More gradual
        if (std::abs(springLayers[i].vy) < 0.0005f)
            springLayers[i].vy *= 0.98f;

        // Only fully zero out when microscopic
        if (std::abs(springLayers[i].vx) < 0.00001f)
            springLayers[i].vx = 0.0f;
        if (std::abs(springLayers[i].vy) < 0.00001f)
            springLayers[i].vy = 0.0f;
    }

    // Update disperse effect
    if (isDispersing)
    {
        disperseTime += dt * 16.67f;
        if (disperseTime > 500.0f)  // Effect lasts ~500ms
        {
            isDispersing = false;
        }
    }

    // Check for idle state - use blur layers to determine true stillness
    float totalVelocity = std::abs(springLayers[0].vx) + std::abs(springLayers[0].vy);
    float blurVelocity = 0.0f;
    for (size_t i = 1; i < springLayers.size(); ++i)
    {
        blurVelocity += std::abs(springLayers[i].vx) + std::abs(springLayers[i].vy);
    }

    if (totalVelocity < 0.001f && blurVelocity < 0.01f && !isDragging && !isDispersing)
    {
        idleTimer += dt * 16.67f;
        if (idleTimer > 500.0f)  // Longer delay before breathing starts
        {
            if (!isBreathing)
            {
                // Start breathing at neutral phase (where sin = 0) to avoid snap
                breatheTime = 0.0f;
                breatheBlend = 0.0f;  // Start blend at 0
                isBreathing = true;
            }
        }
    }
    else
    {
        idleTimer = 0.0f;
        isBreathing = false;  // Stop breathing when motion detected
    }

    // Update breathing animation time and blend
    if (isBreathing)
    {
        breatheTime += 0.025f;

        // Smoothly ramp up breathe blend over ~1 second
        breatheBlend = juce::jmin(1.0f, breatheBlend + 0.015f);
    }
    else
    {
        // Quickly fade out breathing when motion starts
        breatheBlend = juce::jmax(0.0f, breatheBlend - 0.08f);
    }

    repaint();
}

void XYControlComponent::constrainToRoundedBounds(float& x, float& y, float width, float height, float cornerRadius)
{
    // First, basic clamp to rectangle
    x = juce::jlimit(0.0f, width, x);
    y = juce::jlimit(0.0f, height, y);

    // Check if we're in a corner region
    // Top-left corner
    if (x < cornerRadius && y < cornerRadius)
    {
        float cx = cornerRadius;
        float cy = cornerRadius;
        float dx = x - cx;
        float dy = y - cy;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > cornerRadius)
        {
            x = cx + (dx / dist) * cornerRadius;
            y = cy + (dy / dist) * cornerRadius;
        }
    }
    // Top-right corner
    else if (x > width - cornerRadius && y < cornerRadius)
    {
        float cx = width - cornerRadius;
        float cy = cornerRadius;
        float dx = x - cx;
        float dy = y - cy;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > cornerRadius)
        {
            x = cx + (dx / dist) * cornerRadius;
            y = cy + (dy / dist) * cornerRadius;
        }
    }
    // Bottom-left corner
    else if (x < cornerRadius && y > height - cornerRadius)
    {
        float cx = cornerRadius;
        float cy = height - cornerRadius;
        float dx = x - cx;
        float dy = y - cy;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > cornerRadius)
        {
            x = cx + (dx / dist) * cornerRadius;
            y = cy + (dy / dist) * cornerRadius;
        }
    }
    // Bottom-right corner
    else if (x > width - cornerRadius && y > height - cornerRadius)
    {
        float cx = width - cornerRadius;
        float cy = height - cornerRadius;
        float dx = x - cx;
        float dy = y - cy;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > cornerRadius)
        {
            x = cx + (dx / dist) * cornerRadius;
            y = cy + (dy / dist) * cornerRadius;
        }
    }
}
