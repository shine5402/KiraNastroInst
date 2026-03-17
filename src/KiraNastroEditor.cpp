// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "KiraNastroEditor.h"

#include <cmath>
#include <cstring>

#include "BinaryDataImages.h"
#include "data/LabelExporter.h"
#include "ui/MD3Dialog.h"
#include "utils/Fonts.h"
#include "utils/Icons.h"

namespace
{
// Fixed padding: left text margin (48) + timing indicator area (48+16+16) + gap (4)
constexpr float kTextPadTotal = 132.0f;
constexpr float kMinTextScale = 0.7f;
constexpr int kAbsoluteMinW = 400;

float measureTextWidth(const juce::Font &font, const juce::String &text)
{
    if (text.isEmpty())
        return 0.0f;
    juce::GlyphArrangement ga;
    ga.addLineOfText(font, text, 0.0f, 0.0f);
    return ga.getBoundingBox(0, -1, true).getWidth();
}

// Draw text with horizontal compression if too wide — never elides
void drawTextCompressed(juce::Graphics &g, const juce::String &text,
                        juce::Rectangle<float> bounds, const juce::Font &font,
                        juce::Justification just)
{
    if (text.isEmpty())
        return;

    const float textW = measureTextWidth(font, text);
    const float availW = bounds.getWidth();

    g.setFont(font);

    if (textW <= availW) {
        g.drawText(text, bounds.toNearestInt(), just, false);
    } else {
        const float scale = availW / textW;
        juce::Graphics::ScopedSaveState sss(g);
        // Horizontal scale anchored at bounds left edge
        g.addTransform(juce::AffineTransform(scale, 0.0f, bounds.getX() * (1.0f - scale),
                                              0.0f, 1.0f, 0.0f));
        g.drawText(text, juce::Rectangle<float>(bounds.getX(), bounds.getY(),
                   textW + 2.0f, bounds.getHeight()).toNearestInt(),
                   just, false);
    }
}
} // namespace

KiraNastroEditor::KiraNastroEditor(KiraNastroProcessor &p) : AudioProcessorEditor(&p), m_audioProcessor(p)
{
    setLookAndFeel(&m_lookAndFeel);
    juce::LookAndFeel::setDefaultLookAndFeel(&m_lookAndFeel);
    m_lookAndFeel.setDarkMode(m_audioProcessor.getDarkMode());

    // Load brand logo from embedded SVG
    if (auto xml = juce::XmlDocument::parse(
            juce::String::fromUTF8(BinaryDataImages::icon_svg, (int)BinaryDataImages::icon_svgSize)))
    {
        m_logoDrawable = juce::Drawable::createFromSVG(*xml);
    }

    // Load chip icons
    reloadChipIcons();

    // Hamburger menu button (Material Symbols, white for nav bar)
    auto menuIcon = Icons::load(Icons::menuSvg, juce::Colours::white);
    m_menuButton = std::make_unique<juce::DrawableButton>("menu", juce::DrawableButton::ImageFitted);
    m_menuButton->setImages(menuIcon.get());
    m_menuButton->onClick = [this] {
        showMenu();
    };
    addAndMakeVisible(m_menuButton.get());

    // Timing indicator (always present)
    m_timingIndicator = std::make_unique<TimingIndicator>();
    addAndMakeVisible(m_timingIndicator.get());

    // Standalone-only debug controls.
    // NOTE: JUCE compiles shared plugin code with JUCE_STANDALONE_APPLICATION=1
    // even for VST3/AU builds (because all format flags are 1 in shared code).
    // Use wrapperType at runtime to correctly distinguish standalone from plugin.
    const bool isStandalone = (m_audioProcessor.wrapperType == juce::AudioProcessor::wrapperType_Standalone);

    if (isStandalone) {
        m_playbackControls = std::make_unique<PlaybackControls>(m_audioProcessor);
        addAndMakeVisible(m_playbackControls.get());

        m_progressSlider = std::make_unique<juce::Slider>();
        addAndMakeVisible(m_progressSlider.get());
        m_progressSlider->addListener(this);
        m_progressSlider->addMouseListener(this, false);
        m_progressSlider->setRange(0.0, 600.0, 0.1);
        m_progressSlider->setSliderStyle(juce::Slider::LinearHorizontal);
        m_progressSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
        m_progressSlider->setTextValueSuffix(" s");

        m_constrainer.setFixedHeight(268);
        setSize(800, 268);
    }
    else {
        m_constrainer.setFixedHeight(232);
        setSize(800, 232);
    }

    // Custom constrainer: width-only resize (height locked during user drag).
    // No corner resizer — the host/standalone wrapper provides its own resize
    // handles and respects our constrainer.
    setConstrainer(&m_constrainer);
    setResizable(true, false);
    enforceMinWidthForEntry();
    startTimerHz(30);

    // Show setup screen on first launch
    if (!m_audioProcessor.hasCompletedSetup())
        showSetupScreen();
}

