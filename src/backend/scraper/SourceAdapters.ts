/**
 * SourceAdapters (Phase 6)
 * Adapters for external media metadata and cover art sources.
 */
export interface ArtSearchResult {
  imageUrl: string;
  source: string;
  confidence: number;
}

export class SourceAdapters {
  public async searchArt(query: string): Promise<ArtSearchResult[]> {
    console.log(`[SourceAdapters] Searching art for: ${query}`);
    return [];
  }
}
