// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ProjectSetupScreen.h"

#include "BinaryDataImages.h"
#include "../data/BuiltinResources.h"
#include "../utils/Fonts.h"
#include "../utils/Icons.h"

// Layout constants
namespace
{
constexpr int kEdgePad    = 12;
constexpr int kGap        = 8;
constexpr int kNavBarH    = 40;
constexpr int kBtnColW    = 56;  // MD3e Icon Button M size
constexpr int kComboH     = 52;
constexpr int kCustomBtnH = 34;
constexpr int kBadgeSize  = 26;
constexpr int kInnerPad   = 12;
constexpr int kCardRadius = 20;

// 3 columns: [Card1] [gap] [Card2] [gap] [BtnCol]
// Total = edgePad*2 + card*2 + gap*2 + btnColW
int cardWidth(int totalW)
{
    return (totalW - 2 * kEdgePad - 2 * kGap - kBtnColW) / 2;
}
} // namespace

ProjectSetupScreen::ProjectSetupScreen()
{
    // Load brand logo from embedded SVG
    if (auto xml = juce::XmlDocument::parse(
            juce::String::fromUTF8(BinaryDataImages::icon_svg,
                                   static_cast<int>(BinaryDataImages::icon_svgSize))))
    {
        m_logoDrawable = juce::Drawable::createFromSVG(*xml);
    }

    // Load check icon (white for the filled primary button)
    m_checkIcon = Icons::load(Icons::checkSvg, juce::Colours::white);

    buildUI();
}

ProjectSetupScreen::~ProjectSetupScreen() = default;

KiraNastroLookAndFeel *ProjectSetupScreen::getLAF() const
{
    return dynamic_cast<KiraNastroLookAndFeel *>(&getLookAndFeel());
}

