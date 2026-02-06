#ifndef EVENT_BUTTON_COLORPICKER_PAGE_HPP
#define EVENT_BUTTON_COLORPICKER_PAGE_HPP

#include <Arduino.h>

const char colorPicker_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="uk">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>LED Curve Controller</title>
    <style>
        * { box-sizing: border-box; }

        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background: #121212;
            color: white;
            margin: 0;
            padding: 0;
            height: 100vh;
            display: flex;
            flex-direction: column;
            overflow: hidden;
            user-select: none;
        }

        header {
            padding: 15px;
            text-align: center;
            background: #1e1e1e;
            flex-shrink: 0;
            border-bottom: 1px solid #333;
            z-index: 10;
        }

        h2 { margin: 0; font-size: 1.2rem; }
        .status {
            font-size: 0.85rem;
            color: #888;
            margin-top: 5px;
            min-height: 1.2em;
            transition: color 0.3s;
        }

        .canvas-container {
            flex-grow: 1;
            position: relative;
            width: 100%;
            background: #000;
            cursor: crosshair;
            touch-action: none;
        }

        canvas {
            display: block;
            width: 100%;
            height: 90%;
        }

        .controls {
            padding: 15px;
            background: #1e1e1e;
            border-top: 1px solid #333;
            flex-shrink: 0;
            display: flex;
            justify-content: center;
            z-index: 10;
        }

        button {
            padding: 12px 30px;
            font-size: 16px;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            font-weight: 600;
            width: 100%;
            max-width: 300px;
            background: #d32f2f;
            color: white;
            transition: opacity 0.2s;
        }

        button:active { opacity: 0.7; }

    </style>
</head>
<body>

<header>
    <h2>Ambilight Controller</h2>
    <div class="status" id="status">Loading...</div>
</header>

<div class="canvas-container" id="canvasContainer">
    <canvas id="colorCanvas"></canvas>
</div>

<div class="controls">
    <button id="btn-reset">Reset</button>
</div>

