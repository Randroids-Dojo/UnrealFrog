#!/usr/bin/env python3
"""
Procedural 8-bit sound effect generator for UnrealFrog.

Generates 9 deterministic WAV files using square/saw waves with
attack/decay envelopes for authentic arcade-style sound.

Output: 16-bit PCM WAV, 44100Hz, mono
Dependencies: numpy (+ standard library)

Usage:
    python3 Tools/AudioGen/generate_sfx.py

All output written to Content/Audio/SFX/
"""

import os
import struct
import wave

import numpy as np

# Constants
SAMPLE_RATE = 44100
BIT_DEPTH = 16
MAX_AMP = 32767  # max value for signed 16-bit
OUTPUT_DIR = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..", "..", "Content", "Audio", "SFX"
)


def ensure_output_dir():
    os.makedirs(OUTPUT_DIR, exist_ok=True)


def write_wav(filename: str, samples: np.ndarray):
    """Write a mono 16-bit PCM WAV file."""
    filepath = os.path.join(OUTPUT_DIR, filename)
    # Clip and convert to int16
    clipped = np.clip(samples, -1.0, 1.0)
    pcm = (clipped * MAX_AMP).astype(np.int16)

    with wave.open(filepath, "w") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)  # 16-bit = 2 bytes
        wf.setframerate(SAMPLE_RATE)
        wf.writeframes(pcm.tobytes())

    size = os.path.getsize(filepath)
    print(f"  {filename}: {len(pcm)} samples, {size} bytes")


def time_array(duration: float) -> np.ndarray:
    """Create a time array for the given duration."""
    return np.linspace(0, duration, int(SAMPLE_RATE * duration), endpoint=False)


def square_wave(t: np.ndarray, freq: float) -> np.ndarray:
    """Generate a square wave at the given frequency."""
    return np.sign(np.sin(2.0 * np.pi * freq * t))


def saw_wave(t: np.ndarray, freq: float) -> np.ndarray:
    """Generate a sawtooth wave at the given frequency."""
    return 2.0 * (freq * t - np.floor(0.5 + freq * t))


def pulse_wave(t: np.ndarray, freq: float, duty: float = 0.25) -> np.ndarray:
    """Generate a pulse wave with variable duty cycle (NES-style)."""
    phase = (freq * t) % 1.0
    return np.where(phase < duty, 1.0, -1.0)


def noise(length: int, seed: int = 42) -> np.ndarray:
    """Generate deterministic white noise."""
    rng = np.random.RandomState(seed)
    return rng.uniform(-1.0, 1.0, length)


def envelope_ad(length: int, attack_frac: float, decay_frac: float) -> np.ndarray:
    """Attack-decay envelope. Remaining portion is silence."""
    env = np.zeros(length)
    attack_len = int(length * attack_frac)
    decay_start = attack_len
    decay_len = int(length * decay_frac)

    # Attack: ramp up
    if attack_len > 0:
        env[:attack_len] = np.linspace(0.0, 1.0, attack_len)

    # Decay: ramp down
    if decay_len > 0:
        end = min(decay_start + decay_len, length)
        env[decay_start:end] = np.linspace(1.0, 0.0, end - decay_start)

    return env


def envelope_adsr(length: int, a: float, d: float, s_level: float, r: float) -> np.ndarray:
    """ADSR envelope with fractional durations."""
    env = np.zeros(length)
    a_len = int(length * a)
    d_len = int(length * d)
    r_len = int(length * r)
    s_len = length - a_len - d_len - r_len
    if s_len < 0:
        s_len = 0

    idx = 0
    # Attack
    if a_len > 0:
        env[idx:idx + a_len] = np.linspace(0.0, 1.0, a_len)
        idx += a_len
    # Decay
    if d_len > 0:
        env[idx:idx + d_len] = np.linspace(1.0, s_level, d_len)
        idx += d_len
    # Sustain
    if s_len > 0:
        env[idx:idx + s_len] = s_level
        idx += s_len
    # Release
    if r_len > 0:
        remaining = length - idx
        env[idx:idx + remaining] = np.linspace(s_level, 0.0, remaining)

    return env


def sweep_frequency(t: np.ndarray, f_start: float, f_end: float) -> np.ndarray:
    """Generate a square wave with linear frequency sweep."""
    # Instantaneous frequency linearly interpolated
    freqs = np.linspace(f_start, f_end, len(t))
    # Integrate frequency to get phase
    phase = np.cumsum(freqs / SAMPLE_RATE) * 2.0 * np.pi
    return np.sign(np.sin(phase))


