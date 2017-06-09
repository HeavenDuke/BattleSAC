/**
 * Created by heavenduke on 17-6-8.
 */

var random = require('../libs').random;
var supplement = require('../libs').supplement;

var Case = function (player_count) {
    this.initial_location = [random.rand_int(100, 1000), random.rand_int(100, 1000)];
    this.nOpen = Math.floor(player_count / 2) + 1;
    this.status = "close";
    var _generated = supplement.generateCase(player_count, this.nOpen);
    this.secret = _generated.secret;
    this.keys = _generated.keys;
};

Case.prototype.open = function (keys) {
    var result = supplement.openCase({keys: keys, nOpen: this.nOpen});
    if (result) {
        this.status = "open";
    }
    return result;
};

module.exports = Case;