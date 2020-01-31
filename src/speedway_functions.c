#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "hresult.h"
#include <stdbool.h>

#define DEFAULT_PORT 1234
#define BUFFER_SIZE 256

char recvBuffer[BUFFER_SIZE];

HRESULT initRecvBuffer()
{
    memset(recvBuffer, '\0', BUFFER_SIZE);
    return S_OK;
}

HRESULT openSocketToSpeedway
(
    int speedwayPort,               // [in] - port to use connecting to speedway
    int* sockfd,                    // [out] - socket
    struct sockaddr_in* serv_addr,
    struct sockaddr_in* remote_addr
)
{
    if (!sockfd)
        return E_INVALIDARG;

    if (speedwayPort <= 0)
        speedwayPort = DEFAULT_PORT;

    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error: Could not create socket \n");
        return E_FAIL;
    }

    memset(serv_addr, '\0', sizeof(*serv_addr));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(speedwayPort);
    serv_addr->sin_addr.s_addr = inet_addr("192.168.1.100");
    bind(*sockfd, (struct sockaddr *) serv_addr, sizeof(*serv_addr));

    remote_addr->sin_family = AF_INET;
    remote_addr->sin_port = htons(speedwayPort);
    //fixme find speedway port dynamically
    remote_addr->sin_addr.s_addr = inet_addr("192.168.2.4");
    connect(*sockfd, (struct sockaddr *) remote_addr, sizeof(*remote_addr));

    initRecvBuffer();

    return S_OK;
}

HRESULT closeConnectionToSpeedway(int* sockfd)
{
    if (sockfd)
        close(*sockfd);

    return S_OK;
}

HRESULT recvFromSpeedway
(
    int sockfd,             //[in] - socket descriptor/handle
    int* bytesReceived      //[in/out] - in: bytes received on last call, out: bytes received on this call
)
{
    HRESULT hr = S_OK;
    if (!sockfd || !bytesReceived)
    {
        hr = E_INVALIDARG;
        goto cleanup;
    }

    if (*bytesReceived > 0)
        memset(recvBuffer, '\0', *bytesReceived);

    if ((bytesReceived = recv(sockfd, recvBuffer, BUFFER_SIZE - 1, 0)) == -1)
    {
        perror("recv error");
        hr = E_FAIL;
        goto cleanup;
    }

cleanup:
    if (FAILED(hr))
    {
        memset(recvBuffer, '\0', BUFFER_SIZE);
    }

    return hr;
}

char* getRecvBuffer()
{
    //It would eventually be better (well, more efficient at least)
    //to have the receive "walk" the buffer and get as many tags as possible
    //before returning
    return recvBuffer;
}

int hostname_to_ip(char *hostname, char *ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he = gethostbyname(hostname)) == NULL)
    {
        herror("gethostbyname");
        return 1;
    }

    //finish me

    return 1;
}

void *get_in_addr(struct sockaddr *address)
{
    if (address->sa_family == AF_INET)
    {
        return &((struct sockaddr_in*)address)->sin_addr;
    }

    return &((struct sockaddr_in6*)address)->sin6_addr;
}

