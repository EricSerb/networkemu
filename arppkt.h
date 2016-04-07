
class ArpPacket{
	public:
		ArpPacket();
		
		~ArpPacket();
		
		
	private:
	
	short op; /* op =0 : ARP request; op = 1 : ARP response */
	IPAddr srcip;
	MacAddr srcmac;
	IPAddr dstip;
	MacAddr dstmac;
	
};