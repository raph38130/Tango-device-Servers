import time

d0=Device("rp/pytester/0")
d1=Device("rp/pytester/1")
d2=Device("rp/pytester/2")
d3=Device("rp/pytester/3")
d4=Device("rp/leds2/r")
d5=Device("rp/leds2/g")
d6=Device("rp/leds2/b")
d7=Device("rp/leds2/w")

g=Group("leds")
g.add(
("rp/pytester/0","rp/pytester/1","rp/pytester/2","rp/pytester/3",
"rp/leds2/r","rp/leds2/g","rp/leds2/b","rp/leds2/w"))

"""
while True:
    time.sleep(1)
    d0.myinput,d1.myinput,d2.myinput,d3.myinput=0.6,0.6,0.6,0.6
    d4.myinput,d5.myinput,d6.myinput,d7.myinput=0.6,0.6,0.6,0.6
    time.sleep(1)
    d0.myinput,d1.myinput,d2.myinput,d3.myinput=0.1,0.1,0.1,0.1
    d4.myinput,d5.myinput,d6.myinput,d7.myinput=0.1,0.1,0.1,0.1
"""

while True:
    time.sleep(1)
    g.write_attribute("MyInput",(0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6),True,True)
    time.sleep(1)
    g.write_attribute("MyInput",(0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1),True,True)



