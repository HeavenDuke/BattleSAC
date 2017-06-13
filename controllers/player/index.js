/**
 * Created by heavenduke on 17-5-30.
 */

var authentication = require('../../libs').authentication;
var voting = require('../../libs').voting;

module.exports = function (io) {
    io.on('connection', function (socket) {
        var player = global.gameData.distribute_player();
        if (player) {
            global.gameData.player_count++;
            global.gameData.players[player.id].socket = socket;
            setTimeout(function () {
                socket.broadcast.emit('join', {playerCount: global.gameData.player_count});
                socket.emit('join', {playerCount: global.gameData.player_count});
                socket.emit('connected', player.basicInfo());

                socket.on('disconnect', function () {
                    global.gameData.player_count--;
                    global.gameData.players[player.id].controlled = false;
                    var left = [0, 0];
                    for(var id in global.gameData.players) {
                        if (global.gameData.players[id].status != 'die' && global.gameData.players[id].controlled) {
                            left[global.gameData.players[id].position]++;
                        }
                    }
                    if (left[0] == 0) {
                        for(id in global.gameData.players) {
                            if (global.gameData.players[id].position == 1) {
                                global.gameData.players[id].socket.emit('win', {});
                            }
                            else {
                                global.gameData.players[id].socket.emit('lose', {});
                            }
                        }
                    }
                    else if (left[1] == 0) {
                        for(id in global.gameData.players) {
                            if (global.gameData.players[id].position == 1) {
                                global.gameData.players[id].socket.emit('lose', {});
                            }
                            else {
                                global.gameData.players[id].socket.emit('win', {});
                            }
                        }
                    }
                    socket.broadcast.emit('exit', {playerCount: global.gameData.player_count});
                });

                socket.on('update', function (data) {
                    global.gameData.players[data.id].location = [data.x, data.y];
                    socket.broadcast.emit('updated', data);
                });

                socket.on('authentication', function (data) {
                    var target = global.gameData.players[data.playerId];
                    function auth(player1, player2) {
                        var message = player2.code;
                        var secret = authentication.signMessage({
                            key: player2.key.private,
                            message: message
                        });
                        var valid = false;
                        for (var id in player1.public_keys) {
                            var key = player1.public_keys[id];
                            if (authentication.verifyMessage({
                                    signature: secret,
                                    message: player1.code,
                                    key: key
                                })) {
                                if (!player1.authenticated[id]) {
                                    valid = true;
                                    player1.authenticated[id] = global.gameData.players[id];
                                    player2.authenticated_me[player1.id] = player1;
                                }
                                socket.emit('authentication', {
                                    playerId: player2.id,
                                    valid: valid,
                                    isCommander: player2.parent == player2.id
                                });
                                break;
                            }
                        }
                        if (valid == false) {
                            player1.unauthenticated[player2.id] = player2;
                            player1.socket.emit('authentication', {playerId: player2.id, valid: valid});
                        }
                    }
                    auth(player, target);
                    auth(target, player);
                });

                socket.on('openCase', function (data) {
                    var result = global.gameData.case[data.caseId].open(global.gameData.players[data.playerId].caseKeys);
                    if (result) {
                        socket.broadcast.emit("caseOpened", {id: data.caseId});
                        socket.emit("caseOpened", {id: data.caseId});
                    }
                    else {
                        socket.emit("caseNotOpened", {});
                    }

                });

                socket.on('arm', function (data) {
                    if (global.gameData.case[data.caseId].status == "open") {
                        if (global.gameData.case[data.caseId].capacity > 0) {
                            global.gameData.players[data.playerId].armed = true;
                            global.gameData.case[data.caseId].capacity--;
                            socket.broadcast.emit("armed", {id: data.playerId, caseId: data.caseId, capacity: global.gameData.case[data.caseId].capacity});
                            socket.emit("armed", {id: data.playerId, caseId: data.caseId, capacity: global.gameData.case[data.caseId].capacity});
                        }
                        else {
                            socket.emit("armed", {message: "箱子内已无剩余武器"});
                        }
                    }
                    else {
                        socket.emit("armed", {message: '箱子尚未开启'});
                    }
                });

                socket.on('exchange', function (data) {
                    for(var id in player.authenticated) {
                        if (id in player.authenticated_me) {
                            var message = JSON.stringify(data);
                            var secret = authentication.encryptMessage({key: player.authenticated[id].key.public, message: message});
                            var dec = JSON.parse(authentication.decryptMessage({
                                key: player.authenticated[id].key.private,
                                message: secret
                            }));
                            if (dec.isEnemy) {
                                player.authenticated[id].unauthenticated[dec.playerId] = global.gameData.players[data.playerId];
                            }
                            else {
                                player.authenticated[id].authenticated[dec.playerId] = global.gameData.players[data.playerId];
                                global.gameData.players[data.playerId].authenticated_me[id] = player.authenticated[id];
                            }
                            player.authenticated[id].socket.emit("exchange", {type: 'update', data: dec});
                        }
                    }

                    if (player.authenticated_me[data.playerId] && player.authenticated[data.playerId]) {
                        message = JSON.stringify(player.authenticated[data.playerId].positionInfo());


                        for(id in player.authenticated) {
                            if (id in player.authenticated_me) {
                                var friend = player.authenticated[id];
                                secret = authentication.encryptMessage({key: friend.key.public, message: message});
                                dec = JSON.parse(authentication.decryptMessage({
                                    key: friend.key.private,
                                    message: secret
                                }));

                                for(var pi = 0; pi < dec.authenticated.length; pi++) {
                                    var pid = dec.authenticated[pi].playerId;
                                    if (!(pid in friend.authenticated)) {
                                        friend.authenticated[pid] = global.gameData.players[pid];
                                        global.gameData.players[pid].authenticated_me[friend.id] = friend;
                                    }
                                }

                                for(var pi = 0; pi < dec.unauthenticated.length; pi++) {
                                    var pid = dec.unauthenticated[pi];
                                    if (!(pid in friend.unauthenticated)) {
                                        friend.unauthenticated[pid] = global.gameData.players[pid];
                                    }
                                }

                                friend.socket.emit("exchange", {type: 'sync', data: dec});
                            }
                        }

                        message = JSON.stringify(player.positionInfo());

                        var receiver = player.authenticated[data.playerId];
                        for(id in receiver.authenticated) {
                            if (id in receiver.authenticated_me) {
                                friend = receiver.authenticated[id];
                                secret = authentication.encryptMessage({key: friend.key.public, message: message});
                                dec = JSON.parse(authentication.decryptMessage({
                                    key: friend.key.private,
                                    message: secret
                                }));

                                for(pi = 0; pi < dec.authenticated.length; pi++) {
                                    pid = dec.authenticated[pi].playerId;
                                    if (!(pid in friend.authenticated)) {
                                        friend.authenticated[pid] = global.gameData.players[pid];
                                        global.gameData.players[pid].authenticated_me[friend.id] = friend;
                                    }
                                }

                                for(pi = 0; pi < dec.unauthenticated.length; pi++) {
                                    pid = dec.unauthenticated[pi];
                                    if (!(pid in friend.unauthenticated)) {
                                        friend.unauthenticated[pid] = global.gameData.players[pid];
                                    }
                                }

                                friend.socket.emit("exchange", {type: 'sync', data: dec});
                            }
                        }
                    }
                });

                socket.on('comparison', function (data) {
                    if (player.authenticated_me[data.playerId] && player.authenticated[data.playerId]) {
                        var result = voting.compareTitle({
                            launcher: {
                                title: player.title,
                                public: player.key.public,
                                private: player.key.private
                            },
                            receiver: {
                                title: player.authenticated[data.playerId].title,
                                public: player.authenticated[data.playerId].key.public,
                                private: player.authenticated[data.playerId].key.private
                            }
                        });
                        switch (result) {
                            case 0:
                                player.authenticated[data.playerId].socket.emit('comparison', {message: '双方军衔相同'});
                                break;
                            default:
                                var loserId = result == -1 ? player.id : data.playerId;
                                for(var id in player.authenticated) {
                                    if (id in player.authenticated_me) {
                                        player.authenticated[id].socket.emit('comparison', {playerId: loserId});
                                    }
                                }
                                player.socket.emit('comparison', {playerId: loserId});
                                break;
                        }
                    }
                    else {
                        player.socket.emit('comparison', {message: '对方尚未确认您的身份'});
                    }
                });

                socket.on('submission', function (data) {
                    var keys = JSON.stringify(player.caseKeys);
                    var commander = global.gameData.players[data.playerId];
                    var secret = authentication.encryptMessage({key: commander.key.public, message: keys});
                    var dec = JSON.parse(authentication.decryptMessage({
                        key: commander.key.private,
                        message: secret
                    }));
                    commander.caseKeys = commander.caseKeys.concat(dec);
                    commander.socket.emit("submission", {playerId: player.id, keys: dec});
                    player.socket.emit("submission", {message: "已转交补给箱钥匙"});
                });

                socket.on('fly', function (data) {
                    socket.broadcast.emit('fly', data);
                });

                socket.on('hit', function (data) {
                    global.gameData.players[data.playerId].die();
                    var left = [0, 0];
                    for(var id in global.gameData.players) {
                        if (global.gameData.players[id].status != 'die' && global.gameData.players[id].controlled) {
                            left[global.gameData.players[id].position]++;
                        }
                    }
                    if (left[0] == 0) {
                        for(id in global.gameData.players) {
                            if (global.gameData.players[id].position == 1) {
                                global.gameData.players[id].socket.emit('win', {});
                            }
                            else {
                                global.gameData.players[id].socket.emit('lose', {});
                            }
                        }
                    }
                    else if (left[1] == 0) {
                        for(id in global.gameData.players) {
                            if (global.gameData.players[id].position == 1) {
                                global.gameData.players[id].socket.emit('lose', {});
                            }
                            else {
                                global.gameData.players[id].socket.emit('win', {});
                            }
                        }
                    }
                    socket.broadcast.emit('die', data);
                    socket.emit('die', data);
                });

                if (global.gameData.can_start()) {
                    setTimeout(function () {
                        global.gameData.start();
                        var cases = {};
                        for (var key in global.gameData.case) {
                            cases[key] = {id: key, location: global.gameData.case[key].initial_location, capacity: global.gameData.case[key].capacity};
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