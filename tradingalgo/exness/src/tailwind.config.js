/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      fontFamily: {
        'serif': ['Instrument Serif', 'serif'],
        'konkhmer': ['Konkhmer Sleokchher', 'sans-serif'],
        'koho': ['KoHo', 'sans-serif'],
      },
    },
  },
  plugins: [],
}
