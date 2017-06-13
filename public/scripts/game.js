/**
 * Created by heavenduke on 17-5-30.
 */

var players = [];
var destroyed = [];
var cases = [];
var selfId = null;
var self = null;
var gameState = "waiting";
var socket = io.connect(window.location.origin);
var UiPlayers = document.getElementById("players");

var Q = Quintus({audioSupported: [ 'wav','mp3' ]})
    .include('Sprites, Scenes, Input, 2D, Anim, Touch, UI, Audio')
    .setup("game", { maximize: true })
    .enableSound()
    .controls();
Q.touch(Q.SPRITE_ALL);

$("#unknown_menu").children().click(function () {
    socket.emit("authentication", JSON.parse($(this).parent().attr('params')));
    $(this).parent().css("display", "none");
    $("canvas").focus();
});

$("#case_menu").children("#open").click(function () {
    socket.emit("openCase", JSON.parse($(this).parent().attr('params')));
    $(this).parent().css("display", "none");
    $("canvas").focus();
});

$("#case_menu").children("#arm").click(function () {
    socket.emit("arm", JSON.parse($(this).parent().attr('params')));
    $(this).parent().css("display", "none");
    $("canvas").focus();
});

$("#comrade_menu").children().click(function () {
    socket.emit("comparison", JSON.parse($(this).parent().attr('params')));
    $(this).parent().css("display", "none");
    $("canvas").focus();
});

$("#commander_menu").children().click(function () {
    socket.emit("submission", JSON.parse($(this).parent().attr('params')));
    $(this).parent().css("display", "none");
    $("canvas").focus();
});

Q.gravityY = 0;

var objectFiles = [
    '/scripts/Player.js'
];

