import re
 
def topo(f,consumerNum):
        f.write( '''Ptr<Node> Node%d = Names::Find<Node>("%d");\n''' % (50,50))
	for j in range(0,int(consumerNum/10)):	
        	for i in range(0,int(10)):
			if int(j*10+i)==50 : continue	
            		f.write( '''Ptr<Node> Node%d = Names::Find<Node>("%d");\n''' % (int(j*10+i),int(i*10+1)))
	f.write( '''ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node%d);\n''' % 50)
	for j in range(0,consumerNum/10):	
        	for i in range(0,10): 
			if int(j*10+i)==50 : continue	
	       		f.write( '''ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node%d);\n''' % int(j*10+i))
	f.write('''ndn::AppHelper Client("ns3::ndn::ConsumerCbr");\n''')
	f.write('''//ndn::AppHelper Client("ns3::ndn::ConsumerZipfMandelbrot");\n''')
	f.write('''Client.SetPrefix("/ndn/vod/nc");\n''')
	f.write('''Client.SetAttribute("Frequency", StringValue("2"));\n''')
	for j in range(0,consumerNum/10):	
        	for i in range(0,10): 
			if int(j*10+i)==50 : continue	
		        f.write( '''Client.Install (Node%d);\n''' % int(j*10+i))
        
consumerNum = int(input('Please enter the Number of Consumer (0-100):'))
fr=open("scratch/auto-copy-ndn.txt", "r")
f=open("scratch/auto-ndn-test-zfx.cc", "w")
for i in range(1,58):
    s=fr.readline()
    f.write(s)
topo(f,consumerNum)
for i in range(58,94):
    s=fr.readline()
for i in range(94,140):
    s=fr.readline()
    f.write(s)
fr.close()
f.close()
#filename = input("Please enter print z
