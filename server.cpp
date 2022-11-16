#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
        
//sockaddr_in hint; //declaramos una estructura de tipo sockadd_in que contiene estos miembros 
//short            sin_family;   // e.g. AF_INET
//unsigned short   sin_port;     // e.g. htons(3490)
//struct in_addr   sin_addr;     // see struct in_addr, below
//char             sin_zero[8]
//hint.ai_family = AF_UNSPEC; //asignamos el tipo de direccion con la que el socket va a comunicarse (e. en este caso INET hace referencia a direcciones IPV4)
// El kernel de linux soporta 29 familias de direcciones. UNIX -> AF_UNIX, IPX socket -> AF_IPX, Bluetooth -> AF_BLUETOOTH, IPV6 -> AF_INET6, etc
//hint.sin_port = htons(6660); // htons es host-to-network short y le pasamos el puerto por el que se van a comunicar, esta funcion convierte el orden de bytes del host al orden de bytes de la red - info https://www.ibm.com/docs/ja/zvm/7.2?topic=domains-network-byte-order-host-byte-order
//este enlace contiene varias estructuras y sus miembros de la libreria socket http://web.mit.edu/macdev/Development/MITSupportLib/SocketsLib/Documentation/structures.html
//inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
//hint.sin_addr.s_addr = inet_addr("0.0.0.0"); // la funcion inet_addr recibe una direccion de internet en formato numero-punto (n.n.n.n) y lo convierte a binario del orden de bytes de la red     

class server{
    public:
        int socketfd;
        struct addrinfo hint;
        struct addrinfo *serv;
        int errorinfo;
        server(){
            memset(&hint, 0 , sizeof(hint));
            hint.ai_family = AF_UNSPEC;
            hint.ai_socktype = SOCK_STREAM;
            errorinfo = getaddrinfo(NULL,"6660",&hint,&serv); //https://www.ibm.com/docs/es/aix/7.3?topic=g-getaddrinfo-subroutine
            socketfd = socket(serv->ai_family, serv->ai_socktype, 0);
            // Unir el socket a una ip/puerto
            
            bind(socketfd, (sockaddr*)&serv, sizeof(serv));// esta funcion enlaza al filedescriptor la estructura. ahora socketfd apunta a hint.
            // poner a la escucha el filedescriptor
            listen(socketfd, SOMAXCONN);
        }
    
};

class client{
    public:
        sockaddr_in cliente;
        socklen_t clienteSize;
        int clientSocket;
        char host[NI_MAXHOST];      // Client's remote name
        char service[NI_MAXSERV];   // Service (i.e. port) the client is connect on

        client(int socketfd){
            clienteSize = sizeof(cliente);
            clientSocket = accept(socketfd, (sockaddr*)&cliente, &clienteSize);
            memset(host, 0, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
            memset(service, 0, NI_MAXSERV);
        }
 };
 
int main()
{
    server server1;
    if (server1.socketfd == -1)
    {
        std::cerr << "Error al abrir el Socket" << std::endl;
        return -1;
    }
    
    //poll(struct pollfd * fds , nfds_t nfds , int timeout )

    // esperando una conexion
    client cliente1(server1.socketfd);
 
    if (getnameinfo((sockaddr*)&cliente1, sizeof(client), cliente1.host, NI_MAXHOST, cliente1.service, NI_MAXSERV, 0) == 0)
    {
        std::cout << cliente1.host << " connected on port " << cliente1.service << std::endl;
    }
    else
    {
        inet_ntop(AF_INET, &cliente1.cliente.sin_family, cliente1.host, NI_MAXHOST);
        std::cout << cliente1.host << " connected on port " << ntohs(cliente1.cliente.sin_port) << std::endl;
    }
 
    // Close listening socket
    close(server1.socketfd);
 
    // While loop: accept and echo message back to client
    char buf[4096];
 
    while (true)
    {
        memset(buf, 0, 4096);
 
        // Wait for client to send data
        int bytesReceived = recv(cliente1.clientSocket, buf, 4096, 0);
        if (bytesReceived == -1)
        {
            std::cerr << "Error in recv(). Quitting" << std::endl;
            break;
        }
 
        if (bytesReceived == 0)
        {
            std::cout << "Client disconnected " << std::endl;
            break;
        }
 
        std::cout << std::string(buf, 0, bytesReceived) << std::endl;
 
        // Echo message back to client
        send(cliente1.clientSocket, buf, bytesReceived + 1, 0);
       

    }
 
    // Close the socket
    close(cliente1.clientSocket);
 
    return 0;
}