from typing import Any, List, Callable
import math
import struct
import subprocess

# This is not 32-bit ofcourse only to indicate that what we're using is a 32 bit floating point number
Float32 = float 
Sample = Float32
Second = Float32
Hz = Float32
Semitone = Float32
Beat = Float32

# https://en.wikipedia.org/wiki/A440_(pitch_standard)
PITCH_STANDARD: Hz = 440
SAMPLE_RATE = 48_000
MASTER_VOLUME: Float32 = 0.1
BPM = 120
BEAT_DURATION = 60.0 / BPM

def gentle_piano_wave(cycle: Float32) -> Sample:
    TAU = math.pi * 2
    return (
        1.00 * math.sin(TAU*cycle) +
        0.08 * math.sin(TAU*2*cycle) +
        0.025 * math.sin(TAU*3*cycle) +
        0.01 * math.sin(TAU*5*cycle))

def square_wave(cycle: Float32) -> Sample:
    frac = cycle - math.floor(cycle)
    if frac < 0.5:
        return -1.0
    else:
        return 1.0

def synthesize(
        hz: Hz, 
        duration: Second, 
        wave_fn: Callable[[Float32], Sample],
        attack_dur: Float32, 
        decay_dur: Float32, 
        sustain_level: Float32, 
        release_dur: Float32) -> list[Sample]:
    samples_amount = int(SAMPLE_RATE * duration)
    samples = []
    amplitude = 0
    prev_amplitude = 0
    state = "attack" # "attack", "decay", "sustain", "release"

    phase = 0

    attack_dur = attack_dur*samples_amount
    decay_dur  = decay_dur*samples_amount
    release_dur = release_dur*samples_amount
    sustain_dur = samples_amount - (attack_dur + decay_dur + release_dur)

    for i in range(samples_amount):
        match state:
            case "attack":
                amplitude = phase/attack_dur
                if phase + 1 == attack_dur:
                    prev_amplitude = amplitude
                    phase = 0
                    state = "decay"
            case "decay":
                amplitude = prev_amplitude*(1 - (1 - sustain_level) * phase/decay_dur)
                if phase + 1 == decay_dur:
                    prev_amplitude = amplitude
                    phase = 0
                    state = "sustain"
            case "sustain":
                if phase + 1 == sustain_dur:
                    prev_amplitude = amplitude
                    phase = 0
                    state = "release"
            case "release":
                amplitude = prev_amplitude * (1 - phase/release_dur)
        sample = wave_fn(hz*(i/SAMPLE_RATE))*amplitude*MASTER_VOLUME
        samples.append(sample)
        phase += 1
    return samples

def note(semitone: Semitone, beats: Beat) -> List[Sample]:
    hz = PITCH_STANDARD * ((2 ** (1/12)) ** semitone)
    return synthesize(hz, beats * BEAT_DURATION, 
                      gentle_piano_wave, 0.008, 0.6, 0.25, 0.35)

def samples_to_bytes(samples: list[Sample], be: bool = False) -> bytes:
    endian = ">" if be else "<"
    return struct.pack(f"{endian}{len(samples)}f", *samples)

def play(wave: list[Sample]):
    outputfile = "output.bin"
    with open(outputfile, "wb") as file:
        file.write(samples_to_bytes(wave))
    subprocess.run([ "ffplay", "-showmode", "1", "-f", "f32le", "-ar", str(SAMPLE_RATE), outputfile])

import wave
def save_wave(samples: list[Sample], filename: str):
    # Convert float [-1, 1] -> signed 16-bit PCM
    pcm = [
        max(-1.0, min(1.0, sample)) * 32767
        for sample in samples
    ]

    pcm_bytes = struct.pack(f"<{len(pcm)}h", *map(int, pcm))

    with wave.open(filename, "wb") as wav:
        wav.setnchannels(1)          # mono
        wav.setsampwidth(2)          # 16-bit
        wav.setframerate(SAMPLE_RATE)
        wav.writeframes(pcm_bytes)


def mix_sum(wave1: List[Sample], wave2: List[Sample]) -> List[Sample]:
    # Pad shorter waveform with zeros
    max_len = max(len(wave1), len(wave2))
    wave1.extend([0.0] * (max_len - len(wave1)))
    wave2.extend([0.0] * (max_len - len(wave2)))

    # Mix (sum) the waves
    mixed_wave = [wave1[i] + wave2[i] for i in range(max_len)]
    
    # Normalize to avoid clipping
    max_amplitude = max(abs(sample) for sample in mixed_wave) or 1.0  # Avoid division by zero
    normalization_factor = 1.0 / max_amplitude
    return [sample * normalization_factor for sample in mixed_wave]

