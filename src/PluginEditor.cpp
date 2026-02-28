#include "PluginEditor.h"
#include "BinaryDataImages.h"
#include "utils/Fonts.h"
#include "utils/Icons.h"
#include <cmath>

#ifdef JUCE_STANDALONE_APPLICATION
#include "ui/PlaybackControls.h"
#endif

KiraNastroEditor::KiraNastroEditor(KiraNastroProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setLookAndFeel(&lookAndFeel);

  // Load brand logo from embedded SVG
  if (auto xml = juce::XmlDocument::parse(juce::String::fromUTF8(
          BinaryDataImages::icon_svg, (int)BinaryDataImages::icon_svgSize))) {
    logoDrawable = juce::Drawable::createFromSVG(*xml);
  }

  // Load chip icons
  reloadChipIcons();

  // Hamburger menu button (Material Symbols, white for nav bar)
  auto menuIcon = Icons::load(Icons::menuSvg, juce::Colours::white);
  menuButton = std::make_unique<juce::DrawableButton>(
      "menu", juce::DrawableButton::ImageFitted);
  menuButton->setImages(menuIcon.get());
  menuButton->onClick = [this] { showMenu(); };
  addAndMakeVisible(menuButton.get());

  // Timing indicator (always present)
  timingIndicator = std::make_unique<TimingIndicator>();
  addAndMakeVisible(timingIndicator.get());

// Standalone-specific controls
#ifdef JUCE_STANDALONE_APPLICATION
  playbackControls = std::make_unique<PlaybackControls>(audioProcessor);
  addAndMakeVisible(playbackControls.get());

  progressSlider = std::make_unique<juce::Slider>();
  addAndMakeVisible(progressSlider.get());
  progressSlider->addListener(this);
  progressSlider->setRange(0.0, 600.0, 0.1);
  progressSlider->setSliderStyle(juce::Slider::LinearHorizontal);
  progressSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
  progressSlider->setTextValueSuffix(" s");
#endif

#ifdef JUCE_STANDALONE_APPLICATION
  setSize(800, 360);
#else
  setSize(800, 232);
#endif
  startTimerHz(30);
}

KiraNastroEditor::~KiraNastroEditor() {
  stopTimer();
  setLookAndFeel(nullptr);
}

