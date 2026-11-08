//! The file work of the vNAAATS app. Kept apart from the window code so it can be tested on any computer.
use sha2::{Digest, Sha256};
use std::fs;
use std::path::{Path, PathBuf};

pub const DLL_NAME: &str = "VatsimNAAATS.dll";
pub const CFG_NAME: &str = "vnaaats.cfg";
const PLACEHOLDER: &str = "PASTE_YOUR_API_KEY_HERE";
const DEFAULT_SERVER: &str = "https://vnaaats.com";

pub fn sha256_hex(bytes: &[u8]) -> String {
    let mut h = Sha256::new();
    h.update(bytes);
    h.finalize().iter().map(|b| format!("{:02x}", b)).collect()
}

/// Find VatsimNAAATS.dll under `root` (looking up to `max_depth` folders down). Returns the first match, shallowest first.
pub fn find_plugin_dll(root: &Path, max_depth: usize) -> Option<PathBuf> {
    let mut level = vec![root.to_path_buf()];
    for _ in 0..=max_depth {
        let mut next = Vec::new();
        for dir in &level {
            let Ok(rd) = fs::read_dir(dir) else { continue };
            let mut subs = Vec::new();
            for e in rd.flatten() {
                let p = e.path();
                if p.is_dir() {
                    subs.push(p);
                } else if p.file_name().map(|n| n.to_string_lossy().eq_ignore_ascii_case(DLL_NAME)).unwrap_or(false) {
                    return Some(p);
                }
            }
            next.extend(subs);
        }
        if next.is_empty() {
            break;
        }
        level = next;
    }
    None
}

#[derive(Debug, PartialEq, Eq, Clone)]
pub struct CfgInfo {
    pub exists: bool,
    pub has_key: bool,
    pub server: String,
}

pub fn read_cfg(dir: &Path) -> CfgInfo {
    let mut info = CfgInfo { exists: false, has_key: false, server: DEFAULT_SERVER.to_string() };
    let Ok(text) = fs::read_to_string(dir.join(CFG_NAME)) else { return info };
    info.exists = true;
    for line in text.lines() {
        let line = line.trim();
        if line.starts_with('#') {
            continue;
        }
        if let Some((k, v)) = line.split_once('=') {
            let (k, v) = (k.trim(), v.trim());
            if k == "key" && !v.is_empty() && v != PLACEHOLDER {
                info.has_key = true;
            } else if k == "server" && !v.is_empty() {
                info.server = v.to_string();
            }
        }
    }
    info
}

/// A key looks like 1234567.abc... (a personal key) or a long code (the shared key): letters, numbers, dot, dash, underscore.
pub fn valid_key(key: &str) -> bool {
    (8..=120).contains(&key.len()) && key != PLACEHOLDER && key.chars().all(|c| c.is_ascii_alphanumeric() || matches!(c, '.' | '-' | '_'))
}

/// A key as people paste it: with spaces, quotes, or the "key=" label that the vnaaats.cfg line shows. Leaves just the key.
pub fn clean_key(raw: &str) -> String {
    let mut s = raw.trim().trim_matches(|c| c == '"' || c == '\'' || c == '`').trim();
    if s.len() >= 4 && s[..4].eq_ignore_ascii_case("key=") {
        s = s[4..].trim().trim_matches(|c| c == '"' || c == '\'' || c == '`').trim();
    }
    s.to_string()
}

/// Put `key=` into vnaaats.cfg (replacing an existing key line, or adding one; creating the file if it is not there).
/// Every other line is kept as it was, including the line endings.
pub fn set_cfg_key(dir: &Path, key: &str) -> Result<(), String> {
    let key = clean_key(key);
    let key = key.as_str();
    if !valid_key(key) {
        return Err("that does not look like a vNAAATS key (it is long, with letters, numbers and a dot)".into());
    }
    let path = dir.join(CFG_NAME);
    let old = fs::read_to_string(&path).unwrap_or_default();
    let eol = if old.contains("\r\n") || old.is_empty() { "\r\n" } else { "\n" };
    let mut out: Vec<String> = Vec::new();
    let mut done = false;
    for line in old.lines() {
        let t = line.trim_start();
        if !t.starts_with('#') && t.split_once('=').map(|(k, _)| k.trim() == "key").unwrap_or(false) {
            if !done {
                out.push(format!("key={}", key));
                done = true;
            }
        } else {
            out.push(line.to_string());
        }
    }
    if old.is_empty() {
        out.push(format!("server={}", DEFAULT_SERVER));
    }
    if !done {
        out.push(format!("key={}", key));
    }
    let text = out.join(eol) + eol;
    let tmp = dir.join("vnaaats.cfg.tmp");
    fs::write(&tmp, text).map_err(|e| format!("could not write vnaaats.cfg: {}", e))?;
    fs::rename(&tmp, &path).map_err(|e| format!("could not save vnaaats.cfg: {}", e))
}

