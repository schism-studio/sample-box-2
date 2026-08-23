/**
 * Shared IPC Protocol Contracts
 * Strictly typed messages and data structures exchanged between
 * the Node.js backend extension host and the WebGL frontend WebView.
 */

export interface PackMetadata {
  id: string;
  name: string;
  path: string;
  artworkPath: string | null;
  totalSamples: number;
  totalDurationSeconds: number;
  tags?: string[];
}

export interface SampleNode {
  id: string;
  packId: string;
  filePath: string;
  relativePath: string;
  fileName: string;
  durationSeconds: number;
  sampleRate: number;
  channels: number;
  format: string;
  bpm?: number;
  key?: string;
}

export interface FolderNode {
  id: string;
  packId: string;
  folderPath: string;
  relativePath: string;
  folderName: string;
  sampleCount: number;
}

export type ExtensionRequest =
  | { type: "PING" }
  | { type: "SCAN_DIRECTORY"; payload: { rootPath: string } }
  | { type: "GET_ALL_PACKS" }
  | { type: "GET_PACK_SAMPLES"; payload: { packId: string } }
  | { type: "GET_RANDOM_SAMPLE"; payload: { packId: string } }
  | { type: "EXPORT_SINGLE_SAMPLE"; payload: { sampleId: string; targetTrackIndex?: number } }
  | { type: "EXPORT_PACK_HIERARCHY"; payload: { packId: string; startBeat: number; spacerBeats?: number } };

export type ExtensionResponse =
  | { type: "PONG"; payload: { timestamp: number; liveVersion: string } }
  | { type: "SCAN_PROGRESS"; payload: { discovered: number; currentFolder: string } }
  | { type: "SCAN_COMPLETE"; payload: { totalPacks: number; totalSamples: number } }
  | { type: "PACKS_LOADED"; payload: { packs: PackMetadata[] } }
  | { type: "PACK_SAMPLES_LOADED"; payload: { packId: string; samples: SampleNode[] } }
  | { type: "RANDOM_SAMPLE_RESULT"; payload: { sample: SampleNode; audioDataUri?: string } }
  | { type: "EXPORT_STATUS"; payload: { success: boolean; tracksCreated: number; clipsPlaced: number; error?: string } }
  | { type: "ERROR"; payload: { message: string; code?: string } };
