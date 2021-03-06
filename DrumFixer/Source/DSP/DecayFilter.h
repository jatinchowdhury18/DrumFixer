#ifndef DECAYFILTER_H_INCLUDED
#define DECAYFILTER_H_INCLUDED

#include "Filters/Bell.h"

class DecayFilter
{
public:
    struct Params
    {
        Params (float fc, float bw, float tau) :
            centerFreq (fc),
            bandwidth (bw),
            desiredT60 (tau)
        {}

        Params (Params& p) :
            centerFreq (p.centerFreq),
            bandwidth (p.bandwidth),
            desiredT60 (p.desiredT60)
        {}

        float centerFreq;
        float bandwidth;
        float desiredT60;
    };

    DecayFilter (Params& params, const AudioBuffer<float>& audio, double fs);

    Params& getParams() { return params; }
    
    void updateFilter();
    void prepare (double sampleRate, int samplesPerBlock);
    void processBlock (AudioBuffer<float>& buffer);
    void newTransient (int newSampleIdx) { sampleIdx = newSampleIdx; }

    std::unique_ptr<XmlElement> toXml();
    static std::unique_ptr<DecayFilter> fromXml (XmlElement* xml, const AudioBuffer<float>& buffer, float fs);

    bool isSelected() const noexcept { return selected; }
    void setSelected (bool shouldBeSelected) { selected = shouldBeSelected; }

private:
    bool selected = false;

    static float getActualT60 (Params& params, const AudioBuffer<float>& audio, double fs);
    static float getSlope (const std::vector<float> x, const std::vector<float> y);
    static float getGainForT60 (float t60, float sampleRate);

    Params params;
    float actualT60;
    const float sampleFs = 48000.0f;
    const AudioBuffer<float>& sampleBuffer;

    float sampleRate = 48000.0f;
    BellFilter bell[2];
    AudioBuffer<float> filtBuffer;

    int sampleIdx = -1;
    float curGain[2] = { 0.0f, 0.0f };
    float filtGain = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecayFilter)
};

#endif // DECAYFILTER_H_INCLUDED
