#include <stdio.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>

using namespace std;
  
/*Initialize the socket using socket().
Set any options such as blocking etc using setsockopt().
Bind to the local address and port using bind(). For a server use INADDR_ANY as the address.
listen() for connections.
Go into a loop and accept connections usin accept(). Spawn threads (pthread_create) to handle these connections, so you can accept more connections. 
Read and write to the socket. within the thread.
close()
When you are done, close the server socket using close()*/

#define READ_BUFFER_SIZE  2048



//void read_to(const int fd, char *buffer);
void* SocketHandler(void*);
std::string filename;
char *write_buffer;

int main(int argv, char** argc)
{
    int host_port= 2222;
     
    struct sockaddr_in my_addr;
 
    int hsock;
    int * p_int ;
    int err;
 
    socklen_t addr_size = 0;
    int* csock;
    sockaddr_in sadr;
    pthread_t thread_id=0;
 
 
    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        printf("Error initializing socket %d\n", errno);
        goto FINISH;
    }
     
    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;
         
    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) )
    {
        printf("Error setting options %d\n", errno);
        free(p_int);
        goto FINISH;
    }
    free(p_int);
 
    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = INADDR_ANY ;
     
    if( ::bind( hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        fprintf(stderr,"Error binding to socket, make sure nothing else is listening on this port %d\n",errno);
        goto FINISH;
    }
    if(listen( hsock, 10) == -1 ){
        fprintf(stderr, "Error listening %d\n",errno);
        goto FINISH;
    }
 
     //Now lets do the server stuff
 
    addr_size = sizeof(sockaddr_in);
     
    while(true)
    {
        printf("waiting for a connection\n");
        csock = (int*)malloc(sizeof(int));
        if((*csock = accept( hsock, (sockaddr*)&sadr, &addr_size))!= -1)
        {
            printf("---------------------\nReceived connection from %s\n",inet_ntoa(sadr.sin_addr));
            pthread_create(&thread_id,0,&SocketHandler, (void*)csock );
            pthread_detach(thread_id);
        }
        else{
            fprintf(stderr, "Error accepting %d\n", errno);
        }
    }
     
    FINISH:
    ;
 }
 
