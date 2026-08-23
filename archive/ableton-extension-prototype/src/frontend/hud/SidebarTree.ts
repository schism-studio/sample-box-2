import { FolderNode, PackMetadata } from "../../shared/ipc";

/**
 * SidebarTree (Phase 4)
 * Synchronized 2D folder/file tree browser reactive with the 3D cover flow.
 */
export class SidebarTree {
  public renderTree(container: HTMLElement, packs: PackMetadata[], folders: FolderNode[]): void {
    console.log(`[SidebarTree] Rendering sidebar tree with ${packs.length} packs.`);
  }
}