KiraNastroEditor::~KiraNastroEditor()
{
    stopTimer();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

//==============================================================================
void KiraNastroEditor::paint(juce::Graphics &g)
{
    if (m_showingSetup) {
        // Setup screen paints itself; just clear background
        g.fillAll(m_lookAndFeel.background());
        return;
    }

    // 1. Background
    g.fillAll(m_lookAndFeel.background());

    const float w = static_cast<float>(getWidth());
    const float textAreaW = w - kTextPadTotal;

    // 2. Card (MD3 Filled Card — no shadow)
    g.setColour(m_lookAndFeel.cardFilled());
    g.fillRoundedRectangle(juce::Rectangle<float>(16.0f, 8.0f, w - 32.0f, 128.0f), 16.0f);

    // 3. Comment text (Sarasa 16pt, onSurfaceVariant) — horizontally compressed, never elided
    {
        const auto commentFont = juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(16.0f));
        g.setColour(m_lookAndFeel.onSurfaceVariant());
        drawTextCompressed(g, m_currentEntryComment,
                           juce::Rectangle<float>(48.0f, 28.0f, textAreaW, 20.0f),
                           commentFont, juce::Justification::left);
    }

    // 4. Entry text (Sarasa 48pt, primary) — horizontally compressed, never elided
    {
        const auto entryFont = juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(48.0f));
        g.setColour(m_lookAndFeel.primary());
        drawTextCompressed(g, m_currentEntryName,
                           juce::Rectangle<float>(48.0f, 56.0f, textAreaW, 60.0f),
                           entryFont, juce::Justification::left);
    }

    // 5. Info row chips
    {
        const juce::Font chipFont(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(16.0f));
        g.setFont(chipFont);
        const float chipH = 32.0f;
        const float chipY = 144.0f;
        const float iconSize = 14.0f;
        const float padH = 16.0f;
        const float iconTextGap = 8.0f;

        // Helper: measure text width via GlyphArrangement (avoids deprecated
        // getStringWidth)
        auto measureText = [](const juce::Font &font, const juce::String &text) -> float {
            juce::GlyphArrangement ga;
            ga.addLineOfText(font, text, 0.0f, 0.0f);
            return ga.getBoundingBox(0, -1, true).getWidth();
        };

        // a. Next Entry chip (left)
        {
            const float textW = measureText(chipFont, m_nextEntryName);
            const float chipW = padH + iconSize + iconTextGap + textW + padH;
            const juce::Rectangle<float> chipRect(16.0f, chipY, chipW, chipH);

            g.setColour(m_lookAndFeel.secondaryContainer());
            g.fillRoundedRectangle(chipRect, 16.0f);

            if (m_nextIcon) {
                m_nextIcon->drawWithin(
                    g, juce::Rectangle<float>(16.0f + padH, chipY + (chipH - iconSize) / 2.0f, iconSize, iconSize),
                    juce::RectanglePlacement::centred, 1.0f);
            }

            g.setColour(m_lookAndFeel.onSecondaryContainer());
            g.drawText(m_nextEntryName,
                       juce::Rectangle<float>(16.0f + padH + iconSize + iconTextGap, chipY, textW + 2.0f, chipH)
                           .toNearestInt(),
                       juce::Justification::centredLeft, false);
        }

        // b. Progress chip (right-aligned)
        {
            const int total = m_audioProcessor.m_totalEntries.load();
            const int current = std::min(m_lastEntryIndex + 1, total);
            const juce::String progressStr = juce::String(current) + " / " + juce::String(total);
            const float textW = measureText(chipFont, progressStr);
            const float chipW = padH + iconSize + iconTextGap + textW + padH;
            const float chipX = w - 16.0f - chipW;
            const juce::Rectangle<float> chipRect(chipX, chipY, chipW, chipH);

            g.setColour(m_lookAndFeel.secondaryContainer());
            g.fillRoundedRectangle(chipRect, 16.0f);

            if (m_progressIcon) {
                m_progressIcon->drawWithin(
                    g, juce::Rectangle<float>(chipX + padH, chipY + (chipH - iconSize) / 2.0f, iconSize, iconSize),
                    juce::RectanglePlacement::centred, 1.0f);
            }

            g.setColour(m_lookAndFeel.onSecondaryContainer());
            g.drawText(progressStr,
                       juce::Rectangle<float>(chipX + padH + iconSize + iconTextGap, chipY, textW + 2.0f, chipH)
                           .toNearestInt(),
                       juce::Justification::centredLeft, false);
        }
    }

    // 6. Nav bar
    {
        const float navBarY = 192.0f;
        const float navBarH = 40.0f;
        g.setColour(m_lookAndFeel.navBar());
        g.fillRect(0.0f, navBarY, w, navBarH);

        // Font setup
        const auto navFont = juce::Font(juce::FontOptions(Fonts::getLexendRegular()).withPointHeight(14.0f));
        const juce::String brandText = "KiraNastro inst.";

        // Measure text via GlyphArrangement to get accurate pixel height/width
        juce::GlyphArrangement ga;
        ga.addLineOfText(navFont, brandText, 0.0f, 0.0f);
        const auto textBounds = ga.getBoundingBox(0, -1, true);
        const float textW = textBounds.getWidth();

        const float logoSize = 24.0f;
        const float spacing = 2.0f;

        // Starting X to keep them grouped (starting from X=10)
        float currentX = 10.0f;

        // Logo icon (vertically centred in 40px bar)
        if (m_logoDrawable) {
            m_logoDrawable->drawWithin(
                g, juce::Rectangle<float>(currentX, navBarY + (navBarH - logoSize) / 2.0f, logoSize, logoSize),
                juce::RectanglePlacement::centred, 1.0f);
        }

        currentX += logoSize + spacing;

        // "KiraNastro inst." vertically centred relative to nav bar
        g.setColour(m_lookAndFeel.onNavBar());
        g.setFont(navFont);
        g.drawText(brandText, juce::Rectangle<float>(currentX, navBarY, textW + 2.0f, navBarH).toNearestInt(),
                   juce::Justification::centredLeft, false);
    }

    if (m_playbackControls) {
        // 7. Debug area background (y=232)
        g.setColour(juce::Colour(0xFFDBEAFE));
        g.fillRect(0, 232, getWidth(), getHeight() - 232);
    }
}

