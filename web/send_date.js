
function timedCount() {
  var d = new Date();
  var n = d.getTime();
  postMessage(n);
  setTimeout("timedCount()",1000);
}

timedCount();
