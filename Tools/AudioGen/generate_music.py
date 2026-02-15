#!/usr/bin/env python3
"""
Procedural 8-bit music loop generator for UnrealFrog.

Generates 2 seamlessly looping music tracks using square/pulse waves
with NES-style two/three-voice arrangements.

Output: 16-bit PCM WAV, 44100Hz, mono
Dependencies: numpy (+ standard library)

Usage:
    python3 Tools/AudioGen/generate_music.py

All output written to Content/Audio/Music/
"""

import os
import wave

import numpy as np

# ─── Constants (must match generate_sfx.py) ───────────────────────────
SAMPLE_RATE = 44100
BIT_DEPTH = 16
MAX_AMP = 32767  # max value for signed 16-bit
OUTPUT_DIR = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..", "..", "Content", "Audio", "Music"
)


# ─── Wave Primitives (copied from generate_sfx.py) ────────────────────

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
    duration = len(pcm) / SAMPLE_RATE
    print(f"  {filename}: {len(pcm)} samples, {duration:.1f}s, {size} bytes")


def time_array(duration: float) -> np.ndarray:
    """Create a time array for the given duration."""
    return np.linspace(0, duration, int(SAMPLE_RATE * duration), endpoint=False)


def square_wave(t: np.ndarray, freq: float) -> np.ndarray:
    """Generate a square wave at the given frequency."""
    return np.sign(np.sin(2.0 * np.pi * freq * t))


def pulse_wave(t: np.ndarray, freq: float, duty: float = 0.25) -> np.ndarray:
    """Generate a pulse wave with variable duty cycle (NES-style)."""
    phase = (freq * t) % 1.0
    return np.where(phase < duty, 1.0, -1.0)


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


# ─── Note Frequency Table ─────────────────────────────────────────────
# Standard equal temperament frequencies for common notes.

NOTE_FREQ = {
    # Octave 2
    "A2": 110.00, "B2": 123.47,
    # Octave 3
    "C3": 130.81, "D3": 146.83, "E3": 164.81, "F3": 174.61,
    "G3": 196.00, "A3": 220.00, "B3": 246.94,
    # Octave 4
    "C4": 261.63, "D4": 293.66, "E4": 329.63, "F4": 349.23,
    "G4": 392.00, "A4": 440.00, "B4": 493.88,
    # Octave 5
    "C5": 523.25, "D5": 587.33, "E5": 659.25,
}


def note_duration(bpm: float, beats: float = 1.0) -> float:
    """Duration in seconds for a given number of beats at the given BPM."""
    return (60.0 / bpm) * beats


def render_note(freq: float, duration: float, wave_fn, volume: float = 1.0,
                duty: float = 0.25) -> np.ndarray:
    """Render a single note with ADSR envelope.

    Args:
        freq: Frequency in Hz. If 0, returns silence (rest).
        duration: Duration in seconds.
        wave_fn: 'square' or 'pulse'.
        volume: Volume multiplier.
        duty: Duty cycle for pulse wave.

    Returns:
        Numpy array of samples.
    """
    n_samples = int(SAMPLE_RATE * duration)
    if freq == 0 or n_samples == 0:
        return np.zeros(n_samples)

    t = np.linspace(0, duration, n_samples, endpoint=False)

    if wave_fn == "square":
        signal = square_wave(t, freq)
    elif wave_fn == "pulse":
        signal = pulse_wave(t, freq, duty=duty)
    else:
        signal = square_wave(t, freq)

    # Per-note envelope: fast attack, slight decay, good sustain, quick release
    env = envelope_adsr(n_samples, a=0.01, d=0.05, s_level=0.8, r=0.05)
    return signal * env * volume


def render_voice(notes, bpm, wave_fn, volume=1.0, duty=0.25):
    """Render a sequence of (note_name, beat_count) tuples into a sample array.

    Args:
        notes: List of (note_name_or_freq, beats) tuples.
               note_name is a string key into NOTE_FREQ, a float freq, or 0/None for rest.
        bpm: Tempo in beats per minute.
        wave_fn: 'square' or 'pulse'.
        volume: Volume multiplier for all notes.
        duty: Duty cycle for pulse wave.

    Returns:
        Numpy array of the entire voice.
    """
    segments = []
    for note, beats in notes:
        dur = note_duration(bpm, beats)
        if note is None or note == 0:
            freq = 0.0
        elif isinstance(note, str):
            freq = NOTE_FREQ[note]
        else:
            freq = float(note)
        segments.append(render_note(freq, dur, wave_fn, volume, duty))
    return np.concatenate(segments)


