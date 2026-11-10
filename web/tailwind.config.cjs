const colors = require("tailwindcss/colors");
var flattenColorPalette = require("tailwindcss/lib/util/flattenColorPalette")
  .default;

const config = {
  mode: "jit",
  purge: ["./src/**/*.{html,js,svelte,ts}"],
  theme: {
    colors: {
      transparent: "transparent",
      current: "currentColor",
      black: colors.black,
      white: colors.white,
      grey: colors.gray,
      cyan: colors.cyan,
      green: colors.green,
      yellow: colors.yellow,
      red: colors.red,
      discord_purple:  "#5865f2",
      // Custom blue
      blue: {
        0: "#0C1827",
        1: "#142942",
        2: "#1D395D",
        3: "#254A78",
        4: "#2D5B93",
        5: "#3B6EB3", // vNAAATS plugin screen colour
        6: "#427DC5",
        7: "#5D8FCD",
        8: "#78A2D5",
        9: "#93B5DE",
        10: "#b4e5ff", // vNAAATS plugin window border colour
        11: "#bde8ff",
      },
    },
    fontFamily: {
      regular: ["Verdana, sans-serif"],
      mono: ["Courier, mono"],
      bitmap: ["ATC", "sans-serif"],
      web: ["Open Sans, sans-serif"]
    },
    boxShadow: {
      '5xl': '0 35px 60px -15px rgba(0, 0, 0, 0.5)',
    },
    scale: {
      '70': '0.7',
      '80': '0.8',
      '85': '0.85',
      '90': '0.9',
      '100': '1.0',
      '110': '1.1'
    },
    extend: {
      height: {
        "1/10": "10%",
        "1/14": "7.14%",
        "1/25": "4%",
        "1/7": "14.2857143%",
        "2/7": "28.5714286%",
        "3/7": "42.8571429%",
        "4/7": "57.1428571%",
        "5/7": "71.4285714%",
        "6/7": "85.7142857%",
        "1/8": "12.5%",
        "1/14": "7.14285714286%",
        "8/27": "29.6296296296%",
        "10/27": "37.037037037%"
      },
      width: {
        "1/7": "14.2857143%",
        "2/7": "28.5714286%",
        "3/7": "42.8571429%",
        "4/7": "57.1428571%",
        "5/7": "71.4285714%",
        "6/7": "85.7142857%",
        "1/8": "12.5%",
        "1/15": "6.66666%",
        "1/20": "5%",
        "1/10": "10%",
        "1/9": "11.1111111111%",
        "1/14": "7.14285714286%",
        "1/11": "9.72%",
        "3/14": "23.5%",
        0.48: "47.7%", // This is to align the track header with the list items
        0.32: "32.5%",
        0.68: "68.8%",
        18: "4.5rm",
      },
    },
  },
  plugins: [
    // Custom left, top, right and bottom border colours
    ({ addUtilities, e, theme, variants }) => {
      let colors = flattenColorPalette(theme("borderColor"));
      delete colors["default"];

      // Replace or Add custom colors
      if (this.theme?.extend?.colors !== undefined) {
        colors = Object.assign(colors, this.theme.extend.colors);
      }

      const colorMap = Object.keys(colors).map((color) => ({
        [`.border-t-${color}`]: { borderTopColor: colors[color] },
        [`.border-r-${color}`]: { borderRightColor: colors[color] },
        [`.border-b-${color}`]: { borderBottomColor: colors[color] },
        [`.border-l-${color}`]: { borderLeftColor: colors[color] },
      }));
      const utilities = Object.assign({}, ...colorMap);

      addUtilities(utilities, variants("borderColor"));
    },
    require("tailwind-scrollbar"),
  ],
};

module.exports = config;
