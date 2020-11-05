const CompositeView = require("yunos/ui/view/CompositeView");
const TextView = require("yunos/ui/view/TextView");
const TextField = require("yunos/ui/view/TextField");
const ImageButton = require("yunos/ui/widget/ImageButton");
const Toast = require("yunos/ui/widget/Toast");

const IMAGE_PATH = "file://opt/app/ergo-demo.yunos.com/res/default/images/";
const TAG = "ERGO: ";
const BACKGROUND = "#b3cbf2";
const ROWS = 4;
const PADDING = 5;

class UsernameView extends CompositeView {

    constructor(width, height) {
        super();
        this.width = width;
        this.height = height;
        //this.callback = callback;
        this.init();
    }

    init() {
        this.background = BACKGROUND;
        this.borderWidth = 3;
        this.borderRadius = 10;
        this.borderColor = "#999999";
        this.inputButton = this.createInputButton();
        this.usernameForm = this.createUsernameForm();
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
        helpText.text = "Choose a username: ";
        return helpText;
    }

    createUsernameForm() {
        const POSITION = 2;
        let self = this;
        let textField = new TextField();
        this.addChild(textField);
        textField.height = 75;
        textField.width = self.width - 50;
        textField.top = (POSITION*(self.height/ROWS)) - (textField.height/2) + (POSITION * PADDING);
        textField.left = (self.width/2) - (textField.width/2);
        textField.fontSize = 36;
        textField.placeholder = "Username";
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
            console.log(TAG, "tapped button");

            let username = self.usernameForm.text;
            //Test username inputs
            //only allow A-Z, a-z, 0-9, length 1-20
            const usernameRegex = /^([A-Za-z0-9]{1,20})$/;
            if (!username.match(usernameRegex)) {
                let usernameWarning = new Toast();
                const usernameWarningText = "Username must be alphanumeric and less than 20 characters"
                usernameWarning.text = usernameWarningText;
                usernameWarning.show();
            } else {
                console.log(TAG, "match");
                //next input
                //self.callback({"server": server, "room": room, "username": username});
                self.emit('username-select', {"username": username});
            }
        });
        return inputButton;
    }
}

module.exports = UsernameView;
