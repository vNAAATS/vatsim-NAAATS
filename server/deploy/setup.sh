#!/usr/bin/env bash
# One-shot installer for Ubuntu 22.04/24.04. Run as root from the unpacked folder:
#   bash deploy/setup.sh                 -> serves over plain HTTP on the server's IP (testing)
#   bash deploy/setup.sh api.example.net -> serves over HTTPS for that domain (DNS must already point here)
set -euo pipefail
DOMAIN="${1:-}"
APP=/opt/vnaaats
SRC="$(cd "$(dirname "$0")/.." && pwd)"

echo "== Installing packages"
export DEBIAN_FRONTEND=noninteractive
apt-get update -y
apt-get install -y python3 ufw caddy openssl

echo "== Creating service user and copying files"
id vnaaats >/dev/null 2>&1 || useradd --system --home "$APP" --shell /usr/sbin/nologin vnaaats
mkdir -p "$APP" /var/lib/vnaaats
cp "$SRC/vnaaats_server.py" "$SRC/cpdlc.py" "$SRC/clx.py" "$SRC/commands.py" "$SRC/rcl_auto.py" "$SRC/handoff.py" "$SRC/members.py" "$SRC/adminlog.py" "$SRC/usersettings.py" "$SRC/pluginfiles.py" "$SRC/topsky_cpdlc.txt" "$SRC/fixes.json" "$SRC/position_codes.json" "$APP/"
[ -f "$APP/extra_fixes.json" ] || cp "$SRC/extra_fixes.json" "$APP/"      # never overwrite your own additions
if [ -d "$SRC/web" ]; then rm -rf "$APP/web"; cp -r "$SRC/web" "$APP/web"; fi   # built flight data display
chown -R vnaaats:vnaaats "$APP" /var/lib/vnaaats

echo "== Settings (API key)"
if [ ! -f /etc/vnaaats.env ]; then
  KEY="$(openssl rand -hex 24)"
  cat > /etc/vnaaats.env <<ENV
VNAAATS_API_KEY=$KEY
VNAAATS_PORT=8080
VNAAATS_BIND=127.0.0.1
VNAAATS_DB=/var/lib/vnaaats/vnaaats.db
ENV
  chmod 600 /etc/vnaaats.env
  NEWKEY="$KEY"
else
  NEWKEY=""
fi

# Secret the Gander Oceanic website uses to ask for personal member keys - created once
if ! grep -q '^VNAAATS_ISSUER_SECRET=' /etc/vnaaats.env; then
  echo "VNAAATS_ISSUER_SECRET=$(openssl rand -hex 24)" >> /etc/vnaaats.env
  echo "Created VNAAATS_ISSUER_SECRET in /etc/vnaaats.env (the website needs the same value; it is not shown here)."
fi

# Admin log (https://<server>/admin/log): the token unlocks the page. Created once. Logs are kept 7 days in /var/lib/vnaaats/logs
NEWADMIN=""
if ! grep -q '^VNAAATS_ADMIN_TOKEN=' /etc/vnaaats.env; then
  NEWADMIN="$(openssl rand -hex 12)"
  echo "VNAAATS_ADMIN_TOKEN=$NEWADMIN" >> /etc/vnaaats.env
fi
grep -q '^VNAAATS_LOG_DIR=' /etc/vnaaats.env || echo "VNAAATS_LOG_DIR=/var/lib/vnaaats/logs" >> /etc/vnaaats.env

# Web editing password (the FDD login) - created once, change it in /etc/vnaaats.env any time
if grep -q '^VNAAATS_SHARED_PASSWORD=off' /etc/vnaaats.env; then
  NEWPASS=""                                              # shared password switched off on purpose: do not make a new one
elif ! grep -q '^VNAAATS_FDD_PASSWORD=' /etc/vnaaats.env; then
  NEWPASS="$(openssl rand -hex 5)"
  echo "VNAAATS_FDD_PASSWORD=$NEWPASS" >> /etc/vnaaats.env
else
  NEWPASS=""
fi

echo "== Service"
cp "$SRC/deploy/vnaaats.service" /etc/systemd/system/vnaaats.service
mkdir -p "$APP/deploy" && cp "$SRC/deploy/plugin_announce.py" "$APP/deploy/"
cp "$SRC/deploy/vnaaats-plugin-notice.service" "$SRC/deploy/vnaaats-plugin-notice.timer" /etc/systemd/system/
systemctl daemon-reload
systemctl enable --now vnaaats
systemctl enable --now vnaaats-plugin-notice.timer
systemctl restart vnaaats

echo "== Web front (Caddy)"
if [ -n "$DOMAIN" ]; then
  printf '%s {\n    reverse_proxy 127.0.0.1:8080\n}\n' "$DOMAIN" > /etc/caddy/Caddyfile
elif grep -q "reverse_proxy 127.0.0.1:8080" /etc/caddy/Caddyfile 2>/dev/null; then
  echo "(keeping your existing Caddy settings)"        # a re-install never replaces a working domain setup
else
  printf ':80 {\n    reverse_proxy 127.0.0.1:8080\n}\n' > /etc/caddy/Caddyfile
fi
systemctl enable caddy
systemctl restart caddy

echo "== Firewall"
ufw allow OpenSSH
ufw allow 80/tcp
ufw allow 443/tcp
ufw --force enable

sleep 3
echo
echo "== Health check"
curl -fsS http://127.0.0.1:8080/health || echo "(server not answering yet - run: journalctl -u vnaaats -n 50)"
echo
# Optional: tell the Discord channel about the new version (needs VNAAATS_DISCORD_WEBHOOK in /etc/vnaaats.env)
python3 "$(dirname "$0")/announce.py" "$(dirname "$0")/.." || true
echo
if [ -n "$NEWKEY" ]; then
  echo "=============================================================="
  echo " YOUR API KEY (save it somewhere safe, it is shown only once):"
  echo " $NEWKEY"
  echo "=============================================================="
else
  echo "API key unchanged (see /etc/vnaaats.env)."
fi
if [ -n "$NEWADMIN" ]; then
  echo "=============================================================="
  echo " ADMIN LOG TOKEN (open https://vnaaats.com/admin/log and paste it; shown only once):"
  echo " $NEWADMIN"
  echo "=============================================================="
else
  echo "Admin log token unchanged (VNAAATS_ADMIN_TOKEN in /etc/vnaaats.env)."
fi
if [ -n "$NEWPASS" ]; then
  echo "=============================================================="
  echo " FDD CONTROLLER PASSWORD (for editing on the web page):"
  echo " $NEWPASS"
  echo " Change it any time: nano /etc/vnaaats.env  then  systemctl restart vnaaats"
  echo "=============================================================="
else
  echo "FDD password unchanged (VNAAATS_FDD_PASSWORD in /etc/vnaaats.env)."
fi
