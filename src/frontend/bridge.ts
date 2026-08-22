import { ExtensionRequest, ExtensionResponse } from "../shared/ipc";

export type MessageHandler = (res: ExtensionResponse) => void;

export class ResilientBridge {
  private ws: WebSocket | null = null;
  private handlers = new Set<MessageHandler>();
  private reconnectTimer: number | null = null;
  private isConnecting = false;

  constructor() {
    this.connectWebSocket();
    this.listenNativeHost();
  }

  private connectWebSocket(): void {
    if (this.isConnecting || (this.ws && this.ws.readyState === WebSocket.OPEN)) return;
    this.isConnecting = true;

    try {
      this.ws = new WebSocket("ws://127.0.0.1:39002");

      this.ws.onopen = () => {
        this.isConnecting = false;
        console.log("[Bridge] Connected to Node.js host over WebSocket loopback (127.0.0.1:39002).");
      };

      this.ws.onmessage = (event: MessageEvent) => {
        try {
          const data = typeof event.data === "string" ? JSON.parse(event.data) : event.data;
          if (data && data.type) {
            this.handlers.forEach((handler) => handler(data as ExtensionResponse));
          }
        } catch (e) {
          console.error("[Bridge] Failed to parse WebSocket message:", e);
        }
      };

      this.ws.onclose = () => {
        this.isConnecting = false;
        this.ws = null;
        if (!this.reconnectTimer) {
          this.reconnectTimer = window.setTimeout(() => {
            this.reconnectTimer = null;
            this.connectWebSocket();
          }, 2000);
        }
      };

      this.ws.onerror = () => {
        this.isConnecting = false;
      };
    } catch {
      this.isConnecting = false;
    }
  }

  private listenNativeHost(): void {
    window.addEventListener("message", (event: MessageEvent) => {
      if (event.data && typeof event.data === "object" && (event.data as ExtensionResponse).type) {
        this.handlers.forEach((h) => h(event.data as ExtensionResponse));
      }
    });
  }

  public send(request: ExtensionRequest): void {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(request));
      return;
    }

    // Windows WebView2 Fallback
    const chrome = (window as unknown as { chrome?: { webview?: { postMessage: (msg: unknown) => void } } }).chrome;
    if (chrome?.webview?.postMessage) {
      chrome.webview.postMessage(request);
      return;
    }

    // Parent iframe / modal fallback
    if (window.parent && window.parent !== window) {
      window.parent.postMessage(request, "*");
      return;
    }

    console.warn("[Bridge] Standalone mock environment - no active host transport:", request);
  }

  public subscribe(handler: MessageHandler): () => void {
    this.handlers.add(handler);
    return () => {
      this.handlers.delete(handler);
    };
  }

  public isConnected(): boolean {
    return this.ws !== null && this.ws.readyState === WebSocket.OPEN;
  }
}

export const bridge = new ResilientBridge();
