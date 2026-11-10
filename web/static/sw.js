// Makes the Flight Data Display installable as an app. It deliberately caches nothing, so the page is always the newest version.
self.addEventListener("install", () => self.skipWaiting());
self.addEventListener("activate", (e) => e.waitUntil(self.clients.claim()));
self.addEventListener("fetch", () => { /* straight to the network */ });
