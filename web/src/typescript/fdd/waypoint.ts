/*
 *  Waypoint display format. Flight plans arrive in many styles (52/50, 52N050W, 5230N05000W, 4545N ...);
 *  the display always shows oceanic coordinates the same short way, as on the NAT tracks: 52/50.
 *  Longitude is always whole degrees. Latitude keeps its minutes when it is a half degree (RLatSM), so 4730N05000W is shown as
 *  4730/50, never as 47/50 (that would be a different point, 30 minutes south). ARINC 424 forms: 4050N = 40/50, N4050 = 4030/50.
 *  Named fixes are returned unchanged (upper case, without any /M082F390 speed-level suffix).
 *  Anything that is not a north-west coordinate (e.g. east longitudes) is returned unchanged.
 */
export function formatWaypoint(token: string): string {
  const t: string = (token ?? "").trim().toUpperCase().replace(/\/[NMK]\d{3,4}[FASM]\d{3,4}$/, "");
  const deg = (s: string): string => String(parseInt(s, 10)).padStart(2, "0");

  const lat = (d: string, mins?: string): string => (mins && parseInt(mins, 10) !== 0 ? deg(d) + mins : deg(d));

  // 52/50, 5230/50, 52/050
  let m = t.match(/^(\d{2})(\d{2})?\/(\d{2,3})$/);
  if (m) return lat(m[1], m[2]) + "/" + deg(m[3]);

  // 52N050W, 5230N05000W, 52N05030W
  m = t.match(/^(\d{2})(\d{2})?N(\d{3})(?:\d{2})?W$/);
  if (m) return lat(m[1], m[2]) + "/" + deg(m[3]);

  // ARINC 424 half degree: N5020 = 50 30 N 020 W
  m = t.match(/^N(\d{2})(\d{2})$/);
  if (m) return deg(m[1]) + "30/" + deg(m[2]);

  // ICAO shorthand 4545N = 45N 045W (the letter N means north-west)
  m = t.match(/^(\d{2})(\d{2})N$/);
  if (m) return deg(m[1]) + "/" + deg(m[2]);

  return t;
}
