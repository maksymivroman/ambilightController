#ifndef EVENT_BUTTON_HTML_PAGE_HPP
#define EVENT_BUTTON_HTML_PAGE_HPP

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <title>%PAGE_TITLE%</title>
    <style>
        html {
            font-size: 16px;
        }

        h1, h2, h3, h4, h5 {
            line-height: normal;
            margin: 0;
        }

        body {
            font-family: Arial, Helvetica, sans-serif;
            margin: 0;
            background: #eeeaea;
            display: flex;
            flex-direction: column;
            align-items: stretch;
            height: 100vh;
        }

        .mr-m {
            margin-right: .25rem;
        }

        .mt-l {
            margin-top: 1.25rem;
        }

        button {
            padding: 4px;
            min-width: 100px;
        }

        label {
            font-size: .75rem;
            align-self: center;
        }

        input[type=checkbox] {
            margin-right: 8px;
        }

        .header {
            background-color: #475d72;
            padding: 5px;
            display: flex;
            align-items: center;
            justify-content: space-between;
            font-weight: 200;
            color: white;
        }

        .title {
            font-size: 16px;
            line-height: normal;
        }

        .subtitle {
            font-size: .75rem;
            color: lightgray;
        }

        .main {
            display: flex;
            flex-flow: column;
            flex: 1;
            overflow: auto;
            padding: 1rem;
        }

        .footer {
            background: #444444;
            color: lightgray;
            padding: 2px 5px;
        }

        .flex-center {
            display: flex;
            align-items: center;
        }

        .control {
            font-size: 1rem;
            line-height: 1.5;
            color: #495057;
            border: 1px solid #ced4da;
            border-radius: 3px;
            padding: .15rem;
        }

        .grid-container {
            display: grid;
            grid-template-columns: 1fr 3fr;
            row-gap: 5px;
        }

        .item {
            display: flex;
            justify-items: center;
        }

        .settings-container {
            display: flex;
            gap: 2rem;
            flex-wrap: wrap;
        }

        .options-container {
            display: flex;
            gap: .4rem;
            flex-flow: column;
        }

        .button-section {
            display: flex;
            align-items: center;
            gap: 5px;
            justify-content: flex-end;
            flex-wrap: wrap;
        }

        #loggerEnabled:not(:checked) ~ #loggerLevel,
        #useHotspotSsid[type=checkbox]:not(:checked) + label + input {
            color: darkgray;
            pointer-events: none;
        }

    </style>
</head>
<body>
<div class="header">
    <div class="flex-center">
        <div class="flex-center" style="width: max-content">
            <h2 class="title">%PAGE_TITLE%</h2>
            <h2>&nbsp|&nbsp</h2>
            <h4 class="subtitle">%PAGE_SECTION%</h4>
        </div>
    </div>
    <div class="button-section">
        <button type="button" class="save-btn" onclick="saveSettings()">Save</button>
        <button type="button" onclick="restart()">Restart</button>
    </div>
</div>
<div class="main">
    <div class="setup-container">
        <div class="settings-container">

            <div class="grid-container">
                <label class="item" for="networks"></label>
                <select class="item control" id="networks"></select>

                <label class="item" for="wifiSsid">SSID</label>
                <input id="wifiSsid" class="item control">

                <label class="item" for="wifiPass">PASSWORD</label>
                <input id="wifiPass" type="password" class="item control">

                <label class="item" for="wiFiMode">WiFi mode</label>
                <select class="item control" id="wiFiMode">
                    <option value="0">Auto</option>
                    <option value="1">11B</option>
                    <option value="2">11G</option>
                    <option value="3">11N</option>
                </select>
            </div>

            <div class="options-container">
                <div class="item">
                    <input type="checkbox" id="useHotspotSsid">
                    <label class="mr-m" for="useHotspotSsid">Custom hotspot SSID</label>
                    <input maxlength="32" type="text" id="hotspotSsid" class="control">
                </div>

                <div class="flex-center">
                    <input type="checkbox" id="clientWebAccess">
                    <label for="clientWebAccess">Web Access on client mode</label>
                </div>
                <div class="flex-center">
                    <input type="checkbox" id="enableOtaUpdate">
                    <label for="enableOtaUpdate">FW Update on client mode</label>
                </div>
                <div class="item">
                    <input type="checkbox" id="loggerEnabled">
                    <label for="loggerEnabled" class="mr-m">Logger</label>
                    <select class="control" style="padding: 0; font-size: .75rem; width: 120px;" id="loggerLevel">
                        <option value="0">Serial & Local</option>
                        <option value="1">Serial (115200 8-N-1)</option>
                        <option value="2">Local log (/logs)</option>
                    </select>
                </div>
            </div>
        </div>
    </div>

    <div class="mt-l">
        <div class="flex-center mt-l">
            <label for="ledFlowDirection" class="mr-m">LED Flow Direction</label>
            <select class="control" style="padding: 0; font-size: .75rem; width: 120px;" id="ledFlowDirection">
                <option value="0">TRBL</option>
                <option value="1">RBLT</option>
                <option value="2">BTLR</option>
                <option value="3">LTRB</option>
            </select>
        </div>
        <div class="flex-center mt-l">
            <label class="item" for="ledCount">LED count</label>
            <input type="number" min="1" class="control" id="ledCount">
        </div>
        <div class="flex-center mt-l">
            <label class="item" for="saveLastState">Restore Last State After Reload</label>
            <input type="checkbox" class="control" id="saveLastState">
        </div>

    </div>
