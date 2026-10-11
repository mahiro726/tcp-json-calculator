#include <iostream>
#include "Protocol.hpp"
#include <memory.h>
#include "TcpServer.hpp"
#include "Calculator.hpp"

static void Usage(const std::string &proc)
{
    std::cout << "Usage:\n\t" << proc << " server_ip server_port" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }

    uint16_t server_port = std::stoi(argv[1]);

    // 定义计算器
    std::unique_ptr<Calculator> cal = std::make_unique<Calculator>();


    // 定义协议对象
    std::unique_ptr<Protocol> protocol = std::make_unique<Protocol>(
        [&cal](Request& req)->Response
        {
            return cal->Execute(req);
        });


    // 定义网络对象
    std::unique_ptr<TcpServer> tsvr = std::make_unique<TcpServer>(
        [&protocol](std::string& inbuffer)->std::string
        {
            return protocol->ParseRequest(inbuffer);  
        }, server_port
    );

    // 启动
    tsvr->Loop();

    return 0;
}