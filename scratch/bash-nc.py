import re
import sys

def topo(filename,consumerNum):
        #f= open(filename, "w")
        for i in range(0,consumerNum):               
            f.write( '''Ptr<Node> Node%d = Names::Find<Node>("%d");\n''' % (int(i*100/consumerNum),int(i*100/consumerNum)))
#        for i in range(0,consumerNum): 
#       		f.write( '''ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node%d);\n''' % int(i*100/consumerNum))
        f.write( '''ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node%d);\n''' % int(30))
        f.write( '''ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node%d);\n''' % int(60))
        f.write('''ndn::AppHelper Client("ns3::ndn::ConsumerCbrNC");\n''')
        f.write('''//ndn::AppHelper Client("ns3::ndn::ConsumerZipfMandelbrot");\n''')
        f.write('''Client.SetPrefix("/ndn/vod/nc");\n''')
        f.write('''Client.SetAttribute("Frequency", StringValue("%s"));\n''' %sys.argv[4])
      #  f.write('''Client.SetAttribute("Frequency", DoubleValue(3));\n''')
        f.write('''Client.SetAttribute("Randomize", StringValue ("uniform"));\n''')
     #   f.write('''Client.SetAttribute("Randomize", StringValue ("exponential"));n\''')
        for i in range(0,consumerNum): 
	    if int(i*100/consumerNum)==30 : continue
	    if int(i*100/consumerNum)==60 : continue
         #   f.write( '''Client.Install (Node%d);\n''' % int(i*100/consumerNum))
	    f.write( '''ApplicationContainer app%d = Client.Install (Node%d);\n''' %(int(i*100/consumerNum), int(i*100/consumerNum)))
            f.write('''app%d.Start(Seconds(%d));\n''' %(int(i*100/consumerNum),3*i))
        
#consumerNum = int(input('Please enter the Number of Consumer (0-100):'))
###############write variable into file################
f_result=open("data-result.txt","a")
f_result.write("userNum=%s CS=%s runTime=%s frequency=%s Freshness=%s \n" %(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5]))
f_result.close()
#######################################################
consumerNum = int(sys.argv[1])
fr=open("scratch/auto-copy.txt", "r")
f=open("scratch/auto-nc-test-zfx.cc", "w")
for i in range(1,49):
    s=fr.readline()
    f.write(s)
#f.write('''ccnxHelper.SetContentStore ("ns3::ndn::cs::LifetimeBasedGreedy::Lru","MaxSize","%s"); ''' %sys.argv[2])
f.write('''ccnxHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize","%s"); ''' %sys.argv[2])
fr.readline()
for i in range(50,59):
    s=fr.readline()
    f.write(s)
topo('''topo.txt''',consumerNum)
for i in range(59,69):
    s=fr.readline()
for i in range(69,81):
    s=fr.readline()
    f.write(s)
f.write('''consumerHelper.SetAttribute("Freshness", TimeValue (Seconds (%d)));\n''' %float(sys.argv[5]))
#f.write('''consumerHelper.SetAttribute("Freshness", TimeValue (Seconds(1)));\n''' )

for i in range(81,105):
    s=fr.readline()
    f.write(s)
f.write('''Simulator::Stop (Seconds (%d)); \n''' %int(sys.argv[3]))
fr.readline()
for i in range(106,117):
    s=fr.readline()
    f.write(s)
fr.close()
f.close()
#filename = input("Please enter print z
