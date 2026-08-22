export interface SampleClipPlacement {
  sampleId: string;
  filePath: string;
  startBeat: number;
  lengthBeats: number;
  trackIndex: number;
}

/**
 * TimelineSequencer (Phase 5)
 * Calculates zero-overlap timeline placement across all tracks using the global serialization formula:
 * T_start,i = T_cursor
 * T_cursor = T_start,i + D_i + T_spacer
 */
export class TimelineSequencer {
  public calculatePlacements(
    samples: Array<{ id: string; filePath: string; durationSeconds: number; trackIndex: number }>,
    startBeat = 0,
    spacerBeats = 2,
    bpm = 120
  ): SampleClipPlacement[] {
    let cursorBeat = startBeat;
    const placements: SampleClipPlacement[] = [];

    for (const sample of samples) {
      const beatsPerSecond = bpm / 60;
      const lengthBeats = sample.durationSeconds * beatsPerSecond;

      placements.push({
        sampleId: sample.id,
        filePath: sample.filePath,
        startBeat: cursorBeat,
        lengthBeats,
        trackIndex: sample.trackIndex
      });

      cursorBeat += lengthBeats + spacerBeats;
    }

    return placements;
  }
}
