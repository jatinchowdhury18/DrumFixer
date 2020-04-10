# %%
import numpy as np
import scipy.signal as signal
from scipy.io import wavfile
import audio_dspy as adsp

import matplotlib.pyplot as plt
from matplotlib import ticker
plt.style.use('dark_background')

# %% [markdown]
# # Modal Signal Processing and Drum Fixing
#
# In this article, we will briefly discuss modal signal
# processing, and show how we can use ideas from the modal
# approach to "fix" drum sounds.
#
# ## Modal Signal Processing
#
# In signal processing, it is often useful to describe a system
# by looking at the "modes" or resonant frequencies of the system.
# Modes are well known in the field of room acoustics, where
# any set of parallel walls will create a mode at the frequency
# corresponding to the distance between the walls. However, many
# systems can be described by their modal characteristics,
# including
# [bells](http://dafx2019.bcu.ac.uk/papers/DAFx2019_paper_34.pdf),
# [cymbals](http://dafx2019.bcu.ac.uk/papers/DAFx2019_paper_48.pdf),
# even [rocket nozzles](https://keirancantilina.github.io/project/rocket-bells/).
# 
# While there is a lot of physics that describes why certain objects
# have their specific modal characteristics, in signal processing
# we can often get by with simply measuring the modes and
# resynthesizing them. In order to do this, we need to find three
# parameters for each mode.
# - Mode frequency
# - Mode amplitude (and phase if you like)
# - Mode decay rate
#
#
# For example, if I have a bell with a fundamental frequency
# of 200 Hz, the first mode frequency will most likely be at
# 200 Hz. The amplitude of the mode is essentially the volume
# at which the first mode rings relative to how hard it is hit.
# The decay rate describes how long the bell rings out at that
# frequency. In bells, the higher frequency modes decay very
# quickly, while the lower modes ring out for a long time.
#
# ## Drum Ringing
#
# Now how does modal signal processing relate to drums? Drums can
# be synthesized using modal synthesis, but what about using modal
# ideas to improve the timbre of an existing drum recording?
# Let's examine a snare drum recording taken from
# [freesound.com](https://freesound.com).

# %%
def plot_specgram(x, fs, fLow=100, title=''):
    plt.specgram(x, NFFT=1024, noverlap=512, Fs=fs, cmap='inferno', scale='dB', vmin=-130)
    plt.yscale('log')
    plt.ylim(fLow, 20000)
    plt.gca().yaxis.set_major_formatter(ticker.ScalarFormatter())
    plt.title(title)
    plt.xlabel('Time [seconds]')
    plt.ylabel('Frequency [Hz]')

fs, x = wavfile.read('../Snares/hi-snare-1.wav')
x = x[:,0] / 2**15
plot_specgram(x, fs, 200, 'Snare Drum Spectrogram')

# %% [markdown]
# Note that the snare drum seems to ring out for a very long time
# somewhere around 1200 Hz. Listening to the audio file, confirms
# this: the snare has a nasty high "ring" that I don't find
# particularly pleasing. If I was a mixing engineer tasked with
# mixing this snare drum into a song, I would probably try
# using an EQ to make the snare sound quieter around these
# problematic frequencies.

# %%
def static_filt(freq, bandwidth, gainDB, sig):
    sig_filt = np.copy(sig)
    b,a = adsp.design_bell(freq, freq/bandwidth, 10**(gainDB/20), fs)
    sig_filt = signal.lfilter(b, a, sig_filt)
    return sig_filt

yEQ_30 = static_filt(1200, 200, -30, x)
yEQ_60 = static_filt(1200, 200, -60, x)

plt.figure()
plot_specgram(yEQ_30, fs, 200, 'Snare with -30dB EQ')

plt.figure()
plot_specgram(yEQ_60, fs, 200, 'Snare with -60dB EQ')

# %% [markdown]
# As you can see it takes a pretty drastic EQ to get the 1200 Hz
# mode to decay in a reasonable amount of time (relative to the
# rest of the drum sound), and using this much EQ kind of ruins
# the attack of the drum sound since it leaves a hole in the
# frequency spectrum.
#
# ## Modal Processing for Drums
#
# But what if we look at this drum ringing problem through the
# lens of modal signal processing? Essentially, the problem
# is that the drum has a mode at 1200 Hz with too long of a
# decay rate. Unfortunately standard EQ can only change the
# amplitude of this mode, not the decay rate.
#
# A couple months ago, my colleague Mark came up with a brilliantly
# simple solution to this problem. Why not use modal analysis to
# determine the decay rate of the ringing mode? Then you could
# implement an EQ filter with a dynamically changing gain that
# could adjust the decay rate of the mode to achieve some desired
# decay time. In other words, when the drum is struck, the filter
# won't affect the signal at all, but as the signal progresses,
# the filter will gradually damp more of the signal so as to
# supress the ringing.

