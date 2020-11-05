'use strict';

const mongoose = require('mongoose');

let user = new mongoose.Schema({
    name: {
        type: String,
        unique: true,
        required: true
    },
    role: String,
    present: Boolean,
    posture: String
}, {_id: false});

let room = new mongoose.Schema({
    name: String,
    users: [ user ]
});

module.exports = mongoose.model('Room', room);
