/** @type {import('tailwindcss').Config} */
module.exports = {
  content: ["./public/**/*.{html,js}"],
  darkMode: 'class',
  theme: {
    extend: {
      fontFamily: {
        sans: ['Outfit', 'sans-serif'],
        mono: ['JetBrains Mono', 'monospace'],
      },
      colors: {
        accent: '#3b82f6', 
        'channel-x': '#10b981', 
        'channel-y': '#3b82f6', 
        'channel-z': '#f59e0b', 
        'channel-w': '#ef4444', 
      },
      animation: {
        'blob': 'blob 25s infinite', // Slow breathing speed
        'fade-in': 'fadeIn 0.5s ease-out forwards',
      },
      keyframes: {
        blob: {
          '0%': { transform: 'translate(0px, 0px) scale(1)' },
          '33%': { transform: 'translate(30px, -50px) scale(1.1)' },
          '66%': { transform: 'translate(-20px, 20px) scale(0.9)' },
          '100%': { transform: 'translate(0px, 0px) scale(1)' },
        },
        fadeIn: {
          '0%': { opacity: '0', transform: 'translateY(10px)' },
          '100%': { opacity: '1', transform: 'translateY(0)' },
        }
      },
      boxShadow: {
        // Glass Rim Lighting
        'glass-edge': 'inset 0 1px 0 0 rgba(255, 255, 255, 0.5), 0 4px 20px rgba(0, 0, 0, 0.05)',
      }
    },
  },
  plugins: [],
}