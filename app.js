/**
 * Created by Obscurity on 2017/5/26.
 */
var express = require('express');

var app = express();
var server = require('http').Server(app);
var io = require('socket.io')(server);
var GameModel = require('./models/Game');
var GameController = require('./controllers').player;
var routers = require('./routers');

app.set('view engine', 'pug');

app.use(express.static(__dirname + '/public'));

app.use(routers);

global.gameData = new GameModel();

GameController(io);

server.listen(8080);

console.log("Multiplayer app listening on port 8080");