require(objectFiles, function () {

    function setUp (stage) {
        socket.on('join', function (data) {
            UiPlayers.innerHTML = 'Players: ' + data['playerCount'];
        });

        socket.on('exit', function (data) {
            UiPlayers.innerHTML = 'Players: ' + data['playerCount'];
            var actor = players.filter(function (obj) {
                return obj.playerId == data['id'];
            })[0];
            if (actor) {
                actor.player.destroy();
            }
        });

        socket.on('connected', function (data) {
            selfId = data['id'];
            var player = new Q.Player({ playerId: selfId, x: data.initial_location[0], y: data.initial_location[1], socket: socket, stage: stage });
            self = player;
            player.p.sheet = 'commander';
            stage.insert(player);
            stage.insert(player.p.nameLabel);
            stage.add('viewport').follow(player);

            socket.on('authentication', function (data) {
                var actor = players.filter(function (obj) {
                    return obj.playerId == data['playerId'];
                })[0];
                if (actor) {
                    if (data.valid) {
                        actor.player.p.isEnemy = false;
                        if (data.isCommander) {
                            actor.player.p.sheet = actor.player.p.armed ? "armedcommander" : "commander";
                        }
                        else {
                            actor.player.p.sheet = actor.player.p.armed ? "armedfriend" : "friend";
                        }
                        actor.player.p.isCommander = data.isCommander;
                        player.exchange(actor.playerId, false, actor.player.p.isCommander);
                    }
                    else if (data.valid == false) {
                        actor.player.p.isEnemy = true;
                        actor.player.p.sheet = actor.player.p.armed ? "armedenemy" : "enemy";
                        player.exchange(actor.playerId, true);
                    }
                }
            });

            socket.on('exchange', function (data) {
                console.log(data);
                if (data.type == "update") {
                    var actor = players.filter(function (obj) {
                        return obj.playerId == data.data['playerId'];
                    })[0];
                    if (actor) {
                        if (data.data.isEnemy == false) {
                            actor.player.p.isEnemy = false;
                            if (data.data.isCommander) {
                                actor.player.p.sheet = actor.player.p.armed ? "armedcommander" : "commander";
                            }
                            else {
                                actor.player.p.sheet = actor.player.p.armed ? "armedfriend" : "friend";
                            }
                            actor.player.p.isCommander = data.isCommander;
                            if (!actor.player.p.nameVisible) {
                                stage.insert(actor.player.p.nameLabel);
                                actor.player.p.nameVisible = true;
                            }
                        }
                        else if (data.data.isEnemy == true) {
                            actor.player.p.isEnemy = true;
                            actor.player.p.sheet = actor.player.p.armed ? "armedenemy" : "enemy";
                        }
                    }
                }
                else if (data.type == 'sync') {
                    for(var i = 0; i < data.data.authenticated.length; i++) {
                        actor = players.filter(function (obj) {
                            return obj.playerId == data.data.authenticated[i]['playerId'];
                        })[0];
                        if (actor && actor.playerId != selfId) {
                            actor.player.p.isEnemy = false;
                            if (data.data.authenticated[i].isCommander) {
                                actor.player.p.sheet = actor.player.p.armed ? "armedcommander" : "commander";
                            }
                            else {
                                actor.player.p.sheet = actor.player.p.armed ? "armedfriend" : "friend";
                            }
                            actor.player.p.isCommander = data.data.authenticated[i].isCommander;
                            if (!actor.player.p.nameVisible) {
                                stage.insert(actor.player.p.nameLabel);
                                actor.player.p.nameVisible = true;
                            }
                        }
                    }
                    for(i = 0; i < data.data.unauthenticated.length; i++) {
                        actor = players.filter(function (obj) {
                            return obj.playerId == data.data.unauthenticated[i];
                        })[0];
                        if (actor) {
                            actor.player.p.isEnemy = true;
                            actor.player.p.sheet = actor.player.p.armed ? "armedenemy" : "enemy";
                        }
                    }
                }
            });

            socket.on('caseNotOpened', function (data) {
                alert("failed to open supplement case.");
            });

            socket.on('caseOpened', function (data) {
                var _case = cases.filter(function (obj) {
                    return obj.caseId == data['id'];
                })[0];
                if (_case) {
                    _case.case.open(stage);
                }
            });

            socket.on('armed', function (data) {
                if (data['message']) {
                    alert(data['message']);
                }
                else {
                    if (data['id'] == selfId) {
                        self.p.armed = true;
                        if (self.p.isCommander) {
                            self.p.sheet = "armedcommander";
                        }
                        else {
                            self.p.sheet = "armedfriend";
                        }
                    }
                    else {
                        var actor = players.filter(function (obj) {
                            return obj.playerId == data['id'];
                        })[0];
                        if (actor) {
                            actor.player.p.armed = true;
                            if (actor.player.p.isEnemy) {
                                actor.player.p.sheet = "armedenemy";
                            }
                            else if (actor.player.p.isEnemy == false) {
                                if (actor.player.p.isCommander) {
                                    actor.player.p.sheet = "armedcommander";
                                }
                                else {
                                    actor.player.p.sheet = "armedfriend";
                                }
                            }
                            else {
                                actor.player.p.sheet = "armedplayer";
                            }
                            actor.player.p.update = true;
                        }
                    }
                    var _case = cases.filter(function (obj) {
                        return obj.caseId == data['caseId'];
                    })[0];
                    if (_case) {
                        _case.case.p.capacity = data['capacity'];
                        _case.case.p.nameLabel.p.label = "Opened(Remain: " + _case.case.p.capacity + ")";
                    }
                }
            });

            socket.on('updated', function (data) {
                var actor = players.filter(function (obj) {
                    return obj.playerId == data['id'];
                })[0];
                if (actor) {
                    actor.player.p.x = data['x'];
                    actor.player.p.y = data['y'];
                    // actor.player.p.sheet = (actor.player.p.isEnemy == true ? "enemy" : "player");
                    actor.player.p.update = true;
                    actor.player.updateLabel();
                } else {
                    var ded = false;
                    destroyed.forEach(function (id) {
                        if (data['id'] == id) {
                            ded = true;
                        }
                    });
                    if (!ded) {
                        var temp = new Q.Actor({playerId: data['id'], x: data['x'], y: data['y'], sheet: "player"});
                        players.push({player: temp, playerId: data['id']});
                        stage.insert(temp);
                    }
                }
            });

            socket.on('submission', function (data) {
                if(data.message) {
                    alert(data.message);
                }
                else {
                    var message = data.playerId + "号伞兵向你转交了补给箱钥匙，钥匙分别为：";
                    for(var i = 0; i < data.keys.length; i++) {
                        message += "\n(" + data.keys[i].x + ", " + data.keys[i].fx + ")";
                    }
                    alert(message);
                }
            });

            socket.on('comparison', function (data) {
                console.log(data);
                if(data.message) {
                    alert(data.message);
                }
                else {
                    if (data.playerId == selfId) {
                        self.p.isCommander = false;
                        self.p.sheet = self.p.armed ? "armedfriend" : "friend";
                    }
                    else {
                        var actor = players.filter(function (obj) {
                            return obj.playerId == data['playerId'];
                        })[0];
                        if (actor) {
                            actor.player.p.isCommander = false;
                            actor.player.p.sheet = actor.player.p.armed ? "armedfriend" : "friend";
                        }
                    }
                }
            });

            socket.on('fly', function (data) {
                var actor = players.filter(function (obj) {
                    return obj.playerId == data['playerId'];
                })[0];
                if (actor) {
                    var bullet = actor.player.p.bullets.filter(function (obj) {
                        return obj.p.bulletId == data['bulletId'];
                    })[0];
                    if (bullet) {
                        bullet.p.x = data['x'];
                        bullet.p.y = data['y'];
                    }
                    else {
                        actor.player.fire(data, stage);
                    }
                }
            });

            socket.on('die', function (data) {
                var actor = players.filter(function (obj) {
                    return obj.playerId == data.fireId;
                })[0];
                if (actor) {
                    for(var i = 0; i < actor.player.p.bullets.length; i++) {
                        if (actor.player.p.bullets[i].p.bulletId == data.bulletId) {
                            actor.player.p.bullets[i].destroy();
                            actor.player.p.bullets.splice(i, 1);
                        }
                    }
                }
                if (data.playerId == selfId) {
                    self.p.nameLabel.destroy();
                    self.destroy();
                    alert('you are busted!');
                }
                else {
                    actor = players.filter(function (obj) {
                        return obj.playerId == data.playerId;
                    })[0];
                    if (actor) {
                        actor.player.destroy();
                        destroyed.push(actor.playerId);
                        for(i = 0; i < players.length; i++) {
                            if (players[i].playerId == data['playerId']) {
                                players.splice(i, 1);
                                break;
                            }
                        }
                    }
                }
            });

            socket.on('win', function () {
                alert("your team wins!");
            });

            socket.on('lose', function () {
                alert("your team lost!");
            });

        });

        socket.on('crowded', function (data) {
            alert("We currently have enough players.");
        });

        socket.on('start', function (data) {
            for(var key in data) {
                var _case = cases.filter(function (obj) {
                    return obj.caseId == key;
                })[0];
                if (!_case) {
                    var temp = new Q.Case({ caseId: data[key]['id'], x: data[key].location[0], y: data[key].location[1], sheet: 'case', capacity: data[key].capacity});
                    cases.push({case: temp, caseId: key});
                    stage.insert(temp);
                    stage.insert(temp.p.nameLabel);
                }
            }
            var countdown = 3;
            function cnt () {
                if (countdown == 0) {
                    alert('游戏开始');
                    gameState = "started";
                }
                else {
                    countdown--;
                    setTimeout(cnt, 1000);
                }
            }
            cnt();
        });
    }

    Q.scene('arena', function (stage) {
        stage.collisionLayer(new Q.TileLayer({ dataAsset: '/maps/arena.json', sheet: 'tiles' }));

        setUp(stage);

    });

    var files = [
        '/images/tiles.png',
        '/maps/arena.json',
        '/images/sprites.png',
        '/images/sprites.json'
    ];

    Q.load(files.join(','), function () {
        Q.sheet('tiles', '/images/tiles.png', { tilew: 32, tileh: 32 });
        Q.compileSheets('/images/sprites.png', '/images/sprites.json');
        Q.stageScene('arena', 0);
    });
});