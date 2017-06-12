/**
 * Created by heavenduke on 17-5-30.
 */

var random = require('../libs').random;
var authentication = require('../libs').authentication;

var Player = function (id, title, position, key, caseKey, code) {
    this.id = id;
    this.code = code;
    this.initial_location = [random.rand_int(100, 200), random.rand_int(100, 200)];
    this.title = title;
    this.position = position;
    this.controlled = false;
    this.key = key;
    this.caseKeys = [caseKey];
    this.authenticated = {};
    this.authenticated_me = {};
    this.compared_times = 0;
    this.parent = id;
    this.unauthenticated = {};
    this.public_keys = {};
};

Player.prototype.basicInfo = function () {
    return {
        id: this.id,
        initial_location: this.initial_location
    };
};

Player.prototype.positionInfo = function () {
    var authenticated = [];
    for(var id in this.authenticated) {
        authenticated.push({
            playerId: id,
            isCommander: this.authenticated[id].parent == id
        });
    }
    return {
        authenticated: authenticated,
        unauthenticated: Object.keys(this.unauthenticated)
    };
};

module.exports = Player;