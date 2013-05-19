#!/usr/bin/env python                                                            
                                                                                 
import socket                                                                    
import select                                                                    
import time                                                                      

CONNECT = b'\x00'
ACK     = b'\x01'
                                                                                 
def connect(dest_addr):                                                          
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)                      
    sock.setblocking(0)                                                          
    sock.bind(('0.0.0.0', dest_addr[1]))                                         
    while True:                                                                  
        sock.sendto(CONNECT, dest_addr)                                        
        time.sleep(1)                                                            
        ready =  select.select([sock], [], [], 1)                                
        if ready[0]:                                                             
            data, from_addr = sock.recvfrom(1024)                                
            if from_addr[0] != dest_addr[0]:                                     
                continue                                                         
            if data == CONNECT:                                                
                sock.sendto(ACK, from_addr)                                    
                print('have inbound signal')                                     
            elif data == ACK:                                                  
                print('connected to ', from_addr)                                               
                return                                                           
                                                                                 

