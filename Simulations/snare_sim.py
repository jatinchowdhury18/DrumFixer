import numpy as np
from scipy.io import wavfile
import scipy.signal as signal
import matplotlib.pyplot as plt
import audio_dspy as adsp
import scipy.stats as stats

def find_decay_time(freq, x, fs, filt_width, thresh=-60, eta=0.01, dBTime=-60):
    x_filt = adsp.filt_mode(x, freq, fs, filt_width)
    env = adsp.normalize(adsp.energy_envelope(x_filt, fs, eta))

    start = int(np.argwhere(20 * np.log10(env) > -1)[0])
    if len(np.argwhere(20 * np.log10(env[start:]) < thresh)) == 0:
        return 0
    end = int(np.argwhere(20 * np.log10(env[start:]) < thresh)[0])
    slope, _, R, _, _ = stats.linregress(
        np.arange(len(env[start:end])), 20 * np.log10(env[start:end]))
    if R**2 < 0.9:
        return 0
        
    return (dBTime / slope) / fs

def find_decay_times(freqs, x, fs, filt_width, thresh=-60, eta=0.01, dBTime=-60):
    times = [find_decay_time(freq, x, fs, filt_width, thresh, eta, dBTime) for freq in freqs]
    return times

fs, x = wavfile.read ('Snares/hi-snare-1.wav')
x = x[:,0] / 2**15

BANDWIDTH=200
freqs = np.arange(BANDWIDTH/2, 22000, BANDWIDTH)

# Analyze original signal
times = find_decay_times(freqs, x, fs, BANDWIDTH, thresh=-30, dBTime=-45)

time_peaks, _ = signal.find_peaks(times)
freq_widths,_,_,_ = signal.peak_widths(times, time_peaks)
print(freqs[time_peaks])
print(BANDWIDTH * freq_widths)

plt.figure()
f, t, Sxx = signal.spectrogram (x, fs)
plt.pcolormesh(t, f, 20*np.log10(np.abs(adsp.normalize(Sxx))), cmap='inferno', vmin=-180)
# plt.plot(times, freqs, 'r')
plt.xlim(0, 1.6)

# Filter spec
filter_freq = freqs[time_peaks[0]]
filterQ = filter_freq  / (BANDWIDTH * freq_widths[0])
print(filterQ)

original_decay_time = find_decay_time(filter_freq, x, fs,
    BANDWIDTH * freq_widths[0], thresh=-30)
desired_decay_time = 0.3

g_orig = adsp.delay_feedback_gain_for_t60(1, fs, original_decay_time)
g_des = adsp.delay_feedback_gain_for_t60(1, fs, desired_decay_time)
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

y = apply_filt(x, fs, filter_freq, filterQ, g_filt)

plt.figure()
f, t, Syy = signal.spectrogram (y, fs)
plt.pcolormesh(t, f, 20*np.log10(np.abs(adsp.normalize(Syy))), cmap='inferno', vmin=-180)
plt.xlim(0, 1.6)
plt.show()

wavfile.write('drum_out.wav', fs, y)
