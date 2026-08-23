/**
 * PreviewVoice (Phase 4)
 * Monophonic instant-choke Web Audio player.
 * Uses 2ms micro-ramps on gain nodes to prevent speaker clicks when switching samples.
 */
export class PreviewVoice {
  private audioCtx: AudioContext | null = null;
  private currentSource: AudioBufferSourceNode | null = null;
  private gainNode: GainNode | null = null;

  public init(): void {
    const AudioContextClass = window.AudioContext || (window as unknown as { webkitAudioContext: typeof AudioContext }).webkitAudioContext;
    this.audioCtx = new AudioContextClass();
    this.gainNode = this.audioCtx.createGain();
    this.gainNode.connect(this.audioCtx.destination);
  }

  public playBuffer(buffer: AudioBuffer): void {
    if (!this.audioCtx || !this.gainNode) {
      this.init();
    }
    if (!this.audioCtx || !this.gainNode) return;

    // Instant-choke previous voice with 2ms anti-click micro-ramp
    if (this.currentSource) {
      const now = this.audioCtx.currentTime;
      this.gainNode.gain.setValueAtTime(this.gainNode.gain.value, now);
      this.gainNode.gain.linearRampToValueAtTime(0.0001, now + 0.002);
      this.currentSource.stop(now + 0.002);
      this.currentSource.disconnect();
    }

    const newSource = this.audioCtx.createBufferSource();
    newSource.buffer = buffer;

    const newGain = this.audioCtx.createGain();
    newGain.gain.setValueAtTime(0.0001, this.audioCtx.currentTime);
    newGain.gain.linearRampToValueAtTime(1.0, this.audioCtx.currentTime + 0.002);

    newSource.connect(newGain);
    newGain.connect(this.audioCtx.destination);

    newSource.start();
    this.currentSource = newSource;
  }
}
