/**
 * Created by heavenduke on 17-5-30.
 */

var authentication = require('../../libs').authentication;

module.exports = function (io) {
    io.on('connection', function (socket) {
        var player = global.gameData.distribute_player();
        if (player) {
            global.gameData.player_count++;
            global.gameData.players[player.id].socket = socket;
            setTimeout(function () {
                console.log(global.gameData.player_count);
                socket.broadcast.emit('join', { playerCount: global.gameData.player_count });
                socket.emit('join', { playerCount: global.gameData.player_count });
                socket.emit('connected', player.basicInfo());

                socket.on('disconnect', function () {
                    global.gameData.player_count--;
                    global.gameData.players[player.id].controlled = false;
                    if (global.gameData.can_end()) {
                        global.gameData.end();
                    }
                    socket.broadcast.emit('exit', { playerCount: global.gameData.player_count });
                });

                socket.on('update', function (data) {
                    global.gameData.players[data.id].location = [data.x, data.y];
                    socket.broadcast.emit('updated', data);
                });

                socket.on('authentication', function (data) {
                    var message = global.gameData.players[data.playerId].position;
                    var secret = authentication.signMessage({
                        key: global.gameData.players[data.playerId].key.private,
                        message: message
                    });
                    var valid = false;
                    for(var id in global.gameData.players[data.self].public_keys) {
                        var key = global.gameData.players[data.self].public_keys[id];
                        if (authentication.verifyMessage({signature: secret, message: message, key: key})) {
                            valid = true;
                            global.gameData.players[data.self].authenticated.push(id);
                            break;
                        }
                    }
                    socket.emit('authentication', {playerId: data.playerId, valid: valid});
                });

                if (global.gameData.can_start()) {
                    setTimeout(function () {
                        global.gameData.start();
                        var cases = {};
                        for(var key in global.gameData.case) {
                            cases[key] = {id: key, location: global.gameData.case[key].initial_location};
                        }
                        socket.broadcast.emit('start', cases);
                        socket.emit('start', cases);
                    }, 2000);
                }

            }, 1000);
        }
        else {
            socket.emit('crowded');
        }
    });
};