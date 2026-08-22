import { TimelineSequencer } from "../src/backend/lom/TimelineSequencer.js";

/**
 * Timeline Sequencer Unit Tests (Phase 5 & Phase 6)
 * Verifies that calculated clip offsets satisfy the non-overlapping progression formula.
 */
export function testZeroOverlapTimelineMath(): boolean {
  const sequencer = new TimelineSequencer();
  const mockSamples = [
    { id: "s1", filePath: "/samples/kick.wav", durationSeconds: 1.0, trackIndex: 1 },
    { id: "s2", filePath: "/samples/snare.wav", durationSeconds: 0.5, trackIndex: 2 },
    { id: "s3", filePath: "/samples/hat.wav", durationSeconds: 0.25, trackIndex: 3 }
  ];

  const bpm = 120; // 2 beats per second
  const spacerBeats = 2;
  const placements = sequencer.calculatePlacements(mockSamples, 0, spacerBeats, bpm);

  // s1: starts 0, length = 2 beats (1s * 2). Next starts at 0 + 2 + 2 = 4 beats.
  // s2: starts 4, length = 1 beat (0.5s * 2). Next starts at 4 + 1 + 2 = 7 beats.
  // s3: starts 7, length = 0.5 beats (0.25s * 2). Next starts at 7 + 0.5 + 2 = 9.5 beats.
  const isValid =
    placements[0].startBeat === 0 &&
    placements[1].startBeat === 4 &&
    placements[2].startBeat === 7;

  console.log(`[Test] Timeline math verification ${isValid ? "PASSED" : "FAILED"}.`);
  return isValid;
}
