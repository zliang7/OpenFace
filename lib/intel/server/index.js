'use strict';

const express = require('express');
const bodyparser = require('body-parser');
const https = require('https');
const fs = require('fs');
const mongoose = require('mongoose');
const Room = require('./api/Room');

const port = 3000;

mongoose.connect('mongodb://localhost/ergo');
mongoose.connection.on('connected', function() {
    console.log("connected to database");
});

let app = express();
app.use(bodyparser.text());

// User joins Room
app.post('/join', function(request, response) {
    console.log("/join request");
    console.log(request.body);
    request.body = JSON.parse(request.body);
    if (!request.body.room) {
        console.log("missing field");
        response.send(404);
    } else {
        Room.findOneAndUpdate({name: request.body.room},
                              {$addToSet: {users: {name: request.body.user,
                                                   role: request.body.role}}},
                              {upsert: true},
                              function(error, room) {
            if (error) {
                console.warn(error);
                response.send(404);
            }
            response.send(200);
        });
    }
});

// User leaves Room
app.post('/leave', function(request, response) {
    console.log("/leave request");
    console.log(request.body);
    request.body = JSON.parse(request.body);
    if (!request.body.room) {
        console.log("missing field");
        response.send(404);
    } else {
        Room.findOneAndUpdate({name: request.body.room},
                              {$pull: {users: {name: request.body.user}}},
                              function(error, room) {
            if (error) {
                console.warn(error);
                response.send(404);
            }
            response.send(200);
        });
    }
});

// User reports ergonomic status
app.post('/report', function(request, response) {
    console.log("/report request");
    let data = JSON.parse(request.body);
    console.log(data);
    if (!data.room) {
        console.log("missing field");
        response.send(404);
    } else {
        Room.findOneAndUpdate({name: data.room, "users.name": data.user},
                              {$set: {"users.$.name": data.user,
                                      "users.$.posture": data.posture,
                                      "users.$.present": data.present}},
                              {upsert: true},
                              function(error, room) {
                if (error) {
                    console.warn(error);
                    response.send(404);
                }
                if (!room) {
                    //TODO call /join
                    console.log("user did not join properly, skipping...");
                    response.send(404);
                }
                response.send(200);
        });
    }
});

// Teacher requests ergonomic summary
app.post('/summary', function(request, response){
    console.log("/summary request");
    let data = JSON.parse(request.body);
    console.log("gettng summary for " + data.room);
    if (!data.room) {
        console.log("missing field");
        response.send(404);
    } else {
        Room.findOne({name: data.room},
                     function(error, room) {
            if (error) {
                console.warn(error);
            }
            if (!room) {
                console.log("cannot find room");
                response.send(404);
            }
            let present_count = 0;
            let posture_count = 0;
            for (let i=0; i<room.users.length; i++) {
                if (room.users[i].present) {
                    present_count++;
                    if (room.users[i].posture == 2) {
                        posture_count++;
                    }
                }
            }
            console.log(room.users.length + " students");
            console.log(present_count + " students present");
            console.log(posture_count + " students with good posture");
            response.json({students: room.users.length,
                                present: present_count,
                                posture: posture_count});
        });
    }

});

const config = {
    key: fs.readFileSync('./key.pem'),
    cert: fs.readFileSync('./cert.pem')
}

//https.createServer(config, app).listen(port, function() {
//    console.log("Server started on port " + port);
//});
app.listen(port, function() {
    console.log("Server started on port " + port);
});
