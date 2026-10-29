#pragma once
#include "ScratchPattern.h"

struct BankEntry
{
    ScratchPattern pattern;
    bool factory = true;
    juce::File file;   // only for the user's own scratches
};

// The library of scratches: the built-in ones first, then the user's own, which live as files
// in the user's app-data folder so plugin updates never touch them.
class PatternBank
{
public:
    PatternBank();

    static juce::File getUserFolder();

    const std::vector<BankEntry>& getEntries() const noexcept { return entries; }
    void reload();
    bool addUserPattern(const ScratchPattern& pattern);
    bool removeUserPattern(int index);

private:
    std::vector<BankEntry> entries;
};
