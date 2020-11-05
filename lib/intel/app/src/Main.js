'use strict';
// Temporary, removed before release
process.env.CAFUI2 = true;

var Page = require("yunos/page/Page");
var TextView = require("yunos/ui/view/TextView");
const Ergo = require("./ergo.js");
const UsernameView = require("./UsernameView.js");
const ServerView = require("./ServerView.js");
const HttpClient = require("yunos/net/HttpClient");
const ImageButton = require("yunos/ui/widget/ImageButton");

const TAG = "ERGO: ";
const IMAGE_PATH = "file://opt/app/ergo-demo.yunos.com/res/default/images/";
const BACKGROUND = "#b3cbf2";

let ergo = new Ergo();

class Main extends Page {

    onStart() {
        let self = this;
        // Choose a username
        let usernameView = new UsernameView(600, 400);
        usernameView.top = (self.window.height/2) - (usernameView.height/2);
        usernameView.left = (self.window.width/2) - (usernameView.width/2);
        // Join a server
        let serverView = new ServerView(600,400);
        serverView.top = (self.window.height/2) - (serverView.height/2);
        serverView.left = (self.window.width/2) - (serverView.width/2);
        // Ergo low-level status view
        let lowView = new TextView();
        lowView.width = 320;
        lowView.height = 240;
        lowView.background = "#ffb593";
        lowView.top = self.window.height - lowView.height;
        lowView.left = 320;
        lowView.multiLine = true;
        lowView.fontSize = 18;
        // Ergo high-level status view
        let ergoView = new TextView();
        ergoView.width = 320;
        ergoView.height = 240;
        ergoView.background = "#7af276";
        ergoView.top = self.window.height - ergoView.height;
        ergoView.left = 640;
        ergoView.multiLine = true;
        ergoView.fontSize = 24;
        // Teacher results view
        let teacherView = new TextView();
        teacherView.width = 320;
        teacherView.height = 240;
        teacherView.background = "#8eb9ff";
        teacherView.top = self.window.height - teacherView.height;
        teacherView.left = 960;
        teacherView.multiLine = true;
        teacherView.fontSize = 24;
        // Exit the app safely
        let exitButton = new ImageButton();
        exitButton.src = IMAGE_PATH + "exit.jpg";
        exitButton.width = 90;
        exitButton.height = 90;
        exitButton.top = 0;
        exitButton.left = (self.window.width) - (exitButton.width);
        exitButton.background = BACKGROUND;
        exitButton.on("tap", function() {
            console.log(TAG, "exiting..");
            self.leave();
        });
        self.window.addChild(exitButton);
        self.window.addChild(usernameView);

        // Event Listeners
        usernameView.addEventListener('username-select', function(data) {
            console.log(TAG, "username: " + data.username);
            self.username = data.username;
            self.window.removeChild(usernameView);
            self.window.addChild(serverView);
        });
        serverView.addEventListener('server-select', function(data) {
            console.log(TAG, "server: " + data.server);
            self.server = data.server;
            self.window.removeChild(serverView);
            self.window.addChild(lowView);
            self.window.addChild(ergoView);
            self.window.addChild(teacherView);

            ergo.init(self.window);
            ergo.start();

            // Join the server, register user in a room
            // TODO eventually do some token authentication
            let info = {
                "room": "room01",
                "user": self.username
            }
            self.join(info);
        });

        ergo.on('frame', function(result) {
            console.log(TAG, "frame processed");
            self.posture = ergo.getPosture();
            let _posture;
            if (self.posture == 2) {
                _posture = "good";
            } else {
                _posture = "bad";
            }
            self.present = ergo.isPresent();
            // Display low-level results
            let lowLevelStr = "Face detected: " + result.faceDetected+ "\nFace distance: " + result.faceDistance
                                + "\nFace direction: " + result.faceDirection + "\nFace awry: " + result.faceAwry
                                + "\nFace centered: " + result.faceCentered;
            lowView.text = lowLevelStr;
            // Display high-level results
            let highLevelStr = "Posture: " + _posture + "\nPresent: " + self.present;
            ergoView.text = highLevelStr;
            // Send results to server
            let results = {
                "room": "room01",
                "user": self.username,
                "posture": self.posture,
                "present": self.present
            }
            self.report(results);
        });

        // If teacher, poll the server for results
        setInterval(function() {
            let request = {
                "room": "room01"
            }
            let httpOptions = {
                url: "http://"+self.server+"/summary",
                method: HttpClient.Method.POST,
                headers: "Content-Type: text/plain",
                body: JSON.stringify(request)
            }
            let http = new HttpClient(httpOptions);
            http.on('response', (header)=>console.log(TAG, "received response"));
            http.on('data', function(data) {
                console.log(TAG, "data:" + data);
                let json = JSON.parse(data);
                let str1 = json.students + " students total\n" + json.present + " students present\n" + json.posture + " students with good posture";

                teacherView.text = str1;
            });
            http.connect(function(error, id) {
                if (error) {
                    console.log(TAG, error);
                }
            });
        }, 5000);
    }

    //HTTP Clients
    join(info) {
        let self = this;
        let httpOptions = {
            url: "http://"+self.server+"/join",
            method: HttpClient.Method.POST,
            headers: "Content-Type: text/plain",
            body: JSON.stringify(info),
        }
        let http = new HttpClient(httpOptions);
        http.on('response', (header)=>console.log(TAG, "received response"));
        http.connect(function(error, id) {
            if (error) {
                console.log(TAG, error);
            }
        });
    }
    report(results) {
        let self = this;
        let httpOptions = {
            url: "http://"+self.server+"/report",
            method: HttpClient.Method.POST,
            headers: "Content-Type: text/plain",
            body: JSON.stringify(results),
        }
        let http = new HttpClient(httpOptions);
        http.on('response', (header)=>console.log(TAG, "received response"));
        http.connect(function(error, id) {
            if (error) {
                console.log(TAG, error);
            }
        });
    }

    leave() {
        let self = this;
        if (self.username && self.server) {
            console.log(TAG, "leaving...");
            let user = {
                "room": "room01",
                "user": self.username
            }
            let httpOptions = {
                url: "http://"+self.server+"/leave",
                method: HttpClient.Method.POST,
                headers: "Content-Type: text/plain",
                body: JSON.stringify(user),
            }
            let http = new HttpClient(httpOptions);
            http.on('response', (header)=>console.log(TAG, "received response"));
            http.connect(function(error, id) {
                if (error) {
                    console.log(TAG, error);
                }
                // Exit the app
                self.stopPage();
            });
        } else {
            self.stopPage();
        }
    }

}
module.exports = Main;
