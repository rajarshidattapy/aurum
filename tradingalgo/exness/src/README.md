# TradePro — Frontend

A simple React + Vite frontend for the TradePro landing site and UI components.

This repository contains the landing pages, UI primitives, and example components used for the TradePro marketing site.

Quick start

Prerequisites
- Node.js (18+) and npm

Install
```bash
npm install
```

Run development server
```bash
npm run dev
```

Build for production
```bash
npm run build
```

Preview build
```bash
npm run preview
```

Lint & format
```bash
npm run lint      # run ESLint
npm run lint:fix  # fix lintable issues
npm run format    # run Prettier
```

Project layout (high level)
- `src/` — application source (components, pages, styles, assets)
- `public/` — static assets served by Vite
- `package.json` — scripts and dependencies

Notes
- A small set of developer indexer scripts were removed from `scripts/` and their outputs are ignored; the app does not require those scripts to run or build.
- Tailwind CSS is used for styling; most site styles live in `src/index.css` and `styles/`.

Contributing
- Update or add components under `src/components/`.
- Keep changes focused and run `npm run format` before committing.

Contact
- If you need anything restored or want tooling moved to a `tools/` folder, open an issue or send a quick note in the repo.

---
Small, hand-written README to make the repo easier to pick up. If you'd like a longer guide (design tokens, CI, or local debugging tips) I can extend it next.