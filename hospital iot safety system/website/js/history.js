
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
   FIREBASE REFERENCE
============================================================ */

const resultsRef = ref(
    database,
    "hospital/room01/ml_results"
);


/* ============================================================
   DATA
============================================================ */

let records = [];


/* ============================================================
   DOM ELEMENTS
============================================================ */

const searchInput =
    document.getElementById("search");

const statusFilter =
    document.getElementById("status-filter");

const refreshButton =
    document.getElementById("refresh");

const historyTable =
    document.getElementById("history-table");


/* ============================================================
   FORMAT DATE & TIME
   Example:
   27 Sep 2026, 08:05:20 PM
============================================================ */

function formatTimestamp(timestamp) {

    if (
        timestamp === null ||
        timestamp === undefined ||
        timestamp === ""
    ) {
        return "-";
    }


    let date;


    /*
        Handle Unix timestamps
    */

    if (
        typeof timestamp === "number" ||
        !isNaN(Number(timestamp))
    ) {

        let value = Number(timestamp);


        /*
            Unix seconds → milliseconds
        */

        if (value < 10000000000) {
            value *= 1000;
        }


        date = new Date(value);

    } else {

        /*
            Handle ISO/date strings
        */

        date = new Date(timestamp);

    }


    /*
        Invalid date
    */

    if (isNaN(date.getTime())) {

        return escapeHTML(
            String(timestamp)
        );

    }


    /*
        Indian date and time format
    */

    return date.toLocaleString(
        "en-IN",
        {
            day: "2-digit",
            month: "short",
            year: "numeric",
            hour: "2-digit",
            minute: "2-digit",
            second: "2-digit",
            hour12: true
        }
    );

}


/* ============================================================
   GET TIMESTAMP
============================================================ */

function getTimestamp(item) {

    return (
        item.source_timestamp ||
        item.timestamp ||
        item.created_at ||
        item.createdAt ||
        null
    );

}


/* ============================================================
   GET TIMESTAMP FOR SORTING
============================================================ */

function getTimestampValue(item) {

    const timestamp =
        getTimestamp(item);


    if (!timestamp) {
        return 0;
    }


    let value;


    if (
        typeof timestamp === "number" ||
        !isNaN(Number(timestamp))
    ) {

        value = Number(timestamp);


        if (value < 10000000000) {
            value *= 1000;
        }

    } else {

        value =
            new Date(timestamp).getTime();

    }


    return isNaN(value)
        ? 0
        : value;

}


/* ============================================================
   GET READING ID
============================================================ */

function getReadingId(item) {

    return String(
        item.reading_id ||
        item.id ||
        "-"
    );

}


/* ============================================================
   EMPTY STATE
============================================================ */

function renderEmpty(message) {

    historyTable.innerHTML = `

        <tr>

            <td
                colspan="7"
                class="empty"
            >
                ${escapeHTML(message)}
            </td>

        </tr>

    `;

}


/* ============================================================
   RENDER TABLE
============================================================ */

function render() {

    if (!historyTable) {
        return;
    }


    const search =
        searchInput
            ? searchInput.value
                .trim()
                .toLowerCase()
            : "";


    const filter =
        statusFilter
            ? statusFilter.value
            : "ALL";


    /* ========================================================
       FILTER DATA
    ======================================================== */

    const filtered =
        records.filter(
            item => {

                const id =
                    getReadingId(item)
                        .toLowerCase();


                const status =
                    getOverallStatus(item);


                const matchesSearch =
                    id.includes(search);


                const matchesStatus =

                    filter === "ALL"

                    ||

                    (
                        filter === "NORMAL" &&
                        status === "NORMAL"
                    )

                    ||

                    (
                        filter === "ALERT" &&
                        status !== "NORMAL"
                    );


                return (
                    matchesSearch &&
                    matchesStatus
                );

            }
        );


    /* ========================================================
       NO DATA
    ======================================================== */

    if (!filtered.length) {

        renderEmpty(
            search || filter !== "ALL"
                ? "No readings match your filters."
                : "No historical readings found."
        );

        return;
    }


    /* ========================================================
       SORT NEWEST FIRST
    ======================================================== */

    const sorted =
        [...filtered].sort(
            (a, b) =>
                getTimestampValue(b) -
                getTimestampValue(a)
        );


    /* ========================================================
       CREATE TABLE
    ======================================================== */

    historyTable.innerHTML =

        sorted
            .map(item => {

                const values =
                    item.input_values || {};


                const status =
                    getOverallStatus(item);


                const statusClass =
                    status === "NORMAL"
                        ? "normal"
                        : "alert";


                const readingId =
                    getReadingId(item);


                const timestamp =
                    formatTimestamp(
                        getTimestamp(item)
                    );


                return `

                    <tr>

                        <td>
                            <strong>
                                ${escapeHTML(readingId)}
                            </strong>
                        </td>


                        <td>
                            ${timestamp}
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
                                class="sensor-status ${statusClass}"
                            >
                                ${escapeHTML(status)}
                            </span>

                        </td>

                    </tr>

                `;

            })
            .join("");

}


/* ============================================================
   FIREBASE REAL-TIME DATA
============================================================ */

onValue(
    resultsRef,

    snapshot => {

        const data =
            snapshot.val();


        if (!data) {

            records = [];

            render();

            return;
        }


        records =
            Object.entries(data)
                .map(
                    ([id, value]) => ({

                        id,

                        ...(value || {})

                    })
                );


        render();

    },

    error => {

        console.error(
            "Firebase history error:",
            error
        );


        renderEmpty(
            "Unable to load reading history."
        );

    }
);


/* ============================================================
   SEARCH
============================================================ */

if (searchInput) {

    searchInput.addEventListener(
        "input",
        render
    );

}


/* ============================================================
   STATUS FILTER
============================================================ */

if (statusFilter) {

    statusFilter.addEventListener(
        "change",
        render
    );

}


/* ============================================================
   REFRESH
============================================================ */

if (refreshButton) {

    refreshButton.addEventListener(
        "click",
        () => {

            refreshButton.disabled = true;

            const originalText =
                refreshButton.textContent;


            refreshButton.textContent =
                "Refreshing...";


            render();


            setTimeout(
                () => {

                    refreshButton.disabled =
                        false;

                    refreshButton.textContent =
                        originalText;

                },
                500
            );

        }
    );

}


/* ============================================================
   INITIAL RENDER
============================================================ */

render();

