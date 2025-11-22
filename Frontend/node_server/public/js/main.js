/*
 * File: main.js
 * Purpose: Frontend Application Controller.
 */

// --- Global State ---
let currentInputMask = 0;
let currentMinterms = { x:[], y:[], z:[], w:[] };
let cachedNetlistElements = null; 
let lastCsvData = null;           
const logDiv = document.getElementById('server-log'); 

// --- AUDIO: Offline Alert ---
// Ensure 'offline.mp3' exists in 'public/sounds/'
const sfxOffline = new Audio('sounds/offline.mp3');

// --- Socket.IO Initialization ---
const socket = io();
window.socket = socket; 

socket.on('connect', () => {
    const indicator = document.getElementById('status-indicator');
    if (indicator) {
        indicator.innerHTML = '<span class="w-1.5 h-1.5 rounded-full bg-emerald-500"></span> Connected';
        indicator.style.color = "var(--channel-x)";
    }
    console.log("Socket connected");
});

socket.on('disconnect', () => {
    const indicator = document.getElementById('status-indicator');
    
    // Trigger Sound: Catch error if user hasn't interacted with page yet
    sfxOffline.play().catch(e => console.log("Audio blocked until interaction"));

    if (indicator) {
        indicator.innerHTML = '<span class="w-1.5 h-1.5 rounded-full bg-red-500 animate-pulse"></span> Offline';
        indicator.style.color = "var(--channel-w)";
    }
    addLog("⚠️ Connection Lost", "red");
});

socket.on('server_log', (msg) => addLog("UDP: " + msg));

socket.on('state_update', (json) => {
    if (json.type === 'auth') {
        if (json.status === 'success') { 
            setAdminMode(true); 
            closeLogin(); 
            addLog("Auth Success", "var(--status-ok)"); 
            socket.emit('command', 'refresh'); 
        } else { 
            const err = document.getElementById('login-error');
            if (err) err.style.display = 'block'; 
        }
    }
    else if (json.type === 'result') {
        const t = json.target.toLowerCase();
        if (json.mode === 'preview') {
            const pPanel = document.getElementById('preview-panel');
            pPanel.style.display = 'block';
            const colorMap = { 'x': 'var(--color-x)', 'y': 'var(--color-y)', 'z': 'var(--color-z)', 'w': 'var(--color-w)' };
            pPanel.style.borderLeftColor = colorMap[t];
            
            // Set text content
            const pTitle = document.getElementById('preview-title');
            // Clear existing and set new content to preserve icon
            pTitle.innerHTML = `<i class="fas fa-eye text-sm opacity-50 mr-2"></i> Previewing: Circuit ${t.toUpperCase()}`;
            pTitle.className = `text-lg font-bold flex items-center gap-2 text-channel-${t}`;
            
            setText('preview-sop', json.sop);
            setText('preview-pos', json.pos);
            setText('preview-min', json.minterms.join(', '));
            
            if(typeof window.renderReadMap === 'function') {
                let cClass = t==='x'?'x-only':(t==='y'?'y-only':(t==='z'?'z-only':'w-only'));
                window.renderReadMap('preview-kmap-container', json.minterms, cClass);
            }
        } else {
            setText(`sop-${t}`, json.sop);
            setText(`pos-${t}`, json.pos);
            setText(`min-${t}`, json.minterms.join(', ')); // Added Minterms update
            const containerId = `kmap-container-${json.target}`;
            let colorClass = t==='x'?'x-only':(t==='y'?'y-only':(t==='z'?'z-only':'w-only'));
            if(typeof window.renderReadMap === 'function') {
                window.renderReadMap(containerId, json.minterms, colorClass);
            }
        }
    }
    else if (json.type === 'combined') {
        currentMinterms.x = json.mintermsX || [];
        currentMinterms.y = json.mintermsY || [];
        currentMinterms.z = json.mintermsZ || [];
        currentMinterms.w = json.mintermsW || [];
        cachedNetlistElements = json.elements;
        refreshDiagram(); 
        updateLiveIO();
    }
    else if (json.type === 'verification' && json.csv) {
        addLog("TEST SUCCESS", "var(--status-ok)");
        lastCsvData = json.csv;
        const dlBtn = document.getElementById('dl-btn');
        if (dlBtn) dlBtn.style.display = 'block'; 
    }
    if (json.inputs !== undefined) {
        currentInputMask = json.inputs;
        updateLiveIO(); 
        const modes = [
            "EDITING CHANNEL X", "EDITING CHANNEL Y", "EDITING CHANNEL Z", "EDITING CHANNEL W",
            "RUNNING (AUTO)", "TESTING SUITE", "MANUAL (ROTARY)", "HARDWARE (GPIO PINS)"
        ];
        const modeText = modes[json.mode] || "UNKNOWN MODE";
        setText('io-mode-indicator', modeText);
        const locked = (json.mode === 7); 
        document.querySelectorAll('.io-btn').forEach(b => b.disabled = locked);
    }
});

