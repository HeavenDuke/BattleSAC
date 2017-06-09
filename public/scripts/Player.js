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
                sheet: 'player'
            });

            this.add('2d, platformerControls, animation');
            this.on("touch");
        },
        step: function (dt) {
            if (Q.inputs['up']) {
                this.p.vy = -200;
            } else if (Q.inputs['down']) {
                this.p.vy = 200;
            } else if (!Q.inputs['down'] && !Q.inputs['up']) {
                this.p.vy = 0;
            }

            if (Q.inputs['up'] || Q.inputs['down'] || Q.inputs['left'] || Q.inputs['right']) {
                switchMenu("none");
            }

            this.p.socket.emit('update', { id: this.p.playerId, x: this.p.x, y: this.p.y, sheet: this.p.sheet });
            UiLocation.innerHTML = "Location: (" + this.p.x + ", " + this.p.y + ")";
        },
        touch: function (touch) {
            switchMenu("self");
        }
    });

    Q.Sprite.extend('Actor', {
        init: function (p) {
            this._super(p, {
                update: true
            });
            var temp = this;
            setInterval(function () {
                if (!temp.p.update) {
                    temp.destroy();
                }
                temp.p.update = false;
            }, 3000);
            this.on("touch");
        },
        touch: function (touch) {
            switchMenu("unknown", {self: selfId, playerId: this.p.playerId});
        }
    });

    Q.Sprite.extend('Case', {
        init: function (p) {
            this._super(p, {
                sheet: 'ghost'
            });
            this.on("touch");
        },
        touch: function (touch) {
            switchMenu("case", {self: selfId, caseId: this.p.caseId});
        }
    });

});

