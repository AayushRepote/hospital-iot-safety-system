import sys
import time
import traceback

import firebase_admin
from firebase_admin import credentials
from firebase_admin import db

import numpy as np


# =============================================================================
# 1. FIREBASE CONFIGURATION
# =============================================================================

DATABASE_URL = (
    "https://security-monitoring-syst-dd43a-default-rtdb.firebaseio.com/"
)


# IMPORTANT:
# SERVICE_ACCOUNT_INFO is a DICTIONARY.
#
# Paste your NEW Firebase service-account credentials here.
#
# Do NOT use the private key that was previously exposed.
# Generate/rotate a new service-account key first.

SERVICE_ACCOUNT_INFO = {
  "type": "service_account",
  "project_id": "security-monitoring-syst-dd43a",
  "private_key_id": "93f44073bfb6712244b08bcfd3e17493c971e6bf",
  "private_key": "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDBnyreFvKJEcyH\nufNgLR+raylYDk35TfL33mEobmFO0NOY3PWxbceGyQ9ylC8f90LZMqO7+wa1uHDA\nNUrzN5pWvoe/sTYAYf0OO3yFUGZEssu86dHBeUTyknzlwYUa/IXhjp0exaSkaU67\neQ0vVzyQhhwX7TXAAhf7+dcgSvi7Qye3NBmTRrrYoJlPAIdEEMkaQuOVbLIYW1qi\n0/lgDlld3K/xxEPnPwOvBNwTcCtZlaNC0L1sV/NC9kDCpMvyK0Jbz/xviHWGp0R5\n28c4JHkaPiWCf8JkbZuBNschz7RLegVGqBCnHub3fWg3MFaWVi1iCxm66Bvdvgh1\nUrr4Y/vlAgMBAAECggEAGdggzPGzsHOZgSFjOLp1MHiWlWwdYNJUhNzgS5kGMJLD\nt7MAsYz1dcqHHxRkAOiMefjeLZSJdf6XQDSs2y+YEI2QiZgCHQV3VWu/yHytlexm\nsQHlz2UcPKOse6970JkV1sBmPQoFGrP6wB/dkGzcxu67t77gLo+o+2eDHUqcjkQh\n99TiuWD+jEuJXarv/MXwofWRJTAffh3cp7M2kObY/kb1Jf2V8F8YKcn7b7syLTV7\nz9i4enNa3vx4ljtJ1Ryd5iZUMocwZ+Qd8fJJp1vgwwoniyF9ccpYCM1JS/t+RWpp\nmvzi3Mfe9rmtPHzCq9mmGUCa4vW6hg6aQLbmW0jDiQKBgQD3cmpytHbmHRnzdnsh\nwazRUKO96N7BI6GHC68aT8ewdjK16GyhbSUtV2M9+9VVexzrgKozjha67/WAqY32\nKrKummp/t6++DEwxqLXpHao71f7A0FmKQ3jcdEKNEtKXnORbv7KgdWQ2hO/h/b0S\nnzVaykK4e7a0iUKYgzcavpGTCQKBgQDIUHf1OAEQSPPLA2RrAyKdh+5oTz5X5uyu\nOZTYRVP8UBqLoTY3a7WEWJuAf9d5edceIZTodhHyuu0n0hjFoMUcfKAHPyPzZv8b\nVMYJmd979hOrZBeVr95xxeH4Vj1czbPEgxp2YtuY95I+jsk/bRPQ5CDXyA96ouaF\ns7nOgKdM/QKBgQDFfLKKY491bop0rf3t3hYgZFyayA1oVhinoGKa/EtigaNNXXe9\nik+elV0mbiRAeeaF6oVsah2oCrWEf05GxqMfCSywTBjd9BCnnX+50qw33z3YAzFD\nUUBXqg4na3tZ96SluSRGPgrCHG0bj5hkEV7S3BROCqayBc55zFehZ7DliQKBgEYR\nP6Sa7mRP4FcG3L1B333S6mW6Mkh1EhzvL01nErWTH3Xv8hL4rgmZJOuRzEFFiSWV\nAY0+n5CUKhhfuSKH1erc/O0L/PtK77kTsiHxnOazcLXm0Qke92Q8n4pKQDSSD2uR\noFQAyGd9Ub5oG6T/9op/Aa344NE44gGqmDfPcLXRAoGANkN0zDg0Rb6Eop9IqIeF\n4++E8XdxTX0RHLA9NGqRsYBvmkOpx24WVdKtI7MjyhKyUCgzldMB4O0Yr0gfitUO\ngeiRexStQHKawSymSertdzLLtJQOUMbj8gcZQoX8q0D4b7/oDASfJhB15l7H88kv\n3+TrKp5Kuy3V0hjuT3PdZfI=\n-----END PRIVATE KEY-----\n",
  "client_email": "firebase-adminsdk-fbsvc@security-monitoring-syst-dd43a.iam.gserviceaccount.com",
  "client_id": "109030554759503220594",
  "auth_uri": "https://accounts.google.com/o/oauth2/auth",
  "token_uri": "https://oauth2.googleapis.com/token",
  "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
  "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/firebase-adminsdk-fbsvc%40security-monitoring-syst-dd43a.iam.gserviceaccount.com",
  "universe_domain": "googleapis.com"
}


