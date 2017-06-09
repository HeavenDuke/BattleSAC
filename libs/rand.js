/**
 * Created by heavenduke on 17-5-31.
 */

exports.rand_int = function (min, max) {
    return Math.round(Math.random() * (max - min) + min);
};