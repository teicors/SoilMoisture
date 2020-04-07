#!/usr/bin/python
import multiprocessing
import socket, os
import MySQLdb as mdb
import urllib, urllib2
from struct import *
from binascii import hexlify
import json

from threading import Timer
from time import sleep

import sys
from time import sleep
from apscheduler.scheduler import Scheduler

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


def handle(connection, address):
#    import logging
#    logging.basicConfig(level=logging.DEBUG)
#
#   Dati in arrivo dalla periferia:
#     IP sorgente dalla connessione
#     tipo di dato
#       xyyy zzzzz 
#            dove x tipo di dato
#                 yyy seriale del tipo di dato
#                 zzzzz valore del dato con ultima cifra decimale
#                 
#
    logger = logging.getLogger("process-%r" % (address,))
    try:
        logger.debug("Connected %r at %r", connection, address)

        source = list(address)
        print source[0]
        while True:
            data = connection.recv(1024)
            if data == "":
                logger.debug("Socket closed remotely")
                break
            connection.close()
            logger.debug("data pre json received %s" % data)
            data=data[data.index('{'):data.rindex('}') + 1]
            logger.debug("Received data %r", data)
            json_value = json.loads(data)
            logger.debug("data json received %s" % json_value)
            data = data.replace('\r\n', '')
            logger.debug("Received data %r", data)
            tipo=9999
            stato=json_value['sensorid']
            led=json_value['led']
            basetta=json_value['basetta']
            
#            MsgToSend["basetta"] = WifiStation.getIP().toString().c_str();
#            MsgToSend["evento"]  = LampMsg.evento;
#            MsgToSend["oggetto"] = LampMsg.oggetto;
#            MsgToSend["linea"]   = LampMsg.linea;
#            MsgToSend["tempo"]   = LampMsg.tempo;
            
            logger.debug("data received %s %s" % (led, stato))
            con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione'); 
            cur = con.cursor(mdb.cursors.DictCursor)
            if tipo == 9999:
                logger.debug("Received presence")
                sql="update illuminazione.matricole set attiva=1 where ip='%s';" % (source)
                cur.execute(sql)
                con.commit()
                logger.debug("Presence of %s" % (source))
            else:
                logger.debug("Light management")
                with con:
#                    cur = con.cursor(mdb.cursors.DictCursor)
                        sql="select puntoluce from illuminazione.puntoluce where time(now()) between ora_inizio \
                        and ora_fine and puntoluce in (select puntoluce from illuminazione.impianto \
                        where matricola =%s and pulsante=%s);" % (idesp, tipo)
                        cur.execute(sql)
                        if cur.rowcount>0:
                            rows = cur.fetchall()
                            for row in rows:
                                puntoluce=row['puntoluce']
                                sql="update illuminazione.puntoluce set stato=abs(stato-1) where puntoluce=%s\
                                    and (now()) between ora_inizio and ora_fine " % (puntoluce)
                                cur.execute(sql)
                                con.commit()
                                sql="select mat.ip, im.pulsante, im.puntoluce, pl.stato from illuminazione.impianto im, \
                                illuminazione.matricole mat, illuminazione.puntoluce pl \
                                where im.puntoluce=%s and mat.matricola=im.matricola and mat.attiva=1 \
                                and pl.puntoluce=im.puntoluce;" % (puntoluce)
                                cur.execute(sql)
                                rows2 = cur.fetchall()
                                for row2 in rows2:                   
                                    HOST = '%s' % row2["ip"]
                                    PORT = 2222
                                    BUTT = '%s' % row2["pulsante"]
                                    STAT = '%s' % row2["stato"]
                                    bytes = "%s%s" % (BUTT, STAT)
                                    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                                    client.connect((HOST,PORT))
                                    client.send(bytes)
                                    client.close()
                                    logger.debug("Sending conf lights %s %s %s" % (HOST, BUTT, STAT))
                        else:
                            logger.debug("Received various data")
            con.close()
            logger.debug("Sent data")
    except:
        logger.exception("Problem handling request")
    finally:
        logger.debug("Closing socket")

