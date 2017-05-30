/**
 * Created by Obscurity on 2017/5/26.
 */
var express = require('express');

var app = express();
var server = require('http').Server(app);
var io = require('socket.io')(server);

var routers = require('./routers');

app.set('view engine', 'pug');

app.use(express.static(__dirname + '/public'));

app.use(routers);

var playerCount = 0;
var id = 0;

io.on('connection', function (socket) {
    playerCount++;
    id++;
    setTimeout(function () {
        socket.emit('connected', { playerId: id });
        io.emit('join', { playerCount: playerCount });
    }, 1500);

    socket.on('disconnect', function () {
        playerCount--;
        io.emit('join', { playerCount: playerCount });
    });

    socket.on('update', function (data) {
        socket.broadcast.emit('updated', data);
    });

});

server.listen(8080);

console.log("Multiplayer app listening on port 8080");
