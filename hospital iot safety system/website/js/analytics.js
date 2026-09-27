import {
    database
} from "./firebase-config.js";

import {
    ref,
    onValue
} from "https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js";


import {
    getOverallStatus
} from "./app.js";


const resultsRef = ref(
    database,
    "hospital/room01/ml_results"
);


const trendContext =
    document
        .getElementById("trendChart")
        .getContext("2d");


const alertContext =
    document
        .getElementById("alertChart")
        .getContext("2d");


const trendChart =
    new Chart(
        trendContext,
        {

            type: "line",

            data: {

                labels: [],

                datasets: [

                    {
                        label: "Temperature",
                        data: [],
                        borderColor: "#3b82f6",
                        tension: .4
                    },

                    {
                        label: "Smoke",
                        data: [],
                        borderColor: "#06b6d4",
                        tension: .4
                    },

                    {
                        label: "Fire",
                        data: [],
                        borderColor: "#ef4444",
                        tension: .4
                    },

                    {
                        label: "Vibration",
                        data: [],
                        borderColor: "#8b5cf6",
                        tension: .4
                    }

                ]

            },

            options: {

                responsive: true,

                maintainAspectRatio: false,

                scales: {

                    y: {
                        min: 0,
                        max: 1
                    }

                }

            }

        }
    );


const alertChart =
    new Chart(
        alertContext,
        {

            type: "doughnut",

            data: {

                labels: [
                    "Fire",
                    "Smoke",
                    "Vibration"
                ],

                datasets: [

                    {

                        data: [
                            0,
                            0,
                            0
                        ],

                        backgroundColor: [
                            "#ef4444",
                            "#06b6d4",
                            "#8b5cf6"
                        ],

                        borderWidth: 0

                    }

                ]

            },

            options: {

                responsive: true,

                maintainAspectRatio: false

            }

        }
    );


onValue(
    resultsRef,
    snapshot => {

        const data =
            snapshot.val();


        if (!data) {
            return;
        }


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


        document.getElementById(
            "total-readings"
        ).textContent =
            records.length;


        let alerts = 0;

        let normal = 0;


        let fireCount = 0;

        let smokeCount = 0;

        let vibrationCount = 0;


        let tempTotal = 0;

        let smokeTotal = 0;

        let vibrationTotal = 0;


        records.forEach(
            item => {

                const predictions =
                    item.random_forest_predictions
                    || {};


                if (
                    Number(
                        predictions.FIRE_ALERT || 0
                    )
                ) {

                    fireCount++;

                }


                if (
                    Number(
                        predictions.SMOKE_ALERT || 0
                    )
                ) {

                    smokeCount++;

                }


                if (
                    Number(
                        predictions.VIBRATION_ALERT || 0
                    )
                ) {

                    vibrationCount++;

                }


                if (
                    getOverallStatus(item)
                    ===
                    "NORMAL"
                ) {

                    normal++;

                } else {

                    alerts++;

                }


                const values =
                    item.input_values || {};


                tempTotal +=
                    Number(
                        values.temperature || 0
                    );


                smokeTotal +=
                    Number(
                        values.smoke || 0
                    );


                vibrationTotal +=
                    Number(
                        values.vibration || 0
                    );

            }
        );


        document.getElementById(
            "total-alerts"
        ).textContent =
            alerts;


        document.getElementById(
            "normal-readings"
        ).textContent =
            normal;


        if (records.length > 0) {

            document.getElementById(
                "avg-temp"
            ).textContent =
                (
                    tempTotal /
                    records.length *
                    100
                ).toFixed(1) + "%";


            document.getElementById(
                "avg-smoke"
            ).textContent =
                (
                    smokeTotal /
                    records.length *
                    100
                ).toFixed(1) + "%";


            document.getElementById(
                "avg-vibration"
            ).textContent =
                (
                    vibrationTotal /
                    records.length *
                    100
                ).toFixed(1) + "%";

        }


        const chartRecords =
            records.slice(-50);


        trendChart.data.labels =
            chartRecords.map(
                item =>
                    item.reading_id || item.id
            );


        trendChart.data.datasets[0].data =
            chartRecords.map(
                item =>
                    item.input_values?.temperature || 0
            );


        trendChart.data.datasets[1].data =
            chartRecords.map(
                item =>
                    item.input_values?.smoke || 0
            );


        trendChart.data.datasets[2].data =
            chartRecords.map(
                item =>
                    item.input_values?.fire || 0
            );


        trendChart.data.datasets[3].data =
            chartRecords.map(
                item =>
                    item.input_values?.vibration || 0
            );


        trendChart.update();


        alertChart.data.datasets[0].data = [

            fireCount,

            smokeCount,

            vibrationCount

        ];


        alertChart.update();

    }
);
