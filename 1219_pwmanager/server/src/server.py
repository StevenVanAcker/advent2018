#!/usr/bin/env python3

import time, os, cgi, random, string, json
from socketserver import ThreadingMixIn
from http.server import BaseHTTPRequestHandler, HTTPServer


config = json.load(open("config.json"))

SITE_DOMAIN = config["sitedomain"]
SITE_PORT = config["siteport"]
ADS_DOMAIN = config["adsdomain"]
ADS_PORT = config["adsport"]
USERNAME = config["username"]
PASSWORD = config["password"]
FLAG = open("flag").read().strip()


BANNER_URL = "http://"+ADS_DOMAIN+":"+ADS_PORT+"/static/img/adbanner.jpg"
AD_URL = "http://"+ADS_DOMAIN+":"+ADS_PORT+"/ad/"
STATICDIR = "/static/"
AD_DIR = "ads"

def storeAd(msg):
    ad_id = ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(10))
    with open("ads.dir/" + AD_DIR + "/" + ad_id, "w") as fp:
        fp.write(msg)
    return ad_id

def getHostname(headers):
    if "host" in headers:
        return headers["host"].lower().split(":")[0]
    return None

def serveStaticFile(rawtrustedpath, path):

    trustedpathmap = {
        SITE_DOMAIN: "site.dir", 
        ADS_DOMAIN: "ads.dir",
        ADS_DOMAIN + "/" + AD_DIR : "ads.dir/" + AD_DIR
    }

    # check if we know the trustedpath
    if not rawtrustedpath in trustedpathmap:
        return (404, b"", "text/html", [], None)

    trustedpath = trustedpathmap[rawtrustedpath]

    # check if path is in trustedpath
    fullpath = os.path.realpath(os.path.join(trustedpath, path))

    lala = "/opt/pwmanager/server/" + trustedpath + "/"
    if not fullpath.startswith(lala):
        print("lala = {}".format(lala))
        return (404, b"evil haxx0r" , "text/html", [], None)

    if not os.path.isfile(fullpath):
        print("not a file {}".format(fullpath))
        return (404, b"evil haxxor", "text/html", [], None)

    # get the mimetype
    mimetypes = {
        ".html": "text/html",
        ".css": "text/css",
        ".png": "image/png",
        ".jpg": "image/jpeg",
        ".gif": "image/gif",
        ".js": "application/javascript",
        ".json": "application/json",
    }

    contenttype = "application/octet-stream"

    for (e,m) in mimetypes.items():
        if fullpath.endswith(e):
            contenttype = m
            break

    return (200, open(fullpath, "rb").read(), contenttype, [], fullpath)



def fetch_GET(hostname, path):
    (httpcode, content, contenttype, headers, fullpath) = (404, b"", "text/html", [], None)
    
    if hostname == None:
        return (httpcode, content, contenttype, headers, fullpath)

    # serve static files
    if path.startswith(STATICDIR):
        subpath = path[len(STATICDIR):]
        return serveStaticFile(hostname, subpath)

    # dynamic files
    if hostname.lower() == SITE_DOMAIN:
        if path in ["/login.php"]:
            return serveStaticFile(SITE_DOMAIN, "login.html")
        else:
            if path.startswith( "/?" ):
                ad_id = path[2:]

                # try to fetch the content
                (hc,c,_,_,fp) = serveStaticFile(ADS_DOMAIN + "/" + AD_DIR,  ad_id)
                if hc != 200:
                    return (httpcode, content, contenttype, headers, fp)
                fullpath = fp

                # delete the file
                try:
                    os.remove(fp)
                except OSError:
                    print("File didn't exist anymore...")
                    pass

                title = c.decode("utf-8").strip()
            else:
                title = "Not showing any subliminal message"

            # load and render the template
            (_,template,_,_,_) = serveStaticFile(SITE_DOMAIN, "template.html")
            adhtml = "<img src='{}' title='{}'>".format(BANNER_URL, title)
            httpcode = 200
            content = template.decode("utf-8").replace("{{BANNERURL}}", BANNER_URL).replace("{{AD}}", adhtml).encode("utf-8")


    elif hostname.lower() == ADS_DOMAIN:
        if path in ["", "/"]:
            return serveStaticFile(ADS_DOMAIN, "form.html")
        elif path.startswith("/ad/"):
            (cmd, ad_id) = path[1:].split("/", 1)
            (hc,c,_,_,_) = serveStaticFile(ADS_DOMAIN + "/" + AD_DIR,  ad_id)
            if hc != 200:
                return (httpcode, content, contenttype, headers, fullpath)

            # load and render the template
            (_,template,_,_,_) = serveStaticFile(ADS_DOMAIN, "template.html")
            httpcode = 200
            content = template.decode("utf-8").replace("{{BANNERURL}}", BANNER_URL).replace("{{AD}}", c.decode("utf-8").strip()).encode("utf-8")

    return (httpcode, content, contenttype, headers, fullpath)

