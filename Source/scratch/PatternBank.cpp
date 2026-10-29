#include "PatternBank.h"
#include "FactoryPatterns.h"

namespace
{
    const juce::String fileExtension = ".scratch";
}

PatternBank::PatternBank()
{
    reload();
}

juce::File PatternBank::getUserFolder()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Scratch(Sic)").getChildFile("Bank");
}

void PatternBank::reload()
{
    entries.clear();
    for (auto& p : FactoryPatterns::create())
        entries.push_back({ std::move(p), true, {} });

    auto files = getUserFolder().findChildFiles(juce::File::findFiles, false, "*" + fileExtension);
    files.sort();
    for (const auto& file : files)
        if (auto loaded = ScratchPattern::fromXmlString(file.loadFileAsString()))
            entries.push_back({ std::move(*loaded), false, file });
}

bool PatternBank::addUserPattern(const ScratchPattern& pattern)
{
    const auto folder = getUserFolder();
    if (! folder.createDirectory().wasOk())
        return false;

    const auto file = folder.getNonexistentChildFile(juce::File::createLegalFileName(pattern.getName()), fileExtension, false);
    if (! file.replaceWithText(pattern.toXmlString()))
        return false;

    reload();
    return true;
}

bool PatternBank::removeUserPattern(int index)
{
    if (index < 0 || index >= (int) entries.size() || entries[(size_t) index].factory)
        return false;

    const bool deleted = entries[(size_t) index].file.deleteFile();
    reload();
    return deleted;
}
