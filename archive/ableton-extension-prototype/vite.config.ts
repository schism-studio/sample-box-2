import { defineConfig } from "vite";
import { resolve } from "path";
import { viteSingleFile } from "vite-plugin-singlefile";

export default defineConfig({
  root: "src/frontend",
  base: "./",
  plugins: [viteSingleFile()],
  build: {
    outDir: resolve(__dirname, "dist/frontend"),
    emptyOutDir: true,
    rollupOptions: {
      input: {
        main: resolve(__dirname, "src/frontend/index.html")
      }
    }
  },
  server: {
    port: 5173,
    strictPort: true
  }
});
