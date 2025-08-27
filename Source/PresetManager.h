#pragma once
#include <JuceHeader.h>

class PresetManager
{
public:
    using APVTS = juce::AudioProcessorValueTreeState;

    // NEW: take the exact folder to use for user presets
    PresetManager(APVTS& vts, juce::File userDirIn)
        : apvts(vts), userDir(std::move(userDirIn))
    {
        userDir.createDirectory();
        buildFactoryPresets();
        refreshUserPresetFiles();
    }

    PresetManager(APVTS& vts, juce::String org, juce::String product)
        : apvts(vts), orgName(std::move(org)), productName(std::move(product))
    {
        userDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile(orgName).getChildFile(productName).getChildFile("Presets");
        userDir.createDirectory();

        buildFactoryPresets();
        refreshUserPresetFiles();
    }

    // ===== Factory Presets =====
    const juce::StringArray& getFactoryNames() const
    {
        // Rebuild on first call if we’re XML-backed
        if (!factoryXmls.empty())
        {
            const_cast<juce::StringArray&>(factoryNames).clear();
            for (auto& fx : factoryXmls) const_cast<juce::StringArray&>(factoryNames).add(fx.displayName);
        }
        return factoryNames;
    }

    // --- Still inside PresetManager (public:) ---
    void applyFactoryPreset(int index)
    {
        // 1) XML-backed factory presets
        if (index >= 0 && index < (int)factoryXmls.size())
        {
            const auto& fx = factoryXmls[(size_t)index];
            std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(
                juce::String::fromUTF8(static_cast<const char*>(fx.data), fx.size)));
            if (xml)
            {
                auto vt = juce::ValueTree::fromXml(*xml);
                if (vt.isValid())
                {
                    apvts.replaceState(vt);
                    currentFactoryIndex = index;
                    currentUserFile = {};
                    return;
                }
            }
        }

        // 2) Fallback to code-defined ParamSet (what you already had)
        if ((unsigned)index < factoryParamSets.size())
        {
            applyParamSet(factoryParamSets[(size_t)index]);
            currentFactoryIndex = index;
            currentUserFile = {};
        }
    }


    // ===== User Presets (XML snapshots of APVTS) =====
    void refreshUserPresetFiles()
    {
        userFiles.clear();
        userDir.findChildFiles(userFiles, juce::File::findFiles, false, "*.xml");
        userFiles.sort(); // alphabetical
    }
    const juce::Array<juce::File>& getUserFiles() const { return userFiles; }

    bool saveUserPreset(juce::String presetName) const
    {
        presetName = presetName.trim();
        if (presetName.isEmpty()) return false;

        juce::File file = userDir.getChildFile(presetName + ".xml");
        auto state = apvts.copyState();
        if (auto xml = state.createXml())
            return xml->writeTo(file);
        return false;
    }

    bool loadUserPreset(const juce::File& file)
    {
        if (! file.existsAsFile()) return false;
        std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse(file));
        if (! xml) return false;

        juce::ValueTree vt = juce::ValueTree::fromXml(*xml);
        if (! vt.isValid()) return false;

        apvts.replaceState(vt); // safe: attachments update UI
        currentFactoryIndex = -1; currentUserFile = file;
        return true;
    }

    bool deleteUserPreset(const juce::File& file)
    {
        if (! file.existsAsFile()) return false;
        return file.deleteFile();
    }

    // ===== Current selection (optional to query) =====
    int getCurrentFactoryIndex() const { return currentFactoryIndex; }
    juce::File getCurrentUserFile() const { return currentUserFile; }

