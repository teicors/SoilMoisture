import web
import json
import sys
import MySQLdb as mdb
import logging
import subprocess
import urllib2
#import requests

from threading import Timer
from time import sleep


class RepeatedTimer(object):
    def __init__(self, interval, function, *args, **kwargs):
        self._timer     = None
        self.function   = function
        self.interval   = interval
        self.args       = args
        self.kwargs     = kwargs
        self.is_running = False
        self.start()

    def _run(self):
        self.is_running = False
        self.start()
        self.function(*self.args, **self.kwargs)

    def start(self):
        if not self.is_running:
            self._timer = Timer(self.interval, self._run)
            self._timer.start()
            self.is_running = True

    def stop(self):
        self._timer.cancel()
        self.is_running = False


urls = (
    '/', 'index',
    '/json', 'data_per_post'
)

class data_per_post:
    def GET(self):
        return "Hello, world! data"

    def POST(self):
        logger = logging.getLogger("process-POST")
        try:
            data = json.loads(web.data())
            print data
#            json.dumps(data)
##            data = urlparse.parse_qs(r)
#            print json.loads(r['payload'][0])
#            json_value = json.loads(data)
            print len(data)
            if len(data)==3:
                led=data["sensorid"]
                count=data["count"]
                basetta=data["basetta"]
            else:
                basetta=data["basetta"]
                led=data["sensorid"]
            con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione'); 
            cur = con.cursor(mdb.cursors.DictCursor)
            presence=0
            if led == 9999:
                presence=1
                print("Received presence")
                sql="select * from illuminazione.matricole where ip='%s';" % (basetta)
                cur.execute(sql)
                if cur.rowcount==0:
                    parts = basetta.split('.')
                    matricola=(int(parts[2])*256+int(parts[3]))
                    sql="insert into illuminazione.matricole (ip,critica,attiva,matricola) values ('%s',0,0,%s);" % (basetta, matricola)
                    cur.execute(sql)
                    con.commit()
                sql="update illuminazione.matricole set attiva=1 where ip='%s';" % (basetta)
                cur.execute(sql)
                con.commit()
                print("Presence of %s" % (basetta))
                sql="select p.puntoluce from puntoluce p, impianto i, matricole m \
                    where time(now()) between p.ora_inizio and p.ora_fine \
                    and p.puntoluce=i.puntoluce \
                    and m.ip='%s' \
                    and i.matricola=m.matricola \
                    and m.attiva=1;" % (basetta)
            else:
                print "i am in show"
                print "Valori Led %s Basetta %s Count %s" % (led, basetta, count)
                logger.debug("Light management")
                sql="select p.puntoluce from puntoluce p, impianto i, matricole m \
                    where time(now()) between p.ora_inizio and p.ora_fine \
                    and p.puntoluce=i.puntoluce \
                    and m.ip='%s' \
                    and i.pulsante=%s \
                    and i.matricola=m.matricola \
                    and m.attiva=1;" % (basetta, led)
            with con:
                cur = con.cursor(mdb.cursors.DictCursor)
                cur.execute(sql)
                if cur.rowcount>0:
                    rows = cur.fetchall()
                    for row in rows:
                        puntoluce=row['puntoluce']
                        if (presence==0):
                            sql="update illuminazione.puntoluce set stato=abs(stato-1) where puntoluce=%s\
                                and (now()) between ora_inizio and ora_fine " % (puntoluce)
                            cur.execute(sql)
                            con.commit()

                        sql="select c.ip, b.pulsante, a.stato from illuminazione.puntoluce a, illuminazione.impianto b, matricole c \
                            where time(now()) between ora_inizio and ora_fine \
                        and b.puntoluce='%s' \
                        and c.matricola=b.matricola \
                        and b.puntoluce=a.puntoluce \
                        and c.attiva=1;" % (puntoluce)

                        cur.execute(sql)
                        if cur.rowcount>0:
                            rows = cur.fetchall()
                            for row in rows:
                                ip=row["ip"]
                                led=row["pulsante"]
                                status=row["stato"]
                                logger.debug("Check Light management IP %s Status %s Led %s " % (ip,status,led))
                                req = 'http://%s/api/output?status=%s&led=%s' % (ip,status,led)
                                urllib2.urlopen(req).read()
                else:
                    logger.debug("Received various data")
            con.close()
            return
        except IOError as e:
            print "I/O error({0}): {1}".format(e.errno, e.strerror)
        except ValueError:
            print "Could not convert data to an integer."
        except:
            print "Unexpected error:", sys.exc_info()[0]
            raise

#        except Error(e):
#            print e

class index:
    def GET(self):
        return "Hello, world!"
    
def send_conf(text):
    con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione');
    cur = con.cursor(mdb.cursors.DictCursor)
    logger.debug("Send configuration")
    sql="select c.ip, b.pulsante, a.stato from illuminazione.puntoluce a, illuminazione.impianto b, matricole c \
         where time(now()) between ora_inizio and ora_fine \
    and a.puntoluce=b.puntoluce \
    and c.matricola=b.matricola \
    and c.attiva=1;"
    cur.execute(sql)
    if cur.rowcount>0:
        rows = cur.fetchall()
        for row in rows:
            ip=row["ip"]
            led=row["pulsante"]
            status=row["stato"]
            logger.debug("Check send_conf IP %s Status %s Led %s " % (ip,status,led))
            req = 'http://%s/api/output?status=%s&led=%s' % (ip,status,led)
            urllib2.urlopen(req).read()
    con.close()

def presence(text):
    con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione');
    cur = con.cursor(mdb.cursors.DictCursor)
    logger.debug("Check presence")
    sql="SELECT ip FROM illuminazione.matricole where attiva=1;"
    cur.execute(sql)
    if cur.rowcount>0:
        rows = cur.fetchall()
        for row in rows:
            ip=row['ip']
            logger.debug("Check IP %s" % (ip))
            try:
                if subprocess.call("ping -c 1 %s" % ip , shell=True) == 0:
                    print ("host appears to be up")
                else:
                    print ("host appears to be down")
                    con1 = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione');
                    cur1 = con1.cursor(mdb.cursors.DictCursor)
                    sql="update illuminazione.matricole set attiva=0 where ip='%s';" % (ip)
                    cur1.execute(sql)
                    con1.commit()  
                    con1.close()
            except:
                print ("Ping Error:", e)
    con.close()


class MyApplication(web.application):
    def run(self, port=5008, *middleware):
        func = self.wsgifunc(*middleware)
        return web.httpserver.runsimple(func, ('10.42.0.1', port))


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    logger = logging.getLogger("server")
    logger.info("starting...")

    rt = RepeatedTimer(600, send_conf, "Send conf") # it auto-starts, no need of rt.start()
    rt = RepeatedTimer(600, presence, "Presence") # it auto-starts, no need of rt.start()

    try:
        app = MyApplication(urls, globals())
        app.run()

    finally:
        rt.stop() # better in a try/finally block to make sure the program ends!
        logger.info("Shutting down")
        if con:    
            con.close()
           
    logger.info("All done")
