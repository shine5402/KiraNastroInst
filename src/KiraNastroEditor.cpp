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

KiraNastroEditor::KiraNastroEditor(KiraNastroProcessor &p) : AudioProcessorEditor(&p), m_audioProcessor(p)
{
    setLookAndFeel(&m_lookAndFeel);
    juce::LookAndFeel::setDefaultLookAndFeel(&m_lookAndFeel);

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
        m_progressSlider->setRange(0.0, 600.0, 0.1);
        m_progressSlider->setSliderStyle(juce::Slider::LinearHorizontal);
        m_progressSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
        m_progressSlider->setTextValueSuffix(" s");

        setSize(800, 360);
    }
    else {
        setSize(800, 232);
    }

    startTimerHz(30);
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
    // 1. Background
    g.fillAll(m_lookAndFeel.background());

    // 2. Card (MD3 Filled Card — no shadow)
    g.setColour(m_lookAndFeel.cardFilled());
    g.fillRoundedRectangle(juce::Rectangle<float>(16.0f, 8.0f, 768.0f, 128.0f), 16.0f);

    // 3. Comment text (Sarasa 16pt, onSurfaceVariant)
    g.setColour(m_lookAndFeel.onSurfaceVariant());
    g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(16.0f)));
    g.drawFittedText(m_currentEntryComment, juce::Rectangle<int>(48, 28, 668, 20), juce::Justification::left, 1);

    // 4. Entry text (Sarasa 48pt, primary)
    g.setColour(m_lookAndFeel.primary());
    g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(48.0f)));
    g.drawFittedText(m_currentEntryName, juce::Rectangle<int>(48, 56, 668, 60), juce::Justification::left, 1);

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
            const int current = m_lastEntryIndex + 1;
            const int total = m_audioProcessor.m_totalEntries.load();
            const juce::String progressStr = juce::String(current) + " / " + juce::String(total);
            const float textW = measureText(chipFont, progressStr);
            const float chipW = padH + iconSize + iconTextGap + textW + padH;
            const float chipX = 784.0f - chipW;
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
        g.fillRect(0.0f, navBarY, 800.0f, navBarH);

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
        g.fillRect(0, 232, 800, getHeight() - 232);
    }
}

void KiraNastroEditor::resized()
{
    // TimingIndicator: bottom-right inside card padding
    m_timingIndicator->setBounds(720, 84, 48, 32);

    // Hamburger menu button: 24x24 (same as nav bar logo), right-aligned,
    // vertically centred Nav bar: y=192, h=40 → icon y = 192 + (40-24)/2 = 200; x
    // = 800 - 8(pad) - 24 = 768
    m_menuButton->setBounds(768, 200, 24, 24);

    if (m_playbackControls) {
        m_progressSlider->setBounds(20, 242, getWidth() - 40, 32);
        m_progressSlider->setColour(juce::Slider::textBoxTextColourId, KiraNastroLookAndFeel::md3Primary);
        m_playbackControls->setBounds(0, 278, getWidth(), 64);
    }
}

//==============================================================================
void KiraNastroEditor::timerCallback()
{
    auto info = m_audioProcessor.getCurrentEntryInfo();
    bool changed = (info.index != m_lastEntryIndex || info.name != m_currentEntryName);

    if (changed) {
        m_lastEntryIndex = info.index;
        m_currentEntryName = info.name;
        m_currentEntryComment = info.comment;

        auto nextInfo = m_audioProcessor.getNextEntryInfo();
        m_nextEntryName = nextInfo.name;
        m_nextEntryComment = nextInfo.comment;

        repaint();
    }

    m_timingIndicator->setProgress(m_audioProcessor.m_bgmLoopProgress.load(std::memory_order_relaxed));

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
        const double projectLength = bgmLength * 10.0;
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

void KiraNastroEditor::reloadChipIcons()
{
    // Chip icons tinted with current onSecondaryContainer color
    m_nextIcon = Icons::load(Icons::arrowRightSvg, m_lookAndFeel.onSecondaryContainer());
    m_progressIcon = Icons::load(Icons::percentSvg, m_lookAndFeel.onSecondaryContainer());
}

void KiraNastroEditor::showMenu()
{
    juce::PopupMenu menu;
    menu.setLookAndFeel(&m_lookAndFeel);
    menu.addItem(1, "Load Reclist...");
    menu.addItem(2, "Load BGM...");
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
            if (result == 1) {
                m_reclistChooser = std::make_unique<juce::FileChooser>("Select Reclist File", juce::File(), "*.txt");
                m_reclistChooser->launchAsync(juce::FileBrowserComponent::openMode |
                                                  juce::FileBrowserComponent::canSelectFiles,
                                              [this](const juce::FileChooser &f) {
                                                  auto file = f.getResult();
                                                  if (file.exists()) {
                                                      if (!m_audioProcessor.loadReclist(file))
                                                          MD3Dialog::show("Failed to Load Reclist",
                                                                          "The selected file could not be read as a reclist. "
                                                                          "It may be in an unsupported encoding or format. "
                                                                          "KiraNastro inst. supports UTF-8 and Shift-JIS text files.",
                                                                          "OK", this);
                                                  }
                                                  m_reclistChooser.reset();
                                              });
            }
            else if (result == 2) {
                m_bgmChooser = std::make_unique<juce::FileChooser>("Select BGM File", juce::File(), "*.wav");
                m_bgmChooser->launchAsync(juce::FileBrowserComponent::openMode |
                                              juce::FileBrowserComponent::canSelectFiles,
                                          [this](const juce::FileChooser &f) {
                                              auto file = f.getResult();
                                              if (file.exists()) {
                                                  auto bgmResult = m_audioProcessor.loadGuideBGM(file);
                                                  juce::String msg;
                                                  switch (bgmResult) {
                                                      case KiraNastroProcessor::BGMLoadResult::Success:
                                                          break;
                                                      case KiraNastroProcessor::BGMLoadResult::WavLoadFailed:
                                                          msg = "The selected WAV file could not be loaded. "
                                                                "It may be corrupted or in an unsupported format.";
                                                          break;
                                                      case KiraNastroProcessor::BGMLoadResult::TimingFileMissing:
                                                          msg = "No timing description file was found alongside the WAV file. "
                                                                "Place the OREMO timing file (.txt) in the same folder "
                                                                "with the same base name (e.g. Jazz-100-A.txt).";
                                                          break;
                                                      case KiraNastroProcessor::BGMLoadResult::TimingFileInvalid:
                                                          msg = "The timing description file has an unrecognized format. "
                                                                "KiraNastro inst. requires an OREMO-standard 6-row timing file. "
                                                                "Try regenerating it with korede (bundled with OREMO).";
                                                          break;
                                                  }
                                                  if (msg.isNotEmpty())
                                                      MD3Dialog::show("Failed to Load BGM", msg, "OK", this);
                                              }
                                              m_bgmChooser.reset();
                                          });
            }
            else if (result == 3) {
                m_lookAndFeel.setDarkMode(!m_lookAndFeel.getDarkMode());
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
