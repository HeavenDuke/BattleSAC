/**
 * Created by heavenduke on 17-5-30.
 */

var players = [];
var enemies = [];
var socket = io.connect(window.location.origin);
var UiPlayers = document.getElementById("players");

var Q = Quintus({audioSupported: [ 'wav','mp3' ]})
    .include('Sprites, Scenes, Input, 2D, Anim, Touch, UI, Audio')
    .setup("game", { maximize: true })
    .enableSound()
    .controls()
Q.touch(Q.SPRITE_ALL);


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
            var selfId = data['id'];
            var player = new Q.Player({ playerId: selfId, x: 100, y: 100, socket: socket });
            stage.insert(player);
            stage.add('viewport').follow(player);

            socket.on('updated', function (data) {
                var actor = players.filter(function (obj) {
                    return obj.playerId == data['id'];
                })[0];
                if (actor) {
                    actor.player.p.x = data['x'];
                    actor.player.p.y = data['y'];
                    actor.player.p.sheet = data['sheet'];
                    actor.player.p.update = true;
                } else {
                    var temp = new Q.Actor({ playerId: data['id'], x: data['x'], y: data['y'], sheet: data['sheet'] });
                    players.push({ player: temp, playerId: data['id'] });
                    stage.insert(temp);
                }
            });
        });

        socket.on('crowded', function (data) {
            alert("We currently have enough players.");
        });

        socket.on('start', function (data) {
            console.log("game start!");
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