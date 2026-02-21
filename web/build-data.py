#!/usr/bin/env python3
"""Parse UnrealFrog agent transcripts into JSON for the Agent Interaction Replay app."""

import re
import json
import sys
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


def main():
    if not TRANSCRIPT_DIR.exists():
        print(f"Transcript directory not found: {TRANSCRIPT_DIR}")
        sys.exit(1)

    transcripts = []
    files = sorted(TRANSCRIPT_DIR.glob("*.md"))

    for f in files:
        try:
            t = parse_transcript(f)
            has_teammate = any(m['type'] in ('teammate', 'idle') for m in t['messages'])
            if has_teammate and t['messages']:
                transcripts.append(t)
        except Exception as e:
            print(f"Error parsing {f.name}: {e}", file=sys.stderr)

    transcripts.sort(key=lambda t: t['id'], reverse=True)

    OUTPUT_FILE.parent.mkdir(parents=True, exist_ok=True)
    with open(OUTPUT_FILE, 'w') as f:
        json.dump({"agents": AGENT_META, "transcripts": transcripts}, f)

    total_msgs = sum(t['messageCount'] for t in transcripts)
    print(f"Processed {len(files)} transcript files")
    print(f"Found {len(transcripts)} multi-agent transcripts with {total_msgs} messages")
    print(f"Output: {OUTPUT_FILE} ({OUTPUT_FILE.stat().st_size / 1024:.1f} KB)")


if __name__ == "__main__":
    main()
