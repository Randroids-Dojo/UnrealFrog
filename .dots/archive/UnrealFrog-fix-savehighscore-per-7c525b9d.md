---
title: Fix SaveHighScore per-tick writes — persist only at game over / title transition
status: closed
priority: 0
issue-type: task
created-at: "\"\\\"2026-02-15T08:57:13.789763-06:00\\\"\""
closed-at: "2026-02-15T09:19:43.111901-06:00"
close-reason: Removed SaveHighScore from NotifyScoreChanged (per-tick). Now saves only at game over in LoseLife. Added test FScore_HighScoreSavesOnlyAtGameOver.
---

Engine Architect. SaveHighScore() fires on every score change (50+ disk writes per round). Should only persist at game over and title transition. 5-min fix. BLOCKS play-test.