def fetch_POST(hostname, path, params):
    (httpcode, content, contenttype, headers, fullpath) = (404, b"", "text/html", [], None)
    
    if hostname == None:
        return (httpcode, content, contenttype, headers, fullpath)

    # dynamic files
    if hostname.lower() == SITE_DOMAIN:
        if path in ["/login.php"]:
            # check username and password, render result with flag if success
            un = ""
            pw = ""
            try:
                un = params["username"][0]
                pw = params["password"][0]
            except:
                pass

            if un == USERNAME and pw == PASSWORD:
                # load and render the template
                (_,template,_,_,_) = serveStaticFile(SITE_DOMAIN, "login-success.html")
                httpcode = 200
                content = template.decode("utf-8").replace("{{FLAG}}", FLAG).encode("utf-8")
            else:
                return serveStaticFile(SITE_DOMAIN, "login-fail.html")
    elif hostname.lower() == ADS_DOMAIN:
        if path in ["", "/"]:
            msg = ""
            try:
                msg = params["msg"][0]
            except:
                pass

            if len(msg) > 5000:
                content = b"FAIL: Please limit your message to 5000 characters"
                httpcode = 200
            else:
                ad_id = storeAd(msg)

                (_,template,_,_,_) = serveStaticFile(ADS_DOMAIN, "form-reply.html")
                httpcode = 200
                content = template.decode("utf-8").replace("{{LINK}}", AD_URL + ad_id).encode("utf-8")

    return (httpcode, content, contenttype, headers, fullpath)




















class ThreadingSimpleServer(ThreadingMixIn, HTTPServer):
    pass

class S(BaseHTTPRequestHandler):
    def _set_headers(self, httpcode, contenttype, headers):
        self.server_version = "SantaServer/1.0"
        self.sys_version = ""
        self.send_response(httpcode)
        self.send_header('Content-type', contenttype)
        for (h,v) in headers:
            self.send_header(h,v)
        self.end_headers()

    def do_GET(self):
        (errorcode, content, contenttype, headers, _) = fetch_GET(getHostname(self.headers), self.path)
        self._set_headers(errorcode, contenttype, headers)
        self.wfile.write(content)
        
    def do_POST(self):
        postdata = {}
        try:
            postdatablob = self.rfile.read(int(self.headers.get('Content-Length'))).decode("utf-8")
            postdata = cgi.parse_qs(postdatablob, keep_blank_values=1)
        except:
            print("Something went wrong parsing QS")

        (errorcode, content, contenttype, headers, _) = fetch_POST(getHostname(self.headers), self.path, postdata)
        self._set_headers(errorcode, contenttype, headers)
        self.wfile.write(content)
        
def run(server_class=ThreadingSimpleServer, handler_class=S, port=80):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print('Starting httpd...')
    httpd.serve_forever()

if __name__ == "__main__":
    from sys import argv
    run(port=int(config["siteport"]))
