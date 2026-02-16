#!/usr/bin/env python3
"""Convert Claude Code JSONL conversation files to human-readable markdown transcripts.

Usage:
    python3 .claude/scripts/transcribe.py                     # Process all JSONL files
    python3 .claude/scripts/transcribe.py path/to/file.jsonl  # Process a specific file

Output goes to .claude/transcripts/ as markdown files.
Idempotent: skips files whose transcript is already up-to-date (mtime comparison).
"""

import json
import os
import re
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple


def find_project_root() -> Path:
    """Walk up from this script to find the project root (contains .claude/)."""
    path = Path(__file__).resolve().parent
    while path != path.parent:
        if (path / ".claude").is_dir():
            return path
        path = path.parent
    # Fallback: assume script is at .claude/scripts/transcribe.py
    return Path(__file__).resolve().parent.parent.parent


PROJECT_ROOT = find_project_root()
CONVERSATIONS_DIR = PROJECT_ROOT / ".claude" / "conversations"
TRANSCRIPTS_DIR = PROJECT_ROOT / ".claude" / "transcripts"
MANIFEST_PATH = TRANSCRIPTS_DIR / ".manifest.json"


def load_manifest() -> Dict[str, str]:
    """Load the manifest mapping session IDs to transcript filenames."""
    if MANIFEST_PATH.exists():
        try:
            return json.loads(MANIFEST_PATH.read_text())
        except (json.JSONDecodeError, OSError):
            return {}
    return {}


def save_manifest(manifest: Dict[str, str]) -> None:
    """Save the manifest to disk."""
    MANIFEST_PATH.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST_PATH.write_text(json.dumps(manifest, indent=2) + "\n")


def shorten_path(path_str: str) -> str:
    """Convert absolute paths to project-relative paths."""
    root_str = str(PROJECT_ROOT)
    if path_str.startswith(root_str):
        rel = path_str[len(root_str):]
        if rel.startswith("/"):
            rel = rel[1:]
        return rel or "."
    return path_str


def slugify(text: str) -> str:
    """Turn text into a filename-safe slug of the first six words."""
    # Strip markdown, tags, and special chars
    text = re.sub(r"<[^>]+>", "", text)
    text = re.sub(r"[#*`\[\](){}]", "", text)
    text = text.strip()
    words = text.split()[:6]
    slug = "-".join(words).lower()
    slug = re.sub(r"[^a-z0-9\-]", "", slug)
    slug = re.sub(r"-+", "-", slug).strip("-")
    return slug[:80] if slug else "untitled"


def format_timestamp(ts_str: str) -> str:
    """Parse ISO timestamp and return HH:MM UTC."""
    try:
        dt = datetime.fromisoformat(ts_str.replace("Z", "+00:00"))
        return dt.strftime("%H:%M UTC")
    except (ValueError, AttributeError):
        return "??:?? UTC"


def format_date(ts_str: str) -> str:
    """Parse ISO timestamp and return YYYY-MM-DD."""
    try:
        dt = datetime.fromisoformat(ts_str.replace("Z", "+00:00"))
        return dt.strftime("%Y-%m-%d")
    except (ValueError, AttributeError):
        return "unknown-date"


def format_time_for_filename(ts_str: str) -> str:
    """Parse ISO timestamp and return HHMM for use in filenames."""
    try:
        dt = datetime.fromisoformat(ts_str.replace("Z", "+00:00"))
        return dt.strftime("%H%M")
    except (ValueError, AttributeError):
        return "0000"


def extract_text_content(content: Any) -> str:
    """Extract plain text from message content (string or content blocks)."""
    if isinstance(content, str):
        return content.strip()
    if isinstance(content, list):
        parts = []
        for block in content:
            if isinstance(block, dict) and block.get("type") == "text":
                parts.append(block.get("text", ""))
            elif isinstance(block, str):
                parts.append(block)
        return "\n".join(parts).strip()
    return ""


