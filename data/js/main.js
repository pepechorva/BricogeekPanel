var connection = new WebSocket('ws://' + location.hostname + ':80/', ['ESP32']);

connection.onopen = function () 
{
  connection.send('Connect ' + new Date());
};
connection.onerror = function (error) 
{
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) 
{
  updateSliders(e.data);
  console.log('Server: ', e.data);
};
connection.onclose = function () 
{
  console.log('WebSocket connection closed');
};


let jsonData;



function sendData()
{
  var xhttp = new XMLHttpRequest();
  sliders() ;
  let data = JSON.stringify(jsonData);

  xhttp.addEventListener('load', function(event) {
    console.log('OK', xhttp);
  });
  xhttp.addEventListener('error', function(event) {
    console.log('error', xhttp);
  });
  xhttp.open('POST', 'values');
  xhttp.setRequestHeader('Content-Type', 'application/json');
  xhttp.send(data);
}



function getData()
{
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (xhttp.readyState == XMLHttpRequest.DONE) {
           if (xhttp.status == 200) {
           console.log((xhttp.responseText));
           let json = JSON.parse(xhttp.responseText);
              console.log();
           
           let receivedMsg = 'Received: GPIO ' + json.id + ' ' + (json.status == 1 ? "ON" : "OFF");
           document.getElementById('receivedText').textContent = receivedMsg;
           }
           else {
              console.log('error', xhttp);
           }
        }
    };
    xhttp.open("GET", "values", true);
    xhttp.send();
}


function sliders() 
{
  var rf = document.getElementById("RFront");
  var gf = document.getElementById("GFront");
  var bf = document.getElementById("BFront");
  var rb = document.getElementById("RBack");
  var gb = document.getElementById("GBack");
  var bb = document.getElementById("BBack");

  var rfvalue = document.getElementById("RF");
  var gfvalue = document.getElementById("GF");
  var bfvalue = document.getElementById("BF");
  var rbvalue = document.getElementById("RB");
  var gbvalue = document.getElementById("GB");
  var bbvalue = document.getElementById("BB");


  var frontPWM = document.getElementById("PWMFront");
  var backPWM = document.getElementById("PWMBack");

  var pwmfvalue = document.getElementById("pwmf");
  var pwmbvalue = document.getElementById("pwmb");

  rfvalue.innerHTML = rf.value;
  gfvalue.innerHTML = gf.value;
  bfvalue.innerHTML = bf.value;
  rbvalue.innerHTML = rb.value;
  gbvalue.innerHTML = gb.value;
  bbvalue.innerHTML = bb.value;


  pwmfvalue.innerHTML = frontPWM.value;
  pwmbvalue.innerHTML = backPWM.value;

  jsonData = {
    rf: rf.value,
    gf: gf.value,
    bf: bf.value,
    rb: rb.value,
    gb: gb.value,
    bb: bb.value,
    brightf: frontPWM.value,
    brightb: backPWM.value
  }
}


