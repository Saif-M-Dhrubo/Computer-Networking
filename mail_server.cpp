#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
//#include<date.h>

using namespace std;

//Server side

string recipient,to,subj,body;

string RFC(string s)
{
    stringstream ss(s);
    string str;
    ss>>str;

    if(str=="HELO")
    {
        return "250 "+str+", PLEASED TO MEET YOU";
    }
    else if(str=="MAIL")
    {
        ss>>str>>str;
        return "250 "+str+" REQUESTED MAIL ACTION COMPLETED. SENDER OK.";
    }
    else if(str=="RCPT")
    {
        ss>>str>>str;
        int i=-1;
        recipient="";
        while(str[++i]!='@')
            recipient+=str[i];
        recipient+=".txt";
        return "250 "+str+" REQUESTED MAIL ACTION COMPLETED. RECEPIENT OK.";
    }
    else if(str=="DATA")
        return "354 Start mail input; end with \".\"";
    else return "500/503 ERROR : INVALID COMMANND SYNTAX / BAD SEQUENCE";
}


void Extract(string s)
{
    to="";
    subj="";
    body="";

    int i=-1;
    while(s[++i]!=' ')//extracting the mail of receipient
        to+=s[i];

    ++i;
    while(s[++i]!='"')//extracting subject of the mail
        subj+=s[i];

    ++i;
    while(s[++i]!=0)//extracting mail body
        body+=s[i];
}


int main(int argc, char *argv[])
{
    //Application Interface
    cout << "\t\t=============================================" <<endl;
    cout << "\t\t               SMTP Application" <<endl;
    cout << "\t\t=============================================" <<endl;
    cout << "\t\t||               (MAIL SERVER)             ||" <<endl;
    cout << "\t\t---------------------------------------------" <<endl;
    cout << endl;
    cout << "\nPLEASE, ENTER THE LISTENING PORT NUMBER of SMTP SERVER : " <<endl;

    //for the server, we only need to specify a port number
    if(argc != 1)
    {
        cerr << "Usage: port" << endl;
        exit(0);
    }
    //grab the port number
    int port;// = atoi(argv[1]);
    cin>>port;
    //buffer to send and receive messages with
    char msg[1600];

    //setup a socket and connection tools
    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    //open stream oriented socket with internet address
    //also keep track of the socket descriptor
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSd < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    //bind the socket to its local address
    int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr,
                          sizeof(servAddr));
    if(bindStatus < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }
    cout << "\nWAITING FOR A CLIENT TO CONNECT TO THE SERVER ..." << endl<<endl;
    //listen for up to 5 requests at a time
    listen(serverSd, 5);
    //receive a request from client using accept
    //we need a new address to connect with the client
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    int connection_no=0;


    while(1)//for multiple user,one after another
    {
        connection_no++;
        //accept, create a new socket descriptor to
        //handle the new connection with client
        int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
        if(newSd < 0)
        {
            cerr << "Error accepting request from client number "<<connection_no<<"!" << endl;
            exit(1);
        }
        cout << "CONNECTED WITH CLIENT No.  "<<connection_no<<"!" << endl;
        //also keep track of the amount of data sent as well
        int bytesRead, bytesWritten = 0;
        char hostname[1024],command[1024];
        gethostname(hostname,1024);

        memset(&command, 0, sizeof(command));
        strcpy(command,"220 ");
        strcat(command,hostname);
        bytesWritten += send(newSd, (char*)&command, strlen(command), 0);
        cout<<"Server > "<<command<<endl<<endl;

        //tracking of the session time
        struct timeval start1, end1;
        gettimeofday(&start1, NULL);
        bool flag=false;

        while(1)//for a particular client connection establishment
        {
            //receive a message from the client (listen)
            cout << "\n________________WAITING FOR CLIENT "<<connection_no<<" RESPONSE_____________\n" <<endl;
            memset(&msg, 0, sizeof(msg));//clear the buffer
            bytesRead += recv(newSd, (char*)&msg, sizeof(msg), 0);

            cout << "Client "<<connection_no<<" > " << msg << endl;
            string data=RFC(msg),message=msg;

            memset(&msg, 0, sizeof(msg)); //clear the buffer
            strcpy(msg, data.c_str());

            if(recipient.length()!=0 && access(recipient.c_str(),F_OK)==-1)
            {
                memset(&msg, 0, sizeof(msg)); //clear the buffer
                strcpy(msg,"404 THE RECEPIENT IS NOT PERMITTED");
                send(newSd, (char*)&msg, strlen(msg), 0);
                recipient="";
                break;
            }


            if(flag)
            {
                ofstream output;

                Extract(message);

                time_t rawtime;
                struct tm * timeinfo;

                time ( &rawtime );
                timeinfo = localtime ( &rawtime );

                output.open(recipient.c_str(), ofstream::in | ofstream::out | ofstream::app);

                output<<"To: "<<to<<endl;
                cout<<"To: "<<to<<endl;

                output<<"Subject: "<<subj<<endl;
                cout<<"Subject: "<<subj<<endl;

                output<<"Body: "<<body<<endl<<endl;
                cout<<"Body: "<<body<<endl;


                output<<"RECEIVING TIME: "<<asctime (timeinfo)<<endl;
                cout<<"RECEIVING TIME: "<<asctime (timeinfo)<<endl;
                output.close();

                memset(&msg, 0, sizeof(msg)); //clear the buffer
                strcpy(msg,"250 Message accepted for delivery");
                flag=false;
            }


            if(message == "QUIT")
            {
                cout << "Client "<<connection_no<<" has quit the session" << endl;

                memset(&msg, 0, sizeof(msg)); //clear the buffer
                strcpy(msg,"221 ");
                strcat(msg,hostname);
                strcasecmp(msg," SERVICE CLOSING TRANSMISSION CHANNEL");

                cout<<"Server > "<<msg<<endl<<endl;
                bytesWritten += send(newSd, (char*)&msg, strlen(msg), 0);
                break;
            }
            else if(message =="DATA")
                flag=true;

            cout<<"Server > "<<msg<<endl<<endl;
            if(msg[0]=='5')
            break;
            //send the message to client
            bytesWritten += send(newSd, (char*)&msg, strlen(msg), 0);

        }
        //we need to close the socket descriptors after we're all done
        gettimeofday(&end1, NULL);
        close(newSd);

        cout << "----------------CONNECTION No. "<<connection_no<<" IS CLOSED--------------" << endl<<endl<<endl;
    }
    close(serverSd);
    return 0;
}