document.addEventListener('DOMContentLoaded', () => {
    bindClick('btn-theme', toggleTheme);
    bindClick('admin-lock', handleLockClick);
    const slider = document.getElementById('zoom-slider');
    if(slider) slider.oninput = (e) => handleZoom(e.target.value);
    bindClick('btn-login-cancel', closeLogin);
    bindClick('btn-login-submit', submitLogin);
    bindClick('btn-preview-close', closePreview);

    document.querySelectorAll('.btn-eq-pre').forEach(btn => {
        btn.addEventListener('click', () => sendEquation(btn.dataset.target, true));
    });
    document.querySelectorAll('.btn-eq-set').forEach(btn => {
        btn.addEventListener('click', () => sendEquation(btn.dataset.target, false));
    });
    document.querySelectorAll('.btn-kmap-pre').forEach(btn => {
        btn.addEventListener('click', () => applyKMapTo(btn.dataset.target, 'preview'));
    });
    document.querySelectorAll('.btn-kmap-set').forEach(btn => {
        btn.addEventListener('click', () => applyKMapTo(btn.dataset.target, 'program'));
    });
    bindClick('btn-clear-kmap', () => {
        if (typeof window.resetScratchpad === 'function') window.resetScratchpad();
    });
    document.querySelectorAll('.chk-viz').forEach(chk => {
        chk.addEventListener('change', refreshDiagram);
    });
    bindClick('btn-send-raw', sendRaw);
    bindClick('dl-btn', triggerDownload);
    
    const savedTheme = localStorage.getItem('theme');
    if (savedTheme === 'light') document.documentElement.classList.remove('dark');
    else document.documentElement.classList.add('dark');
});