/// Replace the plugin DLL with `bytes` after checking its SHA-256. Fails with a plain message if EuroScope still has it open.
pub fn replace_dll(dll: &Path, bytes: &[u8], expected_sha: &str) -> Result<(), String> {
    if bytes.len() < 2 || &bytes[..2] != b"MZ" {
        return Err("the download is not a DLL".into());
    }
    if !expected_sha.eq_ignore_ascii_case(&sha256_hex(bytes)) {
        return Err("the download does not match its checksum - try again".into());
    }
    let tmp = dll.with_extension("dll.new");
    fs::write(&tmp, bytes).map_err(|e| format!("could not write the new plugin: {}", e))?;
    if let Err(e) = fs::rename(&tmp, dll) {
        let _ = fs::remove_file(&tmp);
        return Err(format!("could not replace the plugin - close EuroScope and try again ({})", e));
    }
    Ok(())
}

/// Replace a plain data file (such as ICAO_Airlines.txt) after checking its SHA-256.
pub fn replace_file(dest: &Path, bytes: &[u8], expected_sha: &str) -> Result<(), String> {
    if bytes.is_empty() {
        return Err("the download is empty".into());
    }
    if !expected_sha.eq_ignore_ascii_case(&sha256_hex(bytes)) {
        return Err("the download does not match its checksum - try again".into());
    }
    let tmp = dest.with_extension("txt.new");
    fs::write(&tmp, bytes).map_err(|e| format!("could not write {}: {}", dest.display(), e))?;
    if let Err(e) = fs::rename(&tmp, dest) {
        let _ = fs::remove_file(&tmp);
        return Err(format!("could not replace {}: {}", dest.display(), e));
    }
    Ok(())
}

pub fn is_exe(path: &Path) -> bool {
    path.is_file() && path.extension().map(|e| e.to_string_lossy().eq_ignore_ascii_case("exe")).unwrap_or(false)
}

/// Bring vnaaats.cfg up to date with the template on the server: the comments and the list of options are replaced by the template's,
/// every value the controller already has (key, server, switches, settingsstamp ...) is kept, options the file does not have are
/// left out (the plugin's own defaults apply, so behaviour does not change), and lines the template does not know are kept at the end.
/// Returns true when the file was rewritten. A file that is not there is left alone (the key step creates it).
pub fn refresh_cfg(dir: &Path, template: &str) -> Result<bool, String> {
    let path = dir.join(CFG_NAME);
    let Ok(old) = fs::read_to_string(&path) else { return Ok(false) };
    let template = template.trim_start_matches('\u{feff}');
    if template.len() > 20000 || !template.starts_with('#') || !template.lines().any(|l| l.starts_with("server=")) {
        return Err("the server did not send a valid vnaaats.example.cfg".into());
    }
    let active = |l: &str| -> Option<(String, String)> {
        let t = l.trim();
        if t.is_empty() || t.starts_with('#') { return None; }
        t.split_once('=').map(|(k, v)| (k.trim().to_string(), v.trim().to_string()))
    };
    let mut have: Vec<(String, String)> = Vec::new();
    for l in old.lines() {
        if let Some((k, v)) = active(l) {
            if !have.iter().any(|(hk, _)| *hk == k) { have.push((k, v)); }
        }
    }
    let eol = if old.contains("\r\n") || old.is_empty() { "\r\n" } else { "\n" };
    let mut out: Vec<String> = Vec::new();
    let mut used: Vec<String> = Vec::new();
    for l in template.lines() {
        match active(l) {
            None => out.push(l.trim_end().to_string()),
            Some((k, _)) => {
                if let Some((_, v)) = have.iter().find(|(hk, _)| *hk == k) {
                    out.push(format!("{}={}", k, v));
                    used.push(k);
                }
            }
        }
    }
    let extras: Vec<&(String, String)> = have.iter().filter(|(k, _)| !used.contains(k)).collect();
    if !extras.is_empty() {
        out.push(String::new());
        for (k, v) in extras { out.push(format!("{}={}", k, v)); }
    }
    while out.last().map(|l| l.is_empty()).unwrap_or(false) { out.pop(); }
    let text = out.join(eol) + eol;
    if text == old { return Ok(false); }
    let tmp = dir.join("vnaaats.cfg.tmp");
    fs::write(&tmp, text).map_err(|e| format!("could not write vnaaats.cfg: {}", e))?;
    fs::rename(&tmp, &path).map_err(|e| format!("could not save vnaaats.cfg: {}", e))?;
    Ok(true)
}

