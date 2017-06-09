/**
 * Created by heavenduke on 17-5-30.
 */

var Player = require('./Player');
var random = require('../libs/rand');

var Game = function () {
    this.max_player_count = 2;
    this.player_count = 0;
    this.players = {};
    this.case = [];
    this.started = false;
    var titles = [[0, 1, 2], [0, 1, 2]];
    var position = 0;
    for(var i = 0; i < this.max_player_count; i++) {
        this.players[i] = new Player(i, titles[position][i], position);
        position = 1 - position;
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
    this.case = [];
    this.started = true;
};

Game.prototype.update = function () {

};

Game.prototype.can_end = function () {
    for(var key in this.players) {
        if (this.players[key].controlled) {
            return false;
        }
    }
    return true;
};

Game.prototype.end = function () {
    this.started = false;
};

module.exports = Game;