/** @type {import('tailwindcss').Config} */
module.exports = {
  content: ["./public/**/*.{html,js}"],
  darkMode: 'class',
  theme: {
    extend: {
      colors: {
        'accent': '#2dd4bf',
        'status-ok': '#22c55e',
        'status-bad': '#ef4444',
        
        // Channels
        'channel-x': '#22c55e', 
        'channel-y': '#3b82f6', 
        'channel-z': '#f97316', 
        'channel-w': '#ef4444', 
      },
      backgroundImage: {
        // Richer, multi-stop gradients
        'aurora-light': 'linear-gradient(120deg, #e0c3fc 0%, #8ec5fc 100%)',
        'nebula-dark': 'radial-gradient(circle at 0% 0%, #3b0764 0%, #0f0c29 50%, #000000 100%)', 
      },
      boxShadow: {
        'glass': '0 8px 32px 0 rgba(0, 0, 0, 0.3)',
        'glass-light': '0 8px 32px 0 rgba(31, 38, 135, 0.1)',
        'glow-sm': '0 0 10px -2px var(--tw-shadow-color)',
        'glow-md': '0 0 20px -5px var(--tw-shadow-color)',
      },
      fontFamily: {
        sans: ['Inter', 'Segoe UI', 'Roboto', 'sans-serif'],
        mono: ['JetBrains Mono', 'Fira Code', 'monospace'],
      }
    },
  },
  plugins: [],
}