void* SocketHandler(void* lp)
{
    int badass;
    int *csock = (int*)lp;
     
    char buffer[1024];
    int buffer_len = 1024;
    int bytecount;
    memset(buffer, 0, buffer_len);

    if(1)
    {
        
        bool flag=false; //success or not
        if((bytecount = recv(*csock, buffer, buffer_len, 0))== -1)
        {
            fprintf(stderr, "Error receiving data %d\n", errno);
            goto FINISH;
        }
        printf("Received bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
        std::string request=buffer;
     
        //parse initialization
        std::vector<std::string> lines;
        std::string::size_type line_begin =0;
        std::string::size_type check_index=0;
     
        //extract the request line by line
        while(check_index< request.size())
        {
            if(request[check_index]=='\r')
            {
                if((check_index+1)==request.size())
                {
                    std::cout<<"not complete reading request..."<<std::endl;
                    goto FINISH;

                }
                else if(request[check_index+1]=='\n')
                {
                    lines.push_back(std::string(request,line_begin,check_index - line_begin));
                    check_index+=2;
                    line_begin=check_index;
                }
                else
                {
 //--------------------------------------------------------------------------
                    std::cout<<"request error"<<std::endl;
                    std::string bufferr="HTTP/1.1 400 bad request\r\n\0";
                    time_t rawtime;
                    struct tm* timeinfo;
                    char buffer[80];
                    time (&rawtime);
                    timeinfo = localtime(&rawtime);
                    strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
                    string str(buffer);
                    std::string s="Server: ll\r\n";
                    std::string res;
                    int flag;
                    res=bufferr+str+"\r\n"+s;
            
                    if((flag = send(*csock, res.c_str(), res.length(), 0))== -1)
                    {
                        fprintf(stderr, "request error %d\n", errno);
                    }

                    goto FINISH;//400
//------------------------------------------------------------------------------------        
                }
            }
            else
    
                ++check_index;
        }
     
     
            std::string requestline=lines[0];
            auto first_ws = std::find_if(requestline.cbegin(), requestline.cend(),[](char c)->bool { return (c == ' ' || c == '\t'); });
            if(first_ws==requestline.cend())
            {
                std::cout<<"request error..."<<std::endl;
//------------------------------------------------------------------------- 
                std::cout<<"request error"<<std::endl;
                std::string bufferr="HTTP/1.1 400 bad request\r\n\0";
                time_t rawtime;
                struct tm* timeinfo;
                char buffer[80];
                time (&rawtime);
                timeinfo = localtime(&rawtime);
                strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
                string str(buffer);
                std::string s="Server: ll\r\n";
                std::string res;
                int flag;
                res=bufferr+str+"\r\n"+s;
            
                if((flag = send(*csock, res.c_str(), res.length(), 0))== -1)
                {
                    fprintf(stderr, "request error %d\n", errno);
                }

                    goto FINISH;//400
//----------------------------------------------------------------------------
  
            }
       
      
      
    //parse request line
        std::string method=std::string(requestline.cbegin(), first_ws);
        auto reverse_last_ws = std::find_if(requestline.crbegin(),requestline.crend(),[](char c)->bool { return (c == ' ' || c == '\t'); });
        auto last_ws = reverse_last_ws.base();
        std::string version = std::string(last_ws, requestline.cend());
        while((*first_ws == ' ' || *first_ws == '\t') && first_ws !=  requestline.cend())
            ++first_ws;
        --last_ws;
        while((*last_ws == ' ' || *last_ws == '\t') && last_ws != requestline.cbegin())
            --last_ws;
        std::string url = std::string(first_ws, last_ws + 1);
      
    //parse headr
        for(int i=1;i<lines.size();++i)
        {
            if(lines[i].empty())
                break;
            else if(strncasecmp(lines[i].c_str(),"Host:",5)==0) //"host"
            {
                auto iter = lines[i].cbegin() + 5;
                while(*iter == ' ' || *iter == '\t')
                    ++iter;
                std::string host= std::string(iter, lines[i].cend());
            }
            else if(strncasecmp(lines[i].c_str(),"Connection:",11)==0)
            {
                auto iter = lines[i].cbegin() + 11;
                while(*iter == ' ' || *iter == '\t')
                    ++iter;
                std::string connection=std::string(iter, lines[i].cend());
            }
            else
            {
                //do not consider other situation
            }
        }
        //check header, request line to decide what to send
        std::string res="";
        std::string bufferr="HTTP/1.1 200 OK\r\n\0";
        //time ::
        time_t rawtime;
        struct tm* timeinfo;
        char buffer[80];
        time (&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
        string t(buffer);

        //start read html file's content
        filename += url;
        cout<<"------------"<<url<<endl;
        int fd = open(filename.c_str(),O_RDONLY,0);
        //read_to(fd,write_buffer);
        //cout<<write_buffer<<endl;
        
        //filename +=request.url;
        std::string s="Server: ll\r\n";
        std::string content="HI IM INDEX PAGE";
        int a = content.size();
        std::string temp=std::to_string(a);
        std::string type="Content-Type: text/html\r\n";
        //std::string length="Content-Length: 4\r\n";
        std::string length = "Content-Length: "+temp+"\r\n";
        res=bufferr+t+"\r\n"+s+length+type+"\n"+content+"\r\n";
        if((bytecount = send(*csock, res.c_str(), res.length(), 0))== -1)
        {
            fprintf(stderr, "Error sending data %d\n", errno);
            goto FINISH;
        }
        printf("Sent bytes %d\n", bytecount);
    }//end while(1).
    FINISH:
    free(csock);
    return 0;
    
}



/*void read_to(const int fd, char *buffer)
{
    size_t bytes_read = 0;
    size_t bytes_left = READ_BUFFER_SIZE;
    char *ptr = buffer;
    while(bytes_left > 0)
    {
        bytes_read = read(fd, ptr, bytes_left);
        if(bytes_read == -1)
        {
            if(errno == EINTR)
                bytes_read = 0;
            else
                return;
        }
        else if(bytes_read == 0)
            break;
        bytes_left -= bytes_read;
        ptr += bytes_read;
    }
}

*/