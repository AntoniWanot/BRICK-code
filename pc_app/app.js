// ==========================================
// UTILITY FUNCTIONS
// ==========================================

function showFeedback(message, type = 'success') {
    const feedbackEl = document.getElementById('feedbackMsg');
    feedbackEl.textContent = message;
    feedbackEl.className = `feedback-msg ${type}`;
    
    setTimeout(() => {
        feedbackEl.className = 'feedback-msg';
    }, 3000);
}


//Update step counter

function updateStepCounter(count) {
    const counter = document.getElementById('stepCounter');
    counter.textContent = `${count} step${count !== 1 ? 's' : ''}`;
}

// =====================================
// BLOCK DEFINITIONS
// =====================

// Parallel Joints Container
Blockly.Blocks['robot_joint_step'] = {
  init: function() {
    this.appendDummyInput().appendField("Move Joints in Parallel");
    this.appendStatementInput("JOINTS").setCheck("RobotJoint");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour(230);
  }
};

// Individual Joint
Blockly.Blocks['robot_joint'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("Joint")
        .appendField(new Blockly.FieldDropdown([["J1","1"], ["J2","2"], ["J3","3"]]), "JOINT_NUM")
        .appendField("Angle")
        .appendField(new Blockly.FieldNumber(0, -180, 180), "ANGLE")
        .appendField("Direction")
        .appendField(new Blockly.FieldDropdown([["CW","1"], ["CCW","0"]]), "DIR");
    this.setPreviousStatement(true, "RobotJoint");
    this.setNextStatement(true, "RobotJoint");
    this.setColour(160);
  }
};

// Absolute XYZ Move
Blockly.Blocks['robot_xyz'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("Move to Absolute XYZ → X:")
        .appendField(new Blockly.FieldNumber(0), "X")
        .appendField("Y:")
        .appendField(new Blockly.FieldNumber(0), "Y")
        .appendField("Z:")
        .appendField(new Blockly.FieldNumber(0), "Z");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour(290); 
  }
};

// XYZ Offset 
Blockly.Blocks['robot_offset'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("Shift Position by Offset → ΔX:")
        .appendField(new Blockly.FieldNumber(0), "DX")
        .appendField("ΔY:")
        .appendField(new Blockly.FieldNumber(0), "DY")
        .appendField("ΔZ:")
        .appendField(new Blockly.FieldNumber(0), "DZ");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour(200); 
  }
};

// Wait
Blockly.Blocks['robot_wait'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("Wait")
        .appendField(new Blockly.FieldNumber(1000, 0), "DURATION")
        .appendField("ms");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour(40);
  }
};

// Gripper Control
Blockly.Blocks['robot_gripper'] = {
  init: function() {
    this.appendDummyInput()
        .appendField("Gripper Tool →")
        .appendField(new Blockly.FieldDropdown([["CLOSE","1"], ["OPEN","0"]]), "STATE");
    this.setPreviousStatement(true, null);
    this.setNextStatement(true, null);
    this.setColour(0);
  }
};

// ====================
// TOOLBOX 
// ========================
const toolbox = {
    'kind': 'categoryToolbox',
    'contents': [
        {
            'kind': 'category',
            'name': 'Robot Motions',
            'contents': [
                { 'kind': 'block', 'type': 'robot_joint_step' },
                { 'kind': 'block', 'type': 'robot_joint' },
                { 'kind': 'block', 'type': 'robot_xyz' },
                { 'kind': 'block', 'type': 'robot_offset' },
                { 'kind': 'block', 'type': 'robot_gripper' }
            ]
        },
        {
            'kind': 'category',
            'name': 'Flow & Logic',
            'contents': [
                { 'kind': 'block', 'type': 'robot_wait' },
                { 'kind': 'block', 'type': 'controls_repeat_ext' },
                { 'kind': 'block', 'type': 'math_number' }
            ]
        }
    ]
};

// ============================
// INITIALIZE WORKSPACE
// ==========================
const workspace = Blockly.inject('blocklyDiv', {
    toolbox: toolbox,
    scrollbars: true,
    trashcan: true
});

