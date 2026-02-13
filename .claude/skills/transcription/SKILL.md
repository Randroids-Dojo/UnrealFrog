# Conversation Transcription

Converts raw Claude Code JSONL conversation logs into human-readable markdown transcripts.

## How It Works

- **Automatic**: A `Stop` hook in `.claude/settings.json` runs the script when Claude Code exits
- **Manual**: `python3 .claude/scripts/transcribe.py` processes all conversations, or pass a specific file
- **Idempotent**: Compares mtimes; already-transcribed files are skipped

## File Locations

| What | Where |
|------|-------|
| Script | `.claude/scripts/transcribe.py` |
| Raw conversations | `.claude/conversations/*.jsonl` (gitignored) |
| Transcripts | `.claude/transcripts/*.md` (committed) |
| Manifest | `.claude/transcripts/.manifest.json` (gitignored) |

## Transcript Format

Each transcript contains:
- Header table with date, session ID, model, token usage, duration
- `## User (HH:MM UTC)` sections with message text
- `## Assistant (HH:MM UTC)` sections with text and tool call summaries
- Tool calls as one-liners: `**Used Read** on \`path/to/file\``

Skipped entries: system messages, progress events, file-history-snapshots, meta/local-command messages, tool results.

## Usage

```bash
# Process all conversations
python3 .claude/scripts/transcribe.py

# Process a specific file
python3 .claude/scripts/transcribe.py .claude/conversations/abc123.jsonl
```

## Requirements

- Python 3.9+ (stdlib only, no dependencies)
