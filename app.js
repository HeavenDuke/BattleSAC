/**
 * Created by Obscurity on 2017/5/26.
 */
var express = require('express');
var app = express();
var server = require('http').Server(app);
var io = require('socket.io')(server);

app.set('view engine', 'pug');

app.use(express.static(__dirname + '/public'));

app.get('/', function(req, res){
    res.render('index');
});

app.get('/admin', function (req, res) {
    res.render('admin/index');
});

server.listen(8080);

console.log("Multiplayer app listening on port 8080");
