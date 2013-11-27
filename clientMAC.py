#!/usr/bin/python

import socket
import sqlite3
import os

#DB communication initialization
conn = sqlite3.connect('~/moe/mac.db')
curs = conn.cursor()

def update_db(mac_addr, value):
    query = "update mac set isin="+str(value)+" where macno=\'"+mac_addr.upper()+"\';"
    curs.execute(query)
    
def check_db(mac_addr):
    #search for mac in DB
    query = "select * from mac where macno=\'"+mac_addr.upper()+"\';"
    curs.execute(query)
    result = curs.fetchone()
    if(result!=None):
        return 1
    else:
        return 0
    
def are_you_alive(ip_addr, mac_addr):
    #ping with Linux ping
    print "Pinging "+ip_addr+"..."
    value = 0
    ans = os.system("ping "+ip_addr+" -c 2 -t 40 > /dev/null")
    #if ans>0, host disconnected
    if ans:
        #user not in --> isin=0
        print "Is dead\n"
        value = 0
        update_db(mac_addr, value)
    #if ans=0, host online
    else:
        #user not in --> isin=0
        print "Is alive\n"
        value = 1
        update_db(mac_addr, value)    

#address and port
host = socket.gethostbyname("160.80.101.36")
port = 3617

#socket initialization
for res in socket.getaddrinfo(host, port, socket.AF_INET, socket.SOCK_STREAM):
    af, socktype, proto, canonname, sa = res
    
    sock = socket.socket(af, socktype, proto)
    #socket option REUSABLE
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    try:
        sock.connect(sa)
    except socket.error as msg:
        print str(msg)
        sock.close()
        exit(1)

#temporary variable for msg received
msg = ""

#dictionary for mac->ip memorization
msg_final = {}

ip = ""
mac =  ""
i=0
while msg!="z":
    if i%2==0:
        size = 14
    else:
        size = 18
    i=i+1
    msg = sock.recv(size)
    #error check
    if msg=="h":
        print "Errore in invio dati da server!\n"
        exit(1)
    
    elif msg!="z":
        #ip received
        if msg[-1]=="x":
            ip = msg.split("x")
        #mac received
        elif msg[-1]=="y":
            mac = msg.split("y")
            if(ip!="" and check_db(mac[0])==1):
                are_you_alive(ip[0],mac[0])

#socket closure
sock.close()
        
#saving DB changes
conn.commit()
conn.close()
print "Execution complete!"      
exit(0)
