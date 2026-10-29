#pragma once

// A real record is heavy: it can't change direction instantly. This follows the drawn needle position
// with a little lag (two smoothing stages in a row), so sharp corners in the graph turn into the
// rounded reversals of a hand-moved platter. The weight sets how heavy the platter feels.
class Platter
{
public:
    void prepare(double sampleRate, float weight);
    void reset(double position) noexcept;
    double process(double target) noexcept;

private:
    double coeff = 1.0, stage1 = 0.0, stage2 = 0.0;
};