<script>
    const SETTINGS_URL = '/settings';
    const UPDATE_URL = '/update';
    const STARTUP_MODE_URL = '/deviceMode';
    const CONTROLLER_URL = "/rgb";

    let SETTINGS = {};
    let LED_COUNT = 60;
    let curvePoints = [];
    let isDrawing = false;

    const canvasContainer = document.getElementById('canvasContainer');
    const canvas = document.getElementById('colorCanvas');
    const ctx = canvas.getContext('2d');
    const statusDiv = document.getElementById('status');


    // --- HTTP REQUEST ---
    function httpRequest(method, url, data, callback) {
        let xhr = new XMLHttpRequest();
        xhr.open(method, url, true);
        xhr.responseType = "json";
        if (method === 'POST') {
            xhr.setRequestHeader('Content-Type', 'application/json');
        }
        xhr.onload = function () {
            if (xhr.status >= 200 && xhr.status < 300) {
                callback(null, xhr.response);
            } else {
                callback('Request failed with status: ' + xhr.status, null);
            }
        };
        xhr.onerror = function () {
            callback('Request failed', null);
        };
        xhr.send(data);
    }

    function getBaseSettings() {
        httpRequest('GET', STARTUP_MODE_URL, null, (error, data) => {
            if (error) console.error("Mode fetch error:", error);
            httpRequest('GET', SETTINGS_URL, null, (error, data) => {
                if (error || !data?.result) {
                    console.error("Settings fetch error:", error);
                    statusDiv.innerText = "Failed to load settings";
                    initUI();
                } else {
                    SETTINGS = data.result;
                    // Оновлюємо LED_COUNT з отриманих даних
                    if (SETTINGS.ledCount) {
                        LED_COUNT = parseInt(SETTINGS.ledCount);
                    }
                    console.log("Loaded LED_COUNT:", LED_COUNT);
                    statusDiv.innerText = "Draw";
                    initUI();
                }
            })
        });
    }

    // --- ІНІЦІАЛІЗАЦІЯ ІНТЕРФЕЙСУ ---
    function initUI() {
        // Створюємо масив на основі реальної кількості діодів
        curvePoints = new Array(LED_COUNT).fill(0.5);
        resizeCanvas();
    }

    getBaseSettings();

    function resizeCanvas() {
        canvas.width = canvasContainer.clientWidth;
        canvas.height = canvasContainer.clientHeight;
        if (curvePoints.length > 0) {
            drawCurve();
        }
    }

    window.addEventListener('resize', resizeCanvas);

    function drawBackground() {
        const gradient = ctx.createLinearGradient(0, 0, canvas.width, 0);
        gradient.addColorStop(0,    "red");
        gradient.addColorStop(0.17, "orange");
        gradient.addColorStop(0.33, "yellow");
        gradient.addColorStop(0.5,  "green");
        gradient.addColorStop(0.67, "cyan");
        gradient.addColorStop(0.83, "blue");
        gradient.addColorStop(1,    "magenta");

        ctx.fillStyle = gradient;
        ctx.fillRect(0, 0, canvas.width, canvas.height);

        ctx.strokeStyle = "rgba(0,0,0,0.2)";
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(canvas.width / 2, 0);
        ctx.lineTo(canvas.width / 2, canvas.height);
        ctx.stroke();
    }

    function drawCurve() {
        drawBackground();

        ctx.beginPath();
        ctx.lineWidth = 4;
        ctx.strokeStyle = "black";
        ctx.lineCap = "round";
        ctx.lineJoin = "round";

        for (let i = 0; i < LED_COUNT; i++) {
            const padding = 10;
            const availableHeight = canvas.height - (padding * 2);
            const y = padding + (i / (LED_COUNT - 1)) * availableHeight;
            const x = curvePoints[i] * canvas.width;

            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
        }
        ctx.stroke();

        ctx.lineWidth = 2;
        ctx.strokeStyle = "white";
        ctx.stroke();
    }

    function hslToHex(h, s, l) {
        l /= 100;
        const a = s * Math.min(l, 1 - l) / 100;
        const f = n => {
            const k = (n + h / 30) % 12;
            const color = l - a * Math.max(Math.min(k - 3, 9 - k, 1), -1);
            return Math.round(255 * color).toString(16).padStart(2, '0');
        };
        return `#${f(0)}${f(8)}${f(4)}`;
    }

    async function sendDataToController() {
        statusDiv.innerText = "Sending...";
        statusDiv.style.color = "#FFD700";

        const colors = curvePoints.map(val => {
            const hue = val * 360;
            return hslToHex(hue, 100, 50);
        });

        const payload = {
            top: colors,
            right: [],
            bottom: [],
            left: []
        };

        try {
            const response = await fetch(CONTROLLER_URL, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });

            if(response.ok) {
                statusDiv.innerText = "✅ Ready!";
                statusDiv.style.color = "#00e676";
            } else {
                statusDiv.innerText = "❌ Failed: " + response.status;
                statusDiv.style.color = "#ff3d00";
            }
        } catch (e) {
            console.error(e);
            statusDiv.innerText = "❌ Connection failed";
            statusDiv.style.color = "#ff3d00";
        }
    }

    function handleInput(y_px, x_px) {
        if (curvePoints.length === 0) return;

        const padding = 10;
        const availableHeight = canvas.height - (padding * 2);
        let yRelative = y_px - padding;

        let ledIndex = Math.round((yRelative / availableHeight) * (LED_COUNT - 1));
        ledIndex = Math.max(0, Math.min(LED_COUNT - 1, ledIndex));

        const xNormalized = Math.max(0, Math.min(1, x_px / canvas.width));

        curvePoints[ledIndex] = xNormalized;
        drawCurve();
    }

    // --- LISTENERS (ПОДІЇ) ---
    // 1. МИША
    canvas.addEventListener('mousedown', (e) => {
        isDrawing = true;
        handleInput(e.offsetY, e.offsetX);
    });

    canvas.addEventListener('mousemove', (e) => {
        if(isDrawing) handleInput(e.offsetY, e.offsetX);
    });

    window.addEventListener('mouseup', () => {
        if (isDrawing) {
            isDrawing = false;
            sendDataToController();
        }
    });

    // 2. ТАЧСКРІН
    const getTouchPos = (touch) => {
        const rect = canvas.getBoundingClientRect();
        return { x: touch.clientX - rect.left, y: touch.clientY - rect.top };
    };

    canvas.addEventListener('touchstart', (e) => {
        isDrawing = true;
        const pos = getTouchPos(e.touches[0]);
        handleInput(pos.y, pos.x);
    }, {passive: false});

    canvas.addEventListener('touchmove', (e) => {
        if(isDrawing) {
            const pos = getTouchPos(e.touches[0]);
            handleInput(pos.y, pos.x);
            e.preventDefault();
        }
    }, {passive: false});

    window.addEventListener('touchend', () => {
        if (isDrawing) {
            isDrawing = false;
            sendDataToController();
        }
    });

    document.getElementById('btn-reset').onclick = () => {
        if (curvePoints.length > 0) {
            curvePoints.fill(0.5);
            drawCurve();
            sendDataToController();
        }
    };

</script>
</body>
</html>
)rawliteral";

#endif //EVENT_BUTTON_COLORPICKER_PAGE_HPP