def mix_interpolated(wave1: List[float], wave2: List[float], alpha: float) -> List[float]:
    """Mixes two waveforms using linear interpolation.
    
    Args:
        wave1: First waveform (List of sample values).
        wave2: Second waveform.
        alpha: Mixing ratio (0.0 = only wave1, 1.0 = only wave2).
        
    Returns:
        A new mixed waveform.
    """
    max_len = max(len(wave1), len(wave2))
    
    # Pad shorter wave with zeros
    wave1.extend([0.0] * (max_len - len(wave1)))
    wave2.extend([0.0] * (max_len - len(wave2)))

    # Apply linear interpolation
    mixed_wave = [(1 - alpha) * wave1[i] + alpha * wave2[i] for i in range(max_len)]

    return mixed_wave

def concat(list_of_items: List[List[Any]]):
    return [item for sublist in list_of_items for item in sublist]

BASE_OCTAVE = 0
AMOUNT = 5

class Note(object):
    def __init__(self, semitone: Semitone):
        self.semitone = semitone

    def __call__(self, beats: Beat) -> List[Sample]:
        return note(self.semitone, beats)


def VOID(beats: Beat) -> List[Sample]:
    samples_amount = int(SAMPLE_RATE * beats * BEAT_DURATION)
    return  [ 0 for _ in range(samples_amount) ]

C  = [ Note(BASE_OCTAVE +  0 + 12 * i) for i in range(AMOUNT) ] 
Cs = [ Note(BASE_OCTAVE +  1 + 12 * i) for i in range(AMOUNT) ] 
D  = [ Note(BASE_OCTAVE +  2 + 12 * i) for i in range(AMOUNT) ] 
Ds = [ Note(BASE_OCTAVE +  3 + 12 * i) for i in range(AMOUNT) ] 
E  = [ Note(BASE_OCTAVE +  4 + 12 * i) for i in range(AMOUNT) ] 
F  = [ Note(BASE_OCTAVE +  5 + 12 * i) for i in range(AMOUNT) ] 
Fs = [ Note(BASE_OCTAVE +  6 + 12 * i) for i in range(AMOUNT) ] 
G  = [ Note(BASE_OCTAVE +  7 + 12 * i) for i in range(AMOUNT) ] 
Gs = [ Note(BASE_OCTAVE +  8 + 12 * i) for i in range(AMOUNT) ] 
A  = [ Note(BASE_OCTAVE +  9 + 12 * i) for i in range(AMOUNT) ] 
As = [ Note(BASE_OCTAVE + 10 + 12 * i) for i in range(AMOUNT) ] 
B  = [ Note(BASE_OCTAVE + 11 + 12 * i) for i in range(AMOUNT) ] 

Aa = [ Note(BASE_OCTAVE +  8 + 12 * i) for i in range(AMOUNT) ] 
Bb = [ Note(BASE_OCTAVE + 10 + 12 * i) for i in range(AMOUNT) ] 
Dd = [ Note(BASE_OCTAVE +  1 + 12 * i) for i in range(AMOUNT) ] 