// ========================
// THE PARSER 
// ==========+++++++++
function parseBlockChain(startBlock, stepsList) {
    let currentBlock = startBlock;

    while (currentBlock) {
        let stepData = {};

        if (currentBlock.type === 'robot_joint_step') {
            let jointBlock = currentBlock.getInputTargetBlock('JOINTS');
            let jointsList = [];
            while (jointBlock && jointBlock.type === 'robot_joint') {
                jointsList.push({
                    "joint": parseInt(jointBlock.getFieldValue('JOINT_NUM')),
                    "angle": parseFloat(jointBlock.getFieldValue('ANGLE')),
                    "direction": parseInt(jointBlock.getFieldValue('DIR'))
                });
                jointBlock = jointBlock.getNextBlock();
            }
            if (jointsList.length > 0) stepData.joints = jointsList;
        } 
        
        else if (currentBlock.type === 'robot_xyz') {
            stepData.xyz = {
                "x": parseFloat(currentBlock.getFieldValue('X')),
                "y": parseFloat(currentBlock.getFieldValue('Y')),
                "z": parseFloat(currentBlock.getFieldValue('Z'))
            };
        } 
        
        else if (currentBlock.type === 'robot_offset') {
            stepData.offset = {
                "dx": parseFloat(currentBlock.getFieldValue('DX')),
                "dy": parseFloat(currentBlock.getFieldValue('DY')),
                "dz": parseFloat(currentBlock.getFieldValue('DZ'))
            };
        } 
        
        else if (currentBlock.type === 'robot_wait') {
            stepData.delay_ms = parseInt(currentBlock.getFieldValue('DURATION'));
        } 
        
        else if (currentBlock.type === 'robot_gripper') {
            stepData.gripper = parseInt(currentBlock.getFieldValue('STATE'));
        }
        
        // LOOP LOGIC HANDLING 
        else if (currentBlock.type === 'controls_repeat_ext') {
            let repeatCountBlock = currentBlock.getInputTargetBlock('TIMES');
            let repeatCount = repeatCountBlock ? parseInt(repeatCountBlock.getFieldValue('NUM')) : 0;
            let loopBodyBlock = currentBlock.getInputTargetBlock('DO');       
            for (let i = 0; i < repeatCount; i++) {
                parseBlockChain(loopBodyBlock, stepsList);
            }
        }
        if (Object.keys(stepData).length > 0) {
            stepsList.push(stepData);
        }

        currentBlock = currentBlock.getNextBlock();
    }
}


//Generate JSON

function generateRobotJson() {
    const topBlocks = workspace.getTopBlocks(true);
    
    if (topBlocks.length === 0) {
        showFeedback('No blocks in workspace. Add blocks to generate a program.', 'error');
        return null;
    }
    
    let stepsArray = [];
    let initialBlock = topBlocks[0];

    try {
        parseBlockChain(initialBlock, stepsArray);

        const finalObject = {
            "program": {
                "current_step": 0,
                "total_steps": stepsArray.length,
                "steps": stepsArray
            }
        };

        return JSON.stringify(finalObject, null, 4);
    } catch (error) {
        console.error('Error generating JSON:', error);
        showFeedback('Error generating JSON. Check console for details.', 'error');
        return null;
    }
}

// ========================================
// BUTTON EVENT LISTENERS
// ===================================


//Generate JSON and display it

document.getElementById('generateBtn').addEventListener('click', function() {
    const jsonResult = generateRobotJson();
    if (jsonResult) {
        const outputEl = document.getElementById('jsonOutput');
        outputEl.textContent = jsonResult;
        
        // Update UI
        document.getElementById('copyBtn').disabled = false;
        document.getElementById('downloadBtn').disabled = false;
        updateStepCounter(JSON.parse(jsonResult).program.total_steps);
        
        showFeedback('JSON generated successfully', 'success');
    }
});

//Copy 

document.getElementById('copyBtn').addEventListener('click', function() {
    const jsonText = document.getElementById('jsonOutput').textContent;
    
    navigator.clipboard.writeText(jsonText).then(() => {
        showFeedback('Copied to clipboard', 'success');
    }).catch(() => {
        showFeedback('Failed to copy to clipboard', 'error');
    });
});


//Download

document.getElementById('downloadBtn').addEventListener('click', function() {
    const jsonText = document.getElementById('jsonOutput').textContent;
    const timestamp = new Date().toISOString().slice(0, 10);
    const filename = `robot_program_${timestamp}.json`;
    
    const element = document.createElement('a');
    element.setAttribute('href', 'data:text/json;charset=utf-8,' + encodeURIComponent(jsonText));
    element.setAttribute('download', filename);
    element.style.display = 'none';
    document.body.appendChild(element);
    element.click();
    document.body.removeChild(element);
    
    showFeedback('Downloaded as ' + filename, 'success');
});


//Clear all blocks from workspace

document.getElementById('clearBtn').addEventListener('click', function() {
    if (confirm('Are you sure you want to clear all blocks? This cannot be undone.')) {
        workspace.clear();
        document.getElementById('jsonOutput').textContent = 'Workspace cleared. Ready for a new program.';
        document.getElementById('copyBtn').disabled = true;
        document.getElementById('downloadBtn').disabled = true;
        updateStepCounter(0);
        showFeedback('Workspace cleared', 'success');
    }
});

// =======================================
// KEYBOARD SHORTCUTS
// =========================================
document.addEventListener('keydown', function(e) {
    // Ctrl+Enter to generate JSON
    if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
        document.getElementById('generateBtn').click();
    }
    
    // Ctrl+Shift+C to copy
    if ((e.ctrlKey || e.metaKey) && e.shiftKey && e.key === 'C') {
        if (!document.getElementById('copyBtn').disabled) {
            document.getElementById('copyBtn').click();
        }
    }
});