# ─── Title Theme (~20s, C major, 120 BPM) ─────────────────────────────

def generate_title_theme():
    """Music_Title.wav: Catchy 8-bit C major melody loop, ~20 seconds.

    Structure: 2 phrases of 4 bars + 2-bar ending tag = 10 bars at 120 BPM.
    40 beats total = 20.0 seconds.
    2 voices: square wave melody + pulse wave bass.
    Chord progression: C | F | G | C per phrase, ending on C for seamless loop.
    """
    bpm = 120.0

    # ── Melody (square wave, 0.5 volume) ──
    # Each phrase is 4 bars of 4/4 = 16 beats.
    # Melody uses quarter and eighth notes for a catchy, singable tune.

    # Phrase A: Opening — bright ascending motif (16 beats)
    phrase_a_melody = [
        ("C4", 1), ("E4", 0.5), ("G4", 0.5), ("E4", 1), ("C5", 1),   # bar 1: C
        ("A4", 1), ("F4", 0.5), ("A4", 0.5), ("G4", 2),               # bar 2: F
        ("G4", 0.5), ("A4", 0.5), ("B4", 0.5), ("C5", 0.5), ("B4", 1), ("G4", 1),  # bar 3: G
        ("E4", 1), ("D4", 0.5), ("E4", 0.5), ("C4", 2),               # bar 4: C
    ]

    # Phrase B: Variation — more rhythmic, slightly different contour (16 beats)
    phrase_b_melody = [
        ("E4", 0.5), ("E4", 0.5), ("G4", 1), ("A4", 1), ("G4", 1),   # bar 1: C
        ("F4", 1), ("A4", 0.5), ("G4", 0.5), ("F4", 1), ("E4", 1),   # bar 2: F
        ("D4", 0.5), ("E4", 0.5), ("F4", 0.5), ("G4", 0.5), ("A4", 1), ("G4", 1),  # bar 3: G
        ("E4", 1.5), ("D4", 0.5), ("C4", 2),                          # bar 4: C
    ]

    # Ending tag: 2-bar resolution that leads back to the top (8 beats)
    tag_melody = [
        ("C5", 1), ("B4", 0.5), ("A4", 0.5), ("G4", 1), ("E4", 1),   # bar 9: G
        ("D4", 0.5), ("E4", 0.5), ("D4", 1), ("C4", 1),               # bar 10: C (resolve)
    ]

    melody_notes = phrase_a_melody + phrase_b_melody + tag_melody

    # ── Bass (pulse wave, 0.3 volume) ──
    # Root notes following the chord progression, half notes.
    # Phrase bass: C | F | G | C (16 beats each)

    def bass_phrase():
        return [
            ("C3", 2), ("C3", 2),     # bar 1: C
            ("F3", 2), ("F3", 2),     # bar 2: F
            ("G3", 2), ("G3", 2),     # bar 3: G
            ("C3", 2), ("C3", 2),     # bar 4: C
        ]

    # Tag bass: G | C (8 beats)
    tag_bass = [
        ("G3", 2), ("G3", 2),         # bar 9: G
        ("C3", 2), ("C3", 2),         # bar 10: C
    ]

    bass_notes = bass_phrase() + bass_phrase() + tag_bass

    # ── Render voices ──
    melody = render_voice(melody_notes, bpm, "square", volume=0.5)
    bass = render_voice(bass_notes, bpm, "pulse", volume=0.3, duty=0.25)

    # Ensure both voices are the same length (pad shorter one)
    max_len = max(len(melody), len(bass))
    if len(melody) < max_len:
        melody = np.pad(melody, (0, max_len - len(melody)))
    if len(bass) < max_len:
        bass = np.pad(bass, (0, max_len - len(bass)))

    # Mix
    mix = melody + bass

    # Apply a very short fade at the loop boundary for seamless looping
    # Crossfade the last 500 samples with the first 500 samples
    fade_len = 500
    fade_out = np.linspace(1.0, 0.0, fade_len)
    fade_in = np.linspace(0.0, 1.0, fade_len)
    mix[-fade_len:] = mix[-fade_len:] * fade_out + mix[:fade_len] * fade_in

    return mix


