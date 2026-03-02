#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include "data/LabelExporter.h"

// ── Helpers ────────────────────────────────────────────────────────────────────

// Reads and parses a JSON file. Caller must keep the returned var alive while
// using any DynamicObject pointer obtained from it (getDynamicObject() is non-owning).
static juce::var loadJson(const juce::File &f)
{
    return juce::JSON::parse(f.loadFileAsString());
}

static LabelExporter::Params makeParams(int numEntries = 3,
                                        double blockDur = 5.0,
                                        double recOffset = 1.0,
                                        double recDur = 2.0)
{
    LabelExporter::Params p;
    for (int i = 0; i < numEntries; ++i)
        p.entryNames.push_back("entry" + juce::String(i));
    p.blockDurationSec = blockDur;
    p.recordingStartOffsetSec = recOffset;
    p.recordingWindowDurationSec = recDur;
    return p;
}

// ── Params::isValid() ─────────────────────────────────────────────────────────

TEST_CASE("LabelExporter: Params::isValid() - empty entryNames is invalid", "[LabelExporter]")
{
    LabelExporter::Params p;
    p.blockDurationSec = 5.0;
    p.recordingWindowDurationSec = 2.0;
    CHECK_FALSE(p.isValid());
}

TEST_CASE("LabelExporter: Params::isValid() - zero blockDuration is invalid", "[LabelExporter]")
{
    LabelExporter::Params p = makeParams();
    p.blockDurationSec = 0.0;
    CHECK_FALSE(p.isValid());
}

TEST_CASE("LabelExporter: Params::isValid() - zero recordingWindow is invalid", "[LabelExporter]")
{
    LabelExporter::Params p = makeParams();
    p.recordingWindowDurationSec = 0.0;
    CHECK_FALSE(p.isValid());
}

TEST_CASE("LabelExporter: Params::isValid() - valid params", "[LabelExporter]")
{
    CHECK(makeParams().isValid());
}

// ── exportToFile error path ────────────────────────────────────────────────────

TEST_CASE("LabelExporter: export with invalid params returns false and sets errorMessage",
          "[LabelExporter]")
{
    juce::TemporaryFile tmp(".kirawavtar-desc.json");
    LabelExporter::Params p; // empty, invalid
    juce::String errMsg;
    CHECK_FALSE(LabelExporter::exportToFile(tmp.getFile(), p, errMsg));
    CHECK_FALSE(errMsg.isEmpty());
}

// ── Root JSON structure ────────────────────────────────────────────────────────

TEST_CASE("LabelExporter: root has version=4 and combiner=nastro_inst", "[LabelExporter]")
{
    juce::TemporaryFile tmp(".kirawavtar-desc.json");
    juce::String err;
    REQUIRE(LabelExporter::exportToFile(tmp.getFile(), makeParams(), err));

    auto json = loadJson(tmp.getFile());
    auto *root = json.getDynamicObject();
    REQUIRE(root != nullptr);
    CHECK(static_cast<int>(root->getProperty("version")) == 4);
    CHECK(root->getProperty("combiner").toString() == "nastro_inst");
}

TEST_CASE("LabelExporter: root has no audio format fields", "[LabelExporter]")
{
    juce::TemporaryFile tmp(".kirawavtar-desc.json");
    juce::String err;
    REQUIRE(LabelExporter::exportToFile(tmp.getFile(), makeParams(), err));

    auto json = loadJson(tmp.getFile());
    auto *root = json.getDynamicObject();
    REQUIRE(root != nullptr);
    CHECK_FALSE(root->hasProperty("sample_rate"));
    CHECK_FALSE(root->hasProperty("sample_type"));
    CHECK_FALSE(root->hasProperty("channel_count"));
    CHECK_FALSE(root->hasProperty("container_format"));
    CHECK_FALSE(root->hasProperty("total_duration"));
    CHECK_FALSE(root->hasProperty("gap_duration"));
}

// ── Descriptions array ────────────────────────────────────────────────────────

TEST_CASE("LabelExporter: entry count matches params", "[LabelExporter]")
{
    juce::TemporaryFile tmp(".kirawavtar-desc.json");
    juce::String err;
    REQUIRE(LabelExporter::exportToFile(tmp.getFile(), makeParams(5), err));

    auto json = loadJson(tmp.getFile());
    auto *root = json.getDynamicObject();
    REQUIRE(root != nullptr);
    const auto *descs = root->getProperty("descriptions").getArray();
    REQUIRE(descs != nullptr);
    REQUIRE(descs->size() == 5);
}

