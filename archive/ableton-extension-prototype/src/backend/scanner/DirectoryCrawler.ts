/**
 * DirectoryCrawler (Phase 2)
 * Asynchronous filesystem walker using fast-glob to index audio files and pack hierarchies.
 */
export class DirectoryCrawler {
  public async crawl(rootPath: string): Promise<string[]> {
    console.log(`[DirectoryCrawler] Crawling path: ${rootPath}`);
    return [];
  }
}
