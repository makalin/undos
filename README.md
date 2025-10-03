# undos 🗑️  
The “Undo” for Any Shell Command

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)  
[![Build](https://img.shields.io/badge/build-passing-brightgreen)](#)  
[![Language: C](https://img.shields.io/badge/language-C-blue)](#)  
[![Helper: Bash](https://img.shields.io/badge/helper-Bash-yellow)](#)

---

## 🚨 Problem
You type `rm -rf` on the wrong folder.  
No trash. No backup. No undo. Just… gone.

---

## 💡 Solution
**undos** is a lightweight `LD_PRELOAD` wrapper that intercepts destructive file operations and moves them into a safe **trash directory** instead of permanently unlinking.  

- Files are recoverable with `undol`.  
- Every destructive command is **logged** for traceability.  
- Optional **fzf integration** lets you quickly browse and restore items.

Think of it as a *recycle bin for your shell*.

---

## ⚙️ Tech Stack
- **C** → tiny preload library (~<100 LOC)  
- **Bash** → helper scripts for restore & logs  
- **fzf** (optional) → fuzzy finder for restoring deleted files  

---

## 🚀 Features
- Transparent `rm`, `unlink`, and friends → moved, not destroyed.  
- Logs what you nuked, when, and where.  
- `undol` command to restore files/folders.  
- Fast + minimal footprint, no daemons.  
- Works across shells (`bash`, `zsh`, etc.).  
- Optional **fzf picker** for interactive recovery.

---

## 🔧 Installation
```bash
git clone https://github.com/makalin/undos.git
cd undos
make
sudo make install
````

Add this to your shell profile (`~/.bashrc`, `~/.zshrc`, etc.):

```bash
export LD_PRELOAD=/usr/local/lib/undos.so
export UNDOS_TRASH=~/.local/share/undos_trash
```

---

## 🗑️ Usage

### Deleting (intercepted automatically)

```bash
rm -rf important-stuff/
# → actually moved to ~/.local/share/undos_trash/ + logged
```

### Restoring

```bash
undol                     # list last deleted items
undol <file>              # restore specific file
undol -i                  # interactive restore via fzf
```

### Viewing Logs

```bash
undol --log
```

---

## 🎁 Bonus

* **fzf support** → Restore with fuzzy search.
* **Configurable trash path** (`$UNDOS_TRASH`).
* **Tiny footprint** → safety without bloat.

---

## ⚠️ Disclaimer

* `undos` is **best-effort**. Don’t rely on it as a full backup solution.
* Always keep real backups for critical data.

---

## 📜 License

MIT – free to use, hack, and improve.

---

## 🌟 Roadmap

* [ ] Support more syscalls (`rename`, `rmdir`)
* [ ] Per-command opt-out / opt-in
* [ ] TUI restore browser

---

## 🤝 Contributing

PRs welcome!

* Fork it
* Create a feature branch
* Submit a pull request

---

## Example Demo

![undos-demo](https://github.com/makalin/undos/raw/main/demo.gif)

---

### Why `undos`?

Because **everyone deserves an undo button** in their shell.
