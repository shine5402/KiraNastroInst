// Copyright (c) 2026 shine_5402
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TimingIndicator.h"

#include "LookAndFeel.h"

void TimingIndicator::paint(juce::Graphics &g)
{
    auto *laf = dynamic_cast<KiraNastroLookAndFeel *>(&getLookAndFeel());
    if (laf == nullptr) { jassertfalse; return; }

    const float prog   = juce::jlimit(0.0f, 1.0f, m_progress);
    const float uStart = juce::jlimit(0.0f, 1.0f, m_utteranceStartFraction);
    const float uEnd   = juce::jlimit(uStart, 1.0f, m_utteranceEndFraction);

    const juce::Rectangle<float> pieBounds{0.0f, 0.0f, 32.0f, 32.0f};
    const juce::Rectangle<float> barBounds{40.0f, 0.0f, 8.0f, 32.0f};

    // --- Pie background ---
    g.setColour(laf->surfaceVariant());
    g.fillEllipse(pieBounds);

    // --- Stage-aware pie fill ---
    if (prog <= uStart) {
        // Stage 1: horizontal clip fills left→right with tertiaryContainer
        if (uStart > 0.0f) {
            const float stage1Progress = prog / uStart;
            const float clipW = pieBounds.getWidth() * stage1Progress;
            g.saveState();
            g.reduceClipRegion(juce::Rectangle<float>(0.0f, 0.0f, clipW, 32.0f).toNearestInt());
            g.setColour(laf->secondaryContainer());
            g.fillEllipse(pieBounds);
            g.restoreState();
        }
    }
    else if (prog <= uEnd) {
        // Stage 2: light primary base (smooth transition from Stage 1) + primary arc on top
        g.setColour(laf->secondaryContainer());
        g.fillEllipse(pieBounds);

        if (uEnd > uStart) {
            const float stage2Progress = (prog - uStart) / (uEnd - uStart);
            juce::Path p;
            p.addPieSegment(pieBounds.getX(), pieBounds.getY(),
                            pieBounds.getWidth(), pieBounds.getHeight(),
                            -juce::MathConstants<float>::halfPi,
                            -juce::MathConstants<float>::halfPi + stage2Progress * juce::MathConstants<float>::twoPi,
                            0.0f);
            g.setColour(laf->primary());
            g.fillPath(p);
        }
    }
    else {
        // Stage 3: primary arc unwinds back to 0 (same pie sweep as Stage 2, reversed),
        // revealing the surfaceVariant background already drawn beneath.
        // Use fillEllipse for the full-circle case to avoid the 12 o'clock seam artifact
        // that addPieSegment produces when start angle == end angle.
        if (1.0f > uEnd) {
            const float stage3Progress = (prog - uEnd) / (1.0f - uEnd);
            // Ease-out quadratic: fast start, decelerates to a gentle stop.
            const float eased = 1.0f - (1.0f - stage3Progress) * (1.0f - stage3Progress);
            // Advance the start angle clockwise (same direction as Stage 2 fills),
            // so the arc is consumed from its beginning rather than its end.
            const float consumed = eased * juce::MathConstants<float>::twoPi;
            const float fromAngle = -juce::MathConstants<float>::halfPi + consumed;
            const float toAngle   = -juce::MathConstants<float>::halfPi + juce::MathConstants<float>::twoPi;
            g.setColour(laf->primary());
            if (consumed <= 0.001f) {
                g.fillEllipse(pieBounds); // full circle — avoid seam artifact at 12 o'clock
            } else if (consumed < juce::MathConstants<float>::twoPi - 0.001f) {
                juce::Path p;
                p.addPieSegment(pieBounds.getX(), pieBounds.getY(),
                                pieBounds.getWidth(), pieBounds.getHeight(),
                                fromAngle, toAngle, 0.0f);
                g.fillPath(p);
            }
        }
    }

    // --- Vertical fill bar (overall progress) ---
    g.setColour(laf->surfaceVariant());
    g.fillRoundedRectangle(barBounds, 4.0f);

    const float fillHeight = barBounds.getHeight() * prog;
    if (fillHeight > 0.0f) {
        g.saveState();
        g.reduceClipRegion(
            juce::Rectangle<float>(barBounds.getX(), barBounds.getY(), barBounds.getWidth(), fillHeight)
                .toNearestInt());
        g.setColour(laf->primary());
        g.fillRoundedRectangle(barBounds, 4.0f);
        g.restoreState();
    }
}

void TimingIndicator::resized()
{
}
