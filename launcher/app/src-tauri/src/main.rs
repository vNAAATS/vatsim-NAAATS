// The vNAAATS app: a window around https://vnaaats.com/fdd plus a few jobs a web page cannot do
// (find the plugin, check and replace the DLL, save the key into vnaaats.cfg, start EuroScope and VACS).
// Only pages from vnaaats.com may use these jobs.
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use serde::{Deserialize, Serialize};
use std::path::{Path, PathBuf};
use std::process::Command;
use std::sync::atomic::{AtomicBool, Ordering};
use tauri::{Manager, WebviewUrl, WebviewWindow, WebviewWindowBuilder};
use tauri_plugin_dialog::DialogExt;

const SITE: &str = "https://vnaaats.com";

#[derive(Serialize, Deserialize, Default, Clone)]
struct Saved {
    root: Option<String>,
    euroscope: Option<String>,
    vacs: Option<String>,
    #[serde(default)]
    afv: Option<String>,
    #[serde(default)]
    trackaudio: Option<String>,
}

fn config_file(app: &tauri::AppHandle) -> Result<PathBuf, String> {
    let dir = app.path().app_config_dir().map_err(|e| e.to_string())?;
    std::fs::create_dir_all(&dir).map_err(|e| e.to_string())?;
    Ok(dir.join("settings.json"))
}

fn load(app: &tauri::AppHandle) -> Saved {
    config_file(app).ok().and_then(|f| std::fs::read_to_string(f).ok()).and_then(|t| serde_json::from_str(&t).ok()).unwrap_or_default()
}

fn store(app: &tauri::AppHandle, s: &Saved) -> Result<(), String> {
    std::fs::write(config_file(app)?, serde_json::to_string_pretty(s).map_err(|e| e.to_string())?).map_err(|e| e.to_string())
}

/// Only the vNAAATS website may ask this app to do anything.
fn check_origin(w: &WebviewWindow) -> Result<(), String> {
    let url = w.url().map_err(|e| e.to_string())?;
    if url.scheme() == "https" && url.host_str() == Some("vnaaats.com") {
        Ok(())
    } else {
        Err("not allowed from this page".into())
    }
}

fn http_get(path: &str, max: u64) -> Result<Vec<u8>, String> {
    use std::io::Read;
    let resp = ureq::get(&format!("{}{}", SITE, path)).timeout(std::time::Duration::from_secs(60)).call().map_err(|e| format!("could not reach vnaaats.com: {}", e))?;
    let mut buf = Vec::new();
    resp.into_reader().take(max).read_to_end(&mut buf).map_err(|e| e.to_string())?;
    Ok(buf)
}

#[derive(Serialize)]
struct Status {
    app_version: String,
    root: Option<String>,
    euroscope: Option<String>,
    vacs: Option<String>,
    afv: Option<String>,
    trackaudio: Option<String>,
    dll_found: bool,
    dll_path: Option<String>,
    dll_sha: Option<String>,
    latest_version: Option<String>,
    latest_sha: Option<String>,
    up_to_date: Option<bool>,
    airlines_found: bool,
    airlines_up_to_date: Option<bool>,
    font_found: bool,
    cfg_exists: bool,
    cfg_has_key: bool,
    error: Option<String>,
}

