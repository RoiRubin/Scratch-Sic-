#include "FactoryPatterns.h"

namespace
{
    // Times run 0..1 across the graph; values are 0..1 across the sample window (0 = its start, 1 = its end).
    // Few points on purpose: the smoothing turns them into natural hand strokes.
    ScratchPattern baby()
    {
        return ScratchPattern::create("Baby", "Baby",
                                      PositionLane({ { 0.0, 0.2f, 0.0f }, { 0.5, 0.8f, 0.0f }, { 1.0, 0.2f, 0.0f } }),
                                      GateLane::open());
    }

    ScratchPattern babyTwice()
    {
        return ScratchPattern::create("Baby x2", "Baby",
                                      PositionLane({ { 0.0, 0.2f, 0.0f }, { 0.25, 0.8f, 0.0f }, { 0.5, 0.2f, 0.0f },
                                                     { 0.75, 0.8f, 0.0f }, { 1.0, 0.2f, 0.0f } }),
                                      GateLane::open());
    }

    // Forward strokes are heard, the pull back is cut off by the fader
    ScratchPattern chirp()
    {
        return ScratchPattern::create("Chirp", "Chirp",
                                      PositionLane({ { 0.0, 0.2f, 0.0f }, { 0.25, 0.8f, 0.0f }, { 0.5, 0.2f, 0.0f },
                                                     { 0.75, 0.8f, 0.0f }, { 1.0, 0.2f, 0.0f } }),
                                      GateLane({ { 0.0, 0.25 }, { 0.5, 0.75 } }));
    }

    // The record keeps moving while the fader chops it into eight pieces
    ScratchPattern transformer()
    {
        std::vector<GateBlock> chops;
        for (int i = 0; i < 8; ++i)
            chops.push_back({ i * 0.125, i * 0.125 + 0.0625 });

        return ScratchPattern::create("Transformer", "Transformer",
                                      PositionLane({ { 0.0, 0.3f, 0.0f }, { 1.0, 0.7f, 0.0f } }),
                                      GateLane(std::move(chops)));
    }

    // One long stroke and back, with the fader clicked twice on the way
    ScratchPattern flare()
    {
        return ScratchPattern::create("Flare (2 clicks)", "Flare",
                                      PositionLane({ { 0.0, 0.1f, 0.0f }, { 0.5, 0.9f, 0.0f }, { 1.0, 0.1f, 0.0f } }),
                                      GateLane({ { 0.0, 0.22 }, { 0.28, 0.72 }, { 0.78, 1.0 } }));
    }

    // Tiny, fast shakes of the record around one spot
    ScratchPattern scribble()
    {
        std::vector<PositionPoint> shakes;
        for (int i = 0; i <= 16; ++i)
            shakes.push_back({ i / 16.0, i % 2 == 0 ? 0.42f : 0.58f, 0.0f });

        return ScratchPattern::create("Scribble", "Scribble", PositionLane(std::move(shakes)), GateLane::open());
    }
}

std::vector<ScratchPattern> FactoryPatterns::create()
{
    return { baby(), babyTwice(), chirp(), transformer(), flare(), scribble() };
}