void KiraNastroEditor::resized()
{
    if (m_showingSetup && m_setupScreen) {
        m_setupScreen->setBounds(getLocalBounds());
        return;
    }

    const int w = getWidth();

    // TimingIndicator: bottom-right inside card padding
    m_timingIndicator->setBounds(w - 80, 84, 48, 32);

    // Hamburger menu button: 24x24 (same as nav bar logo), right-aligned,
    // vertically centred Nav bar: y=192, h=40 → icon y = 192 + (40-24)/2 = 200
    m_menuButton->setBounds(w - 32, 200, 24, 24);

    if (m_playbackControls) {
        // Debug area: y=232..296 (64px). Centre controls + slider on same row.
        const int debugY = 232;
        const int debugH = getHeight() - debugY;
        const int rowH = 28; // match kHitSize
        const int rowY = debugY + (debugH - rowH) / 2;
        m_playbackControls->setBounds(8, rowY, 100, rowH);
        m_progressSlider->setBounds(108, rowY, getWidth() - 128, rowH);
        m_progressSlider->setColour(juce::Slider::textBoxTextColourId, KiraNastroLookAndFeel::md3Primary);
    }
}

//==============================================================================
void KiraNastroEditor::timerCallback()
{
    // Skip main view updates while showing setup screen
    if (m_showingSetup)
        return;

    auto info = m_audioProcessor.getCurrentEntryInfo();
    bool changed = (info.index != m_lastEntryIndex || info.name != m_currentEntryName);

    if (changed) {
        m_lastEntryIndex = info.index;
        m_currentEntryName = info.name;
        m_currentEntryComment = info.comment;

        auto nextInfo = m_audioProcessor.getNextEntryInfo();
        m_nextEntryName = nextInfo.name;
        m_nextEntryComment = nextInfo.comment;

        enforceMinWidthForEntry();
        repaint();
    }

    const bool pastEnd = (m_audioProcessor.m_totalEntries.load() > 0 &&
                          m_lastEntryIndex >= m_audioProcessor.m_totalEntries.load());
    m_timingIndicator->setProgress(pastEnd ? 0.0f : m_audioProcessor.m_bgmLoopProgress.load(std::memory_order_relaxed));
    m_timingIndicator->setVisible(!pastEnd);

    const float uStart = m_audioProcessor.m_utteranceStartFraction.load(std::memory_order_relaxed);
    const float uEnd   = m_audioProcessor.m_utteranceEndFraction.load(std::memory_order_relaxed);
    // Use bitwise comparison to avoid -Wfloat-equal; we want exact change detection.
    auto floatBitsEqual = [](float a, float b) noexcept -> bool {
        uint32_t ai, bi;
        std::memcpy(&ai, &a, 4);
        std::memcpy(&bi, &b, 4);
        return ai == bi;
    };
    if (!floatBitsEqual(uStart, m_cachedUtteranceStartFraction) ||
        !floatBitsEqual(uEnd, m_cachedUtteranceEndFraction)) {
        m_cachedUtteranceStartFraction = uStart;
        m_cachedUtteranceEndFraction   = uEnd;
        m_timingIndicator->setStageBoundaries(uStart, uEnd);
    }

    if (m_progressSlider && m_audioProcessor.isBGMLoaded()) {
        const double currentPos = m_audioProcessor.m_projectPlayPositionSeconds.load();
        // Don't overwrite the slider while the user is dragging it
        if (m_progressSlider->getThumbBeingDragged() == -1)
            m_progressSlider->setValue(currentPos, juce::dontSendNotification);

        const double bgmLength = m_audioProcessor.getBGMLengthSeconds();
        const int totalEntries = m_audioProcessor.m_totalEntries.load();
        // Range = all entries + 1 extra cycle (to test past-last behavior)
        const double projectLength = bgmLength * (totalEntries + 1);
        const double currentMax = m_progressSlider->getMaximum();
        if (bgmLength > 0 && std::abs(currentMax - projectLength) > 0.1) {
            m_progressSlider->setRange(0.0, projectLength, 0.1);
        }
    }
}

