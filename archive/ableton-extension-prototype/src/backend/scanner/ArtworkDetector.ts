/**
 * ArtworkDetector (Phase 2)
 * Searches for local cover.jpg, cover.png, folder.jpg, or embedded ID3 artwork.
 */
export class ArtworkDetector {
  public async findCoverArt(directoryPath: string): Promise<string | null> {
    console.log(`[ArtworkDetector] Finding cover art in: ${directoryPath}`);
    return null;
  }
}
