/*
 * File: main.js
 * Purpose: Frontend Application Controller.
 */

let cachedNetlistElements = null; 
let lastCsvData = null;           
const logDiv = document.getElementById('server-log'); 

// --- Socket.IO Initialization ---
const socket = io();
window.socket = socket; 

socket.on('connect', () => {
    const indicator = document.getElementById('status-indicator');
    if (indicator) {
        indicator.textContent = "⚡ Connected";
        indicator.style.backgroundColor = "var(--status-ok)";
    }
    console.log("Socket connected");
});

socket.on('disconnect', () => {
    const indicator = document.getElementById('status-indicator');
    if (indicator) {
        indicator.textContent = "🔌 Disconnected";
        indicator.style.backgroundColor = "var(--status-bad)";
    }
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
            const pTitle = document.getElementById('preview-title');
            pTitle.innerText = `Previewing: Circuit ${t.toUpperCase()}`;
            pTitle.style.color = colorMap[t];
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
            setText(`min-${t}`, json.minterms.join(', '));
            const containerId = `kmap-container-${json.target}`;
            let colorClass = t==='x'?'x-only':(t==='y'?'y-only':(t==='z'?'z-only':'w-only'));
            if(typeof window.renderReadMap === 'function') {
                window.renderReadMap(containerId, json.minterms, colorClass);
            }
        }
    }
    else if (json.type === 'combined') {
        cachedNetlistElements = json.elements;
        refreshDiagram(); 
    }
    else if (json.type === 'verification' && json.csv) {
        addLog("TEST SUCCESS", "var(--status-ok)");
        lastCsvData = json.csv;
        const dlBtn = document.getElementById('dl-btn');
        if (dlBtn) dlBtn.style.display = 'block'; 
    }
});

// --- UI Binding ---
document.addEventListener('DOMContentLoaded', () => {
    console.log("DOM Loaded - Binding Events");

    bindClick('btn-theme', toggleTheme);
    bindClick('admin-lock', handleLockClick);
    
    const slider = document.getElementById('zoom-slider');
    if(slider) slider.oninput = (e) => handleZoom(e.target.value);

    bindClick('btn-login-cancel', closeLogin);
    bindClick('btn-login-submit', submitLogin);
    bindClick('btn-preview-close', closePreview);

    // Equation Buttons
    document.querySelectorAll('.btn-eq-pre').forEach(btn => {
        btn.addEventListener('click', () => sendEquation(btn.dataset.target, true));
    });
    document.querySelectorAll('.btn-eq-set').forEach(btn => {
        btn.addEventListener('click', () => sendEquation(btn.dataset.target, false));
    });

    // K-Map Buttons
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
});

// --- Logic ---

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
            lock.innerText = "🔓 Admin";
            lock.style.color = "var(--status-bad)";
        }
    } else {
        document.body.classList.remove('admin-unlocked');
        if (lock) {
            lock.innerText = "🔒 Guest";
            lock.style.color = "var(--text-muted)";
        }
    }
}

function toggleTheme() {
    document.body.classList.toggle('light-mode');
}

function closePreview() {
    const p = document.getElementById('preview-panel');
    if(p) p.style.display = 'none';
}

// --- Send Functions ---

function sendEquation(target, isPreview) {
    const input = document.getElementById('eqInput');
    
    // Debug Log
    console.log(`sendEquation triggered for ${target} (Preview: ${isPreview})`);

    if (!input || !input.value.trim()) {
        addLog("Error: Equation input is empty", "orange");
        return;
    }
    
    const cmd = isPreview 
        ? `preview ${target} ${input.value}` 
        : `program ${target} ${input.value}`;
        
    socket.emit('command', cmd);
    addLog(`Sent: ${cmd}`);
}

function applyKMapTo(target, mode) {
    // Debug Log
    console.log(`applyKMapTo triggered for ${target} (Mode: ${mode})`);

    // CRITICAL FIX: Check for the function we added to kmap.js
    if (typeof window.getKMapMinterms !== 'function') {
        console.error("Error: window.getKMapMinterms not found! Check kmap.js loading.");
        addLog("Error: KMap library not loaded", "red");
        return;
    }
    
    const minterms = window.getKMapMinterms();
    const csv = minterms.join(',');
    
    let cmd = '';
    if (mode === 'preview') {
        cmd = `preview_kmap ${target} ${csv}`;
    } else {
        cmd = `kmap ${target} ${csv}`;
    }
    
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

// --- Visualizer ---

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
    const csvContent = lastCsvData.replace(/\\n/g, '\n');
    const blob = new Blob([csvContent], { type: 'text/csv' });
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'report.csv';
    a.click();
    window.URL.revokeObjectURL(url);
}

// --- Helpers ---

function addLog(text, color=null) {
    const entry = document.createElement('div');
    entry.className = 'log-entry';
    if(color) entry.style.color = color;
    const time = new Date().toLocaleTimeString();
    entry.innerHTML = `<span class="log-time">[${time}]</span> ${text}`;
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