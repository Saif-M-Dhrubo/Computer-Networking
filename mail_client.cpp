#include <bits/stdc++.h>
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

using namespace std;

//SMTP Client side

bool checker(string input, string correct){
    if(input != "HELO" && input != "MAIL FROM" && input != "RCPT" && input != "DATA" && input != "QUIT"){
        cout << "500 ERROR : " << "SYNTAX ERROR, COMMAND UNRECOGNISED" << endl;
        return false;
    }

    if(input != correct){
        cout << "503 ERROR : BAD SEQUENCE OF COMMANDS" << endl;
        return false;
    }

    if(input == correct)
        return true;
}

int main()
{
    //Application Interface
    cout << "\t\t=============================================" <<endl;
    cout << "\t\t               SMTP Application" <<endl;
    cout << "\t\t=============================================" <<endl;
    cout << "\t\t||               (MAIL CLIENT)             ||" <<endl;
    cout << "\t\t---------------------------------------------" <<endl;
    cout << endl;
    cout << "\nPLEASE, INSERT COMMAND IN THIS FORMAT : " <<endl;
    cout << "user@host:port <space> SUBJECT(enclosed in \"\") <space> FILE CONTAINING MAIL BODY\n" <<endl;


	string command;
    getline(cin, command);

    //string parsing
    istringstream buffer_command(command);
    istream_iterator<string> beg_command(buffer_command), end_command;

    vector<string> command_tokens(beg_command, end_command);
    string mail_recepient = command_tokens[0];

	string delimiter_1 = "@";

	string user = command_tokens[0].substr(0, command_tokens[0].find(delimiter_1));
	command_tokens[0].erase(0, command_tokens[0].find(delimiter_1) + delimiter_1.length());

	string delimiter_2 = ":";

	string server_IP = command_tokens[0].substr(0, command_tokens[0].find(delimiter_2));
	cout << "SERVER IP : " << server_IP << endl;

	command_tokens[0].erase(0, command_tokens[0].find(delimiter_2) + delimiter_2.length());
	string port_str = command_tokens[0];
	cout << "PORT : " << port_str << endl;

	//grab the IP address and port number
    const char *tmp = server_IP.c_str();
    char *serverIp = new char[server_IP.length() + 1];
    strcpy(serverIp, tmp);
    int port = atoi(port_str.c_str());

    //create a message buffer
    char msg[1500];

    //setup a socket and connection tools
    struct hostent* host = gethostbyname(serverIp);
    sockaddr_in sendSockAddr;
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(port);
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);

    //trying to connect...
    int status = connect(clientSd, (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));

    if(status < 0)
    {
        cout << "451 ERROR : Requested action aborted, local error in processing" << endl;
        exit(0);
    }
    cout << "\n\nConnected to the server!" << endl;

 	//Execution Loop
    int flag_250 = 0;
    while(1)
    {
        cout << "\n\n________________WAITING FOR SERVER RESPONSE_____________\n" << endl;

        //clear the buffer
        memset(&msg, 0, sizeof(msg));
        recv(clientSd, (char*)&msg, sizeof(msg), 0);

        cout << "Server > " << msg << endl;

        //Exit Server
        if(!strcmp(msg, "Quit"))
        {
            cout << "\n\nServer has quitted the session" << endl;
            break;
        }

        //string parsing for RFC Code
        istringstream recvd_Server(msg);
        istream_iterator<string> rfc_beg(recvd_Server), rfc_end;

        vector<string> RFC_code(rfc_beg, rfc_end);


        //RFC Comparisons
        if(RFC_code[0] == "220")
        {
            char hostname[1024];
            gethostname(hostname, 1024);

            string input;
            inp_1 : getline(cin, input);

            bool flag_sequence = checker(input, "HELO");
            if(flag_sequence){
                string welcome = "HELO " + string(hostname);

                cout << "Client > ";
                cout << welcome <<endl;

                memset(&msg, 0, sizeof(msg));

                strcpy(msg, welcome.c_str());

                send(clientSd, (char*)&msg, strlen(msg), 0);
                continue;
            }
            else{
                break;
            }
        }

        else if(RFC_code[0] == "250" && flag_250 == 0)
        {
            char hostname[1024];

            gethostname(hostname, 1024);

            string input_2;
            inp_2 : getline(cin, input_2);

            bool flag_seq2 = checker(input_2, "MAIL FROM");
            if(flag_seq2){
                string mail_to = "MAIL FROM: " + string(getenv("USER")) + "@" + string(hostname);

                cout << "Client > ";
                cout << mail_to <<endl;

                memset(&msg, 0, sizeof(msg));

                strcpy(msg, mail_to.c_str());

                send(clientSd, (char*)&msg, strlen(msg), 0);
                flag_250 = 1;

                continue;
            }
            else{
                break;
            }
        }

        else if(RFC_code[0] == "250" && flag_250 == 1)
        {
            string rcpt;
            inp_3 : getline(cin, rcpt);

            bool flag_seq3 = checker(rcpt, "RCPT");

            if(flag_seq3){
                string rcpt = "RCPT TO: " + mail_recepient;

                cout << "Client > ";
                cout << rcpt <<endl;

                memset(&msg, 0, sizeof(msg));

                strcpy(msg, rcpt.c_str());

                send(clientSd, (char*)&msg, strlen(msg), 0);
                flag_250 = 2;

                continue;
            }
            else{
                break;
            }
        }

        else if(RFC_code[0] == "250" && flag_250 == 2)
        {
            string mail;
            inp_4 : getline(cin, mail);

            bool flag_seq4 = checker(mail, "DATA");

            if(flag_seq4){
                cout << "Client > ";
                cout << mail <<endl;

                memset(&msg, 0, sizeof(msg));

                strcpy(msg, mail.c_str());

                send(clientSd, (char*)&msg, strlen(msg), 0);
                flag_250 = 3;
                continue;
            }
            else{
                break;
            }
        }

        else if(RFC_code[0] == "250" && flag_250 == 3)
        {
            string quit;
            inp_5 : getline(cin, quit);

            bool flag_seq5 = checker(quit, "QUIT");

            if(flag_seq5){
                cout << "Client > ";
                cout << quit <<endl;

                memset(&msg, 0, sizeof(msg));

                strcpy(msg, quit.c_str());

                send(clientSd, (char*)&msg, strlen(msg), 0);
                flag_250 = 0;

                continue;
            }
            else{
                break;
            }
        }

        else if(RFC_code[0] == "354"){
            cout << "Client > MAIL DELIVERED TO SMTP SERVER" << endl;
            string data;
            data = command;

            //string parsing
            istringstream buffer(data);
            istream_iterator<string> beg(buffer), end;

            vector<string> tokens(beg, end);


            //input description from file
            string fileName = tokens[tokens.size() - 1];

            ifstream inpFile(fileName.c_str());
            string content( (istreambuf_iterator<char>(inpFile) ), (istreambuf_iterator<char>()) );

            if(!inpFile.good()){
                cout << "The Specified Mail Description File Doesn't Exist" << endl;
                continue;
            }

            //final SMTP mail
            string toBeSent;

            for(int i = 0; i < tokens.size() - 1; i++)
                toBeSent += (tokens[i] + " ");

            toBeSent += content;

            //clear the buffer
            memset(&msg, 0, sizeof(msg));

            strcpy(msg, toBeSent.c_str());

            if(data == "Quit")
            {
                send(clientSd, (char*)&msg, strlen(msg), 0);
                break;
            }

            send(clientSd, (char*)&msg, strlen(msg), 0);
        }

        else if(RFC_code[0] == "221"){
            cout << "MAIL SENT. SERVER SESSION CLOSED" << endl;
            break;
        }

        else if(RFC_code[0] == "404"){
            cout << "AN ERROR OCCURED WHILE SENDING E-MAIL TO SERVER" << endl;
            break;
        }
    }

    cout << "\n\n-----------------------Connection Closed-----------------------" << endl;
    return 0;
}

