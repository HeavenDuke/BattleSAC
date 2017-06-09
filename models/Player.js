/**
 * Created by heavenduke on 17-5-30.
 */


var Player = function (id, location, title, position) {
    this.id = id;
    this.title = title;
    this.position = position;
    this.location = location;
    this.controlled = false;
    this.authenticated = [];
    this.unauthenticated = [];
    this.keys = {
        public: [],
        private: [],
        case: ""
    }
};

module.exports = Player;