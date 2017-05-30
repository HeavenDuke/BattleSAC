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
                    io.emit('join', { playerCount: global.gameData.player_count });
                });

                socket.on('update', function (data) {
                    socket.broadcast.emit('updated', data);
                });

                if (global.gameData.can_start()) {
                    global.gameData.start();
                    io.emit('start', {
                        enemies: global.gameData.enemies
                    });
                }

            }, 1000);
        }
        else {
            socket.emit('crowded');
        }
    });
};