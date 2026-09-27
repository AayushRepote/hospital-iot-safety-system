import {
    database
} from "./firebase-config.js";

import {
    ref,
    onValue
} from "https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js";

import {
    formatValue,
    getOverallStatus,
    escapeHTML
} from "./app.js";


/* ============================================================
   FIREBASE REFERENCES
============================================================ */

const readingsRef = ref(
    database,
    "hospital/room01/readings"
);

const statusRef = ref(
    database,
    "hospital/room01/ml_status"
);

const resultsRef = ref(
    database,
    "hospital/room01/ml_results"
);


/* ============================================================
   TIME FORMAT
   Converts Firebase UTC timestamp to Indian date/time
============================================================ */

function formatDateTime(timestamp) {

    if (!timestamp) {
        return "-";
    }

    const date = new Date(timestamp);

    // Check whether the timestamp is valid
    if (isNaN(date.getTime())) {
        return timestamp;
    }

    return date.toLocaleString("en-IN", {

        day: "2-digit",

        month: "2-digit",

        year: "numeric",

        hour: "2-digit",

        minute: "2-digit",

        second: "2-digit",

        hour12: true

    });
}


/* ============================================================
   CHART
============================================================ */

let chart;

const ctx =
    document
        .getElementById("sensorChart")
        .getContext("2d");


chart = new Chart(
    ctx,
    {

        type: "line",

        data: {

            labels: [],

            datasets: [

                {
                    label: "Temperature",

                    data: [],

                    borderColor: "#0f9d92",

                    backgroundColor:
                        "rgba(15,157,146,.10)",

                    tension: 0.4,

                    borderWidth: 2,

                    pointRadius: 3
                },

                {
                    label: "Smoke",

                    data: [],

                    borderColor: "#18b8c9",

                    backgroundColor:
                        "rgba(24,184,201,.08)",

                    tension: 0.4,

                    borderWidth: 2,

                    pointRadius: 3
                },

                {
                    label: "Fire",

                    data: [],

                    borderColor: "#e5484d",

                    backgroundColor:
                        "rgba(229,72,77,.08)",

                    tension: 0.4,

                    borderWidth: 2,

                    pointRadius: 3
                },

                {
                    label: "Vibration",

                    data: [],

                    borderColor: "#7c5ce5",

                    backgroundColor:
                        "rgba(124,92,229,.08)",

                    tension: 0.4,

                    borderWidth: 2,

                    pointRadius: 3
                }

            ]

        },


        options: {

            responsive: true,

            maintainAspectRatio: false,


            scales: {

                y: {

                    min: 0,

                    max: 1,

                    ticks: {

                        color: "#71858d"

                    },

                    grid: {

                        color:
                            "rgba(15,118,110,.08)"

                    }

                },


                x: {

                    ticks: {

                        color: "#71858d"

                    },

                    grid: {

                        color:
                            "rgba(15,118,110,.06)"

                    }

                }

            },


            plugins: {

                legend: {

                    labels: {

                        color: "#405a62"

                    }

                }

            }

        }

    }
);


/* ============================================================
   SENSOR DISPLAY
============================================================ */

function setSensor(
    name,
    value,
    prediction
) {

    document
        .getElementById(name)
        .textContent =
        formatValue(value);


    const status =
        document.getElementById(
            `${name}-status`
        );


    const isAlert =
        Number(prediction) === 1;


    status.textContent =
        isAlert
            ? "ALERT"
            : "NORMAL";


    status.className =
        `sensor-status ${
            isAlert
                ? "alert"
                : "normal"
        }`;

}


/* ============================================================
   LIVE SENSOR STATUS
============================================================ */

onValue(
    statusRef,
    snapshot => {

        const data =
            snapshot.val();


        if (!data) {
            return;
        }


        const values =
            data.input_values || {};


        const predictions =
            data.random_forest_predictions || {};


        /* Temperature */

        setSensor(
            "temperature",
            values.temperature,
            predictions.FIRE_ALERT
        );


        /* Smoke */

        setSensor(
            "smoke",
            values.smoke,
            predictions.SMOKE_ALERT
        );


        /* Fire */

        setSensor(
            "fire",
            values.fire,
            predictions.FIRE_ALERT
        );


        /* Vibration */

        setSensor(
            "vibration",
            values.vibration,
            predictions.VIBRATION_ALERT
        );


        /* Alert counts */

        document.getElementById(
            "fire-alert-count"
        ).textContent =
            predictions.FIRE_ALERT || 0;


        document.getElementById(
            "smoke-alert-count"
        ).textContent =
            predictions.SMOKE_ALERT || 0;


        document.getElementById(
            "vibration-alert-count"
        ).textContent =
            predictions.VIBRATION_ALERT || 0;

    }
);


/* ============================================================
   READINGS / TABLE / CHART
============================================================ */

onValue(
    resultsRef,
    snapshot => {

        const data =
            snapshot.val();


        if (!data) {
            return;
        }


        /* Convert Firebase object into array */

        const records =
            Object.entries(data)
                .map(
                    ([id, value]) => ({
                        id,
                        ...value
                    })
                )
                .sort(
                    (a, b) =>
                        Number(
                            a.processed_timestamp || 0
                        )
                        -
                        Number(
                            b.processed_timestamp || 0
                        )
                );


        /* Get latest 20 readings */

        const recent =
            records.slice(-20);


        /* ====================================================
           CHART LABELS
        ==================================================== */

        chart.data.labels =
            recent.map(
                item =>
                    item.reading_id || item.id
            );


        /* Temperature */

        chart.data.datasets[0].data =
            recent.map(
                item =>
                    item.input_values?.temperature || 0
            );


        /* Smoke */

        chart.data.datasets[1].data =
            recent.map(
                item =>
                    item.input_values?.smoke || 0
            );


        /* Fire */

        chart.data.datasets[2].data =
            recent.map(
                item =>
                    item.input_values?.fire || 0
            );


        /* Vibration */

        chart.data.datasets[3].data =
            recent.map(
                item =>
                    item.input_values?.vibration || 0
            );


        chart.update();


        /* ====================================================
           RECENT READINGS TABLE
        ==================================================== */

        const table =
            document.getElementById(
                "recent-table"
            );


        /* Latest 5 readings */

        const latest =
            [...recent]
                .reverse()
                .slice(0, 5);


        table.innerHTML =
            latest
                .map(
                    item => {

                        const values =
                            item.input_values || {};


                        /* Get overall status */

                        const status =
                            getOverallStatus(
                                item
                            );


                        const cls =
                            status.includes("ALERT")
                                ? "alert"
                                : "normal";


                        /* Format Firebase timestamp */

                        const formattedTime =
                            formatDateTime(
                                item.source_timestamp
                            );


                        return `

                        <tr>

                            <td>
                                ${escapeHTML(
                                    item.reading_id ||
                                    item.id
                                )}
                            </td>


                            <td>
                                ${escapeHTML(
                                    formattedTime
                                )}
                            </td>


                            <td>
                                ${formatValue(
                                    values.temperature
                                )}
                            </td>


                            <td>
                                ${formatValue(
                                    values.smoke
                                )}
                            </td>


                            <td>
                                ${formatValue(
                                    values.fire
                                )}
                            </td>


                            <td>
                                ${formatValue(
                                    values.vibration
                                )}
                            </td>


                            <td>

                                <span
                                    class="sensor-status ${cls}"
                                >
                                    ${status}
                                </span>

                            </td>

                        </tr>

                        `;

                    }
                )
                .join("");

    }
);
