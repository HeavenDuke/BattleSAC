/**
 * Created by heavenduke on 17-5-30.
 */

var UiLocation = document.getElementById("location");

var switchMenu = function (action, params) {
    $("div[id$='_menu']").css("display", "none");
    $("#" + action + "_menu").css("display", "block");
    if (params) {
        $("#" + action + "_menu").attr("params", JSON.stringify(params));
    }
};



require([], function () {
    Q.Sprite.extend('Player', {
        init: function (p) {
            this._super(p, {
                update: true,
                isCommander: true,
                sheet: 'player'
            });
            this.p.update = false;
            this.p.nameLabel = new Q.UI.Text({
                label: this.p.playerId + "(You)",
                color: "gray",
                x: this.p.x,
                y: this.p.y - 30,
                size: 12
            });
            this.add('2d, platformerControls, animation');
            this.on("touch");
        },
        step: function (dt) {
            if (Q.inputs['up']) {
                this.p.vy = -200;
            } else if (Q.inputs['down']) {
                this.p.vy = 200;
            } else if (Q.inputs['right']) {
                this.p.vx = 200;
            } else if (Q.inputs['left']) {
                this.p.vx = -200;
            } else {
                if (!Q.inputs['down'] && !Q.inputs['up']) {
                    this.p.vy = 0;
                }
                if (!Q.inputs['left'] && !Q.inputs['right']) {
                    this.p.vx = 0;
                }
            }

            if (Q.inputs['up'] || Q.inputs['down'] || Q.inputs['left'] || Q.inputs['right']) {
                switchMenu("none");
            }

            this.updateLabel();

            this.p.socket.emit('update', { id: this.p.playerId, x: this.p.x, y: this.p.y, sheet: this.p.sheet });
            UiLocation.innerHTML = "Location: (" + this.p.x + ", " + this.p.y + ")";
        },
        exchange: function (playerId, isEnemy, isCommander) {
            this.p.socket.emit('exchange', {playerId: playerId, isEnemy: isEnemy, isCommander: isCommander});
        },
        touch: function (touch) {
            switchMenu("self");
        },
        updateLabel: function () {
            this.p.nameLabel.p.x = this.p.x;
            this.p.nameLabel.p.y = this.p.y - 30;
        }
    });

    Q.Sprite.extend('Actor', {
        init: function (p) {
            this._super(p, {
                update: true
            });
            var temp = this;
            this.p.nameLabel = new Q.UI.Text({
                label: this.p.playerId + "",
                color: "gray",
                x: this.p.x,
                y: this.p.y - 30,
                size: 12
            });
            this.p.nameVisible = false;
            setInterval(function () {
                if (!temp.p.update) {
                    temp.p.nameLabel.destroy();
                    temp.destroy();
                }
                temp.p.update = false;
            }, 3000);
            this.on("touch");
        },
        touch: function (touch) {
            if (this.p.isEnemy == false) {
                if (this.p.isCommander == true && self.p.isCommander == false) {
                    switchMenu("commander", {self: selfId, playerId: this.p.playerId});
                }
                else if (self.p.isCommander == true && this.p.isCommander == true) {
                    switchMenu("comrade", {self: selfId, playerId: this.p.playerId});
                }
            }
            else if (this.p.isEnemy != true) {
                switchMenu("unknown", {self: selfId, playerId: this.p.playerId});
            }
        },
        updateLabel: function () {
            this.p.nameLabel.p.x = this.p.x;
            this.p.nameLabel.p.y = this.p.y - 30;
        }
    });

    Q.Sprite.extend('Case', {
        init: function (p) {
            this._super(p, {
                update: true,
                sheet: 'case',
                opened: false
            });
            this.p.update = false;
            this.on("touch");
        },
        touch: function (touch) {
            if (!this.p.opened) {
                switchMenu("case", {playerId: selfId, caseId: this.p.caseId});
                $("#open").css("display", "block");
                $("#arm").css("display", "none");
            }
            else {
                switchMenu("case", {playerId: selfId, caseId: this.p.caseId});
                $("#open").css("display", "none");
                $("#arm").css("display", "block");
            }
        }
    });

    Q.Sprite.extend('Bullet', {
        init: function (p) {
            this._super(p, {
                sheet: 'bullet'
            });
            this.add('2d, platformerControls, animation');
            this.on("hit",this,"collision");
        },
        step: function (dt) {
            if (this.p.socket) {
                this.p.socket.emit('fly', {playerId: selfId, bulletId: this.p.bulletId, x: this.p.x, y: this.p.y});
            }
        },
        collision: function (col) {
            if (col.obj.p.playerId) {
                this.p.socket.emit('die', {playerId: col.obj.p.playerId});
            }
            this.destroy();
        }
    });

});

