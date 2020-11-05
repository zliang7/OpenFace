'use strict';

const CompositeView = require("yunos/ui/view/CompositeView");
const TextView = require("yunos/ui/view/TextView");
const TextField = require("yunos/ui/view/TextField");
const ImageButton = require("yunos/ui/widget/ImageButton");
const Toast = require("yunos/ui/widget/Toast");

const IMAGE_PATH = "file://opt/app/ergo-demo.yunos.com/res/default/images/";
const TAG = "ERGO: ";
const BACKGROUND = "#b3cbf2";
// Number of elements plus one
const ROWS = 4;
const PADDING = 5;

class ServerView extends CompositeView {

    constructor(width, height) {
        super();
        this.width = width;
        this.height = height;
        this.init();
    }

    init() {
        this.background = BACKGROUND;
        this.borderWidth = 3;
        this.borderRadius = 10;
        this.borderColor = "#999999";
        this.inputButton = this.createInputButton();
        this.serverForm = this.createServerForm();
        this.helpText = this.createHelpText();
    }

    createHelpText() {
        const POSITION = 1;
        let self = this;
        let helpText = new TextView();
        this.addChild(helpText);
        helpText.fontSize = 30;
        helpText.color = "#494949";
        helpText.top = (POSITION*(self.height/ROWS)) - (helpText.height/2) + (POSITION * PADDING);
        helpText.left = (self.width/5);
        helpText.text = "Join a WebRTC server: ";
        return helpText;
    }

    createServerForm() {
        const POSITION = 2;
        let self = this;
        let textField = new TextField();
        this.addChild(textField);
        textField.height = 75;
        textField.width = self.width - 50;
        textField.top = (POSITION*(self.height/ROWS)) - (textField.height/2) + (POSITION * PADDING);
        textField.left = (self.width/2) - (textField.width/2);
        textField.fontSize = 36;
        textField.placeholder = "Server";
        return textField;
    }

    createInputButton() {
        const POSITION = 3;
        let self = this;
        let inputButton = new ImageButton();
        this.addChild(inputButton);
        inputButton.src = IMAGE_PATH + "ic_chb_selected.png";
        inputButton.width = 90;
        inputButton.height = 90;
        inputButton.top = (POSITION*(self.height/ROWS)) - (inputButton.height/2) + (POSITION * PADDING);
        inputButton.left = (self.width/2) - (inputButton.width/2);
        inputButton.background = BACKGROUND;
        inputButton.on("tap", function() {

            //TODO replace regex with just testing if it works

            console.log(TAG, "tapped button");
            let server = self.serverForm.text;
            //test IP address/port not empty
            //const serverRegex = /^(([h][t][t][p][\:]\/{2})([0-9a-zA-Z.\-]+)*([\:][0-9]+)?)$/;
            const serverRegex = /^([0-9a-zA-Z.:\-])+$/;
            if (!server.match(serverRegex)) {
                console.log(TAG, "no match");
                let serverWarning = new Toast();
                const serverWarningText = "Please enter a valid server address";
                serverWarning.text = serverWarningText;
                serverWarning.show();
            } else {
                //TODO getRooms from server
                //self.emit('server-select', {"server": server, "rooms": rooms});
                self.emit('server-select', {"server": server});
            }
        });
        return inputButton;
    }
}

module.exports = ServerView;
