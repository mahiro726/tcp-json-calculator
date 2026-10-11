#include <iostream>
#include <memory>
#include <string>
#include "Protocol.hpp"
#include "Socket.hpp"
#include "InetAddr.hpp"

using std::cin;
using std::cout;
using std::endl;

static void Usage(const std::string &proc)
{
    std::cout << "Usage:\n\t" << proc << " server_ip server_port" << std::endl;
}

static void HandlerResponse(Response &resp)
{
    cout << "result: " << resp._result << "[" << resp._code << "]" << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        Usage(argv[0]);
        exit(1);
    }

    std::string server_ip = argv[1];
    uint16_t server_port = std::stoi(argv[2]);

    std::unique_ptr<Socket> socket = std::make_unique<TcpSocket>();
    socket->BuildTcpClientSockMethod();

    InetAddr serveraddress(server_port, server_ip);
    bool n = socket->Connect(serveraddress);
    if (!n)
    {
        std::cerr << "connect error" << std::endl;
        exit(2);
    }

    Protocol protocol(HandlerResponse);
    std::string inbuffer;

    // 循环向服务器发起请求
    // 双方需要遵守相同的协议
    while (true)
    {
        std::string outbuffer;

        int cnt = 3;
        while (cnt--)
        {
            // 获取用户数据
            int x, y;
            char oper;
            cout << "Please Enter Your x: ";
            cin >> x;
            cout << "Please Enter Your y: ";
            cin >> y;
            cout << "Please Enter Your oper: ";
            cin >> oper;

            // 定义请求对象
            Request req(x, y, oper);

            // 序列化
            std::string json_string;
            req.Serialize(&json_string);

            // 添加报头
            std::string send_string = protocol.Packet(json_string);
            outbuffer += send_string;
        }

        cout << "\n" << outbuffer << endl;

        // 发送
        socket->Send(outbuffer);

        // 接收
        socket->Recv(&inbuffer);

        // 解析
        protocol.ParseResponse(inbuffer);
    }

    // 使用完后关闭文件描述符 sockfd
    socket->Close();

    return 0;
}