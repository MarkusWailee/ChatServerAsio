#include "ChatServer.h"


/*This will be a small rundown on how the server works.
* All Users are stored in a vector of pointers.
* The main thread "acceptor_handle" handles client memory allocation/deallocation or Connection/Disconnection.
* Before accepting new users, the server frees memory from any Disconnected users. Making it thread safe to resize the vector.
* After a client has connected, a new thread is created to handle listening from the user. known as "socket_thread".
* Once a client disconnects from the server, an error is detected and closes the users listening thread("socket_thread") 
and marks the client as Disconnected for memory cleanup later.
* Once a User is connected to the global chat, the user is free to send any message to any other user connected.
* since each thread has access to vector<Client*>, the message the user sends is sent to every user connected in a for loop
*/

/* THREADS
* MAIN THREAD-> Connection/Disconnection
* thread1 -> user1 listening
* thread2 -> user2 listening
* thread3 -> user3 listening
* thread4 -> user4 listening
* ...
*/
int main()
{
	ClientManager cm(8080);

	cm.acceptor_handle();

}