private:
    APVTS& apvts;
    juce::String orgName, productName;
    juce::File userDir;
    juce::Array<juce::File> userFiles;
    int currentFactoryIndex = -1;
    juce::File currentUserFile;

    // A preset is just a map of paramID -> normalized value [0..1]
    using ParamSet = std::map<juce::String, float>;
    std::vector<ParamSet> factoryParamSets;
    juce::StringArray     factoryNames;

    // --- In PresetManager.h (private:) ---
    struct FactoryXmlPreset {
        juce::String displayName;
        const void* data = nullptr;
        int          size = 0;
    };

    std::vector<FactoryXmlPreset> factoryXmls;

    // register an XML preset that lives in BinaryData
    void addFactoryPresetFromBinary(juce::String name, const void* data, int size)
    {
        if (data != nullptr && size > 0)
            factoryXmls.push_back({ std::move(name), data, size });
    }

    // Utilities
    void applyParamSet(const ParamSet& set)
    {
        for (const auto& kv : set)
        {
            if (auto* p = apvts.getParameter(kv.first))
            {
                // kv.second is normalized 0..1
                const float norm = juce::jlimit(0.0f, 1.0f, kv.second);
                p->beginChangeGesture();
                p->setValueNotifyingHost(norm);
                p->endChangeGesture();
            }
        }
    }

    // Normalize choice indexes with getNumSteps()
    static float choiceIndexToNorm(juce::RangedAudioParameter* p, int idx)
    {
        const int steps = juce::jmax(1, p->getNumSteps());
        if (steps <= 1) return 0.0f;
        idx = juce::jlimit(0, steps - 1, idx);
        return (float) idx / (float) (steps - 1);
    }

    void buildFactoryPresets()
    {
        // Helpful aliases for your IDs
        auto D  = "drive",   MIX="mix",   BD="bitdepth", SR="samplerate";
        auto FC = "filterCutoff", RES="filterResonance", FT="filterType";
        auto DT = "distortionType", OS="oversamplingFactor";
        auto IN = "inputGain", OUT="outputGain", THR="limiterThreshold", REL="limiterRelease";
        auto SW = "stereoWidth", BYP="bypass";

        // We'll fill normalized values; for choices we translate later with choiceIndexToNorm
        auto make = [&](juce::String name, std::initializer_list<std::pair<juce::String, float>> pairs)
        {
            ParamSet set (pairs.begin(), pairs.end());

            // Expand any special -1 markers for choices (we’ll detect by paramID)
            auto fixChoice = [&](const char* id, int idx)
            {
                if (auto* p = apvts.getParameter(id))
                    set[id] = choiceIndexToNorm(p, idx);
            };

            if (set.count(FT)  && set[FT]  < 0.0f) fixChoice(FT,  (int) (-set[FT]  - 1)); // temporary pattern
            if (set.count(DT)  && set[DT]  < 0.0f) fixChoice(DT,  (int) (-set[DT]  - 1));
            if (set.count(OS)  && set[OS]  < 0.0f) fixChoice(OS,  (int) (-set[OS]  - 1));
            if (set.count(BYP) && set[BYP] < 0.0f) set[BYP] = set[BYP] < -0.5f ? 1.0f : 0.0f;

            factoryNames.add(name);
            factoryParamSets.push_back(std::move(set));
        };

        // ==== Factory presets (tuned by ear later; these are good starters) ====
        // Use normalized 0..1 for continuous params; use negative “-index-1” placeholders for choices.
        // FilterType: 0=LP,1=BP,2=HP; DistType: 0..3; Oversampling: 0..4
        make("Clean Boost", {
            {IN, 0.55f}, {OUT, 0.50f}, {BYP, 0.0f},
            {D,  0.20f}, {MIX, 1.00f}, {DT, -0.0f - 1}, // type A
            {BD, 0.95f}, {SR,  1.00f},
            {FC, 0.75f}, {RES, 0.20f}, {FT, -0.0f - 1}, // LP
            {OS, -0.0f - 1}, // 1x
            {THR, 0.70f}, {REL, 0.30f}, {SW, 0.50f}
        });

        make("Lo-Fi Phone", {
            {IN, 0.50f}, {OUT, 0.50f},
            {D,  0.25f}, {MIX, 0.75f}, {DT, -1.0f - 1}, // type B
            {BD, 0.15f}, {SR,  0.15f},
            {FC, 0.15f}, {RES, 0.55f}, {FT, -2.0f - 1}, // HP
            {OS, -0.0f - 1},
            {THR, 0.50f}, {REL, 0.40f}, {SW, 0.40f}
        });

        make("Heavy Sizzle", {
            {IN, 0.55f}, {OUT, 0.45f},
            {D,  0.70f}, {MIX, 0.70f}, {DT, -3.0f - 1}, // type D
            {BD, 0.60f}, {SR,  0.50f},
            {FC, 0.65f}, {RES, 0.40f}, {FT, -0.0f - 1}, // LP
            {OS, -3.0f - 1}, // e.g., 8x
            {THR, 0.55f}, {REL, 0.35f}, {SW, 0.60f}
        });

        make("Glue Limiter", {
            {IN, 0.50f}, {OUT, 0.50f},
            {D,  0.10f}, {MIX, 1.00f}, {DT, -2.0f - 1}, // type C
            {BD, 0.95f}, {SR,  1.00f},
            {FC, 0.50f}, {RES, 0.20f}, {FT, -0.0f - 1},
            {OS, -2.0f - 1}, // 4x
            {THR, 0.35f}, {REL, 0.55f}, {SW, 0.50f}
        });
    }
};