#[tauri::command]
async fn status(window: WebviewWindow, app: tauri::AppHandle) -> Result<Status, String> {
    check_origin(&window)?;
    let saved = load(&app);
    let mut st = Status {
        app_version: env!("CARGO_PKG_VERSION").to_string(),
        root: saved.root.clone(),
        euroscope: saved.euroscope.clone(),
        vacs: saved.vacs.clone(),
        afv: saved.afv.clone(),
        trackaudio: saved.trackaudio.clone(),
        dll_found: false,
        dll_path: None,
        dll_sha: None,
        latest_version: None,
        latest_sha: None,
        up_to_date: None,
        airlines_found: false,
        airlines_up_to_date: None,
        font_found: false,
        cfg_exists: false,
        cfg_has_key: false,
        error: None,
    };
    let Some(root) = saved.root else { return Ok(st) };
    let mut airlines_local: Option<String> = None;
    if let Some(dll) = vnaaats_core::find_plugin_dll(Path::new(&root), 6) {
        st.dll_found = true;
        st.dll_path = Some(dll.to_string_lossy().into_owned());
        if let Ok(bytes) = std::fs::read(&dll) {
            st.dll_sha = Some(vnaaats_core::sha256_hex(&bytes));
        }
        if let Some(dir) = dll.parent() {
            airlines_local = std::fs::read(dir.join("ICAO_Airlines.txt")).ok().map(|b| vnaaats_core::sha256_hex(&b));
            st.airlines_found = airlines_local.is_some();
            st.font_found = dir.join("vNAAATS.ttf").is_file();           // the plugin loads this font from its own folder
            let c = vnaaats_core::read_cfg(dir);
            st.cfg_exists = c.exists;
            st.cfg_has_key = c.has_key;
        }
    }
    match http_get("/plugin/latest.json", 4096).and_then(|b| serde_json::from_slice::<serde_json::Value>(&b).map_err(|e| e.to_string())) {
        Ok(v) => {
            st.latest_version = v["version"].as_str().map(String::from);
            st.latest_sha = v["sha256"].as_str().map(String::from);
            if let (Some(a), Some(b)) = (&st.dll_sha, &st.latest_sha) {
                st.up_to_date = Some(a.eq_ignore_ascii_case(b));
            }
            if let Some(want) = v["airlinesSha256"].as_str() {
                st.airlines_up_to_date = Some(airlines_local.as_deref().map(|h| h.eq_ignore_ascii_case(want)).unwrap_or(false));
            }
        }
        Err(e) => st.error = Some(e),
    }
    Ok(st)
}

#[tauri::command]
async fn pick_folder(window: WebviewWindow, app: tauri::AppHandle) -> Result<Option<String>, String> {
    check_origin(&window)?;
    let Some(p) = app.dialog().file().set_title("Choose your EXOCO folder (the one holding Plugins)").blocking_pick_folder() else { return Ok(None) };
    let path = p.into_path().map_err(|e| e.to_string())?;
    let mut s = load(&app);
    s.root = Some(path.to_string_lossy().into_owned());
    store(&app, &s)?;
    Ok(s.root)
}

#[tauri::command]
async fn update_plugin(window: WebviewWindow, app: tauri::AppHandle) -> Result<String, String> {
    check_origin(&window)?;
    let root = load(&app).root.ok_or("choose your EXOCO folder first")?;
    let dll = vnaaats_core::find_plugin_dll(Path::new(&root), 6).ok_or("VatsimNAAATS.dll was not found in that folder")?;
    let info: serde_json::Value = serde_json::from_slice(&http_get("/plugin/latest.json", 4096)?).map_err(|e| e.to_string())?;
    let dir = dll.parent().ok_or("bad folder")?.to_path_buf();
    let mut done: Vec<String> = Vec::new();
    // the airline list (radio callsigns) goes next to the DLL
    if let Some(want) = info["airlinesSha256"].as_str() {
        let list = dir.join("ICAO_Airlines.txt");
        let have = std::fs::read(&list).ok().map(|b| vnaaats_core::sha256_hex(&b));
        if have.as_deref().map(|h| !h.eq_ignore_ascii_case(want)).unwrap_or(true) {
            let bytes = http_get("/ICAO_Airlines.txt", 5 * 1024 * 1024)?;
            vnaaats_core::replace_file(&list, &bytes, want)?;
            done.push("airline list".to_string());
        }
    }
    // the vNAAATS font goes next to the DLL (the plugin loads it from there); an existing one is left alone
    if let Some(want) = info["fontSha256"].as_str() {
        let font = dir.join("vNAAATS.ttf");
        if !font.is_file() {
            let bytes = http_get("/vNAAATS.ttf", 2 * 1024 * 1024)?;
            vnaaats_core::replace_file(&font, &bytes, want)?;
            done.push("vNAAATS font".to_string());
        }
    }
    let sha = info["sha256"].as_str().ok_or("the server did not say which plugin is the latest")?.to_string();
    let current = std::fs::read(&dll).ok().map(|b| vnaaats_core::sha256_hex(&b));
    if current.as_deref().map(|h| !h.eq_ignore_ascii_case(&sha)).unwrap_or(true) {
        let bytes = http_get("/plugin/VatsimNAAATS.dll", 20 * 1024 * 1024)?;
        vnaaats_core::replace_dll(&dll, &bytes, &sha)?;
        done.push(format!("plugin {}", info["version"].as_str().unwrap_or("")));
    }
    Ok(if done.is_empty() { "already up to date".to_string() } else { done.join(" and ") })
}

