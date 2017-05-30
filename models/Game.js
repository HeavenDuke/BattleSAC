/**
 * Created by heavenduke on 17-5-30.
 */

var Player = require('./Player');

var Game = function () {
    this.max_player_count = 5;
    this.player_count = 0;
    this.newcomer_id = 0;
    this.players = {};
    this.enemies = {};
    this.action_logs = [];
    this.started = false;
    var titles = [0, 1, 2, 3, 3];
    for(var i = 0; i < this.max_player_count; i++) {
        this.players[i] = new Player(i, titles[i]);
    }
};

Game.prototype.distribute_player = function () {
    for(var key in this.players) {
        if (!this.players[key].controlled) {
            this.players[key].controlled = true;
            return this.players[key];
        }
    }
    return null;
};

Game.prototype.can_start = function () {
    for(var key in this.players) {
        if (!this.players[key].controlled) {
            return false;
        }
    }
    return true;
};

Game.prototype.start = function () {

};

Game.prototype.end = function () {

};

module.exports = Game;