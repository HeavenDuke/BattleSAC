/**
 * Created by heavenduke on 17-5-30.
 */

module.exports = function (io) {
    io.on('connection', function (socket) {
        var player = global.gameData.distribute_player();
        if (player) {
            global.gameData.player_count++;

            setTimeout(function () {
                socket.emit('connected', player);
                io.emit('join', { playerCount: global.gameData.player_count });

                socket.on('disconnect', function () {
                    global.gameData.player_count--;
                    global.gameData.players[player.id].controlled = false;
                    if (global.gameData.can_end()) {
                        global.gameData.end();
                    }
                    socket.broadcast.emit('exit', { playerCount: global.gameData.player_count });
                    socket.emit('exit', { playerCount: global.gameData.player_count });
                });

                socket.on('update', function (data) {
                    global.gameData.players[data.id].location = [data.x, data.y];
                    socket.broadcast.emit('updated', data);
                });

                if (global.gameData.can_start()) {
                    setTimeout(function () {
                        global.gameData.start();
                        socket.broadcast.emit('start', global.gameData.case);
                        socket.emit('start', {});
                    }, 2000);
                }

            }, 1000);
        }
        else {
            socket.emit('crowded');
        }
    });
};