void KiraNastroEditor::sliderValueChanged(juce::Slider *slider)
{
    if (m_progressSlider && slider == m_progressSlider.get())
        m_audioProcessor.seekBGM(slider->getValue());
}

void KiraNastroEditor::mouseDown(const juce::MouseEvent &event)
{
    // Right-click on progress slider → seek by entry index
    if (m_progressSlider && event.mods.isPopupMenu() &&
        event.originalComponent == m_progressSlider.get()) {
            auto *aw = new juce::AlertWindow("Seek to Entry", "Enter entry index (0-based):",
                                              juce::MessageBoxIconType::NoIcon);
            aw->addTextEditor("entry", juce::String(m_audioProcessor.m_currentEntryIndex.load()));
            aw->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
            aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
            aw->enterModalState(true, juce::ModalCallbackFunction::create(
                [this, aw](int result) {
                    if (result == 1) {
                        auto text = aw->getTextEditorContents("entry");
                        int idx = text.getIntValue();
                        double cycle = m_audioProcessor.getBGMLengthSeconds();
                        if (cycle > 0.0)
                            m_audioProcessor.seekBGM(idx * cycle);
                    }
                    delete aw;
                }), true);
            return;
    }
    juce::Component::mouseDown(event);
}

void KiraNastroEditor::reloadChipIcons()
{
    // Chip icons tinted with current onSecondaryContainer color
    m_nextIcon = Icons::load(Icons::arrowRightSvg, m_lookAndFeel.onSecondaryContainer());
    m_progressIcon = Icons::load(Icons::percentSvg, m_lookAndFeel.onSecondaryContainer());
}

void KiraNastroEditor::showSetupScreen()
{
    if (m_showingSetup)
        return;

    m_showingSetup = true;

    m_setupScreen = std::make_unique<ProjectSetupScreen>();
    m_setupScreen->onComplete = [this](const ProjectSetupScreen::SetupResult &result) {
        applySetupResult(result);
    };

    // Pre-populate from current processor state
    const bool reclistBuiltin = (m_audioProcessor.getReclistSource() ==
                                 KiraNastroProcessor::ResourceSource::Builtin);
    m_setupScreen->setInitialReclistSelection(
        reclistBuiltin,
        m_audioProcessor.getBuiltinReclistId(),
        juce::File(m_audioProcessor.hasCompletedSetup() ? "" : ""));

    const bool bgmBuiltin = (m_audioProcessor.getBGMSource() ==
                             KiraNastroProcessor::ResourceSource::Builtin);
    m_setupScreen->setInitialBGMSelection(
        bgmBuiltin,
        m_audioProcessor.getBuiltinBGMTempo(),
        m_audioProcessor.getBuiltinBGMKey());

    addAndMakeVisible(m_setupScreen.get());

    // Hide standalone controls during setup
    if (m_playbackControls)
        m_playbackControls->setVisible(false);
    if (m_progressSlider)
        m_progressSlider->setVisible(false);

    // Keep current width, set 288px height during setup (compact design)
    m_constrainer.setFixedHeight(288);
    setSize(getWidth(), 288);
}

