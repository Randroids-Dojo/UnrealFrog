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