#[tauri::command]
async fn save_key(window: WebviewWindow, app: tauri::AppHandle, key: String) -> Result<(), String> {
    check_origin(&window)?;
    let root = load(&app).root.ok_or("choose your EXOCO folder first")?;
    let dll = vnaaats_core::find_plugin_dll(Path::new(&root), 6).ok_or("VatsimNAAATS.dll was not found in that folder")?;
    vnaaats_core::set_cfg_key(dll.parent().ok_or("bad folder")?, &key)
}

fn kind_ok(kind: &str) -> Result<(), String> {
    if ["euroscope", "vacs", "afv", "trackaudio"].contains(&kind) { Ok(()) } else { Err("unknown program".into()) }
}

#[tauri::command]
async fn pick_exe(window: WebviewWindow, app: tauri::AppHandle, kind: String) -> Result<Option<String>, String> {
    check_origin(&window)?;
    kind_ok(&kind)?;
    let title = match kind.as_str() {
        "euroscope" => "Choose EuroScope.exe",
        "afv" => "Choose the Audio for VATSIM program",
        "trackaudio" => "Choose the TrackAudio program",
        _ => "Choose the VACS program",
    };
    let Some(p) = app.dialog().file().set_parent(&window).set_title(title).add_filter("Program", &["exe"]).blocking_pick_file() else { return Ok(None) };
    let path = p.into_path().map_err(|e| e.to_string())?;
    if !vnaaats_core::is_exe(&path) {
        return Err("that is not a program (.exe)".into());
    }
    let mut s = load(&app);
    let text = path.to_string_lossy().into_owned();
    match kind.as_str() {
        "euroscope" => s.euroscope = Some(text.clone()),
        "afv" => s.afv = Some(text.clone()),
        "trackaudio" => s.trackaudio = Some(text.clone()),
        _ => s.vacs = Some(text.clone()),
    }
    store(&app, &s)?;
    Ok(Some(text))
}

#[tauri::command]
async fn launch(window: WebviewWindow, app: tauri::AppHandle, kind: String) -> Result<(), String> {
    check_origin(&window)?;
    kind_ok(&kind)?;
    let s = load(&app);
    let path = match kind.as_str() {
        "euroscope" => s.euroscope,
        "afv" => s.afv,
        "trackaudio" => s.trackaudio,
        _ => s.vacs,
    }
    .ok_or("choose the program first")?;
    let p = PathBuf::from(&path);
    if !vnaaats_core::is_exe(&p) {
        return Err("that program is no longer there - choose it again".into());
    }
    let mut cmd = Command::new(&p);
    if let Some(dir) = p.parent() {
        cmd.current_dir(dir);
    }
    match cmd.spawn() {
        Ok(_) => Ok(()),
        // Some programs (Audio for VATSIM can) ask Windows for administrator rights; starting them directly then fails with error 740.
        // Opening the file the way Explorer does shows the normal Windows permission prompt instead.
        Err(e) if e.raw_os_error() == Some(740) => open_like_explorer(&p),
        Err(e) => Err(format!("could not start it: {}", e)),
    }
}

