# TypeU

> **A personalized, zero-latency predictive typing engine that learns as you write. Powered by C++ and WebAssembly.**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus)](https://isocpp.org/)
[![WebAssembly](https://img.shields.io/badge/WebAssembly-WASM-654FF0?logo=webassembly)](https://webassembly.org/)
[![Manifest V3](https://img.shields.io/badge/Chrome_Extension-MV3-4285F4?logo=googlechrome)](https://developer.chrome.com/docs/extensions/mv3/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

---

## Why TypeU?

* 🚀 **Speed Up Repetitive Writing:** Dynamically autocompletes recurring words, common phrases, and code snippets.
* 🧠 **Learns Exclusively From You:** Starts with minimal seeding and adapts to your personal tone with an asymptotic online sequence model.
* 📱 **Desktop Parity:** Mobile keyboards shouldn't have all the fun—bring native inline predictions to any webpage (Discord, Google Docs, Reddit, Gemini).
* 🔒 **100% Private & Client-Side:** No telemetry, no API keys, and no servers. Everything runs entirely in your browser via WebAssembly.

---

## ⌨️ Controls & Shortcuts

| Key Combination | Action |
| :--- | :--- |
| `Tab` | Accept the primary prediction **[1]** |
| `Tab` + `1`, `2`, or `3` | Pick a specific candidate from the floating pill |
| `Escape` | Dismiss suggestions |

---

## 🚀 Quick Setup (No Terminal Needed)

1. **Clone or Download** this repository:
   ```bash
   git clone [https://github.com/yourusername/TypeU.git](https://github.com/yourusername/TypeU.git)
   
### 🚀 Getting Started

1. **Open Extensions in Chrome:** Navigate to `chrome://extensions/` (or click `⋮` $\rightarrow$ **Extensions** $\rightarrow$ **Manage Extensions**).
2. **Enable Developer Mode:** Toggle on **Developer mode** in the top-right corner.
3. **Load the Extension:** Click **Load unpacked** and select the `TypeU/extension` directory.
4. **Start Typing:** Open any text area across the web and start writing!

---

### 🛠️ Architecture

* **Core Engine:** Written in modern C++20 featuring a multi-tier N-gram Markov model with asymptotic weight smoothing.
* **WASM Runtime:** Compiled via Emscripten with zero dynamic execution (`DYNAMIC_EXECUTION=0`) for strict Manifest V3 Content Security Policy compliance.
* **Binary Persistence:** Exports continuous model snapshots directly into `chrome.storage.local` without disk-write overhead.

---

### 👨‍💻 Author

Built by **Nightbear**

---

### Why this version stands out

* **Scannability:** Clear step-by-step installation instructions for immediate onboarding.
* **Technical Credibility:** Highlights the C++20, WASM, and privacy-conscious Manifest V3 architecture.