void ProjectSetupScreen::buildUI()
{
    // --- Reclist combo ---
    m_reclistCombo = std::make_unique<Md3ExpressiveComboBox>("Record List");
    const auto &reclists = BuiltinResources::getReclists();
    for (const auto &r : reclists) {
        juce::String supporting = juce::String(r.entryCount) + " entries; "
                                + juce::String(r.moraCount) + "-mora, "
                                + juce::String::fromUTF8(r.credits);
        if (r.hasComment)
            supporting += " (romaji comment)";
        m_reclistCombo->addItem(r.id, juce::String::fromUTF8(r.displayName), supporting);
    }
    m_reclistCombo->setSelectedId(0, juce::dontSendNotification);
    addAndMakeVisible(m_reclistCombo.get());

    // --- Reclist custom file button ---
    m_reclistCustomButton = std::make_unique<juce::TextButton>("Custom reclist file...");
    m_reclistCustomButton->onClick = [this] {
        m_reclistChooser = std::make_unique<juce::FileChooser>("Select Reclist File",
                                                               juce::File(), "*.txt");
        m_reclistChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser &f) {
                auto file = f.getResult();
                if (file.existsAsFile()) {
                    m_customReclistFile = file;
                    m_usingCustomReclist = true;
                    updateReclistPanel();
                }
                m_reclistChooser.reset();
            });
    };
    addAndMakeVisible(m_reclistCustomButton.get());

    // --- Reclist custom card (shown when file selected) ---
    m_reclistCustomCard = std::make_unique<juce::Component>();
    m_reclistCustomCard->setVisible(false);
    addAndMakeVisible(m_reclistCustomCard.get());

    m_reclistCustomLabel.setJustificationType(juce::Justification::centredLeft);
    m_reclistCustomLabel.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(14.0f)));
    m_reclistCustomLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF364379)); // onPrimaryContainer
    m_reclistCustomLabel.setBorderSize(juce::BorderSize<int>(0));
    m_reclistCustomLabel.setInterceptsMouseClicks(false, false);
    m_reclistCustomCard->addAndMakeVisible(m_reclistCustomLabel);

    m_reclistClearButton = std::make_unique<juce::DrawableButton>("clearReclist",
                                                                   juce::DrawableButton::ImageFitted);
    {
        auto normal  = Icons::load(Icons::closeSvg, juce::Colour(0xFF45464F).withAlpha(0.55f));
        auto hover   = Icons::load(Icons::closeSvg, juce::Colour(0xFF45464F).withAlpha(0.80f));
        auto pressed = Icons::load(Icons::closeSvg, juce::Colour(0xFF45464F));
        m_reclistClearButton->setImages(normal.get(), hover.get(), pressed.get());
    }
    m_reclistClearButton->onClick = [this] {
        m_usingCustomReclist = false;
        m_customReclistFile = juce::File();
        updateReclistPanel();
    };
    m_reclistClearButton->onStateChange = [this] { repaint(); };
    m_reclistCustomCard->addAndMakeVisible(m_reclistClearButton.get());

    // --- Tempo combo ---
    m_tempoCombo = std::make_unique<Md3ExpressiveComboBox>("Tempo");
    for (int tempo : BuiltinResources::getTempos())
        m_tempoCombo->addItem(tempo, juce::String(tempo) + " BPM");
    m_tempoCombo->setSelectedId(120, juce::dontSendNotification);
    addAndMakeVisible(m_tempoCombo.get());

    // --- Key combo ---
    m_keyCombo = std::make_unique<Md3ExpressiveComboBox>("Key");
    const auto &keys = BuiltinResources::getKeys();
    for (int i = 0; i < static_cast<int>(keys.size()); ++i)
        m_keyCombo->addItem(i, keys[static_cast<size_t>(i)]);
    m_keyCombo->setSelectedId(7, juce::dontSendNotification);
    addAndMakeVisible(m_keyCombo.get());

    // --- BGM custom file button ---
    m_bgmCustomButton = std::make_unique<juce::TextButton>("Custom BGM file...");
    m_bgmCustomButton->onClick = [this] {
        m_bgmChooser = std::make_unique<juce::FileChooser>("Select BGM Audio File",
                                                            juce::File(), "*.opus;*.wav;*.ogg;*.flac;*.mp3");
        m_bgmChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser &f) {
                auto file = f.getResult();
                if (file.existsAsFile()) {
                    m_customBGMFile = file;
                    m_usingCustomBGM = true;
                    updateBGMPanel();
                }
                m_bgmChooser.reset();
            });
    };
    addAndMakeVisible(m_bgmCustomButton.get());

    // --- BGM custom card ---
    m_bgmCustomCard = std::make_unique<juce::Component>();
    m_bgmCustomCard->setVisible(false);
    addAndMakeVisible(m_bgmCustomCard.get());

    m_bgmCustomLabel.setJustificationType(juce::Justification::centredLeft);
    m_bgmCustomLabel.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(14.0f)));
    m_bgmCustomLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF364379)); // onPrimaryContainer
    m_bgmCustomLabel.setBorderSize(juce::BorderSize<int>(0));
    m_bgmCustomLabel.setInterceptsMouseClicks(false, false);
    m_bgmCustomCard->addAndMakeVisible(m_bgmCustomLabel);

    m_bgmClearButton = std::make_unique<juce::DrawableButton>("clearBGM",
                                                               juce::DrawableButton::ImageFitted);
    {
        auto normal  = Icons::load(Icons::closeSvg, juce::Colour(0xFF45464F).withAlpha(0.55f));
        auto hover   = Icons::load(Icons::closeSvg, juce::Colour(0xFF45464F).withAlpha(0.80f));
        auto pressed = Icons::load(Icons::closeSvg, juce::Colour(0xFF45464F));
        m_bgmClearButton->setImages(normal.get(), hover.get(), pressed.get());
    }
    m_bgmClearButton->onClick = [this] {
        m_usingCustomBGM = false;
        m_customBGMFile = juce::File();
        updateBGMPanel();
    };
    m_bgmClearButton->onStateChange = [this] { repaint(); };
    m_bgmCustomCard->addAndMakeVisible(m_bgmClearButton.get());

    // --- OK button (we paint the circle ourselves; button handles hit-testing) ---
    m_fabButton = std::make_unique<juce::DrawableButton>("confirm", juce::DrawableButton::ImageFitted);
    m_fabButton->onClick = [this] { onConfirm(); };
    m_fabButton->onStateChange = [this] { repaint(); };
    addAndMakeVisible(m_fabButton.get());
}

void ProjectSetupScreen::setInitialReclistSelection(bool isBuiltin, int builtinId,
                                                    const juce::File &customFile)
{
    if (isBuiltin) {
        m_usingCustomReclist = false;
        m_reclistCombo->setSelectedId(builtinId, juce::dontSendNotification);
    } else {
        m_usingCustomReclist = true;
        m_customReclistFile = customFile;
    }
    updateReclistPanel();
}

