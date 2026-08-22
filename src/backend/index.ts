import { WebSocketServer, WebSocket } from "ws";
import { ExtensionRequest, ExtensionResponse } from "../shared/ipc.js";

export interface ExtensionContext {
  ui: {
    showModalDialog(urlOrHtml: string, width?: number, height?: number): Promise<string>;
    registerContextMenuAction(options: {
      id: string;
      label: string;
      target: "browser" | "track" | "clip";
      callback: () => void;
    }): void;
  };
  commands: {
    registerCommand(id: string, callback: (...args: unknown[]) => void): void;
  };
  getLiveVersion?: () => string;
}

let wss: WebSocketServer | null = null;
let activeSocket: WebSocket | null = null;

export function activate(context?: ExtensionContext): void {
  console.log("[SamplePackVisualBrowser] Activating extension host...");

  // 1. Initialize local loopback server on port 39002 for persistent two-way IPC
  if (!wss) {
    try {
      wss = new WebSocketServer({ port: 39002 });
      
      wss.on("connection", (ws: WebSocket) => {
        console.log("[SamplePackVisualBrowser] Frontend client connected over WebSocket.");
        activeSocket = ws;

        ws.on("message", (raw: string | Buffer) => {
          try {
            const req = JSON.parse(raw.toString()) as ExtensionRequest;
            handleClientRequest(req, ws, context);
          } catch (err) {
            console.error("[Backend] JSON parse error:", err);
            sendResponse(ws, {
              type: "ERROR",
              payload: { message: `Malformed JSON request: ${String(err)}` }
            });
          }
        });

        ws.on("close", () => {
          console.log("[SamplePackVisualBrowser] Frontend client disconnected.");
          if (activeSocket === ws) {
            activeSocket = null;
          }
        });
      });

      console.log("[SamplePackVisualBrowser] WebSocket loopback server listening on ws://127.0.0.1:39002");
    } catch (e) {
      console.warn("[SamplePackVisualBrowser] WebSocket server startup note:", e);
    }
  }

  // 2. Register context menu action in Ableton Live (if running inside Live host)
  if (context?.ui?.registerContextMenuAction) {
    context.ui.registerContextMenuAction({
      id: "com.spvb.open",
      label: "Open Visual Sample Browser",
      target: "browser",
      callback: async () => {
        // Points to frontend dev server in dev mode, or dist singlefile in production
        const isDev = process.env.NODE_ENV !== "production";
        const entryUrl = isDev ? "http://localhost:5173" : "dist/frontend/index.html";
        await context.ui.showModalDialog(entryUrl, 1024, 640);
      }
    });
  }

  console.log("[SamplePackVisualBrowser] Backend initialized successfully.");
}

function handleClientRequest(req: ExtensionRequest, ws: WebSocket, context?: ExtensionContext): void {
  console.log(`[Backend] Received request: ${req.type}`);

  switch (req.type) {
    case "PING": {
      const liveVersion = context?.getLiveVersion?.() ?? "12.4.5+";
      sendResponse(ws, {
        type: "PONG",
        payload: {
          timestamp: Date.now(),
          liveVersion
        }
      });
      break;
    }

    case "SCAN_DIRECTORY": {
      console.log(`[Backend Scanner Stub] Scanning directory: ${req.payload.rootPath}`);
      // Phase 2 stub
      sendResponse(ws, {
        type: "SCAN_COMPLETE",
        payload: { totalPacks: 0, totalSamples: 0 }
      });
      break;
    }

    case "GET_ALL_PACKS": {
      // Phase 2 stub
      sendResponse(ws, {
        type: "PACKS_LOADED",
        payload: { packs: [] }
      });
      break;
    }

    case "GET_RANDOM_SAMPLE": {
      // Phase 4 stub
      console.log(`[Backend Audition Stub] Random sample for pack: ${req.payload.packId}`);
      break;
    }

    case "EXPORT_SINGLE_SAMPLE": {
      // Phase 5 stub
      console.log(`[Backend LOM Stub] Export single sample: ${req.payload.sampleId}`);
      sendResponse(ws, {
        type: "EXPORT_STATUS",
        payload: { success: true, tracksCreated: 1, clipsPlaced: 1 }
      });
      break;
    }

    case "EXPORT_PACK_HIERARCHY": {
      // Phase 5 stub
      console.log(`[Backend LOM Stub] Export pack hierarchy: ${req.payload.packId}`);
      sendResponse(ws, {
        type: "EXPORT_STATUS",
        payload: { success: true, tracksCreated: 3, clipsPlaced: 12 }
      });
      break;
    }

    default:
      console.log(`[Backend] Unhandled request: ${JSON.stringify(req)}`);
  }
}

function sendResponse(ws: WebSocket, res: ExtensionResponse): void {
  if (ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify(res));
  }
}

// Auto-activate in standalone development execution
if (typeof process !== "undefined" && process.env.NODE_ENV !== "production") {
  activate();
}