/// Does `tasklist /FO CSV /NH` output list a process called `exe_name` (case-insensitive)? Lines look like "EuroScope.exe","1234",...
pub fn tasklist_has(output: &str, exe_name: &str) -> bool {
    output.lines().any(|l| {
        let first = l.trim().trim_start_matches('"').split('"').next().unwrap_or("");
        first.eq_ignore_ascii_case(exe_name)
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn refresh_cfg_keeps_values_and_updates_comments() {
        let d = tempfile::tempdir().unwrap();
        let tpl = "# new header\nserver=https://vnaaats.com\nkey=PASTE\n\n# autotype help\nautotype=1\n# highlight help\nhighlight=1\n";
        assert_eq!(refresh_cfg(d.path(), tpl), Ok(false));          // no file: left alone
        fs::write(d.path().join(CFG_NAME), "# old header\r\nserver=https://x.example\r\nkey=1234567.abc\r\nautotype=0\r\nsquawk=0\r\nsettingsstamp=5\r\n").unwrap();
        assert_eq!(refresh_cfg(d.path(), tpl), Ok(true));
        let t = fs::read_to_string(d.path().join(CFG_NAME)).unwrap();
        assert_eq!(t, "# new header\r\nserver=https://x.example\r\nkey=1234567.abc\r\n\r\n# autotype help\r\nautotype=0\r\n# highlight help\r\n\r\nsquawk=0\r\nsettingsstamp=5\r\n");
        assert!(!t.contains("highlight=") && !t.contains("PASTE"));  // not in their file: not added
        assert_eq!(refresh_cfg(d.path(), tpl), Ok(false));          // second time: nothing to do
        assert!(refresh_cfg(d.path(), "<html>nope").is_err());
    }

    #[test]
    fn finds_running_process() {
        let out = "\"System\",\"4\",\"Services\",\"0\",\"1 K\"\r\n\"EuroScope.exe\",\"4242\",\"Console\",\"1\",\"90,000 K\"\r\n";
        assert!(tasklist_has(out, "euroscope.exe"));
        assert!(!tasklist_has(out, "vacs.exe"));
        assert!(!tasklist_has("INFO: No tasks are running which match the specified criteria.", "vacs.exe"));
        assert!(!tasklist_has(out, "Euro"));
    }

    #[test]
    fn finds_dll_in_nested_folder() {
        let d = tempfile::tempdir().unwrap();
        let p = d.path().join("EXOCO/Plugins/vNAAATS");
        fs::create_dir_all(&p).unwrap();
        fs::write(p.join("vatsimnaaats.dll"), b"MZ").unwrap();           // upper/lower case does not matter
        assert_eq!(find_plugin_dll(d.path(), 5).unwrap(), p.join("vatsimnaaats.dll"));
        assert!(find_plugin_dll(d.path(), 1).is_none());                 // too deep for that limit
    }

    #[test]
    fn cfg_key_detection() {
        let d = tempfile::tempdir().unwrap();
        assert!(!read_cfg(d.path()).exists);
        fs::write(d.path().join(CFG_NAME), "server=https://x.example\nkey=PASTE_YOUR_API_KEY_HERE\n").unwrap();
        let i = read_cfg(d.path());
        assert!(i.exists && !i.has_key);
        assert_eq!(i.server, "https://x.example");
        fs::write(d.path().join(CFG_NAME), "# key=abc\nkey=1234567.abcdefghij\n").unwrap();
        assert!(read_cfg(d.path()).has_key);
    }

    #[test]
    fn set_key_keeps_other_lines_and_endings() {
        let d = tempfile::tempdir().unwrap();
        fs::write(d.path().join(CFG_NAME), "server=https://vnaaats.com\r\nkey=PASTE_YOUR_API_KEY_HERE\r\n# autotype=1 ok\r\nautotype=1\r\n").unwrap();
        set_cfg_key(d.path(), " 1234567.abcdefghijkl ").unwrap();
        let t = fs::read_to_string(d.path().join(CFG_NAME)).unwrap();
        assert_eq!(t, "server=https://vnaaats.com\r\nkey=1234567.abcdefghijkl\r\n# autotype=1 ok\r\nautotype=1\r\n");
    }

    #[test]
    fn set_key_creates_or_appends() {
        let d = tempfile::tempdir().unwrap();
        set_cfg_key(d.path(), "1234567.abcdefghijkl").unwrap();
        let i = read_cfg(d.path());
        assert!(i.exists && i.has_key && i.server == "https://vnaaats.com");
        let d2 = tempfile::tempdir().unwrap();
        fs::write(d2.path().join(CFG_NAME), "autotype=1\n").unwrap();
        set_cfg_key(d2.path(), "1234567.abcdefghijkl").unwrap();
        assert_eq!(fs::read_to_string(d2.path().join(CFG_NAME)).unwrap(), "autotype=1\nkey=1234567.abcdefghijkl\n");
    }

    #[test]
    fn a_pasted_key_label_is_dropped() {
        let d = tempfile::tempdir().unwrap();
        for pasted in ["key=1234567.abcdefghijkl", "KEY=1234567.abcdefghijkl", " \"key=1234567.abcdefghijkl\" ", "1234567.abcdefghijkl"] {
            set_cfg_key(d.path(), pasted).unwrap();
            let t = fs::read_to_string(d.path().join(CFG_NAME)).unwrap();
            let keys: Vec<&str> = t.lines().filter(|l| l.starts_with("key=")).collect();
            assert_eq!(keys, vec!["key=1234567.abcdefghijkl"], "{}", pasted);
        }
    }

    #[test]
    fn rejects_bad_keys() {
        let d = tempfile::tempdir().unwrap();
        for bad in ["", "short", "has space in it ok", "PASTE_YOUR_API_KEY_HERE", "a=b\nserver=evil"] {
            assert!(set_cfg_key(d.path(), bad).is_err(), "{}", bad);
        }
        assert!(!d.path().join(CFG_NAME).exists());
    }

    #[test]
    fn replace_file_checks_sha() {
        let d = tempfile::tempdir().unwrap();
        let f = d.path().join("ICAO_Airlines.txt");
        fs::write(&f, b"old").unwrap();
        assert!(replace_file(&f, b"new list", "deadbeef").is_err());
        assert!(replace_file(&f, b"", &sha256_hex(b"")).is_err());
        assert_eq!(fs::read(&f).unwrap(), b"old");
        replace_file(&f, b"new list", &sha256_hex(b"new list")).unwrap();
        assert_eq!(fs::read(&f).unwrap(), b"new list");
        assert!(!d.path().join("ICAO_Airlines.txt.new").exists());
    }

    #[test]
    fn replace_dll_checks_sha_and_header() {
        let d = tempfile::tempdir().unwrap();
        let dll = d.path().join(DLL_NAME);
        fs::write(&dll, b"MZold").unwrap();
        let new = b"MZnew-plugin".to_vec();
        assert!(replace_dll(&dll, &new, "deadbeef").is_err());
        assert!(replace_dll(&dll, b"<html>", &sha256_hex(b"<html>")).is_err());
        assert_eq!(fs::read(&dll).unwrap(), b"MZold");                  // untouched on failure
        replace_dll(&dll, &new, &sha256_hex(&new).to_uppercase()).unwrap();
        assert_eq!(fs::read(&dll).unwrap(), new);
        assert!(!d.path().join("VatsimNAAATS.dll.new").exists());
    }
}
