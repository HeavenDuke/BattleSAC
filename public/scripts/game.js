/**
 * Created by heavenduke on 17-5-30.
 */

var players = [];
var cases = [];
var selfId = null;
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

$("#case_menu").children().click(function () {
    socket.emit("openCase", JSON.parse($(this).parent().attr('params')));
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
            var player = new Q.Player({ playerId: selfId, x: data.initial_location[0], y: data.initial_location[1], socket: socket });
            stage.insert(player);
            stage.add('viewport').follow(player);

            socket.on('authentication', function (data) {
                var actor = players.filter(function (obj) {
                    return obj.playerId == data['playerId'];
                })[0];
                if (actor) {
                    if (data.valid == false) {
                        actor.player.p.isEnemy = true;
                        actor.player.p.sheet = "enemy";
                    }
                    else {
                        actor.player.p.isEnemy = false;
                    }
                }
            });

            socket.on('caseNotOpen', function (data) {
                alert("failed to open supplement case.");
            });

            socket.on('caseOpened', function (data) {
                console.log(data);
            });

            socket.on('updated', function (data) {
                var actor = players.filter(function (obj) {
                    return obj.playerId == data['id'];
                })[0];
                if (actor) {
                    actor.player.p.x = data['x'];
                    actor.player.p.y = data['y'];
                    actor.player.p.sheet = (actor.player.p.isEnemy == true ? "enemy" : "player");
                    actor.player.p.update = true;
                } else {
                    var temp = new Q.Actor({ playerId: data['id'], x: data['x'], y: data['y'], sheet: "player" });
                    players.push({ player: temp, playerId: data['id'] });
                    stage.insert(temp);
                }
            });
        });

        socket.on('crowded', function (data) {
            alert("We currently have enough players.");
        });

        socket.on('start', function (data) {
            for(var key in data) {
                var temp = new Q.Case({ caseId: data[key]['id'], x: data[key].location[0], y: data[key].location[1]});
                cases.push({case: temp, caseId: key});
                stage.insert(temp);
            }
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