# ─── Gameplay Loop (~30s, A minor, 140 BPM) ───────────────────────────

def generate_gameplay_loop():
    """Music_Gameplay.wav: Upbeat 8-bit A minor loop, ~30 seconds.

    Structure: 4 phrases of 4 bars each + 1.5-bar tag = 17.5 bars at 140 BPM.
    70 beats total = 30.0 seconds.
    3 voices: square melody + pulse bass + pulse arpeggio.
    Chord progression: Am | Dm | Em | Am (with variation).
    Faster tempo (140 BPM), more urgent feel.
    """
    bpm = 140.0

    # ── Melody (square wave, 0.4 volume) ──
    # A minor scale: A4, B4, C5, D5, E5 (and A3, B3, C4, D4, E4 below)
    # More eighth-note driven for urgency.

    # Phrase A: Urgent opening motif (16 beats)
    phrase_a = [
        ("A4", 0.5), ("C5", 0.5), ("E5", 0.5), ("C5", 0.5), ("A4", 1), (None, 1),  # bar 1: Am
        ("D4", 0.5), ("F4", 0.5), ("A4", 0.5), ("F4", 0.5), ("D4", 1), (None, 1),  # bar 2: Dm
        ("E4", 0.5), ("G4", 0.5), ("B4", 0.5), ("G4", 0.5), ("E4", 1), (None, 1),  # bar 3: Em
        ("A4", 1), ("E4", 0.5), ("A4", 0.5), ("C5", 1), (None, 1),                  # bar 4: Am
    ]

    # Phrase B: Climbing tension (16 beats)
    phrase_b = [
        ("A4", 0.5), ("B4", 0.5), ("C5", 0.5), ("D5", 0.5), ("E5", 1), ("C5", 1),  # bar 1
        ("D5", 0.5), ("C5", 0.5), ("A4", 0.5), ("F4", 0.5), ("D4", 2),              # bar 2
        ("E4", 0.5), ("G4", 0.5), ("B4", 1), ("E5", 1), ("B4", 1),                  # bar 3
        ("A4", 1), ("G4", 0.5), ("A4", 0.5), ("A4", 2),                              # bar 4
    ]

    # Phrase C: Rhythmic variation (16 beats)
    phrase_c = [
        ("C5", 0.5), ("C5", 0.5), ("A4", 0.5), ("A4", 0.5), ("E4", 1), ("A4", 1),  # bar 1
        ("F4", 0.5), ("F4", 0.5), ("D4", 0.5), ("A4", 0.5), ("D5", 2),              # bar 2
        ("B4", 0.5), ("E5", 0.5), ("B4", 0.5), ("G4", 0.5), ("E4", 2),              # bar 3
        ("A4", 0.5), ("E4", 0.5), ("A4", 0.5), ("C5", 0.5), ("A4", 2),              # bar 4
    ]

    # Phrase D: Resolution for loop (16 beats)
    phrase_d = [
        ("E5", 1), ("D5", 0.5), ("C5", 0.5), ("A4", 1), (None, 1),                  # bar 1
        ("F4", 1), ("A4", 0.5), ("D5", 0.5), ("C5", 2),                              # bar 2
        ("B4", 0.5), ("G4", 0.5), ("E4", 1), ("G4", 0.5), ("B4", 0.5), ("E5", 1),  # bar 3
        ("C5", 0.5), ("B4", 0.5), ("A4", 1), ("A4", 2),                              # bar 4
    ]

    # Ending tag: short turnaround back to A for seamless loop (6 beats)
    tag_melody = [
        ("E4", 0.5), ("G4", 0.5), ("A4", 0.5), ("C5", 0.5), ("B4", 1),  # bar 17
        ("A4", 1), ("A4", 2),                                              # bar 17.5 (resolve)
    ]

    # Full melody: ABCD + tag (70 beats total)
    melody_notes = phrase_a + phrase_b + phrase_c + phrase_d + tag_melody

    # ── Bass (pulse wave, 0.25 volume) ──
    # Root notes, half-note rhythm for driving feel.

    def bass_phrase_am_dm_em_am():
        """One 4-bar phrase = 16 beats."""
        return [
            ("A2", 2), ("A2", 2),     # bar 1: Am
            ("D3", 2), ("D3", 2),     # bar 2: Dm
            ("E3", 2), ("E3", 2),     # bar 3: Em
            ("A2", 2), ("A2", 2),     # bar 4: Am
        ]

    # Tag bass: Em resolving to Am (6 beats)
    tag_bass = [
        ("E3", 2), ("E3", 1),         # 3 beats: Em
        ("A2", 2), ("A2", 1),         # 3 beats: Am
    ]

    bass_notes = bass_phrase_am_dm_em_am() * 4 + tag_bass  # 4 phrases + tag = 70 beats

    # ── Arpeggio (pulse wave, 0.2 volume, higher duty for thinner sound) ──
    # Fast arpeggiated chords, eighth notes, cycling through chord tones.

    def arp_bar(notes_cycle, beats=4):
        """Generate arpeggiated eighth notes cycling through the given notes."""
        arp = []
        count = int(beats / 0.5)  # number of eighth notes
        for i in range(count):
            arp.append((notes_cycle[i % len(notes_cycle)], 0.5))
        return arp

    # Am arpeggio: A3-C4-E4
    # Dm arpeggio: D4-F4-A4
    # Em arpeggio: E4-G4-B4
    arp_am = arp_bar(["A3", "C4", "E4"])
    arp_dm = arp_bar(["D4", "F4", "A4"])
    arp_em = arp_bar(["E4", "G4", "B4"])

    def arp_phrase():
        """4-bar arpeggio phrase = 16 beats."""
        return arp_am + arp_dm + arp_em + arp_am

    # Tag arpeggio: Em (3 beats) + Am (3 beats) = 6 beats
    arp_tag = arp_bar(["E4", "G4", "B4"], beats=3) + arp_bar(["A3", "C4", "E4"], beats=3)

    arp_notes = arp_phrase() * 4 + arp_tag  # 4 phrases + tag = 70 beats

    # ── Render voices ──
    melody = render_voice(melody_notes, bpm, "square", volume=0.4)
    bass = render_voice(bass_notes, bpm, "pulse", volume=0.25, duty=0.25)
    arpeggio = render_voice(arp_notes, bpm, "pulse", volume=0.2, duty=0.125)

    # Ensure all voices are the same length (trim to shortest for clean loop)
    min_len = min(len(melody), len(bass), len(arpeggio))
    melody = melody[:min_len]
    bass = bass[:min_len]
    arpeggio = arpeggio[:min_len]

    # Mix
    mix = melody + bass + arpeggio

    # Crossfade for seamless loop
    fade_len = 500
    fade_out = np.linspace(1.0, 0.0, fade_len)
    fade_in = np.linspace(0.0, 1.0, fade_len)
    mix[-fade_len:] = mix[-fade_len:] * fade_out + mix[:fade_len] * fade_in

    return mix


# ─── Main ──────────────────────────────────────────────────────────────

TRACKS = [
    ("Music_Title.wav", generate_title_theme),
    ("Music_Gameplay.wav", generate_gameplay_loop),
]


def main():
    ensure_output_dir()
    print(f"Generating {len(TRACKS)} music tracks...")
    print(f"Output: {os.path.abspath(OUTPUT_DIR)}")
    print(f"Format: {SAMPLE_RATE}Hz, {BIT_DEPTH}-bit, mono PCM WAV")
    print()

    for filename, generator in TRACKS:
        samples = generator()
        write_wav(filename, samples)

    print()
    print(f"Done. {len(TRACKS)} files written to {os.path.abspath(OUTPUT_DIR)}")


if __name__ == "__main__":
    main()
