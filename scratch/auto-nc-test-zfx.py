import re
import sys

def topo(filename,consumerNum):
        #f= open(filename, "w")
        for i in range(0,consumerNum):               
            f.write( '''Ptr<Node> Node%d = Names::Find<Node>("%d");\n''' % (int(i*100/consumerNum),int(i*100/consumerNum)))
        for i in range(0,consumerNum): 
            f.write( '''ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node%d);\n''' % int(i*100/consumerNum))
        f.write('''ndn::AppHelper Client("ns3::ndn::ConsumerCbr");\n''')
        f.write('''//ndn::AppHelper Client("ns3::ndn::ConsumerZipfMandelbrot");\n''')
        f.write('''Client.SetPrefix("/ndn/vod/nc");\n''')
        f.write('''Client.SetAttribute("Frequency", StringValue("14"));\n''')
      #  f.write('''Client.SetAttribute("Frequency", DoubleValue(3));\n''')
        f.write('''Client.SetAttribute("Randomize", StringValue ("uniform"));\n''')
        for i in range(0,consumerNum): 
	    if int(i*100/consumerNum)==50 : continue
            f.write( '''Client.Install (Node%d);\n''' % int(i*100/consumerNum))
        
#consumerNum = int(input('Please enter the Number of Consumer (0-100):'))
consumerNum = int(sys.argv[1])
fr=open("scratch/auto-copy.txt", "r")
f=open("scratch/auto-nc-test-zfx.cc", "w")
for i in range(1,59):
    s=fr.readline()
    f.write(s)
topo('''topo.txt''',consumerNum)
for i in range(59,69):
    s=fr.readline()
for i in range(69,117):
    s=fr.readline()
    f.write(s)
fr.close()
f.close()
#filename = input("Please enter print z
