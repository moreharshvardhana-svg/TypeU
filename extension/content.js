let pill = null;
let currentSuggestions = [];
let activeTarget = null;
let isTabHeld = false;
let tabNumberSelected = false;

function isContextValid() {
  return typeof chrome !== "undefined" && chrome.runtime && !!chrome.runtime.id;
}

function createPill() {
  if (pill) return pill;
  pill = document.createElement("div");
  pill.id = "typeu-floating-pill";
  pill.style.display = "none";
  document.body.appendChild(pill);
  return pill;
}

function getCaretCoordinates(element) {
  const selection = window.getSelection();
  if (selection && selection.rangeCount > 0) {
    const range = selection.getRangeAt(0).cloneRange();
    range.collapse(true);
    const rect = range.getBoundingClientRect();
    if (rect.top !== 0 || rect.left !== 0) {
      return {
        x: window.scrollX + rect.left,
        y: window.scrollY + rect.top
      };
    }
  }

  const elemRect = element.getBoundingClientRect();
  return {
    x: window.scrollX + elemRect.left + 15,
    y: window.scrollY + elemRect.top
  };
}

function updatePillPosition(element) {
  if (!element) return;
  const coords = getCaretCoordinates(element);
  const p = createPill();
  p.style.position = "absolute";
  p.style.left = `${coords.x}px`;
  p.style.top = `${coords.y - 38}px`;
}

function hidePill() {
  if (pill) pill.style.display = "none";
  currentSuggestions = [];
  isTabHeld = false;
  tabNumberSelected = false;
}

function extractContext(target) {
  if (!target) return { text: "", full: "" };

  if ("value" in target && typeof target.value === "string") {
    const cursorPos = target.selectionStart ?? target.value.length;
    return {
      text: target.value.slice(Math.max(0, cursorPos - 20), cursorPos),
      full: target.value
    };
  }

  const selection = window.getSelection();
  if (selection && selection.anchorNode) {
    const nodeText = selection.anchorNode.textContent || "";
    const offset = selection.anchorOffset;
    return {
      text: nodeText.slice(Math.max(0, offset - 20), offset),
      full: nodeText
    };
  }

  return { text: "", full: "" };
}

function insertSuggestion(target, text) {
  if (!target || !text) return;

  const insertString = text + " ";

  // for normal text input area
  if ("value" in target && typeof target.value === "string") {
    const start = target.selectionStart ?? target.value.length;
    const end = target.selectionEnd ?? target.value.length;
    const val = target.value;

    target.value = val.slice(0, start) + insertString + val.slice(end);
    target.selectionStart = target.selectionEnd = start + insertString.length;
    target.dispatchEvent(new Event("input", { bubbles: true }));
  } 
  // rich text / contenteditable
  else {
    target.focus();
    const inputEvent = new InputEvent("beforeinput", {
      bubbles: true,
      cancelable: true,
      inputType: "insertText",
      data: insertString
    });

    const notHandled = target.dispatchEvent(inputEvent);
    if (notHandled) {
      document.execCommand("insertText", false, insertString);
    }

    const selection = window.getSelection();
    if (selection && selection.rangeCount > 0) {
      const range = selection.getRangeAt(0);
      range.collapse(false);
      selection.removeAllRanges();
      selection.addRange(range);
    }
  }

  // Train on newly inserted text
  const { full: fullText } = extractContext(target);
  if (isContextValid() && fullText) {
    try {
      chrome.runtime.sendMessage({ type: "train", query: fullText });
    } catch (_) {}
  }

  // Immediately get next words
  setTimeout(() => {
    processTyping(target);
  }, 25);
}

function processTyping(target) {
  if (!isContextValid() || !target) return;
  activeTarget = target;

  const { text: context, full: fullText } = extractContext(target);

  if (fullText.endsWith(" ")) {
    try {
      chrome.runtime.sendMessage({ type: "train", query: fullText });
    } catch (_) {}
  }

  if (context.trim().length === 0) {
    hidePill();
    return;
  }

  try {
    chrome.runtime.sendMessage({ type: "predict", query: context }, (response) => {
      if (chrome.runtime.lastError || !response) {
        hidePill();
        return;
      }

      if (response.success && Array.isArray(response.data) && response.data.length > 0) {
        currentSuggestions = response.data;
        const p = createPill();
        p.innerHTML = currentSuggestions
          .map((w, i) => `<span>[${i + 1}] <span class="typeu-candidate">${w}</span></span>`)
          .join("");
        updatePillPosition(target);
        p.style.display = "flex";
      } else {
        hidePill();
      }
    });
  } catch (_) {
    hidePill();
  }
}
document.addEventListener("input", (e) => processTyping(e.target), true);
document.addEventListener("keyup", (e) => {
  if (e.key === "Tab") {
    
    if (isTabHeld && !tabNumberSelected && currentSuggestions.length > 0 && activeTarget) {
      insertSuggestion(activeTarget, currentSuggestions[0]);
    }
    isTabHeld = false;
    tabNumberSelected = false;
    return;
  }

  if (["ArrowLeft", "ArrowRight", "ArrowUp", "ArrowDown", "Backspace"].includes(e.key)) {
    processTyping(e.target);
  }
}, true);

//Shortcuts (tab + 1 , 2 , 3)
document.addEventListener("keydown", (e) => {
  if (currentSuggestions.length === 0 || !activeTarget) return;

  if (e.key === "Tab") {
    e.preventDefault();
    e.stopPropagation();
    isTabHeld = true;
    tabNumberSelected = false;
    return;
  }

  // If Tab is held and 1, 2, or 3 is pressed
  if (isTabHeld && ["1", "2", "3"].includes(e.key)) {
    e.preventDefault();
    e.stopPropagation();
    tabNumberSelected = true;
    const index = parseInt(e.key, 10) - 1;
    if (index < currentSuggestions.length) {
      insertSuggestion(activeTarget, currentSuggestions[index]);
    }
    return;
  }

  if (e.key === "Escape") {
    hidePill();
  }
}, true);

document.addEventListener("focusout", () => {
  setTimeout(hidePill, 150);
}, true);