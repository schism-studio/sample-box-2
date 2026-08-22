import * as THREE from "three";

/**
 * CoverFlowMesh (Phase 3)
 * Textured quad geometries with floor reflection planes and angle positioning.
 */
export class CoverFlowMesh {
  public createCoverQuad(texture: THREE.Texture): THREE.Mesh {
    const geometry = new THREE.PlaneGeometry(2, 2);
    const material = new THREE.MeshBasicMaterial({ map: texture, side: THREE.DoubleSide });
    return new THREE.Mesh(geometry, material);
  }
}
