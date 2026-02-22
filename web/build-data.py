#!/usr/bin/env python3
"""Parse UnrealFrog agent transcripts into JSON for the Agent Interaction Replay app.

Groups individual agent transcript files by sessionId so the replay app
shows one combined timeline per team session with all agents interleaved.
"""

import re
import json
import sys
from collections import defaultdict
from pathlib import Path

TRANSCRIPT_DIR = Path(__file__).parent.parent / ".claude" / "transcripts"
OUTPUT_FILE = Path(__file__).parent / "data" / "transcripts.json"

AGENT_META = {
    "engine-architect": {"initials": "EA", "color": "#10B981", "label": "Engine Architect", "role": "C++ Systems"},
    "game-designer":    {"initials": "GD", "color": "#F59E0B", "label": "Game Designer",    "role": "Mechanics & Feel"},
    "xp-coach":         {"initials": "XP", "color": "#3B82F6", "label": "XP Coach",         "role": "Process Lead"},
    "devops-engineer":  {"initials": "DE", "color": "#8B5CF6", "label": "DevOps Engineer",  "role": "Build & CI"},
    "qa-lead":          {"initials": "QA", "color": "#06B6D4", "label": "QA Lead",           "role": "Testing & Feel"},
    "level-designer":   {"initials": "LD", "color": "#EF4444", "label": "Level Designer",   "role": "Spatial Design"},
    "art-director":     {"initials": "AD", "color": "#EC4899", "label": "Art Director",     "role": "Visual Style"},
    "sound-engineer":   {"initials": "SE", "color": "#F97316", "label": "Sound Engineer",   "role": "Audio Design"},
    "team-lead":        {"initials": "TL", "color": "#6366F1", "label": "Team Lead",        "role": "Coordinator"},
    "user":             {"initials": "U",  "color": "#94A3B8", "label": "Stakeholder",      "role": "Direction"},
}


def parse_metadata(content):
    meta = {}
    for key, pattern in [
        ("date",      r'\| Date \| (\d{4}-\d{2}-\d{2}) \|'),
        ("sessionId", r'\| Session ID \| `([^`]+)` \|'),
        ("model",     r'\| Model \| ([^\|]+) \|'),
        ("duration",  r'\| Duration \| ([^\|]+) \|'),
        ("tokens",    r'\| Tokens \| ([^\|]+) \|'),
    ]:
        m = re.search(pattern, content)
        if m:
            meta[key] = m.group(1).strip()
    return meta


def parse_messages(content):
    messages = []
    sections = re.split(r'^(## (?:User|Assistant) \(\d{2}:\d{2} UTC\))', content, flags=re.MULTILINE)
    last_teammate = None

    for i in range(1, len(sections), 2):
        header = sections[i]
        body = sections[i + 1] if i + 1 < len(sections) else ""

        hm = re.match(r'## (User|Assistant) \((\d{2}:\d{2}) UTC\)', header)
        if not hm:
            continue

        role, time = hm.group(1), hm.group(2)

        tm = re.search(
            r'<teammate-message\s+teammate_id="([^"]+)"(?:\s+color="([^"]*)")?(?:\s+summary="([^"]*)")?\s*>',
            body
        )

        if role == "User" and tm:
            sender = tm.group(1)
            summary = tm.group(3) or ""

            cm = re.search(r'<teammate-message[^>]*>(.*?)</teammate-message>', body, re.DOTALL)
            msg_content = cm.group(1).strip() if cm else body.strip()

            receiver = "team-lead"
            to_match = re.search(r'\[to\s+([a-z][\w-]*)\]', summary, re.IGNORECASE)
            if to_match:
                receiver = to_match.group(1)

            msg_type = "idle" if '"type": "idle_notification"' in msg_content else "teammate"

            messages.append({
                "time": time, "from": sender, "to": receiver,
                "content": msg_content[:800], "summary": summary, "type": msg_type
            })
            last_teammate = sender

        elif role == "User" and not tm:
            msg_content = body.strip()
            if msg_content and len(msg_content) > 10:
                messages.append({
                    "time": time, "from": "user", "to": "team-lead",
                    "content": msg_content[:800], "summary": "", "type": "user"
                })
                last_teammate = None

        elif role == "Assistant":
            msg_content = body.strip()
            lines = [l for l in msg_content.split('\n')
                     if l.strip() and not l.strip().startswith('- **Used')]
            if lines:
                receiver = last_teammate or "all"
                msg_type = "send" if '**Used SendMessage**' in body else "response"
                clean = '\n'.join(lines)
                messages.append({
                    "time": time, "from": "team-lead", "to": receiver,
                    "content": clean[:800], "summary": "", "type": msg_type
                })

    return messages


