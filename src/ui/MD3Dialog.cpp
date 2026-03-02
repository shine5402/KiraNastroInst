#include "MD3Dialog.h"
#include "LookAndFeel.h"
#include "../utils/Fonts.h"

namespace
{
constexpr int padding = 24;
constexpr int titleBodyGap = 16;
constexpr int bodyActionsGap = 24;
constexpr int buttonHeight = 48;
constexpr int buttonInternalPadding = 12;
constexpr float cornerRadius = 28.0f;
constexpr int minDialogWidth = 280;
constexpr int maxDialogWidth = 560;
constexpr int elevationShadowRadius = 6;

constexpr float enterSpatialDurationMs = 500.0f;
constexpr float enterEffectsDurationMs = 200.0f;
constexpr float exitSpatialDurationMs = 200.0f;
constexpr float exitEffectsDurationMs = 150.0f;

constexpr float startScale = 0.85f;
constexpr float exitShrinkScale = 0.92f;
constexpr float scrimMaxAlpha = 0.32f;
}

MD3Dialog::MD3Dialog(const juce::String &title, const juce::String &message,
                     const juce::String &buttonText, juce::Component *parent,
                     Callback onClose)
    : m_parent(parent), m_title(title), m_message(message),
      m_buttonText(buttonText), m_onClose(std::move(onClose))
{
    m_button = std::make_unique<juce::TextButton>(buttonText);
    m_button->setComponentID("md3DialogButton");
    
    m_button->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    
    addAndMakeVisible(*m_button);

    m_button->onClick = [this] { close(); };

    m_updater = std::make_unique<juce::VBlankAnimatorUpdater>(this);

    setComponentID("md3Dialog");
    setInterceptsMouseClicks(true, true);
    setAlwaysOnTop(true);
}

MD3Dialog::~MD3Dialog()
{
}

void MD3Dialog::paint(juce::Graphics &g)
{
    auto *lf = dynamic_cast<KiraNastroLookAndFeel *>(&getLookAndFeel());
    if (lf == nullptr)
        return;

    auto bounds = getLocalBounds().toFloat();
    
    // Scrim - full bounds
    auto scrimColour = juce::Colour::fromRGBA(0, 0, 0, static_cast<juce::uint8>(scrimMaxAlpha * 255 * m_animAlpha));
    g.setColour(scrimColour);
    g.fillRect(bounds);

    // Compute dialog bounds centered in parent
    auto dialogBounds = computeDialogBounds().toFloat();
    
    // Apply scale animation around center
    auto scaledWidth = dialogBounds.getWidth() * m_animScale;
    auto scaledHeight = dialogBounds.getHeight() * m_animScale;
    auto scaledDialogBounds = dialogBounds.withSizeKeepingCentre(scaledWidth, scaledHeight);

    // Elevation shadow
    juce::DropShadow shadow;
    shadow.colour = juce::Colours::black.withAlpha(0.2f * m_animAlpha);
    shadow.radius = elevationShadowRadius;
    shadow.offset = {0, elevationShadowRadius / 2};
    juce::Path dialogShadowPath;
    dialogShadowPath.addRoundedRectangle(scaledDialogBounds, cornerRadius);
    shadow.drawForPath(g, dialogShadowPath);

    // Dialog background
    g.setColour(lf->surfaceContainerHigh().withAlpha(m_animAlpha));
    g.fillRoundedRectangle(scaledDialogBounds, cornerRadius);

    // Content clipping
    auto contentBounds = scaledDialogBounds.reduced(padding);

    // Title
    g.setColour(lf->onSurface().withAlpha(m_animAlpha));
    g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(24.0f)));
    auto titleBounds = contentBounds.removeFromTop(32);
    g.drawFittedText(m_title, titleBounds.toNearestIntEdges(), juce::Justification::topLeft, 1);

    contentBounds.removeFromTop(titleBodyGap);

    // Body
    g.setColour(lf->onSurfaceVariant().withAlpha(m_animAlpha));
    g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(14.0f)));

    auto bodyBounds = contentBounds.withTrimmedBottom(buttonHeight + bodyActionsGap);
    g.drawFittedText(m_message, bodyBounds.toNearestIntEdges(), juce::Justification::topLeft, 10);
}

void MD3Dialog::resized()
{
    m_targetBounds = computeDialogBounds();

    auto contentBounds = m_targetBounds.reduced(padding);
    contentBounds.removeFromTop(32);
    contentBounds.removeFromTop(titleBodyGap);

    auto bodyHeight = contentBounds.getHeight() - buttonHeight - bodyActionsGap;
    contentBounds.removeFromTop(bodyHeight);
    contentBounds.removeFromTop(bodyActionsGap);

    auto buttonWidth = m_button->getBestWidthForHeight(buttonHeight);
    buttonWidth = juce::jmax(buttonWidth, 64 + buttonInternalPadding * 2);

    auto buttonBounds = contentBounds.removeFromRight(buttonWidth + buttonInternalPadding * 2);
    buttonBounds.setHeight(buttonHeight);
    m_button->setBounds(buttonBounds);
}