TEST_CASE("LabelExporter: file_name is entryName + .wav", "[LabelExporter]")
{
    juce::TemporaryFile tmp(".kirawavtar-desc.json");
    juce::String err;
    REQUIRE(LabelExporter::exportToFile(tmp.getFile(), makeParams(1), err));

    auto json = loadJson(tmp.getFile());
    auto *root = json.getDynamicObject();
    REQUIRE(root != nullptr);
    const auto *descs = root->getProperty("descriptions").getArray();
    REQUIRE(descs != nullptr);
    REQUIRE(descs->size() == 1);
    CHECK((*descs)[0].getDynamicObject()->getProperty("file_name").toString() == "entry0.wav");
}

TEST_CASE("LabelExporter: entries have no audio format fields", "[LabelExporter]")
{
    juce::TemporaryFile tmp(".kirawavtar-desc.json");
    juce::String err;
    REQUIRE(LabelExporter::exportToFile(tmp.getFile(), makeParams(1), err));

    auto json = loadJson(tmp.getFile());
    auto *root = json.getDynamicObject();
    REQUIRE(root != nullptr);
    auto *entry = (*root->getProperty("descriptions").getArray())[0].getDynamicObject();
    REQUIRE(entry != nullptr);
    CHECK_FALSE(entry->hasProperty("sample_rate"));
    CHECK_FALSE(entry->hasProperty("sample_type"));
    CHECK_FALSE(entry->hasProperty("channel_count"));
    CHECK_FALSE(entry->hasProperty("container_format"));
}

// ── Timing calculations ────────────────────────────────────────────────────────

TEST_CASE("LabelExporter: begin_time calculation per entry", "[LabelExporter]")
{
    // blockDur=5.0, recOffset=1.2, recDur=2.0
    // entry 0: 0*5.0 + 1.2 =  1.2 -> "00:00:01.200"
    // entry 1: 1*5.0 + 1.2 =  6.2 -> "00:00:06.200"
    // entry 2: 2*5.0 + 1.2 = 11.2 -> "00:00:11.200"
    juce::TemporaryFile tmp(".kirawavtar-desc.json");
    juce::String err;
    REQUIRE(LabelExporter::exportToFile(tmp.getFile(), makeParams(3, 5.0, 1.2, 2.0), err));

    auto json = loadJson(tmp.getFile());
    auto *root = json.getDynamicObject();
    REQUIRE(root != nullptr);
    const auto *descs = root->getProperty("descriptions").getArray();
    REQUIRE(descs != nullptr);
    REQUIRE(descs->size() == 3);
    CHECK((*descs)[0].getDynamicObject()->getProperty("begin_time").toString() == "00:00:01.200");
    CHECK((*descs)[1].getDynamicObject()->getProperty("begin_time").toString() == "00:00:06.200");
    CHECK((*descs)[2].getDynamicObject()->getProperty("begin_time").toString() == "00:00:11.200");
}

TEST_CASE("LabelExporter: all entries share the same duration", "[LabelExporter]")
{
    juce::TemporaryFile tmp(".kirawavtar-desc.json");
    juce::String err;
    REQUIRE(LabelExporter::exportToFile(tmp.getFile(), makeParams(3, 5.0, 1.0, 3.456), err));

    auto json = loadJson(tmp.getFile());
    auto *root = json.getDynamicObject();
    REQUIRE(root != nullptr);
    for (const auto &v : *root->getProperty("descriptions").getArray())
        CHECK(v.getDynamicObject()->getProperty("duration").toString() == "00:00:03.456");
}

TEST_CASE("LabelExporter: toTimecode handles hours and minutes", "[LabelExporter]")
{
    // 3723.456 sec = 1h 2m 3.456s -> "01:02:03.456"
    // Exercised indirectly via the duration field.
    juce::TemporaryFile tmp(".kirawavtar-desc.json");
    LabelExporter::Params p;
    p.entryNames = {"a"};
    p.blockDurationSec = 1.0;
    p.recordingStartOffsetSec = 0.0;
    p.recordingWindowDurationSec = 3723.456;
    juce::String err;
    REQUIRE(LabelExporter::exportToFile(tmp.getFile(), p, err));

    auto json = loadJson(tmp.getFile());
    auto *root = json.getDynamicObject();
    REQUIRE(root != nullptr);
    CHECK((*root->getProperty("descriptions").getArray())[0]
              .getDynamicObject()
              ->getProperty("duration")
              .toString()
          == "01:02:03.456");
}
