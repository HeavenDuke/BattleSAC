/**
 * Created by heavenduke on 17-5-30.
 */

var Player = function (id, location, title) {
    this.id = id;
    this.title = title;
    this.location = location;
    this.controlled = false;
};

module.exports = Player;