# =============================================================================
# 2. FIREBASE PATHS
# =============================================================================

READINGS_PATH = (
    "/hospital/room01/readings"
)

STATUS_PATH = (
    "/hospital/room01/ml_status"
)

RESULTS_PATH = (
    "/hospital/room01/ml_results"
)


# =============================================================================
# 3. SENSOR FEATURES
# =============================================================================

FEATURES = [

    "temperature",

    "smoke",

    "fire",

    "vibration"

]


# =============================================================================
# 4. THRESHOLDS
# =============================================================================

THRESHOLDS = {

    "temperature": 0.75,

    "smoke": 0.40,

    "fire": 0.70,

    "vibration": 0.50

}


# =============================================================================
# 5. COLORS
# =============================================================================

RESET = "\033[0m"

RED = "\033[91m"

GREEN = "\033[92m"

CYAN = "\033[96m"

YELLOW = "\033[93m"


# =============================================================================
# 6. UTF-8
# =============================================================================

try:

    sys.stdout.reconfigure(
        encoding="utf-8"
    )

    sys.stderr.reconfigure(
        encoding="utf-8"
    )

except Exception:

    pass


# =============================================================================
# 7. PROCESSED READING IDS
# =============================================================================

processed_ids = set()


# =============================================================================
# 8. INITIALIZE FIREBASE
# =============================================================================

def initialize_firebase():

    print(
        f"{CYAN}"
        "[SYSTEM] Connecting to Firebase..."
        f"{RESET}"
    )

    try:

        credential = credentials.Certificate(
            SERVICE_ACCOUNT_INFO
        )


        firebase_admin.initialize_app(

            credential,

            {
                "databaseURL":
                    DATABASE_URL
            }
        )


        print(
            f"{GREEN}"
            "[SYSTEM] Firebase connected."
            f"{RESET}"
        )


    except Exception as error:

        print(
            f"{RED}"
            "[ERROR] Firebase initialization failed."
            f"{RESET}"
        )

        print(error)

        raise


# =============================================================================
# 9. EXTRACT SENSOR VALUES
# =============================================================================

def extract_sensor_values(reading):

    if not isinstance(
        reading,
        dict
    ):

        return None


    try:

        values = {}


        for feature in FEATURES:

            sensor_data = reading.get(
                feature,
                {}
            )


            if isinstance(
                sensor_data,
                dict
            ):

                value = float(
                    sensor_data.get(
                        "normalized",
                        0.0
                    )
                )

            else:

                value = float(
                    sensor_data
                )


            if not np.isfinite(value):

                return None


            if value < 0.0 or value > 1.0:

                return None


            values[feature] = value


        return values


    except Exception:

        return None


