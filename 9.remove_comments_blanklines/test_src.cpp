#ifndef _RS232_H_
#define _RS232_H_
/* the maximum number of ports we are willing to open */
#define MAX_PORTS 4
/*this array hold information about each port we have opened */
struct PortInfo{
   int busy;
   char name[32];
   int handle;
  };
int OpenCom(int portNo,const char deviceName[],long baudRate);
int CloseCom(int portNo);
int ComRd(int portNo,char buf[],int maxCnt,int Timeout);
int ComWrt(int portNo,const char * buf,int maxCnt);
//long GetBaudRate(long baudRate);
//int OpenComConfig(int port,
//                  const char deviceName[],
//                  long baudRate,
//                  int parity,
//                  int dataBits,
//                  int stopBits,
//                  int iqSize,
//                  int oqSize);
#endif
