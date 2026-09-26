// =====================================================
// ESP32 ADDRESS
// =====================================================

const ESP32_IP = "192.168.1.105";


// =====================================================
// SENSOR API
// =====================================================

const API_URL =
    "http://" +
    ESP32_IP +
    "/api/data";


// =====================================================
// EVENT HISTORY
// =====================================================

let events = [];

let previousFire = false;

let previousSmoke = false;

let previousVibration = false;

let previousAlarm = false;


// =====================================================
// CHART DATA
// =====================================================

let temperatureHistory = [];

let smokeHistory = [];


// =====================================================
// PAGE NAVIGATION
// =====================================================

function showPage(
    page,
    button
) {

    document
        .querySelectorAll(".page")
        .forEach(
            p => p.classList.remove("active")
        );

    document
        .getElementById(page)
        .classList.add("active");


    document
        .querySelectorAll(".navButton")
        .forEach(
            b => b.classList.remove("active")
        );

    button.classList.add("active");
}


// =====================================================
// ADD EVENT
// =====================================================

function addEvent(message) {

    const now =
        new Date().toLocaleTimeString();

    events.push({
        time: now,
        message: message
    });


    if (events.length > 30) {
        events.shift();
    }


    updateEventTable();
}


// =====================================================
// EVENT TABLE
// =====================================================

function updateEventTable() {

    const table =
        document.getElementById(
            "eventList"
        );


    table.innerHTML = "";


    if (events.length === 0) {

        table.innerHTML =
            "<tr>" +
            "<td colspan='2'>" +
            "No emergency events." +
            "</td>" +
            "</tr>";

        return;
    }


    events
        .slice()
        .reverse()
        .forEach(event => {

            const row =
                document.createElement("tr");

            row.innerHTML =
                "<td>" +
                event.time +
                "</td>" +

                "<td>" +
                event.message +
                "</td>";

            table.appendChild(row);
        });
}


// =====================================================
// UPDATE DATA
// =====================================================

async function updateData() {

    try {

        const response =
            await fetch(API_URL);


        if (!response.ok) {
            throw new Error("ESP32 error");
        }


        const data =
            await response.json();


        // Connection

        document.getElementById(
            "connection"
        ).textContent =
            "● ESP32 Connected";

        document.getElementById(
            "connection"
        ).style.color =
            "#16a34a";


        // =================================================
        // TEMPERATURE
        // =================================================

        const temp =
            Number(data.temperature);


        document.getElementById(
            "temperature"
        ).textContent =
            temp.toFixed(1) + " °C";


        document.getElementById(
            "liveTemperature"
        ).textContent =
            temp.toFixed(1) + " °C";


        temperatureHistory.push(temp);


        if (
            temperatureHistory.length > 40
        ) {
            temperatureHistory.shift();
        }


        // =================================================
        // FIRE
        // =================================================

        if (data.fire) {

            document.getElementById(
                "fire"
            ).textContent =
                "🔥 DETECTED";


            document.getElementById(
                "fire"
            ).className =
                "dangerText";


            document.getElementById(
                "liveFire"
            ).textContent =
                "🔥 DETECTED";


            document.getElementById(
                "liveFire"
            ).className =
                "largeValue dangerText";


            if (!previousFire) {
                addEvent("🔥 Fire detected");
            }

        } else {

            document.getElementById(
                "fire"
            ).textContent =
                "SAFE";


            document.getElementById(
                "fire"
            ).className =
                "safeText";


            document.getElementById(
                "liveFire"
            ).textContent =
                "SAFE";


            document.getElementById(
                "liveFire"
            ).className =
                "largeValue safeText";
        }


        // =================================================
        // SMOKE
        // =================================================

        document.getElementById(
            "smokeValue"
        ).textContent =
            data.smoke;


        smokeHistory.push(
            Number(data.smoke)
        );


        if (
            smokeHistory.length > 40
        ) {
            smokeHistory.shift();
        }


        if (data.smokeDetected) {

            document.getElementById(
                "smoke"
            ).textContent =
                "💨 DETECTED";


            document.getElementById(
                "smoke"
            ).className =
                "dangerText";


            document.getElementById(
                "liveSmoke"
            ).textContent =
                "💨 DETECTED";


            document.getElementById(
                "liveSmoke"
            ).className =
                "largeValue dangerText";


            if (!previousSmoke) {
                addEvent("💨 Smoke detected");
            }

        } else {

            document.getElementById(
                "smoke"
            ).textContent =
                "SAFE";


            document.getElementById(
                "smoke"
            ).className =
                "safeText";


            document.getElementById(
                "liveSmoke"
            ).textContent =
                "SAFE";


            document.getElementById(
                "liveSmoke"
            ).className =
                "largeValue safeText";
        }


        // =================================================
        // VIBRATION
        // =================================================

        if (data.vibration) {

            document.getElementById(
                "vibration"
            ).textContent =
                "📳 DETECTED";


            document.getElementById(
                "vibration"
            ).className =
                "dangerText";


            document.getElementById(
                "liveVibration"
            ).textContent =
                "📳 DETECTED";


            document.getElementById(
                "liveVibration"
            ).className =
                "largeValue dangerText";


            if (!previousVibration) {
                addEvent(
                    "📳 Vibration detected"
                );
            }

        } else {

            document.getElementById(
                "vibration"
            ).textContent =
                "NORMAL";


            document.getElementById(
                "vibration"
            ).className =
                "safeText";


            document.getElementById(
                "liveVibration"
            ).textContent =
                "NORMAL";


            document.getElementById(
                "liveVibration"
            ).className =
                "largeValue safeText";
        }


        // =================================================
        // ALARM
        // =================================================

        const statusBox =
            document.getElementById(
                "statusBox"
            );


        if (data.alarm) {

            statusBox.className =
                "statusBox danger";


            document.getElementById(
                "systemStatus"
            ).textContent =
                "🚨 EMERGENCY DETECTED";


            document.getElementById(
                "statusMessage"
            ).textContent =
                "One or more sensors detected an abnormal condition.";


            if (!previousAlarm) {
                addEvent(
                    "🚨 Emergency alarm activated"
                );
            }

        } else {

            statusBox.className =
                "statusBox safe";


            document.getElementById(
                "systemStatus"
            ).textContent =
                "🟢 SYSTEM NORMAL";


            document.getElementById(
                "statusMessage"
            ).textContent =
                "All sensors are operating normally.";
        }


        // =================================================
        // SYSTEM
        // =================================================

        document.getElementById(
            "ip"
        ).textContent =
            data.ip;


        document.getElementById(
            "systemIP"
        ).textContent =
            data.ip;


        document.getElementById(
            "uptime"
        ).textContent =
            formatUptime(
                data.uptime
            );


        // Save previous states

        previousFire =
            data.fire;

        previousSmoke =
            data.smokeDetected;

        previousVibration =
            data.vibration;

        previousAlarm =
            data.alarm;


        // Charts

        drawTemperatureChart();

        drawSmokeChart();

    }

    catch(error) {

        console.log(error);


        document.getElementById(
            "connection"
        ).textContent =
            "● ESP32 Disconnected";


        document.getElementById(
            "connection"
        ).style.color =
            "#dc2626";
    }
}


