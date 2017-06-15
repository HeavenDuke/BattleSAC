/**
 * Created by heavenduke on 17-5-30.
 */


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
                destroyMenu();
            }

            this.updateLabel();

            this.p.socket.emit('update', { id: this.p.playerId, x: this.p.x, y: this.p.y, sheet: this.p.sheet });
            this.updateLocation([this.p.x.toFixed(2), this.p.y.toFixed(2)]);
        },
        touch: function () {
            if (gameState == "voting") {
                if (this.p.isCommander != false) {
                    var param = {self: selfId, playerId: this.p.playerId, code: self.generateVotingCode()};
                    var socket = self.p.socket;
                    switchMenu([{
                        label: "我的信息",
                        action: function () {
                            socket.emit("info", {});
                            destroyMenu();
                        }
                    }, {
                        label: "投票给我",
                        action: function () {
                            socket.emit("voting", param);
                            eventLog("您投票给自己，票码为" + param.code + "，请等待其他队员确认投票");
                            gameState = "voted";
                            destroyMenu();
                        }
                    }])
                }
            }
            else {
                socket = self.p.socket;
                switchMenu([{
                    label: "我的信息",
                    action: function () {
                        socket.emit("info", {});
                        destroyMenu();
                    }
                }]);
            }
        },
        exchange: function (playerId, isEnemy, isCommander) {
            this.p.socket.emit('exchange', {playerId: playerId, isEnemy: isEnemy, isCommander: isCommander});
        },
        updateLabel: function () {
            this.p.nameLabel.p.x = this.p.x;
            this.p.nameLabel.p.y = this.p.y - 30;
        },
        updateLocation: function (location) {
            LocationPanel.p.label = "坐标: (" + location[0] + ", " + location[1] + ")";
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
                        var param = {self: selfId, playerId: this.p.playerId, code: self.generateVotingCode()};
                        var socket = self.p.socket;
                        switchMenu([{
                            label: "投票给他",
                            action: function () {
                                socket.emit("voting", param);
                                eventLog("您投票给" + param.playerId +"号伞兵，票码为" + param.code + "，请等待其他队员确认投票");
                                gameState = "voted";
                                destroyMenu();
                            }
                        }]);
                    }
                }
                else if (gameState != "voted") {
                    if (this.p.isCommander == true && self.p.isCommander == false) {
                        param = {self: selfId, playerId: this.p.playerId};
                        socket = self.p.socket;
                        switchMenu([{
                            label: "上交钥匙",
                            action: function () {
                                socket.emit("submission", param);
                                destroyMenu();
                            }
                        }]);
                    }
                    else if (self.p.isCommander == true && this.p.isCommander == true) {
                        param = {self: selfId, playerId: this.p.playerId};
                        socket = self.p.socket;
                        switchMenu([{
                            label: "比较军衔",
                            action: function () {
                                socket.emit("comparison", param);
                                destroyMenu();
                            }
                        }]);
                    }
                }
            }
            else if (this.p.isEnemy != true) {
                param = {self: selfId, playerId: this.p.playerId};
                socket = self.p.socket;
                switchMenu([{
                    label: "身份认证",
                    action: function () {
                        socket.emit("authentication", param);
                        destroyMenu();
                    }
                }]);
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
                var param = {playerId: selfId, caseId: this.p.caseId};
                socket = self.p.socket;
                switchMenu([{
                    label: "开启补给箱",
                    action: function () {
                        socket.emit("openCase", param);
                        destroyMenu();
                    }
                }]);
            }
            else {
                param = {playerId: selfId, caseId: this.p.caseId};
                socket = self.p.socket;
                switchMenu([{
                    label: "获取装备",
                    action: function () {
                        socket.emit("arm", param);
                        destroyMenu();
                    }
                }]);
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

