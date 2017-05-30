/**
 * Created by heavenduke on 17-5-30.
 */

exports.game = function (req, res, next) {
    res.render('index', {
        title: "BattleSAC Game"
    });
};

exports.monitor = function (req, res, next) {
    res.render('monitor/index', {
        title: "BattleSAC Monitor"
    });
};