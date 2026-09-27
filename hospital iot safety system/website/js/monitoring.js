import {
    database
} from "./firebase-config.js";

import {
    ref,
    onValue
} from "https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js";

import {
    formatValue
} from "./app.js";


const statusRef = ref(
    database,
    "hospital/room01/ml_status"
);


function updateSensor(
    name,
    value,
    prediction
) {

    document.getElementById(name)
        .textContent =
        formatValue(value);


    const element =
        document.getElementById(
            `${name}-status`
        );


    const alert =
        Number(prediction) === 1;


    element.textContent =
        alert
            ? "ALERT"
            : "NORMAL";


    element.className =
        `sensor-status ${
            alert
                ? "alert"
                : "normal"
        }`;

}


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


        const scores =
            data.random_forest_scores || {};


        updateSensor(
            "temperature",
            values.temperature,
            predictions.FIRE_ALERT
        );


        updateSensor(
            "smoke",
            values.smoke,
            predictions.SMOKE_ALERT
        );


        updateSensor(
            "fire",
            values.fire,
            predictions.FIRE_ALERT
        );


        updateSensor(
            "vibration",
            values.vibration,
            predictions.VIBRATION_ALERT
        );


        document.getElementById(
            "fire-score"
        ).textContent =
            (
                Number(scores.FIRE || 0) *
                100
            ).toFixed(1) + "%";


        document.getElementById(
            "smoke-score"
        ).textContent =
            (
                Number(scores.SMOKE || 0) *
                100
            ).toFixed(1) + "%";


        document.getElementById(
            "vibration-score"
        ).textContent =
            (
                Number(scores.VIBRATION || 0) *
                100
            ).toFixed(1) + "%";


        document.getElementById(
            "last-node"
        ).textContent =
            data.last_evaluated_node || "--";

    }
);