void KiraNastroEditor::hideSetupScreen()
{
    if (!m_showingSetup)
        return;

    m_showingSetup = false;
    m_setupScreen.reset();

    // Restore normal window height, keep current width
    const bool isStandalone = (m_audioProcessor.wrapperType == juce::AudioProcessor::wrapperType_Standalone);
    const int h = isStandalone ? 268 : 232;
    m_constrainer.setFixedHeight(h);
    setSize(getWidth(), h);

    // Show standalone controls again
    if (m_playbackControls)
        m_playbackControls->setVisible(true);
    if (m_progressSlider)
        m_progressSlider->setVisible(true);
}

void KiraNastroEditor::applySetupResult(const ProjectSetupScreen::SetupResult &result)
{
    bool reclistOk = false;
    bool bgmOk = false;

    if (result.reclistSource == ProjectSetupScreen::SetupResult::Source::Builtin) {
        reclistOk = m_audioProcessor.loadBuiltinReclist(result.builtinReclistId);
    } else {
        reclistOk = m_audioProcessor.loadReclist(result.customReclistFile);
    }

    if (!reclistOk) {
        MD3Dialog::show("Failed to Load Reclist",
                        "The selected reclist could not be loaded. "
                        "Please try a different file.",
                        "OK", this);
        return;
    }

    if (result.bgmSource == ProjectSetupScreen::SetupResult::Source::Builtin) {
        auto bgmResult = m_audioProcessor.loadBuiltinBGM(result.builtinBGMTempo, result.builtinBGMKey);
        bgmOk = (bgmResult == KiraNastroProcessor::BGMLoadResult::Success);
        if (!bgmOk) {
            MD3Dialog::show("Failed to Load BGM",
                            "The built-in BGM could not be loaded. "
                            "This is unexpected — please try another tempo/key combination.",
                            "OK", this);
            return;
        }
    } else {
        auto bgmResult = m_audioProcessor.loadGuideBGM(result.customBGMFile);
        bgmOk = (bgmResult == KiraNastroProcessor::BGMLoadResult::Success);
        if (!bgmOk) {
            juce::String msg;
            switch (bgmResult) {
                case KiraNastroProcessor::BGMLoadResult::Success:
                    break; // unreachable (bgmOk == false), but silences -Wswitch-enum
                case KiraNastroProcessor::BGMLoadResult::AudioLoadFailed:
                    msg = "The audio file could not be loaded. "
                          "It may be corrupted or in an unsupported format.";
                    break;
                case KiraNastroProcessor::BGMLoadResult::TimingFileMissing:
                    msg = "No timing description file (.txt) was found alongside the audio file. "
                          "Place the OREMO timing file in the same folder with the same base name.";
                    break;
                case KiraNastroProcessor::BGMLoadResult::TimingFileInvalid:
                    msg = "The timing description file has an unrecognized format. "
                          "KiraNastro inst. requires an OREMO-standard 6-row timing file.";
                    break;
            }
            MD3Dialog::show("Failed to Load BGM", msg, "OK", this);
            return;
        }
    }

    m_audioProcessor.setHasCompletedSetup(true);
    hideSetupScreen();
}

