/**
 * Created by Obscurity on 2017/5/30.
 */

var Router = require('express').Router();
var Controllers = require('../controllers').web;

Router.get('/', Controllers.game);

Router.get('/monitor', Controllers.monitor);

module.exports = Router;