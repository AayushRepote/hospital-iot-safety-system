import {
    database
} from "./firebase-config.js";

import {
    ref,
    onValue
} from "https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js";

import {
    getOverallStatus,
    escapeHTML
} from "./app.js";


const resultsRef = ref(
    database,
    "hospital/room01/ml_results"
);


onValue(
    resultsRef,
    snapshot => {

        const data =
            snapshot.val();


        const table =
            document.getElementById(
                "alerts-table"
            );


        if (!data) {

            table.innerHTML = `

                <tr>

                    <td
                        colspan="6"
                        class="empty"
                    >
                        No alerts found.
                    </td>

                </tr>

            `;

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
                .filter(
                    item =>
                        getOverallStatus(item)
                        !==
                        "NORMAL"
                )
                .reverse();


        if (!records.length) {

            table.innerHTML = `

                <tr>

                    <td
                        colspan="6"
                        class="empty"
                    >
                        No alerts detected.
                    </td>

                </tr>

            `;

            return;
        }


        table.innerHTML =
            records
                .map(
                    item => {

                        const p =
                            item.random_forest_predictions
                            || {};


                        return `

                        <tr>

                            <td>
                                ${escapeHTML(
                                    item.reading_id || item.id
                                )}
                            </td>

                            <td>
                                ${escapeHTML(
                                    item.source_timestamp || "-"
                                )}
                            </td>

                            <td>
                                ${p.FIRE_ALERT ? "🔥 ALERT" : "Normal"}
                            </td>

                            <td>
                                ${p.SMOKE_ALERT ? "💨 ALERT" : "Normal"}
                            </td>

                            <td>
                                ${p.VIBRATION_ALERT ? "📳 ALERT" : "Normal"}
                            </td>

                            <td>

                                <span class="sensor-status alert">
                                    ${getOverallStatus(item)}
                                </span>

                            </td>

                        </tr>

                        `;

                    }
                )
                .join("");

    }
);