void KiraNastroEditor::enforceMinWidthForEntry()
{
    // Measure entry text at full font size (48pt Sarasa)
    const auto entryFont = juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(48.0f));
    const float entryTextW = measureTextWidth(entryFont, m_currentEntryName);

    // Measure comment text at full font size (16pt Sarasa)
    const auto commentFont = juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(16.0f));
    const float commentTextW = measureTextWidth(commentFont, m_currentEntryComment);

    const float maxTextW = std::max(entryTextW, commentTextW);

    // Min width: text should not be compressed below 70% of its natural width
    int minW = std::max(kAbsoluteMinW, static_cast<int>(std::ceil(kMinTextScale * maxTextW + kTextPadTotal)));

    // Max width: 70% of screen width
    int maxW = 1600;
    if (auto *display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
        maxW = static_cast<int>(0.7f * static_cast<float>(display->userArea.getWidth()));
    maxW = std::max(maxW, minW);

    // Only constrain width — never touch height constraints, so screen
    // transitions (setup ↔ main) can freely setSize to a different height.
    if (auto *c = getConstrainer()) {
        c->setMinimumWidth(minW);
        c->setMaximumWidth(maxW);
    }

    if (getWidth() < minW)
        setSize(minW, getHeight());
}

void KiraNastroEditor::showMenu()
{
    juce::PopupMenu menu;
    menu.setLookAndFeel(&m_lookAndFeel);
    menu.addItem(5, "Project Setup...");
    menu.addSeparator();
    const bool canExport = m_audioProcessor.isBGMLoaded() && m_audioProcessor.m_totalEntries.load() > 0;
    menu.addItem(4, "Export KiraWavTar Desc...", canExport);
    menu.addSeparator();
    const bool dark = m_lookAndFeel.getDarkMode();
    menu.addItem(3, dark ? "Switch to Light Mode" : "Switch to Dark Mode");

    menu.showMenuAsync(
        juce::PopupMenu::Options()
            .withTargetComponent(m_menuButton.get())
            .withMinimumWidth(200)
            .withMaximumNumColumns(1),
        [this](int result) {
            if (result == 5) {
                showSetupScreen();
            }
            else if (result == 3) {
                const bool newDark = !m_lookAndFeel.getDarkMode();
                m_lookAndFeel.setDarkMode(newDark);
                m_audioProcessor.setDarkMode(newDark);
                reloadChipIcons();
                repaint();
            }
            else if (result == 4) {
                auto params = m_audioProcessor.getDescExportParams();
                if (!params.isValid()) {
                    MD3Dialog::show("Cannot Export",
                                    "Please load both a reclist and a guide BGM before exporting.",
                                    "OK", this);
                    return;
                }

                m_descChooser =
                    std::make_unique<juce::FileChooser>("Save KiraWavTar Description File",
                                                        juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                                                            .getChildFile("recording.kirawavtar-desc.json"),
                                                        "*.kirawavtar-desc.json");
                m_descChooser->launchAsync(
                    juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                    [this, params](const juce::FileChooser &f) {
                        auto file = f.getResult();
                        if (!file.getFullPathName().isEmpty()) {
                            // Ensure the file has the correct compound extension.
                            // If the user typed "foo.json", replace with
                            // "foo.kirawavtar-desc.json". If they typed "foo" (no
                            // extension), append ".kirawavtar-desc.json".
                            if (!file.getFileName().endsWith(".kirawavtar-desc.json")) {
                                juce::String base = file.getFileName();
                                if (base.endsWith(".json"))
                                    base = base.dropLastCharacters(5);
                                file = file.getParentDirectory().getChildFile(base + ".kirawavtar-desc.json");
                            }
                            juce::String errMsg;
                            bool ok = LabelExporter::exportToFile(file, params, errMsg);
                            if (ok) {
                                const int n = static_cast<int>(params.entryNames.size());
                                const double totalSec = n * params.blockDurationSec + params.recordingStartOffsetSec +
                                                        params.recordingWindowDurationSec;
                                const int totalMin = static_cast<int>(totalSec / 60.0);
                                const double remSec = totalSec - totalMin * 60.0;
                                const juce::String totalStr = juce::String::formatted("%d:%05.2f", totalMin, remSec);
                                MD3Dialog::show(
                                    "Export Successful",
                                    "Description file saved.\n\n"
                                    "Next steps:\n"
                                    "1. In your DAW, export your recording as a WAV file\n"
                                    "   starting from time 0:00.0 to at least " +
                                        totalStr +
                                        ".\n"
                                        "2. Place the desc file in the same folder as your "
                                        "WAV,\n"
                                        "   named <yourwav>.kirawavtar-desc.json.\n"
                                        "3. Open KiraWavTar and select the WAV file to "
                                        "extract.",
                                    "OK", this);
                            }
                            else {
                                MD3Dialog::show("Export Failed", errMsg, "OK", this);
                            }
                        }
                        m_descChooser.reset();
                    });
            }
        });
}
