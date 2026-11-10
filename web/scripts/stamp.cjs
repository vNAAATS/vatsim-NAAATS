// Writes src/build-info.json with the time of this build (shown as a tooltip next to the version in the FDD header).
const fs = require("fs");
const d = new Date();
const p = (n) => String(n).padStart(2, "0");
const built = `${d.getUTCFullYear()}-${p(d.getUTCMonth() + 1)}-${p(d.getUTCDate())} ${p(d.getUTCHours())}:${p(d.getUTCMinutes())}z`;
fs.writeFileSync(__dirname + "/../src/build-info.json", JSON.stringify({ built }) + "\n");