void MD3Dialog::show(const juce::String &title, const juce::String &message,
                     const juce::String &buttonText, juce::Component *parent,
                     Callback onClose)
{
    if (parent == nullptr)
        return;

    auto dialog = new MD3Dialog(title, message, buttonText, parent, std::move(onClose));
    parent->addChildComponent(dialog);
    dialog->setBounds(parent->getLocalBounds());
    dialog->setVisible(true);
    dialog->startEnterAnimation();
}

void MD3Dialog::close()
{
    if (m_isExiting)
        return;
    
    m_isExiting = true;
    
    if (m_onClose)
        m_onClose();

    startExitAnimation();
}

void MD3Dialog::cleanupAndDelete()
{
    if (m_parent != nullptr)
        m_parent->removeChildComponent(this);
    // Defer deletion — we're inside AnimatorUpdater::update() on the call stack.
    // Synchronously deleting m_updater here corrupts the update loop's iterator.
    juce::MessageManager::callAsync([this] { delete this; });
}

juce::Rectangle<int> MD3Dialog::computeDialogBounds() const
{
    auto parentBounds = getLocalBounds();

    juce::Font bodyFont(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(14.0f));

    auto maxWidth = parentBounds.getWidth() - padding * 4;
    maxWidth = juce::jlimit(minDialogWidth, maxDialogWidth, maxWidth);

    juce::StringArray lines;
    lines.addTokens(m_message, "\n", {});

    int bodyHeight = 0;
    int maxLineWidth = 0;
    for (auto &line : lines)
    {
        auto lineWidth = juce::GlyphArrangement::getStringWidthInt(bodyFont, line);
        maxLineWidth = juce::jmax(maxLineWidth, lineWidth);
        bodyHeight += 20;
    }

    int dialogWidth = maxLineWidth + padding * 2;
    dialogWidth = juce::jlimit(minDialogWidth, maxDialogWidth, dialogWidth);

    int dialogHeight = padding + 32 + titleBodyGap + bodyHeight + bodyActionsGap + buttonHeight + padding;

    dialogHeight = juce::jmin(dialogHeight, parentBounds.getHeight() - padding * 2);

    return parentBounds.withSizeKeepingCentre(dialogWidth, dialogHeight);
}

void MD3Dialog::startEnterAnimation()
{
    m_animAlpha = 0.0f;
    m_animScale = startScale;
    m_targetBounds = computeDialogBounds();
    repaint();

    // Spatial: ease-out-back produces ~10% overshoot on the scale delta — clearly visible
    m_spatialAnimator = juce::ValueAnimatorBuilder()
        .withDurationMs(enterSpatialDurationMs)
        .withEasing(juce::Easings::createEaseOutBack())
        .withValueChangedCallback([this](float progress) {
            // progress can exceed 1.0 due to overshoot - don't clamp!
            m_animScale = startScale + (1.0f - startScale) * progress;
            repaint();
        })
        .withOnCompleteCallback([this] {
            m_animScale = 1.0f;
            repaint();
        })
        .build();

    // Effects: expressive default effects - no overshoot
    // cubic-bezier(0.34, 0.80, 0.34, 1.00)
    auto effectsCurve = juce::Easings::createCubicBezier(
        juce::Point<float>(0.34f, 0.80f),
        juce::Point<float>(0.34f, 1.0f)
    );
    
    m_effectsAnimator = juce::ValueAnimatorBuilder()
        .withDurationMs(enterEffectsDurationMs)
        .withEasing(effectsCurve)
        .withValueChangedCallback([this](float progress) {
            m_animAlpha = progress;
            repaint();
        })
        .build();

    m_updater->addAnimator(*m_spatialAnimator);
    m_updater->addAnimator(*m_effectsAnimator);
    
    m_spatialAnimator->start();
    m_effectsAnimator->start();
}

void MD3Dialog::startExitAnimation()
{
    // Hide the button immediately — it's a real child component that doesn't
    // participate in m_animAlpha fading and would ghost over the shrinking card.
    m_button->setVisible(false);

    // Effects: fade out fast
    auto effectsCurve = juce::Easings::createCubicBezier(
        juce::Point<float>(0.34f, 0.80f),
        juce::Point<float>(0.34f, 1.0f)
    );
    
    m_effectsAnimator = juce::ValueAnimatorBuilder()
        .withDurationMs(exitEffectsDurationMs)
        .withEasing(effectsCurve)
        .withValueChangedCallback([this](float progress) {
            m_animAlpha = 1.0f - progress;
            repaint();
        })
        .build();

    // Spatial: emphasized accelerate - no bounce, fast exit
    // cubic-bezier(0.3, 0.0, 0.8, 0.15)
    auto accelerateCurve = juce::Easings::createCubicBezier(
        juce::Point<float>(0.3f, 0.0f),
        juce::Point<float>(0.8f, 0.15f)
    );
    
    m_spatialAnimator = juce::ValueAnimatorBuilder()
        .withDurationMs(exitSpatialDurationMs)
        .withEasing(accelerateCurve)
        .withValueChangedCallback([this](float progress) {
            m_animScale = 1.0f - (1.0f - exitShrinkScale) * progress;
            repaint();
        })
        .withOnCompleteCallback([this] {
            cleanupAndDelete();
        })
        .build();

    m_updater->addAnimator(*m_effectsAnimator);
    m_updater->addAnimator(*m_spatialAnimator);
    
    m_effectsAnimator->start();
    m_spatialAnimator->start();
}
