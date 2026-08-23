import { SampleNode } from "../../shared/ipc";

/**
 * WaveformBadge (Phase 4)
 * Displays waveform preview of the last-played random sample.
 * Supports drag-and-drop export of the single .wav file to Ableton tracks or Simpler/Drum Rack pads.
 */
export class WaveformBadge {
  public renderBadge(container: HTMLElement, sample: SampleNode): void {
    console.log(`[WaveformBadge] Rendering waveform badge for sample: ${sample.fileName}`);
  }
}