// =====================================================
// UPTIME
// =====================================================

function formatUptime(seconds) {

    seconds =
        Number(seconds);


    const hours =
        Math.floor(
            seconds / 3600
        );


    const minutes =
        Math.floor(
            (seconds % 3600) / 60
        );


    const sec =
        seconds % 60;


    return (
        hours +
        "h " +
        minutes +
        "m " +
        sec +
        "s"
    );
}


// =====================================================
// TEMPERATURE CHART
// =====================================================

function drawTemperatureChart() {

    const canvas =
        document.getElementById(
            "temperatureChart"
        );


    const ctx =
        canvas.getContext("2d");


    const width =
        canvas.width =
        canvas.parentElement.clientWidth;


    const height =
        canvas.height =
        250;


    ctx.clearRect(
        0,
        0,
        width,
        height
    );


    drawGrid(
        ctx,
        width,
        height
    );


    drawLine(
        ctx,
        temperatureHistory,
        "#2563eb",
        60
    );
}


// =====================================================
// SMOKE CHART
// =====================================================

function drawSmokeChart() {

    const canvas =
        document.getElementById(
            "smokeChart"
        );


    const ctx =
        canvas.getContext("2d");


    const width =
        canvas.width =
        canvas.parentElement.clientWidth;


    const height =
        canvas.height =
        250;


    ctx.clearRect(
        0,
        0,
        width,
        height
    );


    drawGrid(
        ctx,
        width,
        height
    );


    drawLine(
        ctx,
        smokeHistory,
        "#f97316",
        4095
    );
}


// =====================================================
// GRID
// =====================================================

function drawGrid(
    ctx,
    width,
    height
) {

    ctx.strokeStyle =
        "#e2e8f0";


    for (
        let i = 0;
        i <= 5;
        i++
    ) {

        const y =
            height / 5 * i;


        ctx.beginPath();

        ctx.moveTo(
            0,
            y
        );

        ctx.lineTo(
            width,
            y
        );

        ctx.stroke();
    }
}


// =====================================================
// LINE
// =====================================================

function drawLine(
    ctx,
    data,
    color,
    maxValue
) {

    if (
        data.length < 2
    ) {
        return;
    }


    const width =
        ctx.canvas.width;


    const height =
        ctx.canvas.height;


    ctx.strokeStyle =
        color;


    ctx.lineWidth = 3;


    ctx.beginPath();


    data.forEach(
        (value, index) => {

            const x =
                index *
                width /
                (data.length - 1);


            const y =
                height -
                (
                    value /
                    maxValue
                ) *
                height;


            if (index === 0) {

                ctx.moveTo(
                    x,
                    y
                );

            } else {

                ctx.lineTo(
                    x,
                    y
                );
            }
        }
    );


    ctx.stroke();
}


// =====================================================
// START
// =====================================================

updateData();


setInterval(
    updateData,
    1000
);