#include "DecayFilter.h"
#include "Filters/ModeBandpass.h"
#include "LevelDetectors/RMSDetector.h"
#include <numeric>

DecayFilter::DecayFilter (Params& params, const AudioBuffer<float>& audio, double fs) :
    params (params),
    actualT60 (getActualT60 (params, audio, fs))
{
    updateFilter();
}

void DecayFilter::updateFilter()
{
    auto filterQ = params.centerFreq / params.bandwidth;

    auto gainDes  = getGainForT60 (params.desiredT60, sampleRate);
    auto gainOrig = getGainForT60 (actualT60, sampleRate);
    auto gFiltDB = Decibels::gainToDecibels (gainDes / gainOrig);

    for (int ch = 0; ch < 2; ++ch)
        bell[ch].calcCoefs (params.centerFreq, filterQ, gFiltDB);
}

void DecayFilter::prepare (double fs)
{
    sampleRate = (float) fs;
    updateFilter();
}

void DecayFilter::processBlock (AudioBuffer<float>& buffer)
{
}

float DecayFilter::getActualT60 (Params& params, const AudioBuffer<float>& audio, double fs)
{
    // set up analysis buffer
    auto nChannels = audio.getNumChannels();
    AudioBuffer<float> analysisBuffer;
    analysisBuffer.setSize (1, audio.getNumSamples());
    for (int ch = 0; ch < nChannels; ++ch)
        analysisBuffer.addFrom (0, 0, audio, ch, 0, audio.getNumSamples(), 1.0f / (float) nChannels);

    // Filter around mode
    ModeBandpass modeBPF (params.centerFreq, params.bandwidth, fs);
    modeBPF.processBlock (analysisBuffer);

    // Do RMS level detection
    RMSDetector rmsDetector;
    rmsDetector.reset ((float) fs);
    rmsDetector.setAttackMs (10.0f);
    rmsDetector.setReleaseMs (50.0f);
    rmsDetector.processBlock (analysisBuffer.getWritePointer (0), analysisBuffer.getNumSamples());

    auto magnitude = analysisBuffer.getMagnitude (0, audio.getNumSamples());
    analysisBuffer.applyGain (1.0f / magnitude);

    // compute slope
    std::vector<float> timeSamples;
    std::vector<float> envelopeDB;
    int sampCount = 0;
    const float dBTime = -30.0f;
    for (int n = 0; n < audio.getNumSamples(); ++n)
    {
        auto sampleDB = Decibels::gainToDecibels (analysisBuffer.getSample (0, n));
        if (sampleDB < -1.0f && sampCount == 0)
            continue;

        if (sampleDB < dBTime && sampCount > 0)
            break;

        timeSamples.push_back ((float) sampCount++);
        envelopeDB.push_back (sampleDB);
    }

    float slope = 0.0f;
    if (timeSamples.size() > 0)
        slope = getSlope (timeSamples, envelopeDB);

    return (dBTime / slope) / (float) fs;
}

float DecayFilter::getSlope (const std::vector<float> x, const std::vector<float> y)
{
    const auto n    = x.size();
    const auto s_x  = std::accumulate (x.begin(), x.end(), 0.0f);
    const auto s_y  = std::accumulate (y.begin(), y.end(), 0.0f);
    const auto s_xx = std::inner_product (x.begin(), x.end(), x.begin(), 0.0f);
    const auto s_xy = std::inner_product (x.begin(), x.end(), y.begin(), 0.0f);
    const auto a    = (n * s_xy - s_x * s_y) / (n * s_xx - s_x * s_x);
    return a;
}

float DecayFilter::getGainForT60 (float t60, float sampleRate)
{
    return pow (0.001f, 1.0f / (t60 * sampleRate));
}
