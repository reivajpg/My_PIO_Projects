const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; user-select: none; background-color: #222; color: #fff; }
    .btn { background-color: #444; color: white; border: 2px solid #555; border-radius: 10px; margin: 5px; width: 80px; height: 80px; font-size: 20px; touch-action: none; -webkit-tap-highlight-color: transparent; }
    .btn:active { background-color: #007bff; border-color: #007bff; }
    .row { display: flex; justify-content: center; }
  </style>
</head>
<body>
  <h1>Gamepad Web</h1>
  <div class="row">
    <button class="btn" style="visibility:hidden"></button>
    <button class="btn" ontouchstart="s('up',1,event)" onmousedown="s('up',1,event)" ontouchend="s('up',0,event)" onmouseup="s('up',0,event)">UP</button>
    <button class="btn" style="visibility:hidden"></button>
  </div>
  <div class="row">
    <button class="btn" ontouchstart="s('left',1,event)" onmousedown="s('left',1,event)" ontouchend="s('left',0,event)" onmouseup="s('left',0,event)">LEFT</button>
    <button class="btn" ontouchstart="s('down',1,event)" onmousedown="s('down',1,event)" ontouchend="s('down',0,event)" onmouseup="s('down',0,event)">DOWN</button>
    <button class="btn" ontouchstart="s('right',1,event)" onmousedown="s('right',1,event)" ontouchend="s('right',0,event)" onmouseup="s('right',0,event)">RIGHT</button>
  </div>

  <script>
    let state = {};
    function s(btn, val, e) {
      if (e) e.preventDefault();
      if (state[btn] === val) return;
      state[btn] = val;
      fetch('/update?btn=' + btn + '&val=' + val).catch(console.log);
    }
  </script>
</body>
</html>
)rawliteral";