#[cfg(windows)]
fn open_like_explorer(p: &Path) -> Result<(), String> {
    Command::new("rundll32").arg("url.dll,FileProtocolHandler").arg(p).spawn().map(|_| ()).map_err(|e| format!("could not start it: {}", e))
}

#[cfg(not(windows))]
fn open_like_explorer(_p: &Path) -> Result<(), String> {
    Err("could not start it: it needs administrator rights".into())
}

/// Open a web page in the normal browser. Only a few known sites are allowed, so a page cannot make this open anything else.
#[tauri::command]
async fn open_url(window: WebviewWindow, url: String) -> Result<(), String> {
    check_origin(&window)?;
    let u = tauri::Url::parse(&url).map_err(|_| "not a web address".to_string())?;
    let host = u.host_str().unwrap_or("");
    let ok = u.scheme() == "https" && ["nattrak.vatsim.net", "github.com", "vatsim.net", "vnaaats.com", "vacs.network", "docs.vacs.network", "audio.vatsim.net"].contains(&host);
    if !ok {
        return Err("that address is not allowed".into());
    }
    #[cfg(windows)]
    {
        Command::new("rundll32").arg("url.dll,FileProtocolHandler").arg(u.as_str()).spawn().map(|_| ()).map_err(|e| e.to_string())
    }
    #[cfg(not(windows))]
    {
        Err("opening web pages is only supported on Windows".into())
    }
}

/// Open natTrak's "new clearance" page in its own window with the given fields filled in. It NEVER presses Create: the controller checks and does that.
/// fields = (id of the form field on natTrak, value). Only known field ids are used, and values are passed as data (JSON), never as code.
#[tauri::command]
async fn open_nattrak(window: WebviewWindow, app: tauri::AppHandle, fields: Vec<(String, String)>) -> Result<(), String> {
    check_origin(&window)?;
    const ALLOWED: [&str; 8] = ["callsign", "destination", "flight_level", "mach", "track_id", "entry_fix", "entry_time", "random_routeing"];
    let clean: Vec<(String, String)> = fields
        .into_iter()
        .filter(|(k, v)| ALLOWED.contains(&k.as_str()) && !v.is_empty() && v.len() <= 200)
        .collect();
    let data = serde_json::to_string(&clean).map_err(|e| e.to_string())?;
    let script = format!(
        r#"(function(){{
  var F={data};
  function note(){{ if(document.getElementById('vnaaats-note')||!document.body) return; var d=document.createElement('div'); d.id='vnaaats-note';
    d.style.cssText='position:fixed;z-index:99999;left:0;right:0;top:0;background:#fef3c7;color:#78350f;border-bottom:2px solid #f59e0b;padding:6px 12px;font:13px Arial,sans-serif';
    d.textContent='vNAAATS filled in the highlighted fields. Check them, then press Create yourself.'; document.body.appendChild(d); }}
  function fill(){{
    F.forEach(function(p){{ var el=document.getElementById(p[0]); if(!el) return; var v=p[1];
      if(el.tagName==='SELECT'){{ var o=Array.prototype.find.call(el.options,function(x){{ var t=(x.text||'').trim().toUpperCase(); return t===v.toUpperCase()||t.indexOf('TRACK '+v.toUpperCase())===0||t.indexOf(v.toUpperCase()+' ')===0; }}); if(!o) return; el.value=o.value; }}
      else {{ el.value=v; }}
      el.dispatchEvent(new Event('input',{{bubbles:true}})); el.dispatchEvent(new Event('change',{{bubbles:true}})); el.dispatchEvent(new Event('blur',{{bubbles:true}}));
      el.style.background='#fef9c3'; el.style.outline='2px solid #eab308'; }});
    note(); }}
  function denied(){{
    if(document.getElementById('vnaaats-denied')||!document.body) return false;
    var t=(document.body.innerText||'').toUpperCase();
    if(t.indexOf('THIS ACTION IS UNAUTHORIZED')<0&&t.indexOf('403')<0) return false;
    if(t.length>400) return false;
    var d=document.createElement('div'); d.id='vnaaats-denied';
    d.style.cssText='position:fixed;z-index:99999;left:0;right:0;top:0;background:#fee2e2;color:#7f1d1d;border-bottom:2px solid #dc2626;padding:8px 12px;font:13px Arial,sans-serif';
    d.textContent='natTrak refused this page (403). It means the account signed in here is not an active oceanic controller on natTrak: you must be online on VATSIM on an oceanic position, and signed in here with your own VATSIM login. Sign in again in this window (log out of natTrak first if needed), wait a minute after connecting, then try Prepare in natTrak again.';
    document.body.appendChild(d); return true; }}
  [800,2000,4000,7000].forEach(function(t){{ setTimeout(function(){{ if(!denied()) fill(); }},t); }});
}})();"#,
        data = data
    );
    if let Some(old) = app.get_webview_window("nattrak") {
        let _ = old.close();
    }
    let url = tauri::Url::parse("https://nattrak.vatsim.net/controllers/clx/create").map_err(|e| e.to_string())?;
    WebviewWindowBuilder::new(&app, "nattrak", WebviewUrl::External(url))
        .title("natTrak - new clearance (vNAAATS)")
        .inner_size(1000.0, 800.0)
        .initialization_script(&script)
        .on_page_load(|_w, payload| {
            nat_mark(payload.url());
        })
        .build()
        .map(|_| ())
        .map_err(|e| e.to_string())
}

