#ifndef OTA_HTML_H
#define OTA_HTML_H

const char ota_html[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>CYD Firmware Update</title>
<style>
body {
    font-family: 'Inter', system-ui, sans-serif;
    background: #1e2030;
    color: #cad3f5;
    margin: 0;
    padding: 20px;
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 100vh;
    box-sizing: border-box;
}
.card {
    background: #24273a;
    border-radius: 12px;
    padding: 30px;
    width: 100%;
    max-width: 400px;
    box-shadow: 0 8px 30px rgba(0,0,0,0.5);
    border: 1px solid #363a4f;
    text-align: center;
}
h2 {
    color: #c6a0f6;
    margin-top: 0;
    margin-bottom: 20px;
    font-weight: 600;
}
.drop-zone {
    border: 2px dashed #494d64;
    border-radius: 8px;
    padding: 30px 10px;
    margin-bottom: 20px;
    cursor: pointer;
    transition: border-color 0.2s, background-color 0.2s;
}
.drop-zone:hover, .drop-zone.dragover {
    border-color: #8aadf4;
    background: #363a4f;
}
.drop-zone p {
    margin: 0;
    color: #a5adcb;
    font-size: 14px;
}
input[type="file"] {
    display: none;
}
button {
    width: 100%;
    padding: 12px;
    background: #8aadf4;
    border: none;
    border-radius: 6px;
    color: #181926;
    font-size: 16px;
    font-weight: bold;
    cursor: pointer;
    transition: background-color 0.2s;
}
button:hover {
    background: #c6a0f6;
}
button:disabled {
    background: #494d64;
    color: #a5adcb;
    cursor: not-allowed;
}
.progress-container {
    margin-top: 20px;
    display: none;
}
.progress-bar {
    width: 100%;
    height: 12px;
    background: #363a4f;
    border-radius: 6px;
    overflow: hidden;
    margin-bottom: 8px;
}
.progress-fill {
    height: 100%;
    width: 0%;
    background: #a6da95;
    transition: width 0.1s;
}
.status-text {
    font-size: 14px;
    color: #a5adcb;
}
.error-text {
    color: #ed8796;
}
.success-text {
    color: #a6da95;
}
</style>
</head>
<body>
<div class="card">
    <h2>Firmware Update</h2>
    <p style="color: #a5adcb; font-size: 14px; margin-bottom: 20px;">Upload a compiled .bin file to flash the device.</p>
    <div class="drop-zone" id="drop-zone">
        <p id="file-name">Click or drag & drop binary file here</p>
        <input type="file" id="file-input" accept=".bin">
    </div>
    <button id="upload-btn" disabled>Upload Firmware</button>
    
    <div class="progress-container" id="progress-container">
        <div class="progress-bar">
            <div class="progress-fill" id="progress-fill"></div>
        </div>
        <div class="status-text" id="status-text">Uploading: 0%</div>
    </div>
</div>

<script>
const dropZone = document.getElementById('drop-zone');
const fileInput = document.getElementById('file-input');
const uploadBtn = document.getElementById('upload-btn');
const progressContainer = document.getElementById('progress-container');
const progressFill = document.getElementById('progress-fill');
const statusText = document.getElementById('status-text');
let selectedFile = null;

dropZone.addEventListener('click', () => fileInput.click());

fileInput.addEventListener('change', (e) => {
    if (e.target.files.length > 0) {
        handleFileSelect(e.target.files[0]);
    }
});

dropZone.addEventListener('dragover', (e) => {
    e.preventDefault();
    dropZone.classList.add('dragover');
});

dropZone.addEventListener('dragleave', () => {
    dropZone.classList.remove('dragover');
});

dropZone.addEventListener('drop', (e) => {
    e.preventDefault();
    dropZone.classList.remove('dragover');
    if (e.dataTransfer.files.length > 0) {
        handleFileSelect(e.dataTransfer.files[0]);
    }
});

function handleFileSelect(file) {
    if (file.name.endsWith('.bin')) {
        selectedFile = file;
        document.getElementById('file-name').innerText = file.name;
        uploadBtn.disabled = false;
    } else {
        alert('Please select a valid .bin file');
    }
}

uploadBtn.addEventListener('click', () => {
    if (!selectedFile) return;
    
    uploadBtn.disabled = true;
    dropZone.style.pointerEvents = 'none';
    progressContainer.style.display = 'block';
    
    const formData = new FormData();
    formData.append('update', selectedFile);
    
    const xhr = new XMLHttpRequest();
    xhr.open('POST', '/update', true);
    
    xhr.upload.addEventListener('progress', (e) => {
        if (e.lengthComputable) {
            const percent = Math.round((e.loaded / e.total) * 100);
            progressFill.style.width = percent + '%';
            statusText.innerText = 'Uploading: ' + percent + '%';
        }
    });
    
    xhr.onload = function() {
        if (xhr.status === 200) {
            progressFill.style.backgroundColor = '#a6da95';
            statusText.innerHTML = '<span class="success-text">Success! Firmware uploaded. Rebooting...</span>';
            setTimeout(() => {
                window.location.href = '/';
            }, 5000);
        } else {
            progressFill.style.backgroundColor = '#ed8796';
            statusText.innerHTML = '<span class="error-text">Failed: ' + xhr.responseText + '</span>';
            uploadBtn.disabled = false;
            dropZone.style.pointerEvents = 'auto';
        }
    };
    
    xhr.onerror = function() {
        progressFill.style.backgroundColor = '#ed8796';
        statusText.innerHTML = '<span class="error-text">Connection lost during upload.</span>';
        uploadBtn.disabled = false;
        dropZone.style.pointerEvents = 'auto';
    };
    
    xhr.send(formData);
});
</script>
</body>
</html>
)rawhtml";

#endif // OTA_HTML_H
