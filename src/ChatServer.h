#pragma once
#include <iostream>
#include <vector>
#include <asio.hpp>
#include <thread>
#include <string>
struct Client
{
	static int Client_population;
	std::string username;
	asio::ip::tcp::socket socket;
	bool is_connected = true;

	Client(asio::io_service& io) : socket(io) { Client_population++; }
	~Client() { Client_population--; }
	void send_string(std::string msg, asio::error_code& ec);
	std::string receive_string(asio::error_code& ec);
};
int Client::Client_population = 0;

inline std::string Client::receive_string(asio::error_code& ec)
{
	std::array<char, 128> buf;
	size_t bytes = socket.read_some(asio::buffer(buf), ec);
	if (ec)return "";
	return std::string(buf.data(), bytes);
}
inline void Client::send_string(std::string msg, asio::error_code& ec)
{
	asio::write(socket, asio::buffer(msg), ec);
}







class ClientManager
{
	std::vector<Client*> clients;
	asio::io_service io_service;
	asio::ip::tcp::acceptor acceptor;

public:
	~ClientManager()
	{
		for (int i = 0; i < clients.size(); i++)
			delete clients[i];
	}
	ClientManager(int PortNumber) : acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), PortNumber))
	{
		std::cout << "----ASIO SERVER----\n";
	}
	void acceptor_handle();
private:
	void socket_thread(Client* u);
	bool is_clients_inactive();
};

inline void ClientManager::acceptor_handle()
{
	while (true)
	{
		while (is_clients_inactive())
		{
			int index = -1;
			for (int i = 0; i < clients.size(); i++)
			{
				if (!clients[i]->is_connected)
				{
					index = i;
					break;
				}
			}
			if (index != -1)
			{
				delete clients[index];
				clients.erase(clients.begin() + index);
			}
		}
		std::cout << clients.size() << " Users Connected.\n";
		asio::error_code ec;
		Client* u = new Client(io_service);
		acceptor.accept(u->socket, ec);
		if (ec)
		{
			std::cout << "Connection Error: " << ec.message() << '\n';
			delete u;
			continue;
		}
		clients.push_back(u);
		std::cout << u->socket.remote_endpoint() << " Connected!\n";

		std::thread([this, u]
			{
				socket_thread(u);
				std::cout << u->socket.remote_endpoint()<< ' '<<u->username << " Disconnected!\n";
				asio::error_code temp_error;
				if(u->username.size())//if user is named
				for (int i = 0; i < clients.size(); i++) clients[i]->send_string(u->username + " Disconnected!\n", temp_error);
				u->is_connected = false;
			}).detach();
	}
}

inline void ClientManager::socket_thread(Client* u)
{
	asio::error_code ec;
	u->send_string("Connected to Server!\n", ec);
	if (ec) return;
	while (true)
	{
		u->send_string("Enter username: ", ec);
		u->username = u->receive_string(ec);

		if (ec)return;
		u->send_string("Connected to chat!\n", ec);
		asio::error_code temp_error;
		for (int i = 0; i < clients.size(); i++) clients[i]->send_string(u->username + " Connected!\n", temp_error);
		while (true)
		{
			std::string client_message = u->receive_string(ec);
			if (ec)return;
			std::cout << u->username << ": " << client_message << '\n';
			for (int i = 0; i < clients.size(); i++)
			{
				//if (clients[i]->username == u->username)continue;
				asio::error_code send_error;
				std::string msg = u->username + ": " + client_message + '\n';
				clients[i]->send_string(msg, send_error);
				if (send_error)std::cout << send_error.message() << '\n';
			}
		}
	}
}

inline bool ClientManager::is_clients_inactive()
{
	for (int i = 0; i < clients.size(); i++)
	{
		if (!clients[i]->is_connected) return 1;
	}
	return 0;
}