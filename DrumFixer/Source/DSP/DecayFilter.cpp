#include "DecayFilter.h"
#include "Filters/ModeBandpass.h"
#include "LevelDetectors/RMSDetector.h"
#include <numeric>

DecayFilter::DecayFilter (Params& params, const AudioBuffer<float>& audio, double fs) :
    params (params),
    sampleBuffer (audio),
    sampleFs (fs)
{
    updateFilter();
}

std::unique_ptr<XmlElement> DecayFilter::toXml()
{
    auto filterXml = std::make_unique<XmlElement> ("FilterState");

    filterXml->setAttribute ("CenterFreq", params.centerFreq);
    filterXml->setAttribute ("Bandwidth", params.bandwidth);
    filterXml->setAttribute ("DesiredT60", params.desiredT60);

    return filterXml;
}

std::unique_ptr<DecayFilter> DecayFilter::fromXml (XmlElement* xml, const AudioBuffer<float>& buffer, float fs)
{
    auto fc = (float) xml->getDoubleAttribute ("CenterFreq");
    auto bw = (float) xml->getDoubleAttribute ("Bandwidth");
    auto dt60 = (float) xml->getDoubleAttribute ("DesiredT60");

    Params params (fc, bw, dt60);
    return std::make_unique<DecayFilter> (params, buffer, fs);
}

void DecayFilter::updateFilter()
{
    actualT60 = getActualT60 (params, sampleBuffer, sampleFs);
    auto filterQ = params.centerFreq / jmax (params.bandwidth, 0.1f);

    auto gainDes  = getGainForT60 (params.desiredT60, sampleRate);
    auto gainOrig = getGainForT60 (actualT60, sampleRate);
    filtGain = gainDes / gainOrig;

    for (int ch = 0; ch < 2; ++ch)
        bell[ch].calcCoefs (params.centerFreq, filterQ, -60.0f);
}

void DecayFilter::prepare (double fs, int samplesPerBlock)
{
    sampleRate = (float) fs;
    updateFilter();

    filtBuffer.setSize (2, samplesPerBlock);
}

void DecayFilter::processBlock (AudioBuffer<float>& buffer)
{
    if (curGain[0] == 0.0f && sampleIdx < 0) // nothing to do...
        return;

    // do processing
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        filtBuffer.copyFrom (ch, 0, buffer, ch, 0, buffer.getNumSamples());
        bell[ch].processBlock (filtBuffer.getWritePointer (ch), buffer.getNumSamples());
    }

    if (sampleIdx > 0) // new transient
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* filtData = filtBuffer.getReadPointer (ch);
            auto* xData = buffer.getWritePointer (ch);

            curGain[ch] = 1.0f;
            for (int n = sampleIdx; n < buffer.getNumSamples(); ++n)
            {
                xData[n] = filtData[n] * (1.0f - curGain[ch]) + xData[n] * curGain[ch];
                curGain[ch] *= filtGain;
            }
        }

        sampleIdx = -1;
    }
    else
    {
        // check if filter has been running for too long (and reset if it has)
        if (curGain[0] < Decibels::decibelsToGain (-60.0f)
            || curGain[0] > Decibels::decibelsToGain (30.0f))
        {
            curGain[0] = 0.0f; curGain[1] = 0.0f;
            return;
        }

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* filtData = filtBuffer.getReadPointer (ch);
            auto* xData = buffer.getWritePointer (ch);

            for (int n = 0; n < buffer.getNumSamples(); ++n)
            {
                xData[n] = filtData[n] * (1.0f - curGain[ch]) + xData[n] * curGain[ch];
                curGain[ch] *= filtGain;
            }
        }
    }
}

float DecayFilter::getActualT60 (Params& p, const AudioBuffer<float>& audio, double fs)
{
    if (audio.hasBeenCleared())
        return 0.1f;

    // set up analysis buffer
    auto nChannels = audio.getNumChannels();
    AudioBuffer<float> analysisBuffer;
    analysisBuffer.setSize (1, audio.getNumSamples());
    for (int ch = 0; ch < nChannels; ++ch)
        analysisBuffer.addFrom (0, 0, audio, ch, 0, audio.getNumSamples(), 1.0f / (float) nChannels);

    // Filter around mode
    ModeBandpass modeBPF (p.centerFreq, p.bandwidth, fs);
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
