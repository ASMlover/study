const MODE_THEMES = {
  h75na_hotfix: {
    accent: "#7dff6b",
    accent2: "#29d9c2",
    bg1: "#0b1f18",
    bg2: "#0b2736",
  },
  h75na_server_hotfix: {
    accent: "#ffb86b",
    accent2: "#ff6bcb",
    bg1: "#24120a",
    bg2: "#30112a",
  },
  h75_hotfix: {
    accent: "#57f2f6",
    accent2: "#7c5cff",
    bg1: "#0b0f1f",
    bg2: "#0f1b33",
  },
  h75_server_hotfix: {
    accent: "#6f9bff",
    accent2: "#4bffc2",
    bg1: "#101a33",
    bg2: "#0b2130",
  },
};

const state = {
  modes: [],
  activeMode: null,
  deleteAllowed: false,
};

const elements = {
  tabs: document.getElementById("mode-tabs"),
  grid: document.getElementById("file-grid"),
  template: document.getElementById("file-card-template"),
  summary: document.getElementById("mode-summary"),
};

function setTheme(modeKey) {
  const theme = MODE_THEMES[modeKey];
  if (!theme) {
    return;
  }
  const root = document.documentElement;
  root.style.setProperty("--accent", theme.accent);
  root.style.setProperty("--accent-2", theme.accent2);
  root.style.setProperty("--bg-1", theme.bg1);
  root.style.setProperty("--bg-2", theme.bg2);
}

function formatSize(bytes) {
  if (bytes < 1024) {
    return `${bytes} B`;
  }
  const units = ["KB", "MB", "GB"];
  let value = bytes / 1024;
  let unitIndex = 0;
  while (value >= 1024 && unitIndex < units.length - 1) {
    value /= 1024;
    unitIndex += 1;
  }
  return `${value.toFixed(1)} ${units[unitIndex]}`;
}

function formatTime(isoString) {
  const date = new Date(isoString);
  return date.toLocaleString();
}

function renderTabs() {
  elements.tabs.innerHTML = "";
  state.modes.forEach((mode) => {
    const button = document.createElement("button");
    button.className = "tab";
    if (mode.key === state.activeMode) {
      button.classList.add("active");
    }
    const title = document.createElement("span");
    title.textContent = mode.title;
    const meta = document.createElement("span");
    meta.className = "tab-meta";
    meta.textContent = `${mode.total_count} · ${formatSize(mode.total_size)}`;
    button.appendChild(title);
    button.appendChild(meta);
    button.addEventListener("click", () => {
      state.activeMode = mode.key;
      setTheme(mode.key);
      renderTabs();
      refresh();
    });
    elements.tabs.appendChild(button);
  });
}

function renderFiles() {
  elements.grid.innerHTML = "";
  const mode = state.modes.find((entry) => entry.key === state.activeMode);
  if (mode) {
    elements.summary.textContent = `Files: ${mode.total_count} · Total Size: ${formatSize(
      mode.total_size
    )}`;
  } else {
    elements.summary.textContent = "";
  }
  if (!mode || mode.files.length === 0) {
    const empty = document.createElement("div");
    empty.className = "empty-state";
    empty.textContent = "No files available for this mode.";
    elements.grid.appendChild(empty);
    return;
  }
  mode.files
    .filter((file) => !file.mode || file.mode === mode.key)
    .forEach((file) => {
    const node = elements.template.content.cloneNode(true);
    node.querySelector(".file-name").textContent = file.name;
    node.querySelector(".file-meta").textContent = `${formatTime(
      file.uploaded_at
    )} · ${formatSize(file.size)}`;

    const download = node.querySelector(".action-btn.download");
    download.addEventListener("click", () => {
      window.location.href = `/files/${encodeURIComponent(file.name)}`;
    });

    const deleteBtn = node.querySelector(".action-btn.delete");
    if (!state.deleteAllowed) {
      deleteBtn.style.display = "none";
    } else {
      deleteBtn.addEventListener("click", async () => {
        if (!confirm(`Delete ${file.name}?`)) {
          return;
        }
        await fetch(`/files/${encodeURIComponent(file.name)}`, {
          method: "DELETE",
        });
        await refresh();
      });
    }
    elements.grid.appendChild(node);
  });
}

async function refresh() {
  const response = await fetch("/api/files");
  const data = await response.json();
  state.modes = data.modes || [];
  state.deleteAllowed = Boolean(data.delete_allowed);
  if (!state.activeMode && state.modes.length > 0) {
    state.activeMode = state.modes[0].key;
    setTheme(state.activeMode);
  }
  if (!state.modes.find((mode) => mode.key === state.activeMode)) {
    state.activeMode = state.modes.length > 0 ? state.modes[0].key : null;
    if (state.activeMode) {
      setTheme(state.activeMode);
    }
  }
  renderTabs();
  renderFiles();
}