//==============================================================================
void KiraNastroEditor::paint(juce::Graphics &g) {
  // 1. Background
  g.fillAll(lookAndFeel.background());

  // 2. Card (MD3 Filled Card — no shadow)
  g.setColour(lookAndFeel.cardFilled());
  g.fillRoundedRectangle(juce::Rectangle<float>(16.0f, 8.0f, 768.0f, 128.0f), 16.0f);

  // 3. Comment text (Sarasa 16pt, onSurfaceVariant)
  g.setColour(lookAndFeel.onSurfaceVariant());
  g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(16.0f)));
  g.drawFittedText(currentEntryComment,
                   juce::Rectangle<int>(48, 28, 668, 20),
                   juce::Justification::left, 1);

  // 4. Entry text (Sarasa 48pt, primary)
  g.setColour(lookAndFeel.primary());
  g.setFont(juce::Font(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(48.0f)));
  g.drawFittedText(currentEntryName,
                   juce::Rectangle<int>(48, 56, 668, 60),
                   juce::Justification::left, 1);

  // 5. Info row chips
  {
    const juce::Font chipFont(juce::FontOptions(Fonts::getSarasaRegular()).withHeight(16.0f));
    g.setFont(chipFont);
    const float chipH        = 32.0f;
    const float chipY        = 144.0f;
    const float iconSize     = 14.0f;
    const float padH         = 16.0f;
    const float iconTextGap  = 8.0f;

    // Helper: measure text width via GlyphArrangement (avoids deprecated getStringWidth)
    auto measureText = [](const juce::Font& font, const juce::String& text) -> float {
      juce::GlyphArrangement ga;
      ga.addLineOfText(font, text, 0.0f, 0.0f);
      return ga.getBoundingBox(0, -1, true).getWidth();
    };

    // a. Next Entry chip (left)
    {
      const float textW = measureText(chipFont, nextEntryName);
      const float chipW = padH + iconSize + iconTextGap + textW + padH;
      const juce::Rectangle<float> chipRect(16.0f, chipY, chipW, chipH);

      g.setColour(lookAndFeel.secondaryContainer());
      g.fillRoundedRectangle(chipRect, 16.0f);

      if (nextIcon) {
        nextIcon->drawWithin(
            g,
            juce::Rectangle<float>(16.0f + padH,
                                   chipY + (chipH - iconSize) / 2.0f,
                                   iconSize, iconSize),
            juce::RectanglePlacement::centred, 1.0f);
      }

      g.setColour(lookAndFeel.onSecondaryContainer());
      g.drawText(nextEntryName,
                 juce::Rectangle<float>(16.0f + padH + iconSize + iconTextGap,
                                        chipY, textW + 2.0f, chipH)
                     .toNearestInt(),
                 juce::Justification::centredLeft, false);
    }

    // b. Progress chip (right-aligned)
    {
      const int current = lastEntryIndex + 1;
      const int total   = audioProcessor.totalEntries.load();
      const juce::String progressStr =
          juce::String(current) + " / " + juce::String(total);
      const float textW = measureText(chipFont, progressStr);
      const float chipW = padH + iconSize + iconTextGap + textW + padH;
      const float chipX = 784.0f - chipW;
      const juce::Rectangle<float> chipRect(chipX, chipY, chipW, chipH);

      g.setColour(lookAndFeel.secondaryContainer());
      g.fillRoundedRectangle(chipRect, 16.0f);

      if (progressIcon) {
        progressIcon->drawWithin(
            g,
            juce::Rectangle<float>(chipX + padH,
                                   chipY + (chipH - iconSize) / 2.0f,
                                   iconSize, iconSize),
            juce::RectanglePlacement::centred, 1.0f);
      }

      g.setColour(lookAndFeel.onSecondaryContainer());
      g.drawText(progressStr,
                 juce::Rectangle<float>(chipX + padH + iconSize + iconTextGap,
                                        chipY, textW + 2.0f, chipH)
                     .toNearestInt(),
                 juce::Justification::centredLeft, false);
    }
  }

  // 6. Nav bar
  {
    const float navBarY = 192.0f;
    const float navBarH = 40.0f;
    g.setColour(lookAndFeel.navBar());
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
    const float spacing  = 2.0f;

    // Starting X to keep them grouped (starting from X=10)
    float currentX = 10.0f;

    // Logo icon (vertically centred in 40px bar)
    if (logoDrawable) {
      logoDrawable->drawWithin(
          g,
          juce::Rectangle<float>(currentX, navBarY + (navBarH - logoSize) / 2.0f,
                                 logoSize, logoSize),
          juce::RectanglePlacement::centred, 1.0f);
    }

    currentX += logoSize + spacing;

    // "KiraNastro inst." vertically centred relative to nav bar
    g.setColour(lookAndFeel.onNavBar());
    g.setFont(navFont);
    g.drawText(brandText,
               juce::Rectangle<float>(currentX, navBarY, textW + 2.0f, navBarH)
                   .toNearestInt(),
               juce::Justification::centredLeft, false);
  }

#ifdef JUCE_STANDALONE_APPLICATION
  // 7. Debug area background (y=232)
  g.setColour(juce::Colour(0xFFDBEAFE));
  g.fillRect(0, 232, 800, getHeight() - 232);
#endif
}

void KiraNastroEditor::resized() {
  // TimingIndicator: bottom-right inside card padding
  timingIndicator->setBounds(720, 84, 32, 32);

  // Hamburger menu button: 24x24 (same as nav bar logo), right-aligned, vertically centred
  // Nav bar: y=192, h=40 → icon y = 192 + (40-24)/2 = 200; x = 800 - 8(pad) - 24 = 768
  menuButton->setBounds(768, 200, 24, 24);

#ifdef JUCE_STANDALONE_APPLICATION
  progressSlider->setBounds(20, 242, getWidth() - 40, 32);
  progressSlider->setColour(juce::Slider::textBoxTextColourId,
                            KiraNastroLookAndFeel::md3Primary);

  playbackControls->setBounds(0, 278, getWidth(), 64);
#endif
}

