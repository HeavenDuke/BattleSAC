/**
 * Created by heavenduke on 17-5-30.
 */


var switchMenu = function (action, params) {
    if (gameState != "waiting") {
        $("div[id$='_menu']").css("display", "none");
        $("#" + action + "_menu").css("display", "block");
        if (params) {
            $("#" + action + "_menu").attr("params", JSON.stringify(params));
        }
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
            this.p.bullets = [];
            this.p.bulletId = 0;
            this.p.nameLabel = new Q.UI.Text({
                label: this.p.playerId + "(You)",
                color: "gray",
                x: this.p.x,
                y: this.p.y - 30,
                size: 12
            });
            this.on('touch');
            this.add('2d, platformerControls, animation');
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
            }

            if (!Q.inputs['down'] && !Q.inputs['up']) {
                this.p.vy = 0;
            }
            if (!Q.inputs['left'] && !Q.inputs['right']) {
                this.p.vx = 0;
            }

            if (Q.inputs['up'] || Q.inputs['down'] || Q.inputs['left'] || Q.inputs['right']) {
                switchMenu("none");
            }

            this.updateLabel();

            this.p.socket.emit('update', { id: this.p.playerId, x: this.p.x, y: this.p.y, sheet: this.p.sheet });
            infoDisplayers.location.updateInfo([this.p.x.toFixed(2), this.p.y.toFixed(2)]);
        },
        touch: function () {
            if (gameState == "voting") {
                if (this.p.isCommander != false) {
                    switchMenu("voting", {self: selfId, playerId: this.p.playerId, code: self.generateVotingCode()});
                }
            }
        },
        exchange: function (playerId, isEnemy, isCommander) {
            this.p.socket.emit('exchange', {playerId: playerId, isEnemy: isEnemy, isCommander: isCommander});
        },
        updateLabel: function () {
            this.p.nameLabel.p.x = this.p.x;
            this.p.nameLabel.p.y = this.p.y - 30;
        },
        generateVotingCode: function () {
            return Math.round(Math.random() * 2048);
        },
        fire: function (actor) {
            var dx = actor.p.x - this.p.x;
            var dy = actor.p.y - this.p.y;
            var v = 15, bias = 40;
            var d = Math.sqrt(dx * dx + dy * dy);
            var sina = dy / d, cosa = dx / d;
            var vx = v * cosa, vy = v * sina;
            var bullet = new Q.Bullet({
                playerId: this.p.playerId,
                bulletId: this.p.bulletId++,
                x: this.p.x + bias * cosa,
                y: this.p.y + bias * sina,
                vex: vx,
                vey: vy,
                socket: this.p.socket
            });
            this.p.bullets.push(bullet);
            this.p.stage.insert(bullet);
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
            this.p.bullets = [];
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
                console.log(gameState);
                if (gameState == "voting") {
                    if (this.p.isCommander != false) {
                        switchMenu("voting", {self: selfId, playerId: this.p.playerId, code: self.generateVotingCode()});
                    }
                }
                else if (gameState != "voted") {
                    if (this.p.isCommander == true && self.p.isCommander == false) {
                        switchMenu("commander", {self: selfId, playerId: this.p.playerId});
                    }
                    else if (self.p.isCommander == true && this.p.isCommander == true) {
                        switchMenu("comrade", {self: selfId, playerId: this.p.playerId});
                    }
                }
            }
            else if (this.p.isEnemy != true) {
                switchMenu("unknown", {self: selfId, playerId: this.p.playerId});
            }
            else if (self.p.armed) {
                self.fire(this);
            }
        },
        updateLabel: function () {
            this.p.nameLabel.p.x = this.p.x;
            this.p.nameLabel.p.y = this.p.y - 30;
        },
        fire: function (bulletInfo, stage) {
            var bullet = new Q.Bullet({
                playerId: this.p.playerId,
                bulletId: bulletInfo['bulletId'],
                x: bulletInfo['x'],
                y: bulletInfo['y']
            });
            this.p.bullets.push(bullet);
            stage.insert(bullet);
        }
    });

    Q.Sprite.extend('Case', {
        init: function (p) {
            this._super(p, {
                sheet: 'case',
                opened: false
            });
            this.p.nameLabel = new Q.UI.Text({
                label: "Locked",
                color: "gray",
                x: this.p.x,
                y: this.p.y - 30,
                size: 12
            });
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
        },
        open: function () {
            this.p.opened = true;
            this.p.nameLabel.p.label = "Opened(Remain: " + this.p.capacity + ")";
        }
    });

    Q.Sprite.extend('Bullet', {
        init: function (p) {
            this._super(p, {
                sheet: 'bullet'
            });
            this.add('2d, platformerControls, animation');
            this.on("hit.sprite",  function (collision) {
                if (this.p.socket) {
                    if (collision.obj.isA('Actor')) {
                        this.p.socket.emit('hit', {playerId: collision.obj.p.playerId, bulletId: this.p.bulletId, fireId: selfId});
                    }
                }
                this.destroy();
            });
        },
        step: function (dt) {
            if (this.p.socket) {
                this.p.y += this.p.vey;
                this.p.x += this.p.vex;
                this.p.socket.emit('fly', {playerId: selfId, bulletId: this.p.bulletId, x: this.p.x, y: this.p.y});
            }
        }
    });

});

