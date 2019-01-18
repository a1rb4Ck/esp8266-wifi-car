/*!
 * car.js
 * Version: 0.1
 *
 * Copyright 2019 Pierre Nagorny
 * Released under the MIT license
 */

/* * * * * * * * * * * * */
/* Robot motors control  */
/* * * * * * * * * * * * */
if ('ontouchstart' in window) {
    document.getElementById("forward").addEventListener("touchstart",
        function() { forwardDown(); });
    document.getElementById("forward").addEventListener("touchend",
        function() { release(); });
    document.getElementById("backward").addEventListener("touchstart",
        function() { backwardDown(); });
    document.getElementById("backward").addEventListener("touchend",
        function() { release(); });
    document.getElementById("left").addEventListener("touchstart",
        function() { leftDown(); });
    document.getElementById("left").addEventListener("touchend",
        function() { release(); });
    document.getElementById("right").addEventListener("touchstart",
        function() { rightDown(); });
    document.getElementById("right").addEventListener("touchend",
        function() { release(); });
} else {
    document.getElementById("forward").addEventListener("mousedown",
        function() { forwardDown(); });
    document.getElementById("forward").addEventListener("mouseup",
        function() { release(); });
    document.getElementById("backward").addEventListener("mousedown",
        function() { backwardDown(); });
    document.getElementById("backward").addEventListener("mouseup",
        function() { release(); });
    document.getElementById("left").addEventListener("mousedown",
        function() { leftDown(); });
    document.getElementById("left").addEventListener("mouseup",
        function() { release(); });
    document.getElementById("right").addEventListener("mousedown",
        function() { rightDown(); });
    document.getElementById("right").addEventListener("mouseup",
        function() { release(); });
}
var dataDiv = document.getElementById("data");
document.onkeydown = function(e) {
    e = e || window.event;
    switch (e.which || e.keyCode) {
        case 38:
            forwardDown();
            break;
        case 40:
            backwardDown();
            break;
        case 37:
            leftDown();
            break;
        case 39:
            rightDown();
            break;
    }
};
document.onkeyup = function(e) { release(); };
// const hostname = location.hostname ? location.hostname : 'localhost';
if (location.hostname) {
    var hostname = location.hostname;
} else {
    var hostname = 'localhost';
}
var connection = new WebSocket('ws://' + hostname + ':81/', ['arduino']);
connection.onopen = function() {
    connection.send('Connect ' + new Date());
    document.getElementById("wsmsg").innerHTML = "Connected";
    document.getElementById("loader").style.display = "none";
    document.getElementById("main").style.display = "flex";
};
connection.onerror = function(error) {
    console.log('WebSocket Error ', error);
    document.getElementById("loader").style.display = "block";
    document.getElementById("main").style.display = "none";
    document.getElementById("wsmsg").innerHTML = error;
};
connection.onmessage = function(e) {
    console.log('Server: ', e.data);
    dataDiv.innerHTML = e.data;
    var sonars = e.data.split(',');
    var graph_update=[sonars[1], sonars[2], 0, 0, 0, 0, 0, 0, sonars[0]];
    chart.config.data.datasets[0].data = graph_update;
    chart.update();
};

function forwardDown() { connection.send('#F'); }

function backwardDown() { connection.send('#B'); }

function leftDown() { connection.send('#L'); }

function rightDown() { connection.send('#R'); }

function release() { connection.send('#0'); }

/* * * * * * * * * * * * */
/*   Sonar radar chart   */
/* * * * * * * * * * * * */
var chartCanvas = document.getElementById("chart");
console.log(chartCanvas);
var chart = new Chart(chartCanvas, {
    type: 'radar',
    options: {
        responsive: true,
        animation: {
            duration: 250,
            easing: 'easeInOutQuad'
        },
        scale: {
            ticks: {
                beginAtZero: true,
                max: 40,
                stepSize: 5,
            },
            pointLabels: {
                fontColor: "dark",
                fontSize: 15,
            },
        },
        tooltips: {
            enabled: false,
        }
    },
    data: {
        labels: ['0°', '40°', '', '120°', '', '', '-120°', '', '-40°'],
        datasets: [{
            data: [36, 36, 0, 0, 0, 0, 0, 0, 36],
            backgroundColor: "rgba(0,0,200,0.1)",
            pointBackgroundColor: "rgba(200,0,0,0.7)",
            borderColor: "rgba(0,0,200,0.5)",
            borderWidth: 1,
            borderRadius: 10,
            pointBorderColor: "#fff",
            pointRadius: 4,
        }]
    }
});
chart.options.legend.display = false;

// var myVar;
// function myFunction() {
//   myVar = setTimeout(showPage, 200);
//   myVar = setTimeout(nothing, 6000);
//   // myVar = setTimeout(wsError, 8000);
//   myVar = setTimeout(nothing, 10000);
// }
// function showPage() {
//   document.getElementById("wsmsg").innerHTML = "Connected";
//   document.getElementById("loader").style.display = "none";
//   document.getElementById("main").style.display = "flex";
// }
// function nothing() {}
// function wsError() {
//     console.log('WebSocket Error ', "error");
//     document.getElementById("loader").style.display = "block";
//     document.getElementById("main").style.display = "none";
//     document.getElementById("wsmsg").innerHTML = "error";
// }
//   setInterval(function(){
//   chart.config.data.datasets[0].data[0] = Math.random() * 36;
//   chart.update();
// }, 1000);