//==============================================================================
void KiraNastroEditor::timerCallback() {
  auto info = audioProcessor.getCurrentEntryInfo();
  bool changed =
      (info.index != lastEntryIndex || info.name != currentEntryName);

  if (changed) {
    lastEntryIndex    = info.index;
    currentEntryName  = info.name;
    currentEntryComment = info.comment;

    auto nextInfo   = audioProcessor.getNextEntryInfo();
    nextEntryName   = nextInfo.name;
    nextEntryComment = nextInfo.comment;

    repaint();
  }

  timingIndicator->setProgress(
      audioProcessor.bgmLoopProgress.load(std::memory_order_relaxed));

#ifdef JUCE_STANDALONE_APPLICATION
  if (progressSlider && audioProcessor.isBGMLoaded()) {
    const double currentPos = audioProcessor.projectPlayPositionSeconds.load();
    // Don't overwrite the slider while the user is dragging it
    if (progressSlider->getThumbBeingDragged() == -1)
      progressSlider->setValue(currentPos, juce::dontSendNotification);

    const double bgmLength    = audioProcessor.getBGMLengthSeconds();
    const double projectLength = bgmLength * 10.0;
    const double currentMax   = progressSlider->getMaximum();
    if (bgmLength > 0 && std::abs(currentMax - projectLength) > 0.1) {
      progressSlider->setRange(0.0, projectLength, 0.1);
    }
  }
#endif
}

void KiraNastroEditor::sliderValueChanged(juce::Slider *slider) {
#ifdef JUCE_STANDALONE_APPLICATION
  if (slider == progressSlider.get()) {
    audioProcessor.seekBGM(slider->getValue());
  }
#else
  juce::ignoreUnused(slider);
#endif
}

void KiraNastroEditor::reloadChipIcons() {
  // Chip icons tinted with current onSecondaryContainer color
  nextIcon     = Icons::load(Icons::arrowRightSvg,
                              lookAndFeel.onSecondaryContainer());
  progressIcon = Icons::load(Icons::percentSvg,
                              lookAndFeel.onSecondaryContainer());
}

void KiraNastroEditor::showMenu() {
  juce::PopupMenu menu;
  menu.addItem(1, "Load Reclist...");
  menu.addItem(2, "Load BGM...");
  menu.addSeparator();
  const bool dark = lookAndFeel.getDarkMode();
  menu.addItem(3, dark ? "Switch to Light Mode" : "Switch to Dark Mode");

  menu.showMenuAsync(juce::PopupMenu::Options()
                         .withTargetComponent(menuButton.get())
                         .withMinimumWidth(200)
                         .withMaximumNumColumns(1),
                     [this](int result) {
                       if (result == 1) {
                         reclistChooser = std::make_unique<juce::FileChooser>(
                             "Select Reclist File", juce::File(), "*.txt");
                         reclistChooser->launchAsync(
                             juce::FileBrowserComponent::openMode |
                                 juce::FileBrowserComponent::canSelectFiles,
                             [this](const juce::FileChooser &f) {
                               auto file = f.getResult();
                               if (file.exists())
                                 audioProcessor.loadReclist(file);
                               reclistChooser.reset();
                             });
                       } else if (result == 2) {
                         bgmChooser = std::make_unique<juce::FileChooser>(
                             "Select BGM File", juce::File(), "*.wav");
                         bgmChooser->launchAsync(
                             juce::FileBrowserComponent::openMode |
                                 juce::FileBrowserComponent::canSelectFiles,
                             [this](const juce::FileChooser &f) {
                               auto file = f.getResult();
                               if (file.exists())
                                 audioProcessor.loadGuideBGM(file);
                               bgmChooser.reset();
                             });
                       } else if (result == 3) {
                         lookAndFeel.setDarkMode(!lookAndFeel.getDarkMode());
                         reloadChipIcons();
                         repaint();
                       }
                     });
}