function updateClock() {
  const now = new Date();
  const hh = String(now.getHours()).padStart(2, "0");
  const mm = String(now.getMinutes()).padStart(2, "0");
  const ss = String(now.getSeconds()).padStart(2, "0");
  document.getElementById("clock-digital").textContent = `${hh}:${mm}:${ss}`;
  document.getElementById("clock-date").textContent = now.toLocaleDateString();

  const hourAngle = ((now.getHours() % 12) + now.getMinutes() / 60) * 30 - 90;
  const minuteAngle = (now.getMinutes() + now.getSeconds() / 60) * 6 - 90;
  const secondAngle = now.getSeconds() * 6 - 90;
  document.getElementById("hand-hour").style.transform = `rotate(${hourAngle}deg)`;
  document.getElementById(
    "hand-minute"
  ).style.transform = `rotate(${minuteAngle}deg)`;
  document.getElementById(
    "hand-second"
  ).style.transform = `rotate(${secondAngle}deg)`;
}

updateClock();
setInterval(updateClock, 1000);
refresh();

const rain = {
  canvas: document.getElementById("rain-canvas"),
  ctx: null,
  drops: [],
  intensity: 0.0,
  running: false,
};

function resizeRainCanvas() {
  if (!rain.canvas) {
    return;
  }
  const ratio = window.devicePixelRatio || 1;
  rain.canvas.width = window.innerWidth * ratio;
  rain.canvas.height = window.innerHeight * ratio;
  rain.ctx = rain.canvas.getContext("2d");
  rain.ctx.scale(ratio, ratio);
}

function initDrops(count) {
  rain.drops = Array.from({ length: count }, () => ({
    x: Math.random() * window.innerWidth,
    y: Math.random() * window.innerHeight,
    speed: 4 + Math.random() * 6,
    length: 8 + Math.random() * 12,
  }));
}

function renderRain() {
  if (!rain.ctx) {
    return;
  }
  rain.ctx.clearRect(0, 0, window.innerWidth, window.innerHeight);
  if (rain.intensity <= 0.01) {
    return;
  }
  rain.ctx.strokeStyle = "rgba(180, 220, 255, 0.4)";
  rain.ctx.lineWidth = 1;
  for (const drop of rain.drops) {
    rain.ctx.beginPath();
    rain.ctx.moveTo(drop.x, drop.y);
    rain.ctx.lineTo(drop.x + 2, drop.y + drop.length);
    rain.ctx.stroke();
    drop.y += drop.speed * (0.3 + rain.intensity);
    if (drop.y > window.innerHeight) {
      drop.y = -drop.length;
      drop.x = Math.random() * window.innerWidth;
    }
  }
}

function tickRain() {
  if (!rain.running) {
    return;
  }
  renderRain();
  requestAnimationFrame(tickRain);
}

function setFogIntensity(value) {
  const fog = document.querySelector(".fog");
  if (!fog) {
    return;
  }
  fog.style.opacity = `${Math.min(0.6, Math.max(0.0, value))}`;
}

async function fetchWeather(lat, lon) {
  const url =
    `https://api.open-meteo.com/v1/forecast?latitude=${lat}` +
    `&longitude=${lon}&current=precipitation,rain,cloud_cover&timezone=auto`;
  const response = await fetch(url);
  return response.json();
}

function applyWeatherEffect(data) {
  const current = data.current || {};
  const cloud = Number(current.cloud_cover || 0);
  const rainAmount = Number(current.rain || current.precipitation || 0);
  const rainIntensity = Math.min(1, rainAmount / 6);
  rain.intensity = rainIntensity;
  setFogIntensity(Math.min(0.6, cloud / 100));
  if (rain.intensity > 0.05 && !rain.running) {
    rain.running = true;
    tickRain();
  }
}

async function initWeather() {
  if (!navigator.geolocation) {
    return;
  }
  navigator.geolocation.getCurrentPosition(
    async (position) => {
      try {
        const data = await fetchWeather(
          position.coords.latitude,
          position.coords.longitude
        );
        applyWeatherEffect(data);
      } catch (err) {
        console.warn("Weather fetch failed", err);
      }
    },
    () => {
      // Location denied: keep subtle background only.
    },
    { enableHighAccuracy: false, timeout: 6000 }
  );
}

window.addEventListener("resize", () => {
  resizeRainCanvas();
  initDrops(Math.max(40, Math.floor(window.innerWidth / 18)));
});

resizeRainCanvas();
initDrops(Math.max(40, Math.floor(window.innerWidth / 18)));
initWeather();