</div>
<div class="footer">
    <span>%VERSION%</span>
</div>
</body>
</html>

<script>
    const SETTINGS_URL = '/settings';
    const NETWORKS_URL = '/networks';
    const RESTART_URL = '/restart';
    const UPDATE_URL = '/update';
    const STARTUP_MODE_URL = '/deviceMode';

    const networkSelectCtrl = document.getElementById('networks');

    const checkboxSettings = ['useHotspotSsid', 'clientWebAccess', 'enableOtaUpdate', 'loggerEnabled', 'saveLastState'];
    const optionSettings = ['loggerLevel', 'ledFlowDirection', 'wiFiMode'];
    const inputsSettings = ['wifiSsid', 'wifiPass', 'hotspotSsid', 'ledCount'];

    networkSelectCtrl.addEventListener('change', () => {
        document.getElementById('wifiSsid').value = networkSelectCtrl.value;
    })

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
            let mode = 'unknown';
            if (error || !data?.result) {
                console.log(error);
            } else {
                const {deviceMode} = data.result;
                mode = deviceMode;
            }
            httpRequest('GET', SETTINGS_URL, null, (error, data) => {
                if (error || !data?.result) {
                    console.log(error);
                } else {
                    const settings = data.result;
                    for (let key in settings) {
                        if (checkboxSettings.some(k => k === key)) {
                            document.getElementById(key).checked = settings[key];
                        } else if (optionSettings.some(k => k === key)) {
                            document.getElementById(key).selectedIndex = settings[key];
                        } else if (inputsSettings.some(k => k === key)) {
                            document.getElementById(key).value = settings[key];
                        }
                    }
                    if (mode === 'SETUP' || settings.enableOtaUpdate) {
                        let otaUpdateBtn = document.createElement("button");
                        otaUpdateBtn.innerText = "FW Update";
                        otaUpdateBtn.addEventListener('click', () => { location.href = UPDATE_URL })
                        document.querySelector(".button-section").append(otaUpdateBtn);
                    }
                }
            })
        });
    }

    function getNetworkList() {
        console.log("Get WiFi list");
        httpRequest('GET', NETWORKS_URL, null, (error, data) => {
            if (error || !data?.result) {
                console.error(error);
            } else {
                const networks = data.result;
                networkSelectCtrl.innerHTML = "";
                const defaultOption = document.createElement("option");
                defaultOption.text = "--Available networks--";
                defaultOption.value = "";
                networkSelectCtrl.appendChild(defaultOption);
                networks.forEach(network => {
                    let option = document.createElement("option");
                    option.text = network;
                    option.value = network;
                    networkSelectCtrl.appendChild(option);
                })
            }
        })
    }

    function saveSettings() {
        console.log("Save device settings");
        let data = {};
        checkboxSettings.forEach(value => {
            Object.assign(data, {[value]: Number(document.getElementById(value).checked)})
        });
        optionSettings.forEach(value => {
            Object.assign(data, {[value]: document.getElementById(value).selectedIndex})
        });
        inputsSettings.forEach(value => {
            Object.assign(data, {[value]: document.getElementById(value).value})
        });

        httpRequest('POST', SETTINGS_URL, JSON.stringify({"configuration": data, "data": null}), (error, _) => {
            error ? window.alert(error) : onSettingsSaved();
        })
        console.log(data);
    }

    function restart() {
        console.log("Restart device");
        if (window.confirm("restart device?")) {
            httpRequest('GET', RESTART_URL, null, (error, _) => {
                error ? window.alert(error) : window.alert('Success');
            });
        }
    }

    function onSettingsSaved() {
        window.alert('Saved');
        console.log("Reload page to update settings...");
        document.location.reload();
    }

    getBaseSettings()
    getNetworkList()
</script>
)rawliteral";

#endif //EVENT_BUTTON_HTML_PAGE_HPP
