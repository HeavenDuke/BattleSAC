/**
 * Created by Obscurity on 2017/5/30.
 */

exports.hello = require('./build/Release/addon');

exports.authentication = require('./build/Release/authentication.node');

console.log(exports.authentication.generateKey());