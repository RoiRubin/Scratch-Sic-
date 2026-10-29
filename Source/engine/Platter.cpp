#include "Platter.h"
#include <cmath>

namespace
{
    constexpr double heaviestLagSeconds = 0.012;   // per stage, at full weight
}

void Platter::prepare(double sampleRate, float weight)
{
    const double lag = (double) weight * heaviestLagSeconds;
    coeff = lag < 1.0e-6 ? 1.0 : 1.0 - std::exp(-1.0 / (lag * sampleRate));
}

void Platter::reset(double position) noexcept
{
    stage1 = stage2 = position;
}

double Platter::process(double target) noexcept
{
    stage1 += coeff * (target - stage1);
    stage2 += coeff * (stage1 - stage2);
    return stage2;
}