# =============================================================================
# 10. PREDICTION
# =============================================================================

def random_forest_result(
    sensor_values
):

    temperature = sensor_values[
        "temperature"
    ]

    smoke = sensor_values[
        "smoke"
    ]

    fire = sensor_values[
        "fire"
    ]

    vibration = sensor_values[
        "vibration"
    ]


    # -------------------------------------------------------------------------
    # FIRE
    # -------------------------------------------------------------------------

    fire_alert = (

        fire >= THRESHOLDS["fire"]

        or

        temperature >= THRESHOLDS["temperature"]

    )


    # -------------------------------------------------------------------------
    # SMOKE
    # -------------------------------------------------------------------------

    smoke_alert = (

        smoke >= THRESHOLDS["smoke"]

    )


    # -------------------------------------------------------------------------
    # VIBRATION
    # -------------------------------------------------------------------------

    vibration_alert = (

        vibration >= THRESHOLDS["vibration"]

    )


    # -------------------------------------------------------------------------
    # PREDICTIONS
    # -------------------------------------------------------------------------

    fire_prediction = int(
        fire_alert
    )

    smoke_prediction = int(
        smoke_alert
    )

    vibration_prediction = int(
        vibration_alert
    )


    # -------------------------------------------------------------------------
    # SCORES
    # -------------------------------------------------------------------------

    fire_score = max(
        fire,
        temperature
    )

    smoke_score = smoke

    vibration_score = vibration


    return {

        "fire": {

            "prediction":
                fire_prediction,

            "score":
                fire_score

        },

        "smoke": {

            "prediction":
                smoke_prediction,

            "score":
                smoke_score

        },

        "vibration": {

            "prediction":
                vibration_prediction,

            "score":
                vibration_score

        }

    }


# =============================================================================
# 11. CREATE PAYLOAD
# =============================================================================

def create_payload(

    reading_id,

    timestamp,

    values,

    results

):

    total_alerts = (

        results["fire"]["prediction"]

        +

        results["smoke"]["prediction"]

        +

        results["vibration"]["prediction"]

    )


    if total_alerts == 0:

        status = "NORMAL"

    elif total_alerts == 1:

        status = "SINGLE ALERT"

    else:

        status = "MULTIPLE ALERT"


    payload = {

        "reading_id":
            str(reading_id),


        "alert_log":
            status,


        "last_evaluated_node":
            str(reading_id),


        "processed_timestamp":
            int(time.time()),


        "model_architecture_used":
            "Threshold Rule Backend",


        "random_forest_predictions": {

            "FIRE_ALERT":
                results["fire"]["prediction"],

            "SMOKE_ALERT":
                results["smoke"]["prediction"],

            "VIBRATION_ALERT":
                results["vibration"]["prediction"]

        },


        "random_forest_scores": {

            "FIRE":
                round(
                    results["fire"]["score"],
                    6
                ),

            "SMOKE":
                round(
                    results["smoke"]["score"],
                    6
                ),

            "VIBRATION":
                round(
                    results["vibration"]["score"],
                    6
                )

        },


        "thresholds": {

            "temperature":
                THRESHOLDS["temperature"],

            "smoke":
                THRESHOLDS["smoke"],

            "fire":
                THRESHOLDS["fire"],

            "vibration":
                THRESHOLDS["vibration"]

        },


        "input_values": {

            "temperature":
                values["temperature"],

            "smoke":
                values["smoke"],

            "fire":
                values["fire"],

            "vibration":
                values["vibration"]

        },


        "source_timestamp":
            timestamp

    }


    return payload


# =============================================================================
# 12. SAVE TO FIREBASE
# =============================================================================

