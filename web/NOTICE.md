Flight data display adapted from https://github.com/vNAAATS/vNAAATS-web (Apache-2.0, Andrew Ogden).
Changes: API address is now the same server that serves the page (VITE_API_BASE to override); CORS proxy removed;
natTrak panel reads /GetNatTrakData from this project's server.
Build:  npm ci && NODE_OPTIONS=--openssl-legacy-provider npm run build   (output in web/build)
