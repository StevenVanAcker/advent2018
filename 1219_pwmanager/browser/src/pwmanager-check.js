/*
 * Check whether we are on the right page
 */
(function() {
    var endsWith = function(str, suffix) {
		return str.indexOf(suffix, str.length - suffix.length) !== -1;
    };

    if(window.location.host != "NETLOC") {
		window.pwmanagerLog = "Failure: Not on the correct host ("+window.location.host+" <> NETLOC)";
		return false;
    }

    if(!endsWith(window.location.href, "/login.php")) {
		window.pwmanagerLog = "Failure: URL does not end in /login.php";
		return false;
    }

    if(document.forms.length == 0) {
		window.pwmanagerLog = "Failure: No forms present";
		return false;
    }

    if(!endsWith(document.forms[0].action, "/login.php")) {
		window.pwmanagerLog = "Failure: Login form action does not end in /login.php";
		return false;
    }

    if(document.forms[0].getElementsByTagName("input").length != 2) {
		window.pwmanagerLog = "Failure: Login form does not contain 2 <input> elements";
		return false;
    }

    if(document.forms[0].getElementsByTagName("input")[0].name != "username") {
		window.pwmanagerLog = "Failure: Name of first <input> is not 'username'";
		return false;
    }

    if(document.forms[0].getElementsByTagName("input")[0].type != "text") {
		window.pwmanagerLog = "Failure: Type of first <input> is not 'text'";
		return false;
    }

    if(document.forms[0].getElementsByTagName("input")[1].name != "password") {
		window.pwmanagerLog = "Failure: Name of second <input> is not 'password'";
		return false;
    }

    if(document.forms[0].getElementsByTagName("input")[1].type != "password") {
		window.pwmanagerLog = "Failure: Type of second <input> is not 'password'";
		return false;
    }

    window.pwmanagerLog = "Success! Valid login form recognized.";
    return true;
})();
