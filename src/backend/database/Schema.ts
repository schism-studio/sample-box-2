/**
 * SQLite Database Schema Definitions (Phase 2)
 */
export const SCHEMA_SQL = `
  CREATE TABLE IF NOT EXISTS packs (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    path TEXT UNIQUE NOT NULL,
    artwork_path TEXT,
    total_samples INTEGER DEFAULT 0,
    total_duration_seconds REAL DEFAULT 0,
    created_at INTEGER DEFAULT (strftime('%s', 'now'))
  );

  CREATE TABLE IF NOT EXISTS folders (
    id TEXT PRIMARY KEY,
    pack_id TEXT NOT NULL,
    folder_path TEXT NOT NULL,
    relative_path TEXT NOT NULL,
    sample_count INTEGER DEFAULT 0,
    FOREIGN KEY(pack_id) REFERENCES packs(id) ON DELETE CASCADE
  );

  CREATE TABLE IF NOT EXISTS samples (
    id TEXT PRIMARY KEY,
    pack_id TEXT NOT NULL,
    folder_id TEXT,
    file_path TEXT NOT NULL,
    relative_path TEXT NOT NULL,
    file_name TEXT NOT NULL,
    duration_seconds REAL NOT NULL,
    sample_rate INTEGER NOT NULL,
    channels INTEGER NOT NULL,
    format TEXT NOT NULL,
    bpm REAL,
    musical_key TEXT,
    FOREIGN KEY(pack_id) REFERENCES packs(id) ON DELETE CASCADE
  );

  CREATE TABLE IF NOT EXISTS artwork_cache (
    pack_id TEXT PRIMARY KEY,
    source_url TEXT,
    cached_path TEXT NOT NULL,
    fetched_at INTEGER DEFAULT (strftime('%s', 'now')),
    FOREIGN KEY(pack_id) REFERENCES packs(id) ON DELETE CASCADE
  );
`;