void ProjectSetupScreen::setInitialBGMSelection(bool isBuiltin, int tempo, const juce::String &key,
                                                 const juce::File &customFile)
{
    if (isBuiltin) {
        m_usingCustomBGM = false;
        m_tempoCombo->setSelectedId(tempo, juce::dontSendNotification);
        const auto &keys = BuiltinResources::getKeys();
        for (int i = 0; i < static_cast<int>(keys.size()); ++i) {
            if (keys[static_cast<size_t>(i)] == key) {
                m_keyCombo->setSelectedId(i, juce::dontSendNotification);
                break;
            }
        }
    } else {
        m_usingCustomBGM = true;
        m_customBGMFile = customFile;
    }
    updateBGMPanel();
}

void ProjectSetupScreen::updateReclistPanel()
{
    const bool custom = m_usingCustomReclist;
    m_reclistCombo->setVisible(!custom);
    m_reclistCustomButton->setVisible(!custom);
    m_reclistCustomCard->setVisible(custom);
    if (custom)
        m_reclistCustomLabel.setText(m_customReclistFile.getFileName(), juce::dontSendNotification);
    resized();
    repaint();
}

void ProjectSetupScreen::updateBGMPanel()
{
    const bool custom = m_usingCustomBGM;
    m_tempoCombo->setVisible(!custom);
    m_keyCombo->setVisible(!custom);
    m_bgmCustomButton->setVisible(!custom);
    m_bgmCustomCard->setVisible(custom);
    if (custom)
        m_bgmCustomLabel.setText(m_customBGMFile.getFileName(), juce::dontSendNotification);
    resized();
    repaint();
}

void ProjectSetupScreen::onConfirm()
{
    if (!onComplete)
        return;

    SetupResult result;

    if (m_usingCustomReclist) {
        result.reclistSource = SetupResult::Source::Custom;
        result.customReclistFile = m_customReclistFile;
    } else {
        result.reclistSource = SetupResult::Source::Builtin;
        result.builtinReclistId = m_reclistCombo->getSelectedId();
    }

    if (m_usingCustomBGM) {
        result.bgmSource = SetupResult::Source::Custom;
        result.customBGMFile = m_customBGMFile;
    } else {
        result.bgmSource = SetupResult::Source::Builtin;
        result.builtinBGMTempo = m_tempoCombo->getSelectedId();
        const auto &keys = BuiltinResources::getKeys();
        const int keyIdx = m_keyCombo->getSelectedId();
        if (keyIdx >= 0 && keyIdx < static_cast<int>(keys.size()))
            result.builtinBGMKey = keys[static_cast<size_t>(keyIdx)];
        else
            result.builtinBGMKey = "G";
    }

    onComplete(result);
}