class Server(object):
    def __init__(self, hostname, port):
        import logging
        self.logger = logging.getLogger("server")
        self.hostname = hostname
        self.port = port

    def start(self):
        self.logger.debug("listening")
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind((self.hostname, self.port))
        self.socket.listen(1)

        while True:
            conn, address = self.socket.accept()
            self.logger.debug("Got connection")
            process = multiprocessing.Process(target=handle, args=(conn, address))
            process.daemon = True
            process.start()
            self.logger.debug("Started process %r", process)

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
                if os.system("ping -c 1 %s" % ip ) == 0:
                    print ("host appears to be up")
                    sql="update illuminazione.matricole set attiva=1 where ip='%s';" % (ip)
                    cur.execute(sql)
                    con.commit()              
            except:
                print ("Ping Error:", e)

def send_conf(text):
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
                if os.system("ping -c 1 %s" % ip ) == 0:
                    print ("host appears to be up")
                    sql="update illuminazione.matricole set attiva=1 where ip='%s';" % (ip)
                    cur1.execute(sql)
                    con1.commit()  
#                    sql="select matricola from illuminazione.matricole where ip='%s';" % (ip)
#                    cur1.execute(sql)
#                    if cur1.rowcount>0:
#                        rows1 = cur1.fetchall()
#                        for row1 in rows1:
#                            mat=row['matricola']
#                            PORT = 2222
#                            bytes = "MATR %s" % (mat)
#                            client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#                            client.connect((HOST,PORT))
#                            client.send(bytes)
#                            client.close()
            except:
                print ("Ping Error:", e)

def hello(name):
    logger.info("Hello %s!" % name)

def ciao(name):
    logger.info("Ciao %s!" % name)

def ReloadPic():
    logger.info("Reload conf PIC!")
    con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione');
    with con:
        cur = con.cursor(mdb.cursors.DictCursor)
        sql="select puntoluce from puntoluce where stato=1"
        cur.execute(sql)
        rows = cur.fetchall()
        for row in rows:
            puntoluce=row['puntoluce']
            sql="select mat.ip, im.pulsante, im.puntoluce from illuminazione.impianto im, illuminazione.matricole mat where im.puntoluce=%s and mat.matricola=im.matricola and mat.attiva=1;" % (puntoluce)
            cur.execute(sql)
            rows = cur.fetchall()
            for row in rows:
                 HOST = '%s' % row["ip"]
#                 HOST = ipaddr.ip_address(row["ip"])
#                 PORT = 2222
                 PORT = 80
                 BUTT = '%s' % row["pulsante"]
                 PUNT = '%s' % row["puntoluce"]
                 logger.info(HOST)
#                 bytes = "%s %s" % (HOST, BUTT)
                 bytes = "%s" % (PUNT)
                 client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                 client.connect((HOST,PORT))
                 client.send(bytes)
                 client.close()
                 logger.debug("Sending conf lights %s %s %s" % (HOST, BUTT, PUNT))
    con.close()        

def GetVarData60():
    logging.info("GetVarData 60 sec")


if __name__ == "__main__":
    import logging
    logging.basicConfig(level=logging.DEBUG)
    logger = logging.getLogger("server")

#    hdlr = logging.FileHandler('/tmp/server.log')
#    formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
#    hdlr.setFormatter(formatter)
#    logger.addHandler(hdlr) 
#    logger.setLevel(logging.DEBUG)

    sched = Scheduler()
    sched.start()        # start the scheduler

# from now on, execute my_job every minute
    job = sched.add_interval_job(hello, minutes=1, args=['text'])

#    server = Server("192.168.150.1", 5008)
#   la connessione al server python  sulla porta 5008
#   la connessione al client  sulla porta 2222
    server = Server("10.42.0.1", 5008)
    con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione');

    logger.info("starting...")
    rt = RepeatedTimer(300, send_conf, "Send conf") # it auto-starts, no need of rt.start()
    rt = RepeatedTimer(60, presence, "Presence") # it auto-starts, no need of rt.start()
#    rt = RepeatedTimer(15, hello, "World") # it auto-starts, no need of rt.start()
#    rt = RepeatedTimer( 5, ciao, "Angelo") # it auto-starts, no need of rt.start()
    ReloadPic()
    GetVarData60()

    try:
        logger.info("Listening")
        server.start()
    except:
        logger.exception("Unexpected exception")
    finally:
        rt.stop() # better in a try/finally block to make sure the program ends!
        logger.info("Shutting down")
        if con:    
            con.close()
        for process in multiprocessing.active_children():
            logger.info("Shutting down process %r", process)
            process.terminate()
            process.join()
           
    logger.info("All done")