def summarize_tool_use(name: str, inp: Dict[str, Any]) -> str:
    """Produce a one-liner summary of a tool call."""
    if name == "Read":
        path = shorten_path(inp.get("file_path", "?"))
        extra = ""
        if "offset" in inp or "limit" in inp:
            parts = []
            if "offset" in inp:
                parts.append(f"offset={inp['offset']}")
            if "limit" in inp:
                parts.append(f"limit={inp['limit']}")
            extra = f" ({', '.join(parts)})"
        return f"**Used Read** on `{path}`{extra}"

    if name == "Write":
        path = shorten_path(inp.get("file_path", "?"))
        return f"**Used Write** on `{path}`"

    if name == "Edit":
        path = shorten_path(inp.get("file_path", "?"))
        return f"**Used Edit** on `{path}`"

    if name == "Glob":
        pattern = inp.get("pattern", "?")
        path = shorten_path(inp.get("path", "."))
        return f"**Used Glob** for `{pattern}` in `{path}`"

    if name == "Grep":
        pattern = inp.get("pattern", "?")
        path = shorten_path(inp.get("path", "."))
        return f"**Used Grep** for `{pattern}` in `{path}`"

    if name == "Bash":
        cmd = inp.get("command", "?")
        desc = inp.get("description", "")
        if desc:
            return f"**Used Bash**: {desc}"
        # Truncate long commands
        if len(cmd) > 80:
            cmd = cmd[:77] + "..."
        return f"**Used Bash**: `{cmd}`"

    if name == "Task":
        desc = inp.get("description", inp.get("prompt", "?"))
        if len(desc) > 80:
            desc = desc[:77] + "..."
        return f"**Used Task**: {desc}"

    if name in ("TaskCreate", "TaskUpdate", "TaskList", "TaskGet"):
        subject = inp.get("subject", "")
        task_id = inp.get("taskId", "")
        if subject:
            return f"**Used {name}**: {subject}"
        if task_id:
            return f"**Used {name}** on task {task_id}"
        return f"**Used {name}**"

    if name == "EnterPlanMode":
        return "**Entered plan mode**"

    if name == "ExitPlanMode":
        return "**Exited plan mode**"

    if name == "WebSearch":
        query = inp.get("query", "?")
        return f"**Used WebSearch**: `{query}`"

    if name == "WebFetch":
        url = inp.get("url", "?")
        return f"**Used WebFetch** on `{url}`"

    if name in ("SendMessage", "TeamCreate", "TeamDelete"):
        return f"**Used {name}**"

    if name == "NotebookEdit":
        path = shorten_path(inp.get("notebook_path", "?"))
        return f"**Used NotebookEdit** on `{path}`"

    if name == "AskUserQuestion":
        questions = inp.get("questions", [])
        if questions and isinstance(questions[0], dict):
            q = questions[0].get("question", "?")
            if len(q) > 80:
                q = q[:77] + "..."
            return f"**Asked user**: {q}"
        return "**Asked user a question**"

    if name == "Skill":
        skill = inp.get("skill", "?")
        return f"**Used Skill**: {skill}"

    # Generic fallback
    return f"**Used {name}**"


def should_skip(entry: Dict[str, Any]) -> bool:
    """Return True if this JSONL entry should be excluded from the transcript."""
    entry_type = entry.get("type", "")

    # Always skip these types
    if entry_type in ("file-history-snapshot", "progress"):
        return True

    # Skip meta messages (local-command caveats)
    if entry.get("isMeta", False):
        return True

    # Skip system/local-command messages
    if entry_type == "system":
        return True

    # Skip tool results (they are "user" type with tool_result content)
    if entry_type == "user":
        msg = entry.get("message", {})
        content = msg.get("content", "")
        if isinstance(content, list):
            for block in content:
                if isinstance(block, dict) and block.get("type") == "tool_result":
                    return True
        # Skip local-command and bash session messages
        if isinstance(content, str):
            if "<local-command" in content or "<command-name>" in content:
                return True
            if "<bash-input>" in content or "<bash-stdout>" in content:
                return True

    return False


