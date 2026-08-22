import { SampleNode } from "../../shared/ipc.js";

/**
 * AudioInspector (Phase 2)
 * Parses duration, sample rate, channels, bit depth, and format using music-metadata.
 */
export class AudioInspector {
  public async inspectAudioFile(filePath: string, packId: string): Promise<SampleNode | null> {
    console.log(`[AudioInspector] Inspecting audio file: ${filePath}`);
    return null;
  }
}
