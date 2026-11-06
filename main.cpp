#include "arinc429_decoder.hpp"
#include "arinc429_bnr.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>

#if defined(_WIN32)
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
#endif

#define PORT 5005

int main() {
#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    int serverFd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(serverFd, (struct sockaddr*)&address, sizeof(address));

    std::cout << "[C++ Engine] Receiver & KPI Profiler Online (Port " << PORT << ")...\n";

    uint32_t rawWord = 0;
    uint64_t totalFrames = 0;
    uint64_t validFrames = 0;
    uint64_t corruptedFrames = 0;
    double totalLatencyNs = 0.0;

    auto startTime = std::chrono::high_resolution_clock::now();

    while (true) {
        ssize_t bytesRead = recv(serverFd, reinterpret_cast<char*>(&rawWord), sizeof(rawWord), 0);
        if (bytesRead > 0) {
            // Measure parsing latency
            auto t1 = std::chrono::high_resolution_clock::now();
            
            Arinc429Word word = ArincDecoder::decodeRawWord(rawWord);
            
            // Decode payload if frame is valid
            if (word.isValid) {
                if (word.label == 0204) {
                    volatile double alt = BnrConverter::toPhysicalValue(word.rawData, 17, 131072.0, true);
                    (void)alt;
                } else if (word.label == 0206) {
                    volatile double spd = BnrConverter::toPhysicalValue(word.rawData, 14, 1024.0, false);
                    (void)spd;
                }
                validFrames++;
            } else {
                corruptedFrames++;
            }

            auto t2 = std::chrono::high_resolution_clock::now();
            totalLatencyNs += std::chrono::duration<double, std::nano>(t2 - t1).count();
            totalFrames++;

            // Output KPIs every 10,000 processed frames
            if (totalFrames % 10000 == 0) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsedSec = std::chrono::duration<double>(now - startTime).count();
                double throughput = totalFrames / elapsedSec;
                double avgLatencyUs = (totalLatencyNs / totalFrames) / 1000.0;

                std::cout << "\n================ [PERFORMANCE METRICS] ================\n";
                std::cout << "Total Processed : " << totalFrames << " frames\n";
                std::cout << "Throughput      : " << std::fixed << std::setprecision(0) << throughput << " words/sec\n";
                std::cout << "Avg Latency     : " << std::setprecision(3) << avgLatencyUs << " microseconds/word\n";
                std::cout << "Valid / Faults  : " << validFrames << " Valid | " << corruptedFrames << " Corrupted\n";
                std::cout << "========================================================\n";
            }
        }
    }

#if defined(_WIN32)
    closesocket(serverFd);
    WSACleanup();
#else
    close(serverFd);
#endif
    return 0;
}