def parse_jsonl(filepath: Path) -> Tuple[List[Dict[str, Any]], Dict[str, Any]]:
    """Parse a JSONL file into entries and extract session metadata."""
    entries = []
    metadata = {
        "session_id": "",
        "model": "",
        "date": "",
        "slug": "",
        "first_timestamp": "",
        "last_timestamp": "",
        "total_input_tokens": 0,
        "total_output_tokens": 0,
    }

    with open(filepath, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                entry = json.loads(line)
            except json.JSONDecodeError:
                continue
            entries.append(entry)

            # Extract metadata from entries
            if not metadata["session_id"] and entry.get("sessionId"):
                metadata["session_id"] = entry["sessionId"]
            if not metadata["slug"] and entry.get("slug"):
                metadata["slug"] = entry["slug"]

            ts = entry.get("timestamp", "")
            if ts:
                if not metadata["first_timestamp"]:
                    metadata["first_timestamp"] = ts
                metadata["last_timestamp"] = ts

            # Extract model and token usage from assistant messages
            if entry.get("type") == "assistant":
                msg = entry.get("message", {})
                if msg.get("model") and not metadata["model"]:
                    metadata["model"] = msg["model"]
                usage = msg.get("usage", {})
                metadata["total_input_tokens"] += usage.get("input_tokens", 0)
                metadata["total_input_tokens"] += usage.get("cache_read_input_tokens", 0)
                metadata["total_input_tokens"] += usage.get("cache_creation_input_tokens", 0)
                metadata["total_output_tokens"] += usage.get("output_tokens", 0)

    metadata["date"] = format_date(metadata["first_timestamp"]) if metadata["first_timestamp"] else "unknown"
    return entries, metadata


def compute_duration(first_ts: str, last_ts: str) -> str:
    """Compute human-readable duration between two ISO timestamps."""
    try:
        t1 = datetime.fromisoformat(first_ts.replace("Z", "+00:00"))
        t2 = datetime.fromisoformat(last_ts.replace("Z", "+00:00"))
        delta = t2 - t1
        total_seconds = int(delta.total_seconds())
        if total_seconds < 60:
            return f"{total_seconds}s"
        minutes = total_seconds // 60
        seconds = total_seconds % 60
        if minutes < 60:
            return f"{minutes}m {seconds}s"
        hours = minutes // 60
        minutes = minutes % 60
        return f"{hours}h {minutes}m"
    except (ValueError, AttributeError):
        return "unknown"


def build_transcript(entries: List[Dict[str, Any]], metadata: Dict[str, Any]) -> str:
    """Convert parsed entries into a markdown transcript."""
    lines = []

    # Header
    duration = compute_duration(metadata["first_timestamp"], metadata["last_timestamp"])
    lines.append(f"# Conversation: {metadata['date']}")
    lines.append("")
    lines.append("| Field | Value |")
    lines.append("|-------|-------|")
    lines.append(f"| Date | {metadata['date']} |")
    lines.append(f"| Session ID | `{metadata['session_id']}` |")
    lines.append(f"| Model | {metadata['model'] or 'unknown'} |")
    lines.append(f"| Tokens | ~{metadata['total_input_tokens']:,} input, ~{metadata['total_output_tokens']:,} output |")
    lines.append(f"| Duration | {duration} |")
    lines.append("")
    lines.append("---")
    lines.append("")

    # Track which assistant message IDs we've already emitted text for,
    # since the same assistant message can span multiple JSONL entries
    # (text in one entry, tool_use in the next, both sharing the same requestId)
    emitted_assistant_text = set()

    for entry in entries:
        if should_skip(entry):
            continue

        entry_type = entry.get("type", "")
        ts = entry.get("timestamp", "")
        time_str = format_timestamp(ts) if ts else ""

        if entry_type == "user":
            msg = entry.get("message", {})
            text = extract_text_content(msg.get("content", ""))
            if not text:
                continue
            # Skip interrupted messages that are just noise
            if text == "[Request interrupted by user for tool use]":
                continue
            lines.append(f"## User ({time_str})")
            lines.append("")
            lines.append(text)
            lines.append("")

        elif entry_type == "assistant":
            msg = entry.get("message", {})
            content = msg.get("content", [])
            request_id = entry.get("requestId", "")

            # Collect text blocks and tool_use blocks
            text_parts = []
            tool_summaries = []

            if isinstance(content, list):
                for block in content:
                    if isinstance(block, dict):
                        if block.get("type") == "text":
                            text_parts.append(block.get("text", ""))
                        elif block.get("type") == "tool_use":
                            name = block.get("name", "unknown")
                            inp = block.get("input", {})
                            tool_summaries.append(summarize_tool_use(name, inp))
            elif isinstance(content, str):
                text_parts.append(content)

            text = "\n".join(text_parts).strip()

            # Only emit text if we haven't already for this requestId
            has_new_text = False
            if text and request_id not in emitted_assistant_text:
                has_new_text = True
                if request_id:
                    emitted_assistant_text.add(request_id)

            # Only emit a section if there's text or tool calls
            if has_new_text or tool_summaries:
                if has_new_text:
                    lines.append(f"## Assistant ({time_str})")
                    lines.append("")
                    lines.append(text)
                    lines.append("")

                if tool_summaries:
                    for summary in tool_summaries:
                        lines.append(f"- {summary}")
                    lines.append("")

    return "\n".join(lines)


def get_first_user_text(entries: List[Dict[str, Any]]) -> str:
    """Extract the first real user message text for slug generation."""
    for entry in entries:
        if entry.get("type") != "user":
            continue
        if entry.get("isMeta", False):
            continue
        msg = entry.get("message", {})
        text = extract_text_content(msg.get("content", ""))
        if not text or "<local-command" in text or "<command-name>" in text:
            continue
        if "<bash-input>" in text or "<bash-stdout>" in text:
            continue
        if text == "[Request interrupted by user for tool use]":
            continue
        return text
    return ""


def count_messages(entries: List[Dict[str, Any]]) -> int:
    """Count the number of real user + assistant messages (excluding skipped entries)."""
    count = 0
    for entry in entries:
        if should_skip(entry):
            continue
        entry_type = entry.get("type", "")
        if entry_type == "user":
            msg = entry.get("message", {})
            text = extract_text_content(msg.get("content", ""))
            if text and text != "[Request interrupted by user for tool use]":
                count += 1
        elif entry_type == "assistant":
            msg = entry.get("message", {})
            content = msg.get("content", [])
            has_content = False
            if isinstance(content, list):
                for block in content:
                    if isinstance(block, dict) and block.get("type") in ("text", "tool_use"):
                        has_content = True
                        break
            elif isinstance(content, str) and content.strip():
                has_content = True
            if has_content:
                count += 1
    return count


def transcript_filename(metadata: Dict[str, Any], entries: List[Dict[str, Any]]) -> str:
    """Generate a transcript filename: YYYY-MM-DD-HHMM-first-six-words.md"""
    date = metadata["date"]
    time = format_time_for_filename(metadata["first_timestamp"]) if metadata.get("first_timestamp") else "0000"
    first_text = get_first_user_text(entries)
    if first_text:
        slug = slugify(first_text)
    elif metadata.get("slug"):
        slug = metadata["slug"]
    else:
        slug = "untitled"
    return f"{date}-{time}-{slug}.md"


def manifest_key(jsonl_path: Path) -> str:
    """Generate a stable manifest key from the JSONL file path.

    Uses the path relative to the conversations dir (or the absolute path as
    fallback) so that the main conversation and its subagent files each get
    their own entry, even though they share a sessionId.
    """
    try:
        return str(jsonl_path.relative_to(CONVERSATIONS_DIR))
    except ValueError:
        return str(jsonl_path)


def process_file(jsonl_path: Path, manifest: Dict[str, str], force: bool = False) -> Optional[str]:
    """Process a single JSONL file. Returns the transcript filename or None if skipped."""
    entries, metadata = parse_jsonl(jsonl_path)

    if not entries:
        return None

    # Skip conversations with 4 or fewer real messages (noise: shutdown exchanges,
    # stale task assignments, duplicate echoes)
    if count_messages(entries) <= 4:
        return None

    key = manifest_key(jsonl_path)

    # Determine output filename (use manifest to prevent drift)
    if key in manifest:
        out_name = manifest[key]
    else:
        out_name = transcript_filename(metadata, entries)
        # Deduplicate: if another file already claimed this name, add a suffix
        existing_names = set(manifest.values())
        if out_name in existing_names:
            stem = out_name.rsplit(".", 1)[0]
            suffix = 2
            while f"{stem}-{suffix}.md" in existing_names:
                suffix += 1
            out_name = f"{stem}-{suffix}.md"

    out_path = TRANSCRIPTS_DIR / out_name

    # Idempotency check: skip if transcript exists and is newer than source
    if not force and out_path.exists():
        src_mtime = jsonl_path.stat().st_mtime
        dst_mtime = out_path.stat().st_mtime
        if dst_mtime >= src_mtime:
            return None  # Up to date

    # Build and write transcript
    transcript = build_transcript(entries, metadata)

    # Skip files that produce no message content (only header)
    if "## " not in transcript.split("---", 1)[-1]:
        return None

    TRANSCRIPTS_DIR.mkdir(parents=True, exist_ok=True)
    out_path.write_text(transcript)

    # Update manifest
    manifest[key] = out_name
    return out_name


def find_all_jsonl_files() -> List[Path]:
    """Find all JSONL conversation files, including subagent files."""
    files = []
    if CONVERSATIONS_DIR.exists():
        for path in CONVERSATIONS_DIR.rglob("*.jsonl"):
            files.append(path)
    return sorted(files)


def main():
    # Determine which files to process
    if len(sys.argv) > 1:
        jsonl_files = [Path(arg).resolve() for arg in sys.argv[1:]]
    else:
        jsonl_files = find_all_jsonl_files()

    if not jsonl_files:
        print("No JSONL files found.")
        return

    manifest = load_manifest()
    processed = 0
    skipped = 0

    for jsonl_path in jsonl_files:
        if not jsonl_path.exists():
            print(f"  Not found: {jsonl_path}")
            continue

        rel = jsonl_path.relative_to(PROJECT_ROOT) if jsonl_path.is_relative_to(PROJECT_ROOT) else jsonl_path
        result = process_file(jsonl_path, manifest)
        if result:
            print(f"  Transcribed: {rel} -> {result}")
            processed += 1
        else:
            print(f"  Skipped (up to date): {rel}")
            skipped += 1

    save_manifest(manifest)
    print(f"\nDone. {processed} transcribed, {skipped} skipped.")
    print(f"Transcripts in: .claude/transcripts/")


if __name__ == "__main__":
    main()
