var system = require('system');
var page = require('webpage').create();
page.settings.userAgent = 'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/37.0.2062.120 Safari/537.36';

var fs = require('fs');
var config = JSON.parse(fs.read('config.json'));
var pwmanagercheck = fs.read('pwmanager-check.js');
var netloc = config["sitedomain"] + ":" + config["siteport"]
pwmanagercheck = pwmanagercheck.replace(/NETLOC/g, netloc);

var ad_id = "";


if(system.args.length > 1) {
    ad_id = system.args[1];
}

page.open('http://'+netloc+'/?'+ad_id, function() {
  setTimeout(function(){
	  var isLoginPage = page.evaluate(function(c) { return eval(c); }, pwmanagercheck);
	  console.log(ad_id + ": " + page.evaluate(function() { return window.pwmanagerLog}));

	  if(isLoginPage) {
		  page.evaluate(function(un, pw) { 
		  document.forms[0].getElementsByTagName("input")[0].value = un;
		  document.forms[0].getElementsByTagName("input")[1].value = pw;
		  }, config["username"], config["password"]);
	  }

	  setTimeout(function(){
		  //page.render('screenshot.png');
		  phantom.exit();
	  }, 5000); // wait 5 seconds after password injected
  }, 5000); // wait 1 second for page to load
});

