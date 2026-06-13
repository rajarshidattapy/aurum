import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
// path not needed (alias removed)
// https://vite.dev/config/
export default defineConfig({
  plugins: [react()],
  // No path alias for '@' any more — imports use `src/...` or relative paths.
})
