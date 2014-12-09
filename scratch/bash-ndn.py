import re
import sys

def topo(filename,consumerNum):
        #f= open(filename, "w")
        for i in range(0,consumerNum):               
            f.write( '''Ptr<Node> Node%d = Names::Find<Node>("%d");\n''' % (int(i*100/consumerNum),int(i*100/consumerNum)))
#        for i in range(0,consumerNum): 
 #           f.write( '''ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/ndn", Node%d);\n''' % int(i*100/consumerNum))
        f.write( '''ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/ndn", Node%d);\n''' % 30)
        f.write( '''ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/ndn", Node%d);\n''' % int(60))
        f.write('''ndn::AppHelper Client("ns3::ndn::ConsumerCbr");\n''')
        f.write('''//ndn::AppHelper Client("ns3::ndn::ConsumerZipfMandelbrot");\n''')
        f.write('''Client.SetPrefix("/ndn/vod/ndn");\n''')
        f.write('''Client.SetAttribute("Frequency", StringValue("%s"));\n''' %sys.argv[4])
      #  f.write('''Client.SetAttribute("Frequency", DoubleValue(3));\n''')
        f.write('''Client.SetAttribute("Randomize", StringValue ("uniform"));\n''')
        for i in range(0,consumerNum): 
	    if int(i*100/consumerNum)==30 : continue
	    if int(i*100/consumerNum)==60 : continue
            f.write( '''Client.Install (Node%d);\n''' % int(i*100/consumerNum))
        
#consumerNum = int(input('Please enter the Number of Consumer (0-100):'))
###############write variable into file################
f_result=open("data-result-ndn.txt","a")
f_result.write("userNum=%s CS=%s runTime=%s frequency=%s \n" %(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]))
f_result.close()
#######################################################
consumerNum = int(sys.argv[1])
fr=open("scratch/auto-copy-ndn.txt", "r")
f=open("scratch/auto-ndn-test-zfx.cc", "w")
for i in range(1,48):
    s=fr.readline()
    f.write(s)
f.write('''ccnxHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize","%s"); ''' %sys.argv[2])
fr.readline()
for i in range(49,58):
    s=fr.readline()
    f.write(s)
topo('''topo.txt''',consumerNum)
for i in range(58,94):
    s=fr.readline()
for i in range(94,129):
    s=fr.readline()
    f.write(s)
f.write('''Simulator::Stop (Seconds (%d)); \n''' %int(sys.argv[3]))
fr.readline()
for i in range(130,140):
    s=fr.readline()
    f.write(s)
fr.close()
f.close()
#filename = input("Please enter print z
