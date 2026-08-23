/**
 * InertiaController (Phase 3)
 * Mouse-wheel & drag physics with momentum, spring friction, and center snap.
 */
export class InertiaController {
  private velocity = 0;
  private position = 0;

  public update(friction = 0.92): void {
    this.position += this.velocity;
    this.velocity *= friction;
  }

  public addImpulse(delta: number): void {
    this.velocity += delta;
  }
}