//==============================================================================
void ProjectSetupScreen::paint(juce::Graphics &g)
{
    auto *laf = getLAF();
    const auto bgColor         = laf ? laf->background()            : juce::Colour(0xFFFBF8FF);
    const auto panelColor      = laf ? laf->surfaceContainerHigh()  : juce::Colour(0xFFE9E7EF);
    const auto navBarColor     = laf ? laf->navBar()                : juce::Colour(0xFF3755C3);
    const auto onNavBar        = laf ? laf->onNavBar()              : juce::Colours::white;
    const auto onSurface       = laf ? laf->onSurface()             : juce::Colour(0xFF1A1B21);
    const auto primaryColor    = laf ? laf->primary()               : juce::Colour(0xFF4E5B92);
    const auto primaryCont     = laf ? laf->primaryContainer()      : juce::Colour(0xFFDDE1FF);
    const auto onPrimaryCont   = laf ? laf->onPrimaryContainer()    : juce::Colour(0xFF364379);

    const int w = getWidth();
    const int h = getHeight();
    const int navBarY = h - kNavBarH;
    const int cw = cardWidth(w);

    // --- Background ---
    g.fillAll(bgColor);

    // --- Panel 1 (Record List) ---
    const int p1x = kEdgePad;
    const int panelH = navBarY - 2 * kEdgePad;
    g.setColour(panelColor);
    g.fillRoundedRectangle(juce::Rectangle<int>(p1x, kEdgePad, cw, panelH).toFloat(),
                           static_cast<float>(kCardRadius));

    // --- Panel 2 (Guide BGM) ---
    const int p2x = kEdgePad + cw + kGap;
    g.setColour(panelColor);
    g.fillRoundedRectangle(juce::Rectangle<int>(p2x, kEdgePad, cw, panelH).toFloat(),
                           static_cast<float>(kCardRadius));

    // --- Step badges + titles ---
    auto drawBadgeTitle = [&](int panelX, const juce::String &num, const juce::String &title) {
        const int bx = panelX + kInnerPad;
        const int by = kEdgePad + kInnerPad;
        g.setColour(onSurface);
        g.fillEllipse(juce::Rectangle<int>(bx, by, kBadgeSize, kBadgeSize).toFloat());
        g.setColour(bgColor);
        g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(13.0f)));
        g.drawText(num, juce::Rectangle<int>(bx, by, kBadgeSize, kBadgeSize),
                   juce::Justification::centred);
        g.setColour(onSurface);
        g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(15.0f)));
        g.drawText(title, juce::Rectangle<int>(bx + kBadgeSize + 8, by, cw - kBadgeSize - 40, kBadgeSize),
                   juce::Justification::centredLeft);
    };
    drawBadgeTitle(p1x, "1", "Record List");
    drawBadgeTitle(p2x, "2", "Guide BGM");

    // --- Custom file cards ---
    auto drawCustomCard = [&](juce::Component *card, juce::Button *clearBtn) {
        if (!card || !card->isVisible())
            return;
        auto r = card->getBounds().toFloat();
        g.setColour(primaryCont);
        g.fillRoundedRectangle(r, 16.0f);

        // Overline text
        g.setColour(onPrimaryCont.withAlpha(0.6f));
        g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaSemiBold()).withHeight(10.0f)));
        g.drawText("CUSTOM FILE SELECTED",
                   juce::Rectangle<int>(card->getX() + 16, card->getY() + 10,
                                        card->getWidth() - 56, 12),
                   juce::Justification::centredLeft);

        // Close button state layer — circular bg on hover/press
        if (clearBtn && (clearBtn->isOver() || clearBtn->isDown())) {
            auto btnBounds = clearBtn->getBounds().toFloat().translated(
                static_cast<float>(card->getX()), static_cast<float>(card->getY()));
            // Expand to a 28dp circle centered on the 18dp icon
            auto circle = btnBounds.withSizeKeepingCentre(28.0f, 28.0f);
            g.setColour(onPrimaryCont.withAlpha(clearBtn->isDown() ? 0.10f : 0.08f));
            g.fillEllipse(circle);
        }
    };
    if (m_usingCustomReclist)
        drawCustomCard(m_reclistCustomCard.get(), m_reclistClearButton.get());
    if (m_usingCustomBGM)
        drawCustomCard(m_bgmCustomCard.get(), m_bgmClearButton.get());

    // --- OK button (MD3e Icon Button M: 56×56 filled circle) ---
    {
        auto fabBounds = m_fabButton->getBounds().toFloat();
        const auto onPrimaryCol = laf ? laf->onPrimary() : juce::Colours::white;

        // Primary filled circle
        g.setColour(primaryColor);
        g.fillEllipse(fabBounds);

        // State layers: On Primary overlay at 8% hover, 10% press
        if (m_fabButton->isDown()) {
            g.setColour(onPrimaryCol.withAlpha(0.10f));
            g.fillEllipse(fabBounds);
        } else if (m_fabButton->isOver()) {
            g.setColour(onPrimaryCol.withAlpha(0.08f));
            g.fillEllipse(fabBounds);
        }

        // Check icon (24dp centered)
        if (m_checkIcon) {
            const float iconSize = 24.0f;
            auto iconBounds = fabBounds.withSizeKeepingCentre(iconSize, iconSize);
            m_checkIcon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }

    // --- Nav bar ---
    g.setColour(navBarColor);
    g.fillRect(0, navBarY, w, kNavBarH);

    // Logo
    float curX = 10.0f;
    const float logoSize = 24.0f;
    if (m_logoDrawable) {
        m_logoDrawable->drawWithin(
            g, juce::Rectangle<float>(curX, static_cast<float>(navBarY) + (kNavBarH - logoSize) / 2.0f,
                                      logoSize, logoSize),
            juce::RectanglePlacement::centred, 1.0f);
    }
    curX += logoSize + 2.0f;

    // "KiraNastro inst." — Lexend, white
    const auto lexendFont = juce::Font(juce::FontOptions(Fonts::getLexendRegular()).withPointHeight(14.0f));
    juce::GlyphArrangement ga;
    ga.addLineOfText(lexendFont, "KiraNastro inst.", 0.0f, 0.0f);
    const float brandW = ga.getBoundingBox(0, -1, true).getWidth();

    g.setColour(onNavBar);
    g.setFont(lexendFont);
    g.drawText("KiraNastro inst.",
               juce::Rectangle<float>(curX, static_cast<float>(navBarY), brandW + 2.0f,
                                      static_cast<float>(kNavBarH))
                   .toNearestInt(),
               juce::Justification::centredLeft, false);
    curX += brandW + 8.0f;

    // "Project Setup" — Lexend Regular, reduced opacity, smaller size
    const auto subtitleFont = juce::Font(juce::FontOptions(Fonts::getLexendRegular()).withPointHeight(11.0f));
    g.setColour(onNavBar.withAlpha(0.50f));
    g.setFont(subtitleFont);
    g.drawText("Project Setup",
               juce::Rectangle<int>(static_cast<int>(curX), navBarY, 150, kNavBarH),
               juce::Justification::centredLeft, false);
}

