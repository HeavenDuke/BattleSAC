/**
 * Created by heavenduke on 17-5-30.
 */

var Enemy = function (id, location) {
    this.id = id;
    this.location = location;
    this.dead = false;
};

module.exports = Enemy;