/**
 * Created by heavenduke on 17-6-14.
 */

exports.unique = function (array) {
    var temp = {};
    for(var i = 0; i < array.length; i++) {
        temp[array[i]] = array[i];
    }
    var result = [];
    for(var key in temp) {
        result.push(temp[key]);
    }
    return result;
};