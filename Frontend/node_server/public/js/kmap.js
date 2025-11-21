/*
 * File: kmap.js
 * Purpose: Interactive K-Map Logic.
 * * Description:
 * - Renders the 8x8 Grid.
 * - Handles clicks to toggle cells (0 -> 1 -> X -> 0).
 * - EXPOSES data to main.js via window object.
 */

let scratchpadMinterms = [];

document.addEventListener('DOMContentLoaded', () => {
    renderScratchpad();
});

/* --- EXPORTED API FOR MAIN.JS --- */
// This was missing, causing main.js to fail silently
window.getKMapMinterms = () => {
    return scratchpadMinterms;
};

window.resetScratchpad = () => {
    scratchpadMinterms = [];
    renderScratchpad();
};

/* --- INTERNAL RENDERING LOGIC --- */
function renderScratchpad() {
    const container = document.getElementById('kmap-scratchpad');
    if (!container) return;
    
    container.innerHTML = '';
    const grayCode = [0, 1, 3, 2, 6, 7, 5, 4]; 
    const table = document.createElement('table');
    table.className = 'kmap-table';
    
    // Header
    const headerRow = document.createElement('tr');
    headerRow.innerHTML = '<th class="kmap-header-cell">ABC\\DEF</th>';
    
    grayCode.forEach(colVal => {
        const th = document.createElement('th');
        th.innerText = colVal.toString(2).padStart(3, '0');
        th.className = 'kmap-header-cell';
        headerRow.appendChild(th);
    });
    table.appendChild(headerRow);

    // Body
    grayCode.forEach(rowVal => {
        const tr = document.createElement('tr');
        const th = document.createElement('th');
        th.innerText = rowVal.toString(2).padStart(3, '0');
        th.className = 'kmap-header-cell';
        tr.appendChild(th);

        grayCode.forEach(colVal => {
            const td = document.createElement('td');
            const idx = (rowVal << 3) | colVal;
            
            if (scratchpadMinterms.includes(idx)) {
                td.className = 'on';
                td.innerText = '1';
            } else {
                td.className = 'off';
                td.innerText = '0';
            }
            
            td.style.cursor = 'pointer';
            td.onclick = () => toggleScratchpad(idx);
            tr.appendChild(td);
        });
        table.appendChild(tr);
    });
    container.appendChild(table);
}

function toggleScratchpad(idx) {
    if (scratchpadMinterms.includes(idx)) {
        scratchpadMinterms = scratchpadMinterms.filter(i => i !== idx);
    } else {
        scratchpadMinterms.push(idx);
    }
    renderScratchpad();
}

// Global function for Read-Only maps (used by main.js to show hardware state)
window.renderReadMap = function(containerId, minterms, colorClass) {
    const container = document.getElementById(containerId);
    if (!container) return;
    container.innerHTML = '';
    
    const grayCode = [0, 1, 3, 2, 6, 7, 5, 4]; 
    const table = document.createElement('table');
    table.className = 'kmap-table';

    const headerRow = document.createElement('tr');
    headerRow.innerHTML = '<th class="kmap-header-cell"></th>';
    grayCode.forEach(c => {
        const th = document.createElement('th');
        th.innerText = c.toString(2).padStart(3,'0');
        th.className = 'kmap-header-cell';
        headerRow.appendChild(th);
    });
    table.appendChild(headerRow);

    grayCode.forEach(r => {
        const tr = document.createElement('tr');
        const th = document.createElement('th');
        th.innerText = r.toString(2).padStart(3,'0');
        th.className = 'kmap-header-cell';
        tr.appendChild(th);
        grayCode.forEach(c => {
            const td = document.createElement('td');
            const idx = (r << 3) | c;
            if (minterms.includes(idx)) {
                td.classList.add(colorClass);
                td.innerText = '1';
            } else {
                td.className = 'off';
                td.innerText = '0';
            }
            tr.appendChild(td);
        });
        table.appendChild(tr);
    });
    container.appendChild(table);
}