# ─── Sound Generators ────────────────────────────────────────────────

def generate_hop():
    """SFX_Hop.wav: Quick ascending pitch sweep (100Hz -> 400Hz), 0.1s."""
    duration = 0.1
    t = time_array(duration)
    signal = sweep_frequency(t, 100.0, 400.0)
    env = envelope_ad(len(t), attack_frac=0.05, decay_frac=0.95)
    return signal * env * 0.8


def generate_death_squish():
    """SFX_DeathSquish.wav: Noise burst with descending pitch, 0.3s."""
    duration = 0.3
    t = time_array(duration)
    n_samples = len(t)

    # Descending square wave tone (300Hz -> 60Hz)
    tone = sweep_frequency(t, 300.0, 60.0)

    # Noise burst, louder at start
    nz = noise(n_samples, seed=101)
    noise_env = envelope_ad(n_samples, attack_frac=0.01, decay_frac=0.5)

    # Mix: tone + noise
    signal = tone * 0.5 + nz * noise_env * 0.5

    # Overall decay envelope
    env = envelope_ad(n_samples, attack_frac=0.02, decay_frac=0.98)
    return signal * env * 0.8


def generate_death_splash():
    """SFX_DeathSplash.wav: Filtered noise with reverb-like decay, 0.4s."""
    duration = 0.4
    t = time_array(duration)
    n_samples = len(t)

    # Filtered noise: use moving average to low-pass
    raw_noise = noise(n_samples, seed=202)

    # Simple low-pass via cumulative averaging (window of ~20 samples at 44100Hz)
    kernel_size = 20
    kernel = np.ones(kernel_size) / kernel_size
    filtered = np.convolve(raw_noise, kernel, mode="same")

    # Add a subtle descending tone for "splash" feel
    tone = sweep_frequency(t, 200.0, 50.0) * 0.3

    signal = filtered * 0.7 + tone * 0.3

    # Long decay envelope simulating reverb
    env = np.exp(-4.0 * t / duration)
    # Quick attack
    attack_samples = int(0.01 * SAMPLE_RATE)
    env[:attack_samples] *= np.linspace(0.0, 1.0, attack_samples)

    return signal * env * 0.8


def generate_death_offscreen():
    """SFX_DeathOffScreen.wav: Two-tone descending beep, 0.3s."""
    duration = 0.3
    t = time_array(duration)
    n_samples = len(t)
    half = n_samples // 2

    # First beep: higher pitch
    signal = np.zeros(n_samples)
    t1 = t[:half]
    signal[:half] = square_wave(t1, 440.0)

    # Second beep: lower pitch
    t2 = t[half:]
    signal[half:] = square_wave(t2, 220.0)

    # Per-beep envelopes (quick attack, moderate decay)
    env = np.zeros(n_samples)
    env[:half] = envelope_ad(half, attack_frac=0.05, decay_frac=0.85)
    env[half:] = envelope_ad(n_samples - half, attack_frac=0.05, decay_frac=0.85)

    return signal * env * 0.7


def generate_home_slot():
    """SFX_HomeSlot.wav: Ascending arpeggio C-E-G, 0.3s."""
    duration = 0.3
    t = time_array(duration)
    n_samples = len(t)

    # C4=261.63, E4=329.63, G4=392.00
    notes = [261.63, 329.63, 392.00]
    third = n_samples // 3

    signal = np.zeros(n_samples)
    for i, freq in enumerate(notes):
        start = i * third
        end = start + third if i < 2 else n_samples
        segment_t = time_array((end - start) / SAMPLE_RATE)
        # Use pulse wave for chime-like quality
        segment = pulse_wave(segment_t, freq, duty=0.25)
        # Per-note envelope
        seg_env = envelope_ad(end - start, attack_frac=0.05, decay_frac=0.9)
        signal[start:end] = segment[:end - start] * seg_env

    return signal * 0.7


