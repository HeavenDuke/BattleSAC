/**
 * Created by Obscurity on 2017/5/30.
 */

exports.hello = require('./build/Release/addon');

exports.authentication = require('./build/Release/authentication.node');

exports.supplement = require('./build/Release/supplement.node');

exports.voting = require('./build/Release/voting.node');

exports.random = require('./rand');

var _case = exports.supplement.generateCase(5, 1);

console.log(_case);

console.log(exports.supplement.openCase({
    secret: _case.secret,
    keys: _case.keys.slice(0, 1),
    nOpen: 1
}));