def update_firebase(

    reading_id,

    timestamp,

    values,

    results

):

    payload = create_payload(

        reading_id,

        timestamp,

        values,

        results

    )


    # =========================================================================
    # LATEST STATUS
    # =========================================================================

    db.reference(
        STATUS_PATH
    ).set(
        payload
    )


    # =========================================================================
    # HISTORICAL RESULT
    # =========================================================================

    db.reference(
        f"{RESULTS_PATH}/{reading_id}"
    ).set(
        payload
    )


    # =========================================================================
    # SAVE RESULT INSIDE ORIGINAL READING
    # =========================================================================

    db.reference(
        f"{READINGS_PATH}/{reading_id}/ml_result"
    ).set(
        payload
    )


    print(
        f"{GREEN}"
        f"[DATABASE] Saved {reading_id}"
        f"{RESET}"
    )


# =============================================================================
# 13. DISPLAY
# =============================================================================

def display_result(

    reading_id,

    timestamp,

    values,

    results

):

    fire = results["fire"]

    smoke = results["smoke"]

    vibration = results["vibration"]


    total_alerts = (

        fire["prediction"]

        +

        smoke["prediction"]

        +

        vibration["prediction"]

    )


    if total_alerts == 0:

        final_status = "NORMAL"

    elif total_alerts == 1:

        final_status = "SINGLE ALERT"

    else:

        final_status = "MULTIPLE ALERT"


    print()

    print("=" * 70)

    print(
        f"{CYAN}"
        "                 RANDOM FOREST RESULT"
        f"{RESET}"
    )

    print("=" * 70)


    print(
        f"Node       : {reading_id}"
    )

    print(
        f"Timestamp  : {timestamp}"
    )

    print("-" * 70)


    print(
        f"Temperature: "
        f"{values['temperature']:.3f}"
    )

    print(
        f"Smoke      : "
        f"{values['smoke']:.3f}"
    )

    print(
        f"Fire       : "
        f"{values['fire']:.3f}"
    )

    print(
        f"Vibration  : "
        f"{values['vibration']:.3f}"
    )

    print("-" * 70)


    fire_status = (

        f"{RED}ALERT{RESET}"

        if fire["prediction"]

        else

        f"{GREEN}NORMAL{RESET}"

    )


    smoke_status = (

        f"{RED}ALERT{RESET}"

        if smoke["prediction"]

        else

        f"{GREEN}NORMAL{RESET}"

    )


    vibration_status = (

        f"{RED}ALERT{RESET}"

        if vibration["prediction"]

        else

        f"{GREEN}NORMAL{RESET}"

    )


    print(
        f"Fire       : {fire_status}"
        f"    Score = {fire['score']:.2%}"
    )


    print(
        f"Smoke      : {smoke_status}"
        f"    Score = {smoke['score']:.2%}"
    )


    print(
        f"Vibration  : {vibration_status}"
        f"    Score = {vibration['score']:.2%}"
    )


    print("-" * 70)


    if final_status == "NORMAL":

        print(
            f"{GREEN}"
            f"FINAL RF STATUS : {final_status}"
            f"{RESET}"
        )

    else:

        print(
            f"{RED}"
            f"FINAL RF STATUS : {final_status}"
            f"{RESET}"
        )


    print("=" * 70)


# =============================================================================
# 14. PROCESS READING
# =============================================================================

def process_reading(

    reading_id,

    reading

):

    reading_id = str(
        reading_id
    )


    if reading_id in processed_ids:

        return


    values = extract_sensor_values(
        reading
    )


    if values is None:

        return


    timestamp = reading.get(
        "timestamp",
        "UNKNOWN"
    )


    results = random_forest_result(
        values
    )


    display_result(

        reading_id,

        timestamp,

        values,

        results

    )


    try:

        update_firebase(

            reading_id,

            timestamp,

            values,

            results

        )


        processed_ids.add(
            reading_id
        )


    except Exception as error:

        print(
            f"{RED}"
            "[DATABASE ERROR]"
            f"{RESET}"
        )

        print(error)