def generate_round_complete():
    """SFX_RoundComplete.wav: Major scale run + held chord, 1.5s."""
    duration = 1.5
    n_samples = int(SAMPLE_RATE * duration)

    # C major scale ascending: C4, D4, E4, F4, G4, A4, B4, C5
    scale_freqs = [261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25]

    # Scale run takes first 0.8s
    run_duration = 0.8
    run_samples = int(SAMPLE_RATE * run_duration)
    note_len = run_samples // len(scale_freqs)

    signal = np.zeros(n_samples)

    # Scale run
    for i, freq in enumerate(scale_freqs):
        start = i * note_len
        end = start + note_len
        seg_t = time_array(note_len / SAMPLE_RATE)
        seg = square_wave(seg_t, freq)
        seg_env = envelope_ad(note_len, attack_frac=0.05, decay_frac=0.85)
        signal[start:end] = seg[:note_len] * seg_env

    # Held chord: C5 + E5 + G5 for remaining 0.7s
    chord_start = run_samples
    chord_len = n_samples - chord_start
    chord_t = np.linspace(0, chord_len / SAMPLE_RATE, chord_len, endpoint=False)

    c5 = square_wave(chord_t, 523.25)
    e5 = square_wave(chord_t, 659.25)
    g5 = square_wave(chord_t, 783.99)

    chord = (c5 + e5 + g5) / 3.0
    chord_env = envelope_adsr(chord_len, a=0.05, d=0.1, s_level=0.7, r=0.3)
    signal[chord_start:] = chord * chord_env

    return signal * 0.7


def generate_game_over():
    """SFX_GameOver.wav: Descending minor arpeggio, 1.0s."""
    duration = 1.0
    n_samples = int(SAMPLE_RATE * duration)

    # Descending C minor: C5, G4, Eb4, C4
    notes = [523.25, 392.00, 311.13, 261.63]
    note_len = n_samples // len(notes)

    signal = np.zeros(n_samples)

    for i, freq in enumerate(notes):
        start = i * note_len
        end = start + note_len if i < len(notes) - 1 else n_samples
        seg_len = end - start
        seg_t = time_array(seg_len / SAMPLE_RATE)
        # Use saw wave for a sadder, thicker tone
        seg = saw_wave(seg_t, freq)
        seg_env = envelope_adsr(seg_len, a=0.03, d=0.15, s_level=0.5, r=0.3)
        signal[start:end] = seg[:seg_len] * seg_env

    # Overall fade
    overall = np.linspace(1.0, 0.3, n_samples)
    return signal * overall * 0.7


def generate_extra_life():
    """SFX_ExtraLife.wav: Classic ascending 5-note pattern, 0.5s."""
    duration = 0.5
    n_samples = int(SAMPLE_RATE * duration)

    # Classic 1-up pattern: E5, G5, E6, C6, D6
    notes = [659.25, 783.99, 1318.51, 1046.50, 1174.66]
    note_len = n_samples // len(notes)

    signal = np.zeros(n_samples)

    for i, freq in enumerate(notes):
        start = i * note_len
        end = start + note_len if i < len(notes) - 1 else n_samples
        seg_len = end - start
        seg_t = time_array(seg_len / SAMPLE_RATE)
        seg = pulse_wave(seg_t, freq, duty=0.5)
        seg_env = envelope_ad(seg_len, attack_frac=0.05, decay_frac=0.85)
        signal[start:end] = seg[:seg_len] * seg_env

    return signal * 0.7


def generate_timer_warning():
    """SFX_TimerWarning.wav: Sharp pulse for rapid repetition, 0.2s."""
    duration = 0.2
    t = time_array(duration)
    n_samples = len(t)

    # Sharp square wave tick at 880Hz (high A)
    signal = square_wave(t, 880.0)

    # Very fast attack, quick decay — designed to sound crisp when repeated
    env = envelope_ad(n_samples, attack_frac=0.02, decay_frac=0.6)

    # Add silence padding at the end so it doesn't clip into the next repeat
    return signal * env * 0.7


# ─── Main ────────────────────────────────────────────────────────────

SOUNDS = [
    ("SFX_Hop.wav", generate_hop),
    ("SFX_DeathSquish.wav", generate_death_squish),
    ("SFX_DeathSplash.wav", generate_death_splash),
    ("SFX_DeathOffScreen.wav", generate_death_offscreen),
    ("SFX_HomeSlot.wav", generate_home_slot),
    ("SFX_RoundComplete.wav", generate_round_complete),
    ("SFX_GameOver.wav", generate_game_over),
    ("SFX_ExtraLife.wav", generate_extra_life),
    ("SFX_TimerWarning.wav", generate_timer_warning),
]


def main():
    ensure_output_dir()
    print(f"Generating {len(SOUNDS)} sound effects...")
    print(f"Output: {os.path.abspath(OUTPUT_DIR)}")
    print(f"Format: {SAMPLE_RATE}Hz, {BIT_DEPTH}-bit, mono PCM WAV")
    print()

    for filename, generator in SOUNDS:
        samples = generator()
        write_wav(filename, samples)

    print()
    print(f"Done. {len(SOUNDS)} files written to {os.path.abspath(OUTPUT_DIR)}")


if __name__ == "__main__":
    main()
