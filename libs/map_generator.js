/**
 * Created by heavenduke on 17-5-31.
 */

var random = require('./rand');

exports.generate = function () {

    var size = 100, block_num = 100;
    var map = [];
    for(var i = 0; i < size; i++) {
        map[i] = [];
        for(var j = 0; j < size; j++) {
            map[i][j] = 0;
        }
    }
    for(i = 0; i < size; i++) {
        map[0][i] = 1;
        map[i][0] = 1;
        map[size - 1][i] = 1;
        map[i][size - 1] = 1;
    }

    for(i = 0; i < block_num; i++) {
        var index = random.rand_int(0, size * size - 1);
        map[Math.round(index / size)][index % size] = 1;
    }
    return map;
};