/// Set once a natTrak window has reached natTrak's controller area, which it only does after the controller has signed in.
static NAT_SIGNED_IN: AtomicBool = AtomicBool::new(false);

fn nat_mark(url: &tauri::Url) {
    if url.host_str() == Some("nattrak.vatsim.net") && url.path().starts_with("/controllers") {
        NAT_SIGNED_IN.store(true, Ordering::Relaxed);
    }
}

/// Has the controller signed in to natTrak (in the app's own natTrak window) during this run of the app?
#[tauri::command]
async fn nattrak_signed_in(window: WebviewWindow) -> Result<bool, String> {
    check_origin(&window)?;
    Ok(NAT_SIGNED_IN.load(Ordering::Relaxed))
}

/// Open natTrak in the app's own window. With login=true it opens the natTrak home page for the VATSIM sign-in; once the controller is
/// signed in the page moves on by itself to the pending list. Otherwise it opens the pending list. The window keeps its sign-in.
#[tauri::command]
async fn nattrak_open(window: WebviewWindow, app: tauri::AppHandle, login: bool) -> Result<(), String> {
    check_origin(&window)?;
    let target = if login { "https://nattrak.vatsim.net/" } else { "https://nattrak.vatsim.net/controllers/clx/pending" };
    if let Some(w) = app.get_webview_window("nattrak-site") {
        let url = tauri::Url::parse(target).map_err(|e| e.to_string())?;
        let _ = w.navigate(url);
        let _ = w.show();
        let _ = w.set_focus();
        return Ok(());
    }
    let script = r#"(function(){
  function go(){
    if(location.hostname!=='nattrak.vatsim.net') return;
    var p=location.pathname.replace(/\/+$/,'')||'/';
    if((p==='/'||p==='/home')&&document.querySelector('a[href*="deauthenticate"],form[action*="deauthenticate"]')){ location.href='/controllers/clx/pending'; }
  }
  [800,2000,4000].forEach(function(t){ setTimeout(go,t); });
})();"#;
    let url = tauri::Url::parse(target).map_err(|e| e.to_string())?;
    WebviewWindowBuilder::new(&app, "nattrak-site", WebviewUrl::External(url))
        .title("natTrak (vNAAATS)")
        .inner_size(1100.0, 850.0)
        .initialization_script(script)
        .on_page_load(|w, payload| {
            let _ = w;
            nat_mark(payload.url());
        })
        .build()
        .map(|_| ())
        .map_err(|e| e.to_string())
}

#[derive(Serialize)]
struct AppUpdate {
    version: String,
    notes: String,
}

