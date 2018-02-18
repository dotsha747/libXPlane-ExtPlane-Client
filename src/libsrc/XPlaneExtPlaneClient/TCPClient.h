/*
 This file is part of libXPlane-ExtPlane-Client
 A C++ Library to access X-Plane via the ExtPlane Plugin

 Copyright (C) 2018 shahada abubakar

 <shahada@abubakar.net>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.

 */

#ifndef LIBXPLANEEXTPLANECLIENT_SRC_TCPCLIENT_H_
#define LIBXPLANEEXTPLANECLIENT_SRC_TCPCLIENT_H_

#include <vector>
#include <string>
#include <time.h>

/** @brief Communicate with a TCP Server.
 *
 * This provides everything needed for a functioning TCP client. It's designed
 * to be inherited to a class that implements an actual protocol.
 *
 * Its main body is in the mainloop() method, so this should either be called
 * from your main() or run in it's own thread. This loop will attempt to
 * connect to the server repeatedly.
 *
 * Once connected, initConnection() is called. You can extend this to implement
 * any initial communications as required (be sure to call the base class method).
 *
 * To send something, simply call sendData() with the string to be sent.
 *
 * Received data is buffered until "lines" are identified, at which point
 * processLine() is called. You can override this to parse data received from the
 * server.
 *
 * You can extend dropConnection() to close the connection to the server. It is
 * also called if the server closes the connection first. Once mainLoop() notices
 * the connection is closed, it will reattempt to connect to the server.
 *
 * eventTick is called every 10 m/s (TODO: we need some way to tweak this, and also a
 * better resolution for the time passed around). You may extend this to implement
 * any logic that occurs outside of a response to a command.
 *
 */

namespace XPlaneExtPlaneClient {

	class TCPClient {
	protected:

		// holds a list of strings in teh form "host:port"
		std::vector<std::string> hostList;

		// the current host we are trying to connect to
		unsigned int hostNow = 0;

		// current server (for logging)
		std::string host;

		// current port (for logging)
		int port;

		/// the different states the client->server connection can be in.
		enum {
			CONN_DISCONNECTED, CONN_CONNECTING, CONN_CONNECTED
		} connectState;

		/// handle to the network socket
		int clientSock;

		// select flags
		/// this is set to true whenever there is data in the outputBuffer to be sent, and triggers the mainLoop() to transmit and trim the buffer.
		bool wantWrite;

		/// this is set to true whenever there is a connection to a server, and triggers the mainLoop() to listen for incoming data.
		bool wantRead;

		// i/o buffers
		/// buffer of data received from server. mainLoop() calls processInput() regularly to scan for "lines", which are passed to processLine() when found.
		std::string inputBuffer;
		/// buffer of data to be sent to server. mainLoop() will transmit this out and trim the buffer.
		std::string outputBuffer;

		// timeouts
		time_t lastDataReceivedTime;
		int connectTimeout;

		// delims
		std::string EOLString;

		/// internal method called when there is data on the socket to be read. Reads the data and calls processInput().
		virtual void eventRead(time_t time);

		/// internal method called when the socket is available for writing.
		virtual void eventWrite(time_t time);

		/// flag to indicate thread is running
		bool isRunning;

		/// flag to indicate thread stop is requested
		bool stopRequested;

		/// debug level
		int debug;

	public:

		/// constructor
		TCPClient();

		/// destructor
		virtual ~TCPClient();

		/// sets the server hostname or IP.
		void addHost(std::string _host);
		unsigned int getHostCount();

		/** @brief mainLoop for the TCPClient.
		 *
		 * Call this from your main() or launch a thread. It will run continuously on its own,
		 * connecting to the client and staying connected as much as it can.
		 *
		 * exitFlag should be set to false when entering. You can set it to true (from another
		 * thread or interrupt) to cause the mainLoop() to exit, it should respond within 10 m/s.
		 */

		virtual void mainLoop();

		/** @brief regular "timer tick" event
		 *
		 * This is called by mainLoop() every 10 m/s. You may extend it (but not replace it) to
		 * implement any protocol logic as needed. Be sure to check the connectState so you know
		 * whether there is a server connected or not.
		 *
		 */

		virtual void eventTick(time_t time);

		/// I think we're going to drop this soon.
		double getHighResolutionTime();

		/** @brief initialize a connection.
		 *
		 * This is called once a connection to the server is established. You may extend it to
		 * implement any protocol logic as needed.
		 */
		virtual void initConnection(time_t time);

		/** @brief drop a connection.
		 *
		 * This is called to drop a connection to the server, or after the server has dropped the
		 * connection. If you extend it, bear in mind the socket is already closed at this point.
		 */
		virtual void dropConnection(time_t time);

		/** @brief Looks for "lines" in inputBuffer.
		 *
		 * Looks for "lines" in inputBuffer, and calls processLine() for each one found.
		 *
		 */
		virtual void processInput(time_t _time);

		/** @brief process a line from the server.
		 *
		 * You definitely want to override this. Place here your protocol logic for processing
		 * lines received from the server.
		 */
		virtual void processLine(time_t time, std::string line);

		/** @brief add a string to be sent to the server to the outputBuffer.
		 *
		 * You can call this from anywhere, it adds a string to be sent to the server to the
		 * outputBuffer.
		 */
		virtual void sendData(std::string data);

		/** @Brief set debug level
		 *
		 */

		virtual void setDebug(int _debug) {
			debug = _debug;
		}

	};
}
#endif /* PIEXTPLANE_SRC_TCPCLIENT_H_ */