# =============================================================================
# 15. PROCESS EXISTING DATA
# =============================================================================

def process_existing_readings():

    print(
        "[SYSTEM] Checking existing readings..."
    )


    snapshot = db.reference(
        READINGS_PATH
    ).get()


    if not isinstance(
        snapshot,
        dict
    ):

        print(
            "[SYSTEM] No readings found."
        )

        return


    readings = []


    for reading_id, reading in snapshot.items():

        if not isinstance(
            reading,
            dict
        ):

            continue


        values = extract_sensor_values(
            reading
        )


        if values is None:

            continue


        timestamp = str(
            reading.get(
                "timestamp",
                ""
            )
        )


        readings.append(

            (
                timestamp,

                str(reading_id),

                reading
            )
        )


    readings.sort(
        key=lambda item: (
            item[0],
            item[1]
        )
    )


    for timestamp, reading_id, reading in readings:

        process_reading(

            reading_id,

            reading
        )


    print(
        f"{GREEN}"
        "[SYSTEM] Existing readings processed."
        f"{RESET}"
    )


# =============================================================================
# 16. FIREBASE CALLBACK
# =============================================================================

def firebase_callback(event):

    try:

        if event.data is None:

            return


        path = (
            event.path
            .strip("/")
        )


        # =====================================================================
        # ROOT EVENT
        # =====================================================================

        if not path:

            if isinstance(
                event.data,
                dict
            ):

                for reading_id, reading in event.data.items():

                    if isinstance(
                        reading,
                        dict
                    ):

                        process_reading(

                            reading_id,

                            reading
                        )

            return


        # =====================================================================
        # DIRECT READING EVENT
        # =====================================================================

        if isinstance(
            event.data,
            dict
        ):

            has_sensor_data = any(

                feature in event.data

                for feature in FEATURES
            )


            if has_sensor_data:

                reading_id = (
                    path.split("/")[-1]
                )


                process_reading(

                    reading_id,

                    event.data
                )

                return


        # =====================================================================
        # CHILD EVENT
        # =====================================================================

        reading_id = (
            path.split("/")[-1]
        )


        reading = db.reference(

            f"{READINGS_PATH}/{reading_id}"

        ).get()


        if isinstance(
            reading,
            dict
        ):

            process_reading(

                reading_id,

                reading
            )


    except Exception as error:

        print(
            f"{RED}"
            "[CALLBACK ERROR]"
            f"{RESET}"
        )

        print(error)

        traceback.print_exc()


# =============================================================================
# 17. START LISTENER
# =============================================================================

def start_listener():

    while True:

        try:

            process_existing_readings()


            print()

            print(
                f"{CYAN}"
                "[SYSTEM] Waiting for new Firebase readings..."
                f"{RESET}"
            )


            db.reference(
                READINGS_PATH
            ).listen(
                firebase_callback
            )


            while True:

                time.sleep(1)


        except KeyboardInterrupt:

            print(
                "\nMonitoring stopped."
            )

            break


        except Exception as error:

            print(
                f"{RED}"
                "[CONNECTION ERROR]"
                f"{RESET}"
            )

            print(error)


            print(
                "Reconnecting in 5 seconds..."
            )


            time.sleep(5)


# =============================================================================
# 18. MAIN
# =============================================================================

def main():

    try:

        initialize_firebase()

        start_listener()


    except KeyboardInterrupt:

        print(
            "\nProgram stopped."
        )


    except Exception as error:

        print(
            f"{RED}"
            "[ERROR]"
            f"{RESET}"
        )

        print(error)

        traceback.print_exc()

        sys.exit(1)


# =============================================================================
# 19. START PROGRAM
# =============================================================================

if __name__ == "__main__":

    main()
