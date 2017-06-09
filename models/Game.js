/**
 * Created by heavenduke on 17-5-30.
 */

var Player = require('./Player');
var random = require('../libs/rand');
var authentication = require('../libs').authentication;

var Game = function () {
    this.max_player_count = 2;
    this.player_count = 0;
    this.players = {};
    this.case = [];
    this.started = false;
    var titles = [[0, 1, 2], [0, 1, 2]];
    var position = 0;
    for(var i = 0; i < this.max_player_count; i++) {
        this.players[i] = new Player(i, titles[position][i], position, authentication.generateKey());
        position = 1 - position;
    }
    for(i = 0; i < this.max_player_count; i++) {
        for(var j = 0; j < this.max_player_count; j++) {
            if (this.players[i].position == this.players[j].position) {
                this.players[i].public_keys[j] = this.players[j].key.public;
            }
        }
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