function handleLockClick() {
    if (document.body.classList.contains('admin-unlocked')) {
        setAdminMode(false);
        addLog("Locked");
    } else {
        showLogin();
    }
}
function showLogin() {
    const modal = document.getElementById('login-modal');
    const passInput = document.getElementById('admin-pass');
    const err = document.getElementById('login-error');
    if (modal) modal.style.display = 'flex';
    if (passInput) { passInput.value = ''; passInput.focus(); }
    if (err) err.style.display = 'none';
}
function closeLogin() {
    const modal = document.getElementById('login-modal');
    if (modal) modal.style.display = 'none';
}
function submitLogin() {
    const passInput = document.getElementById('admin-pass');
    if (passInput && passInput.value) {
        socket.emit('command', `login ${passInput.value}`);
    }
}
function setAdminMode(isUnlocked) {
    const lock = document.getElementById('admin-lock');
    if (isUnlocked) {
        document.body.classList.add('admin-unlocked');
        if (lock) {
            lock.innerHTML = '<i class="fas fa-unlock text-xs text-red-500"></i> <span class="text-xs font-bold text-red-500">Admin</span>';
        }
    } else {
        document.body.classList.remove('admin-unlocked');
        if (lock) {
            lock.innerHTML = '<i class="fas fa-lock text-xs text-slate-400 group-hover:text-accent"></i> <span class="text-xs font-bold text-slate-500 dark:text-slate-400 group-hover:text-slate-800 dark:group-hover:text-white">Guest</span>';
        }
    }
}
function toggleTheme() {
    const html = document.documentElement;
    if (html.classList.contains('dark')) {
        html.classList.remove('dark');
        localStorage.setItem('theme', 'light'); 
    } else {
        html.classList.add('dark');
        localStorage.setItem('theme', 'dark');
    }
}
function closePreview() {
    const p = document.getElementById('preview-panel');
    if(p) p.style.display = 'none';
}
function sendEquation(target, isPreview) {
    const input = document.getElementById('eqInput');
    if (!input || !input.value.trim()) {
        addLog("Error: Equation empty", "orange");
        return;
    }
    const cmd = isPreview ? `preview ${target} ${input.value}` : `program ${target} ${input.value}`;
    socket.emit('command', cmd);
    addLog(`Sent: ${cmd}`);
}
function applyKMapTo(target, mode) {
    if (typeof window.getKMapMinterms !== 'function') return;
    const minterms = window.getKMapMinterms();
    const csv = minterms.join(',');
    let cmd = (mode === 'preview') ? `preview_kmap ${target} ${csv}` : `kmap ${target} ${csv}`;
    socket.emit('command', cmd);
    addLog(`Sent [${mode}]: ${cmd}`);
}
function sendRaw() {
    const input = document.getElementById('rawCmd');
    if (input && input.value) {
        socket.emit('command', input.value);
        addLog(`Raw: ${input.value}`);
        input.value = '';
    }
}
window.toggleInput = function(bit) {
    const newMask = currentInputMask ^ (1 << bit); 
    socket.emit('command', `set_input ${newMask}`);
};
function updateLiveIO() {
    for(let i=0; i<6; i++) {
        const btn = document.getElementById(`btn-io-${String.fromCharCode(97+i)}`); 
        if(btn) {
            if ((currentInputMask >> i) & 1) btn.classList.add('active');
            else btn.classList.remove('active');
        }
    }
    updateLed('x', currentMinterms.x.includes(currentInputMask));
    updateLed('y', currentMinterms.y.includes(currentInputMask));
    updateLed('z', currentMinterms.z.includes(currentInputMask));
    updateLed('w', currentMinterms.w.includes(currentInputMask));
}
function updateLed(label, isOn) {
    const led = document.getElementById(`led-io-${label}`);
    if(led) {
        if(isOn) led.classList.add('on');
        else led.classList.remove('on');
    }
}
function handleZoom(val) {
    if (window.cyInstances && window.cyInstances['XY']) {
        const cy = window.cyInstances['XY'];
        cy.zoom({
            level: parseFloat(val),
            position: { x: cy.width()/2, y: cy.height()/2 }
        });
    }
}
function refreshDiagram() {
    if (!cachedNetlistElements) return;
    const active = [];
    if(isChecked('show-x')) active.push('X');
    if(isChecked('show-y')) active.push('Y');
    if(isChecked('show-z')) active.push('Z');
    if(isChecked('show-w')) active.push('W');
    if(typeof window.renderCircuit === 'function') {
        window.renderCircuit('XY', cachedNetlistElements, active);
    }
}
function triggerDownload() {
    if (!lastCsvData) return;
    const blob = new Blob([lastCsvData], { type: 'text/csv' });
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url; a.download = 'report.csv'; a.click();
    window.URL.revokeObjectURL(url);
}
function addLog(text, color=null) {
    const entry = document.createElement('div');
    entry.className = 'log-entry';
    if(color) entry.style.color = color;
    const time = new Date().toLocaleTimeString();
    entry.innerHTML = `<span class="opacity-50 mr-2">[${time}]</span> ${text}`;
    if(logDiv) {
        logDiv.appendChild(entry);
        logDiv.scrollTop = logDiv.scrollHeight;
    }
}
function bindClick(id, handler) {
    const el = document.getElementById(id);
    if(el) el.addEventListener('click', handler);
}
function setText(id, text) {
    const el = document.getElementById(id);
    if(el) el.innerText = text;
}
function isChecked(id) {
    const el = document.getElementById(id);
    return el ? el.checked : false;
}