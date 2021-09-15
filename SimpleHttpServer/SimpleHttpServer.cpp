// SimpleHttpServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <ctime>

// link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define BUFFERSIZE (4 * 1024)

int main()
{
    char ac[80];
    WSADATA wsaData;
    int iResult;
    INT iRetval;

    struct addrinfo* ptr = NULL;
   // struct addrinfo hints ;

    struct sockaddr_in* sockaddr_ipv4;
    //    struct sockaddr_in6 *sockaddr_ipv6;
    LPSOCKADDR sockaddr_ip;

    wchar_t ipstringbuffer[INET6_ADDRSTRLEN];
    DWORD ipbufferlength = INET6_ADDRSTRLEN;

   // memset(&hints, 0, sizeof(addrinfo));

    int i = 1;

    //alize Winsock
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR)
    {
        std::cerr << "Error" << WSAGetLastError() << "when accessing local host name." << std::endl;
        return -1;
    }
    
    std::cout << "host name is: " << ac << " ." << std::endl;

    //%WINDIR%\system32\drivers\etc\services
    std::string server_namestr = "nameserver";

    struct addrinfo hint;
    memset(&hint, 0, sizeof(addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    struct addrinfo *info = nullptr;

    if (getaddrinfo(ac, NULL, &hint, &info) != 0) //server_namestr.c_str()
    {
        std::cerr << "Error retrieving local node address info: " << gai_strerror(WSAGetLastError()) << std::endl;
        return -1;
    }


    // Retrieve each address and print out the hex bytes
    for (ptr = info; ptr != NULL; ptr = ptr->ai_next) {

        printf("getaddrinfo response %d\n", i++);
        printf("\tFlags: 0x%x\n", ptr->ai_flags);
        printf("\tFamily: ");
        switch (ptr->ai_family) {
        case AF_UNSPEC:
            printf("Unspecified\n");
            break;
        case AF_INET:
            printf("AF_INET (IPv4)\n");
            sockaddr_ipv4 = (struct sockaddr_in*)ptr->ai_addr;
            _tprintf(TEXT("\tIPv4 address %s\n"),
                InetNtop(AF_INET, &sockaddr_ipv4->sin_addr, ipstringbuffer, ipbufferlength));
            break;
        case AF_INET6:
            printf("AF_INET6 (IPv6)\n");
            // the InetNtop function is available on Windows Vista and later
            // sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
            // printf("\tIPv6 address %s\n",
            //    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

            // We use WSAAddressToString since it is supported on Windows XP and later
            sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
            // The buffer length is changed by each call to WSAAddresstoString
            // So we need to set it for each iteration through the loop for safety
            ipbufferlength = 46;
            iRetval = WSAAddressToString(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
                ipstringbuffer, &ipbufferlength);
            if (iRetval)
                printf("WSAAddressToString failed with %u\n", WSAGetLastError());
            else
                _tprintf(L"\tIPv6 address %s\n",ipstringbuffer);
            break;
        case AF_NETBIOS:
            printf("AF_NETBIOS (NetBIOS)\n");
            break;
        default:
            printf("Other %ld\n", ptr->ai_family);
            break;
        }
        printf("\tSocket type: ");
        switch (ptr->ai_socktype) {
        case 0:
            printf("Unspecified\n");
            break;
        case SOCK_STREAM:
            printf("SOCK_STREAM (stream)\n");
            break;
        case SOCK_DGRAM:
            printf("SOCK_DGRAM (datagram) \n");
            break;
        case SOCK_RAW:
            printf("SOCK_RAW (raw) \n");
            break;
        case SOCK_RDM:
            printf("SOCK_RDM (reliable message datagram)\n");
            break;
        case SOCK_SEQPACKET:
            printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
            break;
        default:
            printf("Other %ld\n", ptr->ai_socktype);
            break;
        }
        printf("\tProtocol: ");
        switch (ptr->ai_protocol) {
        case 0:
            printf("Unspecified\n");
            break;
        case IPPROTO_TCP:
            printf("IPPROTO_TCP (TCP)\n");
            break;
        case IPPROTO_UDP:
            printf("IPPROTO_UDP (UDP) \n");
            break;
        default:
            printf("Other %ld\n", ptr->ai_protocol);
            break;
        }
        printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
        printf("\tCanonical name: %s\n", ptr->ai_canonname);
    }

    freeaddrinfo(info);

    _tprintf(L"Binding and listenning ....");

    int sd = -1;

    if ((sd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "socket() failed";
        return -1;
    }

    SOCKADDR_IN6 serveraddr;
    u_short SERVER_PORT = 8081;

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin6_family = AF_INET6;
    serveraddr.sin6_port = htons(SERVER_PORT);
    serveraddr.sin6_addr = in6addr_any;

    if (bind(sd,
        (struct sockaddr*)&serveraddr,
        sizeof(serveraddr)) < 0)
    {
        std::cerr << "bind() failed";
        return -1;
    }

    if (listen(sd, 10) < 0)
    {
        std::cerr << "listen() failed";
        return -1;
    }
    _tprintf(L"Ready for client connect().\n");
    /********************************************************************/
    /* The server uses the accept() function to accept an incoming */
    /* connection request. The accept() call will block indefinitely */
    /* waiting for the incoming connection to arrive from an IPv4 or */
    /* IPv6 client. */
    /********************************************************************/

    SOCKET sdconn;
    SOCKADDR_IN6 clientaddr;
    memset(&clientaddr, 0, sizeof(SOCKADDR_IN6));
    int addrlen = sizeof(SOCKADDR_IN6);
    char str[INET6_ADDRSTRLEN] = {0};

    if ((sdconn = accept(sd, NULL, NULL)) < 0)
    {
        std::cerr << "accept() failed";
        return -1;
    }
    else
    {
        /*****************************************************************/
        /* Display the client address. Note that if the client is */
        /* an IPv4 client, the address will be shown as an IPv4 Mapped */
        /* IPv6 address. */
        /*****************************************************************/
        getpeername(sdconn, (struct sockaddr*)&clientaddr, &addrlen);
        if (inet_ntop(AF_INET6, &clientaddr.sin6_addr, str, sizeof(str))) {
            printf("Client address is %s\n", str);
            printf("Client port is %d\n", ntohs(clientaddr.sin6_port));
        }
    }

    size_t bytes_received = -1;
    char buffer[BUFFERSIZE] = { 0 };

    while (bytes_received = recv(sdconn, buffer, BUFFERSIZE, 0) > 0)
    {
        printf("\nbytes readL %d\ndata: ", bytes_received);

        for (size_t i = 0; i < strlen(buffer); i++)
            std::cout << std::hex << (0xff & (u_short)buffer[i]);
        printf("  => %s", buffer);

        if (bytes_received < BUFFERSIZE) break;

    }

    std::string format = "HTTP/1.1 200 OK\r\n"
        "Location: localhost:8081\r\n"
        "Server : socket\r\n"
        "X-Powered-By: native c++\r\n"
        "Date: %a, %e %b %G %T GMT\r\n"
        "Content-Length: 0\r\n\r\n";

    time_t rawtime = (0);
    struct tm gmt_time = {0};
    time(&rawtime);
    gmtime_s(&gmt_time, &rawtime);
    memset(buffer, 0, BUFFERSIZE);
    strftime(buffer, BUFFERSIZE, format.data(), &gmt_time);
    std::string response(buffer);

    bytes_received = send(sdconn, response.c_str(), response.size(), 0);
    if (bytes_received < 0)
    {
        std::cout << ("send() failed");
    }

    if (sd != -1)
        //close(sd);
        closesocket(sd);
    if (sdconn != -1)
        //close(sdconn);
        closesocket(sdconn);

    WSACleanup();

    _tprintf(L"\n\nPress any key to exit;");
    std::cin.get();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
