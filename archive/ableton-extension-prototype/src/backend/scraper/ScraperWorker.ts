/**
 * ScraperWorker (Phase 6)
 * Background worker queue fetching missing box art and metadata.
 */
export class ScraperWorker {
  public async queuePack(packName: string, packId: string): Promise<void> {
    console.log(`[ScraperWorker] Queued pack for scraping: ${packName} (${packId})`);
  }
}