# %%
from IPython.display import Image
Image(filename='../Screenshots/PlotBell.gif') 

# %% [markdown]
# Let's see how it works! First let's find the decay time of our
# problem mode. We can do this by filtering our signal around the
# mode frequency, measuring the envelope of the signal, and
# finding the slope of the decay. A convenient way to measure
# decay time is often using T60, or the time it takes for the
# signal to decay 60 Decibels.

# %%
import scipy.stats as stats
def find_decay_time(freq, x, fs, filt_width, thresh=-60, eta=0.005, dBTime=-60):
    x_filt = adsp.filt_mode(x, freq, fs, filt_width) 
    env = adsp.normalize(adsp.energy_envelope(x_filt, fs, eta))
    
    plt.plot(x_filt)
    plt.plot(env)
    plt.title('Energy Envelope of Filtered Signal')
    plt.xlabel('Time [samples]')

    start = int(np.argwhere(20 * np.log10(env) > -1)[0])
    if len(np.argwhere(20 * np.log10(env[start:]) < thresh)) == 0:
        return 0
    end = int(np.argwhere(20 * np.log10(env[start:]) < thresh)[0])
    slope, _, R, _, _ = stats.linregress(
        np.arange(len(env[start:end])), 20 * np.log10(env[start:end]))
    if R**2 < 0.9:
        return 0
        
    plt.figure()
    plt.plot(20*np.log10(np.abs(x_filt)))
    plt.plot(20*np.log10(np.abs(env)))
    plt.plot(np.arange(len(env))*slope, color='red', linewidth=3)
    plt.ylim(-60)
    plt.title('Linear Fit of Envelope Slope')
    plt.xlabel('Time [Samples]')
    plt.ylabel('Magnitude [dB]')

    return (dBTime / slope) / fs

print('Decay time: ' + str(find_decay_time(1200, x, fs, 200, thresh=-30, dBTime=-45)) + ' seconds')

# %% [markdown]
# So now that we've determined the T60 of the problematic mode
# to be ~1.5 seconds. Let's pick a desired decay time, say 0.5
# seconds. Now as discussed above, let's say we want to apply
# a time-varying filter to the signal that can create this change
# in T60. What should the the trajectory of the filter gain look
# like? I won't go through all the math here, but essentially,
# you find a gain envelope that corresponds to each T60 time,
# and divide them. When we apply the time-varying filter to our
# signal, here is the result:

# %%
g_orig = adsp.delay_feedback_gain_for_t60(1, fs, 1.5)
g_des = adsp.delay_feedback_gain_for_t60(1, fs, 0.5)
g_filt = g_des / g_orig

def apply_filt (x, fs, freq, Q, gain):
    filt = adsp.Filter(2, fs)
    y = np.zeros_like(x)

    b, a = adsp.design_bell(freq, Q, 10**(-60/20), fs)
    filt.set_coefs(b, a)

    for n in range(len(x)):
        curGain = gain**n
        y[n] = filt.process_sample(x[n]) * (1.0 - curGain) + x[n] * curGain
    return y

y = apply_filt(x, fs, 1200, 1200 / 100, g_filt)

plt.figure()
plot_specgram(x, fs, 200, 'Dry Signal')

plt.figure()
plot_specgram(y, fs, 200, 'Processed Signal')

# %% [markdown]
# From looking at these spectrograms we can see that the ringing
# frequencies have been damped pretty succesfully, without
# affecting the attack of the drum too much. From fine-tuning
# the filter frequency, bandwidth, and desired decay time, it is
# possible to obtain an even better result.
#
# ## Implementation
#
# To make this tool available to mixing engineers and producers
# I've built this algorithm into an audio plugin (VST/AU),
# using the JUCE/C++ framework. To see the source code and
# try it for yourself, check out our
# [GitHub page](https://github.com/jatinchowdhury18/DrumFixer).
#
# Thanks for reading!


# %%
