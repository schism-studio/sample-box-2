import { SCHEMA_SQL } from "./Schema.js";

/**
 * DatabaseClient (Phase 2)
 * SQLite client wrapper using better-sqlite3 for high-performance indexing and querying.
 */
export class DatabaseClient {
  private dbPath: string;

  constructor(dbPath = "sample_browser.db") {
    this.dbPath = dbPath;
  }

  public init(): void {
    console.log(`[DatabaseClient] Initializing database at: ${this.dbPath}`);
    // Will execute SCHEMA_SQL with better-sqlite3 in Phase 2
  }
}
