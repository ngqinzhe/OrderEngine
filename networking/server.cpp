#include <iostream>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "orderEngine.h"
#include <time.h>

using namespace boost::asio;
using ip::tcp;

class ConnectionHandler : public boost::enable_shared_from_this<ConnectionHandler>
{
private:
    tcp::socket sock;
    std::string message = "FROM SERVER: Server received your order.\n";
    enum { max_length = 1024};
    //char data[max_length];
    std::vector<std::string> orderbuf = std::vector<std::string>(7);
    Orderbook _orderbook;

public:
    typedef boost::shared_ptr<ConnectionHandler> pointer;
    ConnectionHandler(boost::asio::io_context& io_context, Orderbook& orderbook) : sock(io_context) 
    {
        _orderbook = orderbook;
    }
    
    // pointer creation
    static pointer create(boost::asio::io_context& io_context, Orderbook& orderbook)
    {
        return pointer(new ConnectionHandler(io_context, orderbook));
    }

    //socket creation
    tcp::socket& socket()
    {
        return sock;
    }

    void start()
    {
        sock.async_read_some(
            boost::asio::buffer(orderbuf),
            boost::bind(&ConnectionHandler::handle_read,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)
        );

        sock.async_write_some(
            boost::asio::buffer(message, max_length),
            boost::bind(&ConnectionHandler::handle_write,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred)
        );
    }

    void handle_read(const boost::system::error_code& err, size_t bytes_transferred)
    {
        if (!err) 
        {
            std::cout << "FROM CLIENT: ";
            for (const std::string& x : orderbuf)
                std::cout << x << " ";
            std::cout << std::endl;

            parseInput(_orderbook, orderbuf);
            std::cout << _orderbook << std::endl;
            time_t myTime = time(NULL);
            std::cout << "orderbook updated at " << ctime(&myTime) << std::endl;
        }
        else 
        {
            std::cerr << "Error: " << err.message() << std::endl;
            sock.close();
        }
    }

    void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
    {
        if (!err) 
        {
            
        }
        else 
        {
            std::cerr << "Error: " << err.message() << std::endl;
            sock.close();
        }
    }
};

class Server 
{
private:
    tcp::acceptor acceptor_;
    Orderbook ob;

private:    
    void start_accept()
    {
        //socket
        ConnectionHandler::pointer connection = ConnectionHandler::create((boost::asio::io_context&) acceptor_.get_executor().context(), ob);

        acceptor_.async_accept(connection->socket(),
        boost::bind(&Server::handle_accept, this, connection, 
        boost::asio::placeholders::error));
    }

public:
    Server(boost::asio::io_context& io_context) : acceptor_(io_context, tcp::endpoint(tcp::v4(), 1234))
    {
        ob = Orderbook();
        start_accept();
    }

    void handle_accept(ConnectionHandler::pointer connection, const boost::system::error_code& err)
    {
        if (!err)
            connection->start();
        start_accept();
    }
};


int main()
{
    try
    {
        boost::asio::io_context io_context;
        Server server(io_context);
        io_context.run();   
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}