/// Is there a newer version of this app? (Signed update files from the project's own release; the signature is checked against the key built in.)
#[tauri::command]
async fn app_update_check(window: WebviewWindow, app: tauri::AppHandle) -> Result<Option<AppUpdate>, String> {
    check_origin(&window)?;
    use tauri_plugin_updater::UpdaterExt;
    let up = app.updater().map_err(|e| e.to_string())?.check().await.map_err(|e| e.to_string())?;
    Ok(up.map(|u| AppUpdate { version: u.version.clone(), notes: u.body.clone().unwrap_or_default() }))
}

/// Download and install the newer version, then start the app again. Only runs when the controller pressed Update.
#[tauri::command]
async fn app_update_install(window: WebviewWindow, app: tauri::AppHandle) -> Result<(), String> {
    check_origin(&window)?;
    use tauri_plugin_updater::UpdaterExt;
    let up = app.updater().map_err(|e| e.to_string())?.check().await.map_err(|e| e.to_string())?.ok_or("there is no newer version")?;
    up.download_and_install(|_, _| {}, || {}).await.map_err(|e| e.to_string())?;
    app.restart();
}

/// Open one of our own pages (a part of the display) in a separate window that can be dragged to another screen.
#[tauri::command]
async fn open_popout(window: WebviewWindow, app: tauri::AppHandle, which: String, width: f64, height: f64) -> Result<(), String> {
    check_origin(&window)?;
    if !["cpdlc", "audio"].contains(&which.as_str()) {
        return Err("unknown window".into());
    }
    let label = format!("pop-{}", which);
    if let Some(w) = app.get_webview_window(&label) {
        let _ = w.set_focus();
        return Ok(());
    }
    let url = tauri::Url::parse(&format!("{}/fdd/popout?w={}", SITE, which)).map_err(|e| e.to_string())?;
    WebviewWindowBuilder::new(&app, &label, WebviewUrl::External(url))
        .title("vNAAATS")
        .inner_size(width.clamp(300.0, 1600.0), height.clamp(300.0, 1400.0))
        .build()
        .map(|_| ())
        .map_err(|e| e.to_string())
}

// The saved sign-in lives in the Windows Credential Manager (where Windows keeps its own saved passwords), never in a plain file.
#[derive(Serialize, Deserialize)]
struct Login {
    name: String,
    password: String,
}

fn cred_entry() -> Result<keyring::Entry, String> {
    keyring::Entry::new("vNAAATS", "flight-data-display").map_err(|e| e.to_string())
}

#[tauri::command]
async fn cred_get(window: WebviewWindow) -> Result<Option<Login>, String> {
    check_origin(&window)?;
    match cred_entry()?.get_password() {
        Ok(text) => Ok(serde_json::from_str(&text).ok()),
        Err(keyring::Error::NoEntry) => Ok(None),
        Err(e) => Err(e.to_string()),
    }
}

#[tauri::command]
async fn cred_save(window: WebviewWindow, name: String, password: String) -> Result<(), String> {
    check_origin(&window)?;
    if name.trim().is_empty() || password.is_empty() || name.len() > 100 || password.len() > 200 {
        return Err("nothing to save".into());
    }
    let text = serde_json::to_string(&Login { name: name.trim().to_string(), password }).map_err(|e| e.to_string())?;
    cred_entry()?.set_password(&text).map_err(|e| e.to_string())
}

#[tauri::command]
async fn cred_forget(window: WebviewWindow) -> Result<(), String> {
    check_origin(&window)?;
    match cred_entry()?.delete_credential() {
        Ok(()) | Err(keyring::Error::NoEntry) => Ok(()),
        Err(e) => Err(e.to_string()),
    }
}

fn main() {
    tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_updater::Builder::new().build())
        .invoke_handler(tauri::generate_handler![status, pick_folder, update_plugin, save_key, pick_exe, launch, open_url, cred_get, cred_save, cred_forget, open_nattrak, nattrak_open, nattrak_signed_in, open_popout, app_update_check, app_update_install])
        .run(tauri::generate_context!())
        .expect("the vNAAATS app could not start");
}
