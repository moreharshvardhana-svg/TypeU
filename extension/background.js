importScripts('typeu.js');

let typeu = null;

async function initEngine() {
  try {
    typeu = await createTypeU();
    console.log("[TypeU] WebAssembly engine initialized in Service Worker!");

    const stored = await chrome.storage.local.get("brain_binary");
    if (stored && stored.brain_binary) {
      typeu.importBinaryState(stored.brain_binary);
    } else {
      typeu.initDefaults();
      persistState();
    }
  } catch (err) {
    console.error("[TypeU WASM Init Error]", err);
  }
}

function persistState() {
  if (!typeu) return;
  const binaryState = typeu.exportBinaryState();
  chrome.storage.local.set({ "brain_binary": binaryState });
}

initEngine();

chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
  if (!typeu) {
    sendResponse({ success: false, error: "Engine initializing" });
    return false;
  }

  if (request.type === "predict") {
    const vec = typeu.predictNext(request.query, 3);
    const results = [];
    for (let i = 0; i < vec.size(); i++) {
      results.push(vec.get(i));
    }
    vec.delete();
    sendResponse({ success: true, data: results });
    return true;
  }

  if (request.type === "train") {
    typeu.trainModel(request.query);
    persistState();
    sendResponse({ success: true });
    return true;
  }
});