import { bridge } from "./bridge";
import * as THREE from "three";

// UI Elements
const statusDot = document.getElementById("status-dot");
const statusText = document.getElementById("status-text");
const btnPing = document.getElementById("btn-ping") as HTMLButtonElement | null;
const canvas = document.getElementById("coverflow-canvas") as HTMLCanvasElement | null;

let lastPingTime = 0;

// Subscribe to IPC messages from backend host
bridge.subscribe((res) => {
  switch (res.type) {
    case "PONG": {
      const latencyMs = lastPingTime > 0 ? Date.now() - lastPingTime : 0;
      if (statusDot) {
        statusDot.classList.add("connected");
      }
      if (statusText) {
        statusText.style.color = "#00ffaa";
        statusText.innerText = `[Live ${res.payload.liveVersion}] Connected | Latency: ${latencyMs}ms`;
      }
      break;
    }

    case "PACKS_LOADED": {
      console.log(`[Frontend] Loaded ${res.payload.packs.length} packs from backend.`);
      break;
    }

    case "ERROR": {
      console.error("[Frontend] Host error:", res.payload.message);
      if (statusText) {
        statusText.style.color = "#ff4444";
        statusText.innerText = `Error: ${res.payload.message}`;
      }
      break;
    }

    default:
      console.log("[Frontend] Received message:", res);
  }
});

function sendPing(): void {
  lastPingTime = Date.now();
  if (statusText) {
    statusText.innerText = "Pinging host bridge...";
  }
  bridge.send({ type: "PING" });
}

if (btnPing) {
  btnPing.addEventListener("click", () => {
    sendPing();
  });
}

// Initial Handshake
window.addEventListener("DOMContentLoaded", () => {
  setTimeout(() => {
    sendPing();
  }, 500);
});

// Minimal Three.js WebGL Mount & 30 FPS Baseline Verification
if (canvas) {
  const renderer = new THREE.WebGLRenderer({
    canvas,
    antialias: true,
    alpha: true,
    powerPreference: "high-performance"
  });
  
  renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
  renderer.setSize(canvas.clientWidth, canvas.clientHeight, false);

  const scene = new THREE.Scene();
  const camera = new THREE.PerspectiveCamera(45, canvas.clientWidth / canvas.clientHeight, 0.1, 100);
  camera.position.set(0, 0, 5);

  // Subtle ambient + directional light
  const ambientLight = new THREE.AmbientLight(0xffffff, 0.8);
  scene.add(ambientLight);

  const dirLight = new THREE.DirectionalLight(0x00ffcc, 1.2);
  dirLight.position.set(2, 4, 3);
  scene.add(dirLight);

  // Demo Cover Mesh Placeholder
  const geometry = new THREE.BoxGeometry(1.6, 1.6, 0.1);
  const material = new THREE.MeshStandardMaterial({
    color: 0x1f242d,
    metalness: 0.3,
    roughness: 0.4
  });
  const placeholderMesh = new THREE.Mesh(geometry, material);
  scene.add(placeholderMesh);

  // Handle Resize
  const handleResize = () => {
    if (!canvas) return;
    const width = canvas.clientWidth;
    const height = canvas.clientHeight;
    camera.aspect = width / height;
    camera.updateProjectionMatrix();
    renderer.setSize(width, height, false);
  };
  window.addEventListener("resize", handleResize);

  // 30 FPS Delta-capped animation loop
  let lastFrameTime = 0;
  const targetFrameInterval = 1000 / 30; // 33.3ms for 30 FPS baseline

  const animate = (currentTime: number) => {
    requestAnimationFrame(animate);
    const delta = currentTime - lastFrameTime;
    if (delta >= targetFrameInterval) {
      lastFrameTime = currentTime - (delta % targetFrameInterval);
      
      // Gentle idle sway
      placeholderMesh.rotation.y += 0.01;
      renderer.render(scene, camera);
    }
  };

  requestAnimationFrame(animate);
}
