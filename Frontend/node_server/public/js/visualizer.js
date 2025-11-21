/*
 * File: visualizer.js
 * Purpose: Circuit Rendering Logic.
 * * Description:
 * - Uses Cytoscape.js to render the JSON Netlist received from the C App.
 * - Applies custom styles (shapes, colors) to represent logic gates.
 * - Handles "Input Merging" to clean up the diagram.
 */

// --- 1. Logic to Merge Duplicate Inputs ---
// The raw netlist creates a separate "A" node for every gate that uses "A".
// This function merges them so "A" appears only once on the far left.
function mergeCommonInputs(elements) {
    const nodeMap = {};
    const nodesToRemove = new Set();
    const newElements = [];

    // Pass 1: Identify duplicate variable nodes
    elements.forEach(el => {
        const data = el.data;
        if (data.id && data.type === 'var') {
            if (nodeMap[data.label]) {
                // We already have a node for variable "A", mark this one for removal
                nodesToRemove.add(data.id);
            } else {
                // First time seeing "A", record its ID
                nodeMap[data.label] = data.id;
            }
        }
    });

    // Pass 2: Reconstruct list and rewire edges
    elements.forEach(el => {
        if (el.data.source) { 
            // If this is an Edge coming from a duplicate node...
            if (nodesToRemove.has(el.data.source)) {
                // Find the duplicate node's label (e.g. "A")
                const dupNode = elements.find(n => n.data.id === el.data.source);
                // Find the ID of the Master "A" node
                const masterId = nodeMap[dupNode.data.label];
                // Rewire edge to the Master node
                el.data.source = masterId;
            }
            newElements.push(el);
        } else if (!nodesToRemove.has(el.data.id)) {
            // If this is a Node and NOT marked for removal, keep it
            newElements.push(el);
        }
    });
    return newElements;
}

// --- 2. Main Render Function ---
/**
 * Renders the Cytoscape graph.
 * @param {string} target - Suffix for the container ID (e.g., 'XY')
 * @param {Array} rawElements - JSON elements from C App
 * @param {Array} activeLabels - List of output labels to show (e.g., ['X', 'Y'])
 */
function renderCircuit(target, rawElements, activeLabels = null) {
    const containerId = `cy-${target}`;
    const container = document.getElementById(containerId);
    
    if (!container) {
        console.error(`Visualizer: Container ${containerId} not found`);
        return;
    }

    // Pre-process elements to merge inputs
    const elements = mergeCommonInputs(rawElements);

    // Initialize Cytoscape
    const cy = cytoscape({
        container: container,
        elements: elements,
        
        // Interaction Settings
        zoomingEnabled: true,
        userZoomingEnabled: true,
        panningEnabled: true,
        minZoom: 0.2,
        maxZoom: 3.0,
        boxSelectionEnabled: false,

        // --- VISUAL STYLING ---
        style: [
            // Base Node Style
            {
                selector: 'node',
                style: {
                    'label': 'data(label)',
                    'color': '#fff',
                    'font-size': '14px',
                    'font-weight': 'bold',
                    'text-valign': 'center',
                    'text-halign': 'center',
                    'width': '60px',
                    'height': '40px',
                    'border-width': 2,
                    'border-color': '#fff',
                    'background-color': '#555' // Default gray
                }
            },

            // Logic Gates (Blue Rounded Box)
            {
                selector: 'node[type="gate"]',
                style: {
                    'background-color': '#007acc',
                    'shape': 'round-rectangle',
                    'width': '70px',
                    'height': '50px'
                }
            },
            
            // Input Variables (Green Rectangle)
            {
                selector: 'node[type="var"]',
                style: {
                    'background-color': '#28a745',
                    'shape': 'rectangle',
                    'width': '50px',
                    'height': '40px'
                }
            },

            // Outputs (Red Rectangle)
            {
                selector: 'node[type="output"]',
                style: {
                    'background-color': '#dc3545',
                    'shape': 'rectangle',
                    'width': '50px',
                    'height': '40px'
                }
            },

            // Wires (Taxi/Manhattan Routing)
            {
                selector: 'edge',
                style: {
                    'width': 3,
                    'line-color': '#aaa',
                    'target-arrow-color': '#aaa',
                    'target-arrow-shape': 'triangle',
                    
                    // Taxi style creates 90-degree bends
                    'curve-style': 'taxi',
                    'taxi-direction': 'horizontal', // Wires flow Left-to-Right
                    'taxi-turn': 20,                // Turn radius
                    'taxi-turn-min-distance': 15,
                    
                    // Enforce connection points (Right side of source -> Left side of target)
                    'source-endpoint': '0deg',
                    'target-endpoint': '180deg'
                }
            }
        ]
    });

    // --- FILTERING LOGIC (Checkbox Support) ---
    // Allows hiding specific output trees (e.g., Hide 'Z' and 'W' circuits)
    if (activeLabels && Array.isArray(activeLabels)) {
        // 1. Find desired output nodes
        const outputSelector = activeLabels.map(label => `node[type="output"][label="${label}"]`).join(',');
        const desiredOutputs = cy.nodes(outputSelector);
        
        if (desiredOutputs.length > 0) {
            // 2. Trace backwards (recursively) to find all contributing nodes
            const predecessors = desiredOutputs.predecessors();
            const toKeep = desiredOutputs.union(predecessors);
            
            // 3. Remove anything NOT in that list
            cy.remove(cy.elements().not(toKeep));
        } else {
            // If no outputs selected, clear everything
            cy.elements().remove();
        }
    }

    // --- LAYOUT ENGINE (Dagre) ---
    // Dagre creates hierarchical layouts (like flowcharts)
    cy.layout({
        name: 'dagre', 
        rankDir: 'LR',   // Left-to-Right flow
        align: 'UL',     // Align Upper-Left
        
        // Spacing Settings for Clean 90-degree wires
        nodeSep: 60,     // Vertical space between nodes
        rankSep: 150,    // Horizontal space (runway for wires)
        
        padding: 40,
        animate: true,
        animationDuration: 400,
        ranker: 'network-simplex'
    }).run();

    // --- EXPOSE FOR SLIDER ---
    // Save instance globally so the Zoom Slider can access it
    if (!window.cyInstances) window.cyInstances = {};
    window.cyInstances[target] = cy;

    // --- Auto-Fit ---
    cy.ready(() => {
        cy.fit(cy.elements(), 40);
    });
}