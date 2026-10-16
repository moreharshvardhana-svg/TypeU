const element = document.getElementById("button_ball");
const toggleButton = document.getElementById("toggle_typeU");
const state = document.getElementById("status");
const clHistory = document.getElementById("clearHistory");
const deletedat=document.getElementById("deletedat");
const deleteword=document.getElementById("deleteword");
const tutoggle= document.getElementById("toggle_typeU");
const modeBtn = document.getElementById("mode");
const mainbody = document.getElementById("mainbody");
const bin=["Off","On"];

const colorToggle = {
  on: "#00eace",
  off: "#ea200a"
};
let isTrackingOn = true;
let hover=false;
function updateUI(active) {
  state.textContent = bin[active*1];
  element.style.transition = "transform 0.2s ease-in, background-color 0.2s ease-out";
  element.style.backgroundColor = active ? colorToggle.on : colorToggle.off;
element.style.transform = active ? "translateX(30px)" : "translateX(0px)";
element.classList.toggle("is-squished", active);
  if(hover)tutoggle.style.boxShadow=`0px 0px 25px ${element.style.backgroundColor}`;
}

toggleButton.addEventListener('mouseenter', () => {
  tutoggle.style.boxShadow=`0px 0px 25px ${element.style.backgroundColor}`;
  hover=true;
});

toggleButton.addEventListener('mouseleave', () => {
 tutoggle.style.boxShadow=`0px 0px 0px`;
 hover=false;
 });

chrome.storage.local.get("isEnabled", (res) => {
  isTrackingOn = res.isEnabled !== undefined ? res.isEnabled : true;
  updateUI(isTrackingOn);
});

toggleButton.onclick = function() {
  isTrackingOn = !isTrackingOn;
  updateUI(isTrackingOn);
  chrome.storage.local.set({ isEnabled: isTrackingOn });
};

deletedat.onclick = function(){
if(deleteword.value==''){
  document.getElementById("hide-confirmation").style.display = "block";
  deleteword.style.display = "block";
  deleteword.classList.toggle('show');
  }
  else{
  const word = deleteword.value.trim();
  if (!word){ 
      deletedat.textContent = "Type Something";
      
    setTimeout(() => {
      deletedat.textContent = "Remove Word";
    }, 1200);
  return;}

  chrome.runtime.sendMessage({ type: "removeword", query: word }, (res) => {
    if (res && res.success) {
      deletedat.textContent = "Word Removed!";
      deleteword.value = '';
    } else {
      deletedat.textContent = "Failed!";
      deleteword.value = '';
    }

    setTimeout(() => {
      deletedat.textContent = "Remove Word";
    }, 1200);
  });
  }
};

clHistory.onclick = function(){
if(document.getElementById("deleteconfirmation").checked){
 clHistory.style.backgroundColor = "#99ff2b";
  clHistory.textContent = "Clearing...";
  chrome.runtime.sendMessage({ type: "ClearHistory" }, (res) => {
    if (res && res.success) {
      clHistory.textContent = "Cleared!";
    }
  });
  }
  else{
    clHistory.style.backgroundColor = "#ff9966";
    clHistory.textContent = "Confirm First";
    if (document.getElementById("hidecheckbox").style.display === 'none') {
    document.getElementById("hidecheckbox").style.display='block';
  } else {
    document.getElementById("hidecheckbox").style.display = 'none';
  }
  }
};


let isLight = true;
function renderThemeUI(light) {
  isLight = light;
  mainbody.style.transition = "background-image 1s ease-out,color 0.2s ease-out";
  //modeBtn.style.transition = "background-color 0.5s ease-out,color 0.2s ease-out";
    //modeBtn.style.boxShadow=`0px 0px 25px ${mainbody.style.backgroundColor}`;
  if (isLight) {
    mainbody.style.backgroundColor ="#c5d1ce";//lightcolor
    mainbody.style.backgroundImage="url('menu-light.png')";
    mainbody.style.setProperty("color", "#000000");
    modeBtn.textContent = "Dark Mode";
    modeBtn.style.backgroundColor = "#1e1e2e";
    modeBtn.style.setProperty("color", "#ffffff");
    document.body.classList.remove("dark-popup");
  } else {
    mainbody.style.backgroundColor ="#1e1e2e";//darkcolor
    mainbody.style.backgroundImage="url('menu-dark.png')";
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


let themehover=false;
modeBtn.onclick = function() {
  const newTheme = isLight ? "dark" : "light";
  renderThemeUI(newTheme === "light");
  chrome.storage.local.set({ typeu_theme: newTheme });
if(themehover)modeBtn.style.boxShadow=`0px 0px 25px ${isLight ? "black" : "white"}`;
};

modeBtn.addEventListener('mouseenter', () => {
  modeBtn.style.boxShadow=`0px 0px 25px ${isLight ? "black" : "white"}`;
  themehover=true;
});

modeBtn.addEventListener('mouseleave', () => {
 modeBtn.style.boxShadow=`0px 0px 0px`;
 themehover=false;
 });
