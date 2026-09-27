// js/firebase-config.js

import {
    initializeApp
} from "https://www.gstatic.com/firebasejs/10.12.2/firebase-app.js";

import {
    getDatabase
} from "https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js";


const firebaseConfig = {

    apiKey: "AIzaSyCSvsse-mAoOGDnf_rCL7ryLeNFd8nn91U",

    authDomain:
        "security-monitoring-syst-dd43a.firebaseapp.com",

    databaseURL:
        "https://security-monitoring-syst-dd43a-default-rtdb.firebaseio.com/",

    projectId:
        "security-monitoring-syst-dd43a",

    storageBucket:
        "YOUR_STORAGE_BUCKET",

    messagingSenderId:
        "YOUR_MESSAGING_SENDER_ID",

    appId:
        "YOUR_APP_ID"
};


const app = initializeApp(
    firebaseConfig
);


const database = getDatabase(
    app
);


export {
    database
};
