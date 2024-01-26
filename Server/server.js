//Socket server
var fs = require('fs');
var url = require('url');
var http = require('http');
var WebSocket = require('ws');

//Mysql database 
var express = require('express');
var database = require('./database');
var app = express();

function requestHandler(request, response) {
    fs.readFile('./index.html', function(error, content) {
        response.writeHead(200, {
            'Content-Type': 'text/html'
        });
        response.end(content);
    });
}
// function gửi yêu cầu(response) từ phía server hoặc nhận yêu cầu (request) của client gửi lên
var server = http.createServer(app);
var server = http.createServer(requestHandler);
// create http server
// var server = http.createServer(requestHandler);
var ws = new WebSocket.Server({
    server
});


var clients = [];
var message_btn = "";

function broadcast(socket, data) {
    console.log(clients.length);
    for (var i = 0; i < clients.length; i++) {
        if (clients[i] != socket) {
            clients[i].send(data);
        }
    }
}

var UIDReceived = '';

ws.on('connection', function(socket, req) {
    clients.push(socket);
    console.log("connected!");
    socket.on('message', async function(message) {

        UIDReceived = message;
        console.log('received: %s', message);
        broadcast(socket, message);
        try {
            const data = await database.getByLicensePlate(UIDReceived)
            if (data.length > 0) {
                socket.send("BR_ON");
                broadcast(socket, "BR_ON1");
            } else {
                socket.send("BR_OFF");
                broadcast(socket, "BR_OFF1");
            }
        } catch (error) {

        }
    });
    socket.on('close', function() {
        var index = clients.indexOf(socket);
        clients.splice(index, 1);
        console.log('disconnected');
    });
});

app.get('/listUser', function(req, res) {
    database.getAllUser(function(resultQuery) {
        res.json(resultQuery);
    });
});

app.get('/findUser', function(req, res) {
    var license_plate = req.query.license_plate;
    var UID = req.query.UID;

    database.getByLicensePlate(license_plate, UID, function(resultQuery) {
        res.json(resultQuery);
    });
});


server.listen(3000, function() {
    console.log('listening on: 3000');
});