def parse_transcript(filepath):
    content = filepath.read_text()
    metadata = parse_metadata(content)
    messages = parse_messages(content)

    name = filepath.stem
    parts = name.split('-', 4)
    title = parts[4].replace('-', ' ').title() if len(parts) > 4 else name

    participants = set()
    for msg in messages:
        participants.add(msg['from'])
        if msg['to'] != 'all':
            participants.add(msg['to'])

    return {
        "id": name, "title": title, "metadata": metadata,
        "messages": messages, "participants": sorted(participants),
        "messageCount": len(messages)
    }


def merge_session(transcripts):
    """Merge multiple transcript files from the same session into one combined timeline."""
    # Use the largest transcript's title as the session title, or the first one
    primary = max(transcripts, key=lambda t: t['messageCount'])
    meta = primary['metadata'].copy()

    # Collect all participants across all transcripts
    all_participants = set()
    for t in transcripts:
        all_participants.update(t['participants'])

    # Merge and interleave messages by timestamp.
    # Within the same minute, preserve the original file order by using
    # the transcript's file-sort position as a tiebreaker.
    merged = []
    for file_idx, t in enumerate(transcripts):
        for msg_idx, msg in enumerate(t['messages']):
            merged.append((msg['time'], file_idx, msg_idx, msg))

    # Sort by (time, file_index, message_index_within_file)
    merged.sort(key=lambda x: (x[0], x[1], x[2]))
    messages = [m[3] for m in merged]

    session_id = meta.get('sessionId', primary['id'])
    return {
        "id": session_id,
        "title": primary['title'],
        "metadata": meta,
        "messages": messages,
        "participants": sorted(all_participants),
        "messageCount": len(messages),
        "fileCount": len(transcripts),
    }


def main():
    if not TRANSCRIPT_DIR.exists():
        print(f"Transcript directory not found: {TRANSCRIPT_DIR}")
        sys.exit(1)

    # Parse all transcript files
    all_transcripts = []
    files = sorted(TRANSCRIPT_DIR.glob("*.md"))

    for f in files:
        try:
            t = parse_transcript(f)
            has_teammate = any(m['type'] in ('teammate', 'idle') for m in t['messages'])
            if has_teammate and t['messages']:
                all_transcripts.append(t)
        except Exception as e:
            print(f"Error parsing {f.name}: {e}", file=sys.stderr)

    # Group by sessionId
    by_session = defaultdict(list)
    for t in all_transcripts:
        sid = t['metadata'].get('sessionId', t['id'])
        by_session[sid].append(t)

    # Merge each session's transcripts into one combined timeline
    sessions = []
    for sid, group in by_session.items():
        sessions.append(merge_session(group))

    # Sort newest first by date then session id
    sessions.sort(key=lambda s: (s['metadata'].get('date', ''), s['id']), reverse=True)

    OUTPUT_FILE.parent.mkdir(parents=True, exist_ok=True)
    with open(OUTPUT_FILE, 'w') as f:
        json.dump({"agents": AGENT_META, "transcripts": sessions}, f)

    total_msgs = sum(s['messageCount'] for s in sessions)
    total_files = sum(s['fileCount'] for s in sessions)
    print(f"Processed {len(files)} transcript files")
    print(f"Merged {total_files} multi-agent transcripts into {len(sessions)} sessions")
    print(f"Total messages: {total_msgs}")
    print(f"Output: {OUTPUT_FILE} ({OUTPUT_FILE.stat().st_size / 1024:.1f} KB)")


if __name__ == "__main__":
    main()