//==============================================================================
void ProjectSetupScreen::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    const int contentH = h - kNavBarH;
    const int cw = cardWidth(w);
    const int panelH = contentH - 2 * kEdgePad;

    // Inner content region within each card
    const int panelInnerTop = kEdgePad + kInnerPad + kBadgeSize + kInnerPad;
    const int panelInnerBottom = kEdgePad + panelH - kInnerPad;

    // --- Column X positions ---
    const int col1X = kEdgePad;
    const int col2X = kEdgePad + cw + kGap;
    const int col3X = col2X + cw + kGap;

    // Inner padding within cards
    const int innerX1 = col1X + kInnerPad;
    const int innerW1 = cw - 2 * kInnerPad;
    const int innerX2 = col2X + kInnerPad;
    const int innerW2 = cw - 2 * kInnerPad;

    // --- Left panel (Record List) ---
    if (!m_usingCustomReclist) {
        m_reclistCombo->setBounds(innerX1, panelInnerTop, innerW1, kComboH);
        m_reclistCombo->setVisible(true);

        m_reclistCustomButton->setBounds(innerX1, panelInnerBottom - kCustomBtnH,
                                         innerW1, kCustomBtnH);
        m_reclistCustomButton->setVisible(true);
        m_reclistCustomCard->setVisible(false);
    } else {
        m_reclistCombo->setVisible(false);
        m_reclistCustomButton->setVisible(false);

        const int cardH = 64;
        m_reclistCustomCard->setBounds(innerX1, panelInnerTop, innerW1, cardH);
        m_reclistCustomCard->setVisible(true);
        m_reclistCustomLabel.setBounds(16, 26, innerW1 - 56, 20);
        m_reclistClearButton->setBounds(innerW1 - 34, 16, 18, 18);
    }

    // --- Middle panel (Guide BGM) ---
    if (!m_usingCustomBGM) {
        m_tempoCombo->setBounds(innerX2, panelInnerTop, innerW2, kComboH);
        m_tempoCombo->setVisible(true);
        m_keyCombo->setBounds(innerX2, panelInnerTop + kComboH + 6, innerW2, kComboH);
        m_keyCombo->setVisible(true);

        m_bgmCustomButton->setBounds(innerX2, panelInnerBottom - kCustomBtnH,
                                     innerW2, kCustomBtnH);
        m_bgmCustomButton->setVisible(true);
        m_bgmCustomCard->setVisible(false);
    } else {
        m_tempoCombo->setVisible(false);
        m_keyCombo->setVisible(false);
        m_bgmCustomButton->setVisible(false);

        const int cardH = 64;
        m_bgmCustomCard->setBounds(innerX2, panelInnerTop, innerW2, cardH);
        m_bgmCustomCard->setVisible(true);
        m_bgmCustomLabel.setBounds(16, 26, innerW2 - 56, 20);
        m_bgmClearButton->setBounds(innerW2 - 34, 16, 18, 18);
    }

    // --- OK button (MD3e L: 96×96, bottom of 3rd column) ---
    const int fabY = contentH - kEdgePad - kBtnColW;
    m_fabButton->setBounds(col3X, fabY, kBtnColW, kBtnColW);
}
