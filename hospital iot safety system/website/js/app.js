// js/app.js

export function formatValue(value) {

    if (value === undefined || value === null) {
        return "--";
    }

    return (Number(value) * 100).toFixed(1) + "%";
}


export function getStatus(prediction) {

    return Number(prediction) === 1
        ? "ALERT"
        : "NORMAL";
}


export function statusClass(prediction) {

    return Number(prediction) === 1
        ? "alert"
        : "normal";
}


export function getOverallStatus(data) {

    if (!data) {
        return "NO DATA";
    }


    const predictions =
        data.random_forest_predictions || {};


    const alerts =

        Number(predictions.FIRE_ALERT || 0)

        +

        Number(predictions.SMOKE_ALERT || 0)

        +

        Number(predictions.VIBRATION_ALERT || 0);


    if (alerts === 0) {
        return "NORMAL";
    }

    if (alerts === 1) {
        return "SINGLE ALERT";
    }

    return "MULTIPLE ALERT";
}


export function escapeHTML(value) {

    if (value === undefined || value === null) {
        return "";
    }

    return String(value)

        .replaceAll("&", "&amp;")

        .replaceAll("<", "&lt;")

        .replaceAll(">", "&gt;")

        .replaceAll('"', "&quot;")

        .replaceAll("'", "&#039;");
}
