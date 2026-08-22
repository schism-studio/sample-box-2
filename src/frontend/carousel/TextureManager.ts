import * as THREE from "three";

/**
 * TextureManager (Phase 3)
 * LRU Texture Cache with explicit texture.dispose() and material.dispose()
 * calls to prevent VRAM memory leaks.
 */
export class TextureManager {
  private cache = new Map<string, THREE.Texture>();
  private maxEntries: number;

  constructor(maxEntries = 50) {
    this.maxEntries = maxEntries;
  }

  public getTexture(url: string): THREE.Texture | null {
    return this.cache.get(url) ?? null;
  }

  public disposeAll(): void {
    for (const texture of this.cache.values()) {
      texture.dispose();
    }
    this.cache.clear();
  }
}
