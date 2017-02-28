#!/usr/bin/env python

import tornado.ioloop
import tornado.web
from tornado.web import URLSpec as url
import os
import urllib2
import json
import tempfile
from os import listdir
from os.path import isfile, join, isdir, exists
import requests
from time import gmtime, strftime
import datetime
from subprocess import call


DEF_U_REMOTE_USER_FILE="/usr/urtica/etc/users.json"
DEF_U_LOGS_DIR="/usr/urtica/log"
DEF_U_LOGS_DIR_ROTATE="/usr/urtica/log/rotate"
DEF_U_REMOTE_LOG_FILE="/tmp/u-remote.log"


'''
export U_REMOTE_USER_FILE="/tmp/jsonuserfile.txt"
export U_LOGS_DIR="/usr/urtica/log"
export U_LOGS_DIR_ROTATE="/usr/urtica/log/rotate"
'''
'''
DEBUG_BASEDIR="/home/mario/store/urtica-ws/tmp/"
DEF_U_REMOTE_USER_FILE="/usr/urtica/etc/users.json"
DEF_U_LOGS_DIR=DEBUG_BASEDIR+"log"
DEF_U_LOGS_DIR_ROTATE=DEBUG_BASEDIR+"log/rotate"
DEF_U_REMOTE_LOG_FILE=DEBUG_BASEDIR+"u-remote.log"
'''

DEF_U_REMOTE_USER_FILE="/usr/urtica/etc/users.json"
DEF_U_LOGS_DIR="/urs/urtica/log"
DEF_U_LOGS_DIR_ROTATE="/urs/urtica/log/rotate"
DEF_U_REMOTE_LOG_FILE="/tmp/u-remote.log"

def logstr(str):
    fn = os.getenv('U_REMOTE_LOG_FILE', DEF_U_REMOTE_LOG_FILE)
    t=strftime("%Y-%m-%d %H:%M:%S", gmtime())
    s = t+" "+str+"\n"
    with open(fn, "a") as f:
        f.write(s)

class GwUpdateUserHandler(tornado.web.RequestHandler):
    def get(self):
        userFile = os.getenv('U_REMOTE_USER_FILE', DEF_U_REMOTE_USER_FILE)

        link = self.get_argument("link", "", True)
        port = self.get_argument("port", "", True)

        #check form missing link
        if len(link)<=0:
            return self.write("Missing link" )

        # build remote link
        remoteIp =  self.request.remote_ip
        user_dwl_link = "http://"+remoteIp
        if len(port) > 0:
            user_dwl_link += ":"+port
        user_dwl_link += link

        # get the file and check it
        response = urllib2.urlopen(user_dwl_link)
        json_users = response.read()
        try:
            json_object = json.loads( json_users )
        except ValueError, e:
            return self.write("Missing link")

        # save the file to the proper location
        fd, path = tempfile.mkstemp()
        os.write(fd, json_users)
        os.close(fd)
        os.rename(path, userFile)
        self.write( "OK" )


class GwSendLogHandler(tornado.web.RequestHandler):
    def get(self):
        logstr("About to send log files")
        link = self.get_argument("link", "", True)
        port = self.get_argument("port", "", True)

        # check form missing link
        if len(link) <= 0:
            return self.write("Missing link")

        # build remote link
        remoteIp = self.request.remote_ip
        post_log_link = "http://" + remoteIp
        if len(port) > 0:
            post_log_link += ":" + port
        post_log_link += link

        # get the log list
        log_file_path = os.getenv('U_LOGS_DIR', DEF_U_LOGS_DIR)

        if not os.path.exists(log_file_path):
            return "Errors: log directory not set"

        # TODO check errors....
        logs = [f for f in listdir(log_file_path) if isfile(join(log_file_path, f))]

        active_log_file=''
        if os.path.exists( DEF_U_LOGS_DIR+'/urtica-id' ):
            lines = [line.rstrip('\n') for line in open(DEF_U_LOGS_DIR+'/urtica-id')]
            active_log_file=lines[1]

        # build the logrotate directory if not exists
        logrotate_file_path = os.getenv('U_LOGS_DIR_ROTATE', DEF_U_LOGS_DIR_ROTATE)
        if not os.path.exists(logrotate_file_path):
            os.makedirs(logrotate_file_path)

        # send the log file one by one
        for log_file in logs:
            logstr("Found log: " + log_file)

            if log_file == 'urtica-id' or log_file == active_log_file:
                logstr("Skipping")
                continue

            logstr("Sending...")
            fname = log_file_path+'/'+log_file
            files = {'file': open(fname, 'rb')}
            req = requests.post(post_log_link, files=files)
            if req.text == 'OK':
                logstr("Log sent")

                # move the logfile to a backup dir
                os.rename(fname, logrotate_file_path+'/'+log_file)

            else:
                logstr("Problem in sending log "+log_file+" : " + req.text)

        logstr("Finished")
        self.write("FINISHED")

class SyncTimeHandler(tornado.web.RequestHandler):
    def post(self):
        new_curr_time = self.get_argument('new_curr_time', '')
        if len( new_curr_time ) <= 0:
            self.redirect('/?msg=Problema in aggiornamento orario&t=e')
        else:
            strdate_sys =datetime.datetime.fromtimestamp( int(new_curr_time) ).strftime('%Y.%m.%d-%H:%M')
            desktop = os.getenv('U_DESKTOP', "")
            if len(desktop)<=0:
                dt = call(["date", "-s "+strdate_sys])
                hw = call(["hwclock", "--systohc"])
            strdate = datetime.datetime.fromtimestamp( int(new_curr_time) ).strftime('%Y-%m-%d %H:%M:%S')
            logstr("Date time updated to " + strdate)
            self.redirect('/?msg=Tempo aggiornato a '+strdate+' . Reboot requested!')
    def get(self):
        self.write("DWL URL OK")
        self.redirect('/')
        #self.render('index.html', page_title="", body_id="", messages="whatever",title="home")

class DownloadLogHandler(tornado.web.RequestHandler):
   def get(self):
       self.write("DWL URL OK")
       #self.render('index.html', page_title="", body_id="", messages="whatever",title="home")

class MainHandler(tornado.web.RequestHandler):
  def get(self):
      message = self.get_argument("msg", "", True)
      type = self.get_argument("t", "", True)
      message_type = "alert-success"
      if type == "w":
          message_type = "alert-warning"
      elif type == "e":
          message_type = "alert-danger"
      self.render('index.html', message=message, message_type=message_type )



settings = {
   "static_path":"/usr/urtica/u-remote/static",
   "template_path":"/usr/urtica/u-remote/templates",
    "debug":True,
}

handlers = [
    (r"/", MainHandler),
    (r"/gw_update_user", GwUpdateUserHandler),
    (r"/gw_send_log", GwSendLogHandler),
    url(r"/gw_download_log", DownloadLogHandler, name="download_url"),
    url(r"/gw_sync_time", SyncTimeHandler, name="sync_time"),
#    (r'/(favicon\.ico)', tornado.web.StaticFileHandler, {'path': favicon_path})
]




if __name__ == "__main__":
    port = os.getenv('U_REMOTE_PORT', 80)
    machine = os.getenv('URTICA_MACHINE_ID', "")
    if len(machine) <= 0: #assume desktop
        settings["static_path"] = os.path.join(os.path.dirname(__file__), 'static')
        settings["template_path"] = "templates"

    app = tornado.web.Application(handlers, **settings)
    app.listen(port)
    tornado.ioloop.IOLoop.current().start()
