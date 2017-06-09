/**
 * Created by heavenduke on 17-5-30.
 */

var random = require('../libs').random;


var Player = function (id, title, position, key, caseKey, code) {
    this.id = id;
    this.code = code;
    this.initial_location = [random.rand_int(100, 1000), random.rand_int(100, 1000)];
    this.title = title;
    this.position = position;
    this.controlled = false;
    this.key = key;
    this.caseKeys = [caseKey];
    this.authenticated = [];
    this.unauthenticated = [];
    this.public_keys = {};
};

Player.prototype.basicInfo = function () {
    return {
        id: this.id,
        initial_location: this.initial_location
    };
};

module.exports = Player;