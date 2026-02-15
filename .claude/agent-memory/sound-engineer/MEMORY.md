# Sound Engineer Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Sprint 4

### SFX Generation
- Script: `Tools/AudioGen/generate_sfx.py` — generates 9 deterministic 8-bit WAV files
- Output: `Content/Audio/SFX/` — 16-bit PCM WAV, 44100Hz, mono
- Dependencies: numpy only (standard library for wave module)
- Waveforms used: square, saw, pulse (variable duty cycle), filtered noise
- Deterministic: uses np.random.RandomState with fixed seeds for noise
- All durations match spec exactly (0.1s to 1.5s)

### Design Decisions
- Hop: ascending sweep 100-400Hz in 0.1s — snappy feel
- Deaths: each has distinct character (noise burst for squish, filtered noise for splash, two-tone beep for offscreen)
- HomeSlot: C-E-G arpeggio with pulse wave for chime quality
- RoundComplete: full C major scale run followed by held C-E-G chord
- GameOver: descending C minor arpeggio (C5-G4-Eb4-C4) using saw wave for sadness
- ExtraLife: classic 1-up pattern (E5-G5-E6-C6-D6) using 50% duty pulse
- TimerWarning: sharp 880Hz square wave tick, designed for rapid repetition

### Music Generation
- Script: `Tools/AudioGen/generate_music.py` — generates 2 seamlessly looping 8-bit music tracks
- Output: `Content/Audio/Music/` — 16-bit PCM WAV, 44100Hz, mono
- Dependencies: numpy only (same as generate_sfx.py)
- Primitives copied from generate_sfx.py (not imported): square_wave, pulse_wave, envelope_ad, envelope_adsr
- Helper functions: NOTE_FREQ lookup table, note_duration(bpm, beats), render_note, render_voice
- Crossfade: 500-sample crossfade at loop boundary for seamless looping

### Music Design Decisions
- **Title Theme** (Music_Title.wav): 20.0s, C major, 120 BPM
  - 2 voices: square melody (vol 0.5) + pulse bass (vol 0.3, duty 0.25)
  - Structure: 2x 4-bar phrases (A, B) + 2-bar ending tag = 10 bars, 40 beats
  - Chord progression: C-F-G-C per phrase
  - Melody: catchy quarter/eighth note patterns using C major scale
  - Tag resolves on C4 for seamless loop restart
- **Gameplay Loop** (Music_Gameplay.wav): 30.0s, A minor, 140 BPM
  - 3 voices: square melody (vol 0.4) + pulse bass (vol 0.25) + pulse arpeggio (vol 0.2, duty 0.125)
  - Structure: 4x 4-bar phrases (A, B, C, D) + 1.5-bar tag = 17.5 bars, 70 beats
  - Chord progression: Am-Dm-Em-Am per phrase
  - Arpeggio: eighth-note chord tone cycling (A3-C4-E4, D4-F4-A4, E4-G4-B4)
  - More urgent feel via faster tempo and eighth-note-driven melody
