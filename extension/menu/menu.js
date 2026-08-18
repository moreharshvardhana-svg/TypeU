const element = document.getElementById("button_ball");
const toggleButton = document.getElementById("toggle_typeU");
const state = document.getElementById("status");
const clHistory = document.getElementById("clearHistory");
const bin=["Off","On"];

const colorToggle = {
  on: "#00eace",
  off: "#ea200a"
};
let isTrackingOn = true;
function updateUI(active) {
  state.textContent = bin[active*1];
  element.style.transition = "transform 0.2s ease-out, background-color 0.2s ease-out";
  element.style.backgroundColor = active ? colorToggle.on : colorToggle.off;
  element.style.transform = active ? "translateX(30px)" : "translateX(0px)";
}

chrome.storage.local.get("isEnabled", (res) => {
  isTrackingOn = res.isEnabled !== undefined ? res.isEnabled : true;
  updateUI(isTrackingOn);
});

toggleButton.onclick = function() {
  isTrackingOn = !isTrackingOn;
  updateUI(isTrackingOn);
  chrome.storage.local.set({ isEnabled: isTrackingOn });
};

const delconfirm = document.getElementById("deleteConfirmation");

clHistory.onclick = function() {
if(delconfirm.value.toLowerCase()=="clear"){
  clHistory.style.backgroundColor = "#eeaacc";
  clHistory.textContent = "Clearing...";
  chrome.runtime.sendMessage({ type: "ClearHistory" }, (res) => {
    if (res && res.success) {
      clHistory.textContent = "Cleared!";
      setTimeout(() => {
        clHistory.textContent = "Clear History";
        clHistory.style.backgroundColor = "";
      }, 1200);
    }
  });
  }
  else{
  document.getElementById("hide-confirmation").style.display = "block";
  delconfirm.style.display = "block";
  
  delconfirm.classList.toggle('show');
  
   clHistory.textContent = "Confirm First";
      setTimeout(() => {
        clHistory.textContent = "Clear History";
        clHistory.style.backgroundColor = "";
      }, 1200);
  }
};
const modeBtn = document.getElementById("mode");
const mainbody = document.getElementById("mainbody");

let isLight = true;

function renderThemeUI(light) {
  isLight = light;
  mainbody.style.transition = "background-color 0.5s ease-out,color 0.2s ease-out";
  modeBtn.style.transition = "background-color 0.5s ease-out,color 0.2s ease-out";
  if (isLight) {
    mainbody.style.backgroundColor ="#c5d1ce";//lightcolor
    mainbody.style.setProperty("color", "#000000");
    modeBtn.textContent = "Dark Mode";
    modeBtn.style.backgroundColor = "#1e1e2e";
    modeBtn.style.setProperty("color", "#ffffff");
    document.body.classList.remove("dark-popup");
  } else {
    mainbody.style.backgroundColor ="#1e1e2e";//darkcolor
    mainbody.style.setProperty("color", "#ffffff");
    modeBtn.textContent = "Light Mode";
    modeBtn.style.backgroundColor = "#c5d1ce";
    modeBtn.style.setProperty("color", "#000000");
    document.body.classList.add("dark-popup");
  }
}
chrome.storage.local.get("typeu_theme", (res) => {
  const currentTheme = res.typeu_theme || "light";
  renderThemeUI(currentTheme === "light");
});
modeBtn.onclick = function() {
  const newTheme = isLight ? "dark" : "light";
  renderThemeUI(newTheme === "light");
  chrome.storage.local.set({ typeu_theme: newTheme });
};