BASE_BEAT = 0.375
my_princess = lambda : concat([
    As[0](BASE_BEAT*2),
    F [0](BASE_BEAT*2),
    F [1](BASE_BEAT*2),
    Ds[1](BASE_BEAT*2),
    VOID (BASE_BEAT*2),
    D [1](BASE_BEAT*2),
    C [1](BASE_BEAT*2),
    As[0](BASE_BEAT*2),

    As[0](BASE_BEAT*2),
    F [0](BASE_BEAT*2),
    F [1](BASE_BEAT*2),
    Ds[1](BASE_BEAT*2),
    VOID (BASE_BEAT*2),
    D [1](BASE_BEAT*2),
    C [1](BASE_BEAT*2),
    As[0](BASE_BEAT*2),

    As[0](BASE_BEAT*2),
    F [0](BASE_BEAT*2),
    F [1](BASE_BEAT*2),
    Ds[1](BASE_BEAT*2),
    VOID (BASE_BEAT*1),
    D [1](BASE_BEAT*2),
    C [1](BASE_BEAT*2),
    D [1](BASE_BEAT*2),
    F [1](BASE_BEAT*2),
    As[0](BASE_BEAT*2),
    As[1](BASE_BEAT*2),
    A [1](BASE_BEAT*2),
    VOID (BASE_BEAT*1),
    F [1](BASE_BEAT*2),
    G [1](BASE_BEAT*2),
    F [1](BASE_BEAT*2),

    As[0](BASE_BEAT*2),
    F [0](BASE_BEAT*2),
    F [1](BASE_BEAT*2),
    Ds[1](BASE_BEAT*2),
    VOID (BASE_BEAT*1),
    D [1](BASE_BEAT*2),
    Ds[1](BASE_BEAT*2),
    F [1](BASE_BEAT*2),
    Fs[1](BASE_BEAT*2),
    G [1](BASE_BEAT*2),
    A [1](BASE_BEAT*2),
    As[1](BASE_BEAT*2),
    VOID (BASE_BEAT*1),
    F [1](BASE_BEAT*2),
    Ds[1](BASE_BEAT*2),
    D [1](BASE_BEAT*2),

    As[0](BASE_BEAT*2),
    F [0](BASE_BEAT*2),
    F [1](BASE_BEAT*2),
    Ds[1](BASE_BEAT*2),
    VOID (BASE_BEAT*2),
    D [1](BASE_BEAT*2),
    C [1](BASE_BEAT*2),
    As[0](BASE_BEAT*2),
    As[0](BASE_BEAT*2),

    As[0](BASE_BEAT*2),
    F [0](BASE_BEAT*2),
    D [1](BASE_BEAT*2),
    F [1](BASE_BEAT*2),
    A [1](BASE_BEAT*2),
    As[1](BASE_BEAT*2),
    A [1](BASE_BEAT*2),
    F [1](BASE_BEAT*2),

    # REFF

    As[1](BASE_BEAT*1),
    F [1](BASE_BEAT*1),
    Ds[1](BASE_BEAT*1),
    D [1](BASE_BEAT*1),
    C [1](BASE_BEAT*1),
    As[0](BASE_BEAT*1),
    As[0](BASE_BEAT*1),
    F [0](BASE_BEAT*1),

    As[1](BASE_BEAT*1),
    F [1](BASE_BEAT*1),
    Ds[1](BASE_BEAT*1),
    D [1](BASE_BEAT*1),
    C [1](BASE_BEAT*1),
    As[0](BASE_BEAT*1),
    As[0](BASE_BEAT*1),
    F [0](BASE_BEAT*1),

    As[1](BASE_BEAT*1),
    F [1](BASE_BEAT*1),
    Ds[1](BASE_BEAT*1),
    D [1](BASE_BEAT*1),
    C [1](BASE_BEAT*1),
    As[0](BASE_BEAT*1),
    D [1](BASE_BEAT*0.5),
    F [1](BASE_BEAT*0.5),
    A [1](BASE_BEAT*0.5),
    As[1](BASE_BEAT*1.5),

    F [1](BASE_BEAT*1),
    Ds[1](BASE_BEAT*1),
    D [1](BASE_BEAT*1),
    C [1](BASE_BEAT*1),
    As[0](BASE_BEAT*1),
    As[0](BASE_BEAT*1),
    F [0](BASE_BEAT*1),

    As[1](BASE_BEAT*1),
    F [1](BASE_BEAT*1),
    Ds[1](BASE_BEAT*1),
    D [1](BASE_BEAT*1),
    C [1](BASE_BEAT*1),
    As[0](BASE_BEAT*1),
    As[0](BASE_BEAT*1),
    F [0](BASE_BEAT*1),

    As[1](BASE_BEAT*1),
    F [1](BASE_BEAT*1),
    Ds[1](BASE_BEAT*1),
    D [1](BASE_BEAT*1),
    C [1](BASE_BEAT*1),
    As[0](BASE_BEAT*1),
    D [1](BASE_BEAT*0.5),
    F [1](BASE_BEAT*0.5),
    A [1](BASE_BEAT*0.5),
    As[1](BASE_BEAT*1),
])

shooting_stars = lambda : concat([
    F [1](BASE_BEAT*3),
    A [0](BASE_BEAT*2),
    As[0](BASE_BEAT*3),

    VOID(BASE_BEAT*1),

    F [0](BASE_BEAT*2),
    As[0](BASE_BEAT*2),
    D [1](BASE_BEAT*2),
    C [1](BASE_BEAT*2),
    VOID(BASE_BEAT*1),
    As[0](BASE_BEAT*3),
    D [1](BASE_BEAT*4),
    VOID(BASE_BEAT*1),

    D [1](BASE_BEAT*2),
    Ds[1](BASE_BEAT*2),
    F [1](BASE_BEAT*3),
    A [0](BASE_BEAT*3),
    As[0](BASE_BEAT*2),
    VOID(BASE_BEAT*1),
    F [0](BASE_BEAT*2),
    As[0](BASE_BEAT*2),
    D [1](BASE_BEAT*2),
    C [1](BASE_BEAT*3),
    D [1](BASE_BEAT*3),
    As[0](BASE_BEAT*2),

    D [1](BASE_BEAT*3),
    C [1](BASE_BEAT*3),
    As[0](BASE_BEAT*2),

    F [1](BASE_BEAT*3),
    A [0](BASE_BEAT*2),
    As[0](BASE_BEAT*3),

    VOID(BASE_BEAT*1),

    F [0](BASE_BEAT*2),
    As[0](BASE_BEAT*2),
    D [1](BASE_BEAT*2),
    C [1](BASE_BEAT*2),
    VOID(BASE_BEAT*1),
    As[0](BASE_BEAT*3),
    D [1](BASE_BEAT*4),
    VOID(BASE_BEAT*1),

    D [1](BASE_BEAT*2),
    Ds[1](BASE_BEAT*2),
    F [1](BASE_BEAT*3),
    A [0](BASE_BEAT*3),
    As[0](BASE_BEAT*2),
    VOID(BASE_BEAT*1),
    F [0](BASE_BEAT*2),
    As[0](BASE_BEAT*2),
    D [1](BASE_BEAT*2),
    C [1](BASE_BEAT*3),
    D [1](BASE_BEAT*3),
    As[0](BASE_BEAT*2),
])

play(shooting_stars())

