#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")
#define BuffSize 2048

int main(int argc, char *argv[])
{

    if(argc != 3){
       printf("Example: download.exe img.netbian.com /file/2022/0607/002401aE1VA.jpg\n");
       system("pause");
       return 0;
    }
      
    WSADATA wsaData;
    int err;

    err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != 0) 
        return -1;
  
    SOCKET m_handleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_handleSocket == INVALID_SOCKET)
       return -1;
    
    struct hostent *h;
    h = gethostbyname(argv[1]);     //img.netbian.com
 
    sockaddr_in m_targetAddr; 
    m_targetAddr.sin_family = AF_INET;          
    m_targetAddr.sin_port = htons(80);          
    memcpy( &m_targetAddr.sin_addr, h->h_addr, h->h_length);

    err = connect(m_handleSocket, (sockaddr*)&m_targetAddr, sizeof(m_targetAddr));
    if(err == SOCKET_ERROR)
       return -1;
    
    char strSend[256];
    //"GET /file/2022/0607/002401aE1VA.jpg HTTP/1.1\r\nHost: img.netbian.com\r\nConnection: close\r\n\r\n";
    sprintf(strSend,"GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", argv[2], argv[1]);
    printf("%s",strSend);
    send(m_handleSocket,strSend,256,0);

    char *pFileName = argv[2];
    int m_nameLen = strlen(pFileName); 
    char m_fileName[10];
    memcpy(m_fileName, &pFileName[m_nameLen-8],10);

    FILE *fp; 
    fp = fopen(m_fileName,"wb");          
   
	char c;
   char buffer[BuffSize];

   int nRecv = 0;
   int bJpgPos = 0;
   char *pStr = &c;
   int  pSize = 1;
   unsigned long int m_bytes = 0;

	while(1){
	   nRecv = recv(m_handleSocket, pStr, pSize*sizeof(char), 0);
      
      switch(bJpgPos){        //compare "\r\n\r\n"
      case 0:
         if(c == 0x0D)     
            bJpgPos = 1;
            else
            bJpgPos = 0;
      break;
      case 1:
         if(c == 0x0A)
            bJpgPos = 2;
            else
            bJpgPos = 0;
      break;
      case 2:
         if(c == 0x0D)
            bJpgPos = 3;
            else
            bJpgPos = 0;
      break;
      case 3:
         if(c == 0x0A){
              bJpgPos = 4;
             
              pSize = BuffSize;      //ready for recv  jpg 
              pStr = buffer;
              printf("Download...\n"); 
            }else{
              bJpgPos = 0;
            }
      break;
      case 4:                 //found   "\r\n\r\n"
         if(nRecv > 0){
    	        fwrite(pStr, nRecv*sizeof(char), 1, fp);
              m_bytes += nRecv;
         }   
      break;
      default:
      break;
      }
     
       
      if(nRecv==0)
         break;
          
      if(nRecv < 0)
		  printf("ERROR reading from socket");
	   
	}
	 
    printf("Download: %u bytes.\n", m_bytes); 
    fclose(fp);
    closesocket(m_handleSocket);
    WSACleanup();
    
    system("pause");
}
