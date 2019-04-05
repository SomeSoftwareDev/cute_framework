/*
	Cute Framework
	Copyright (C) 2019 Randy Gaul https://randygaul.net

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
*/

#include <cute_client.h>
#include <cute_server.h>
#include <cute_alloc.h>

using namespace cute;

CUTE_TEST_CASE(test_client_server_handshake, "Test out barebones connection between client and server instances.");
int test_client_server_handshake()
{
	client_t* client = client_alloc(NULL);
	CUTE_TEST_CHECK_POINTER(client);

	server_t* server = server_alloc(NULL);
	CUTE_TEST_CHECK_POINTER(server);

	crypto_key_t pk, sk;
	CUTE_TEST_CHECK(crypto_generate_keypair(&pk, &sk));
	CUTE_TEST_CHECK(server_start(server, "127.0.0.1:5000", &pk, &sk, NULL));
	CUTE_TEST_CHECK(client_connect(client, 501, "127.0.0.1:5000", &pk));
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTING);

	float dt = 1.0f / 60.0f;

	// Send hello.
	client_update(client, dt);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTING);

	// Accept connection, send connection response.
	server_update(server, dt);

	// Recieve connection accepted. Client should report as connected.
	client_update(client, dt);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTED);

	client_disconnect(client);
	server_stop(server);

	client_destroy(client);
	server_destroy(server);

	return 0;
}

CUTE_TEST_CASE(test_keep_alive_packets, "Make sure keepalive packets fly between idle client and server, and properly update timestamps.");
int test_keep_alive_packets()
{
	client_t* client = client_alloc(NULL);
	CUTE_TEST_CHECK_POINTER(client);
	server_t* server = server_alloc(NULL);
	CUTE_TEST_CHECK_POINTER(server);

	crypto_key_t pk, sk;
	CUTE_TEST_CHECK(crypto_generate_keypair(&pk, &sk));
	CUTE_TEST_CHECK(server_start(server, "127.0.0.1:5000", &pk, &sk, NULL));
	CUTE_TEST_CHECK(client_connect(client, 501, "127.0.0.1:5000", &pk));

	// Assert connectivity.
	// Advance client/server time significantly to trigger the need for keepalive packets upon the subsequent updates.
	client_update(client, 0);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTING);
	server_update(server, CUTE_KEEPALIVE_RATE);
	client_update(client, CUTE_KEEPALIVE_RATE);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTED);

	// Assert new client connected event arrival.
	server_event_t event;
	CUTE_TEST_CHECK(server_poll_event(server, &event));
	CUTE_TEST_ASSERT(event.type == SERVER_EVENT_TYPE_NEW_CONNECTION);
	cute::handle_t client_id = event.u.new_connection.client_id;

	// Event queue should be empty now.
	CUTE_TEST_ASSERT(server_poll_event(server, &event) < 0);

	// Assert timestamps as needing keepalive packets from the opposing endpoints.
	CUTE_TEST_ASSERT(client_get_last_packet_recieved_time(client) == CUTE_KEEPALIVE_RATE);
	CUTE_TEST_ASSERT(server_get_last_packet_recieved_time_from_client(server, client_id) == CUTE_KEEPALIVE_RATE);

	// Update each endpoint to trigger keepalive packets.
	server_update(server, 0); // Send keepalive.
	client_update(client, 0); // Get a keepalive, send a keepalive. Timestamp cleared.
	server_update(server, 0); // Get a keepalive. Timestamp cleared.

	// Assert timestamps were cleared.
	CUTE_TEST_ASSERT(client_get_last_packet_recieved_time(client) == 0);
	CUTE_TEST_ASSERT(server_get_last_packet_recieved_time_from_client(server, client_id) == 0);

	client_disconnect(client);
	server_stop(server);

	client_destroy(client);
	server_destroy(server);

	return 0;
}

CUTE_TEST_CASE(test_no_server_response_on_client_connect, "Client should attempt to connect to non-existant server and retry 3 times before disconnecting.");
int test_no_server_response_on_client_connect()
{
	client_t* client = client_alloc(NULL);
	CUTE_TEST_CHECK_POINTER(client);

	crypto_key_t pk, sk;
	CUTE_TEST_CHECK(crypto_generate_keypair(&pk, &sk));
	CUTE_TEST_CHECK(client_connect(client, 501, "127.0.0.1:5000", &pk));

	client_update(client, CUTE_KEEPALIVE_RATE);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTING);

	client_update(client, CUTE_KEEPALIVE_RATE);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTING);

	client_update(client, CUTE_KEEPALIVE_RATE);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTING);

	client_update(client, CUTE_KEEPALIVE_RATE);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_DISCONNECTED);

	client_disconnect(client);
	client_destroy(client);

	return 0;
}

CUTE_TEST_CASE(test_server_timeout, "Client connects to server, then server times out. Client waits for grace period, then disconnects itself.");
int test_server_timeout()
{
	client_t* client = client_alloc(NULL);
	CUTE_TEST_CHECK_POINTER(client);
	server_t* server = server_alloc(NULL);
	CUTE_TEST_CHECK_POINTER(server);

	crypto_key_t pk, sk;
	CUTE_TEST_CHECK(crypto_generate_keypair(&pk, &sk));
	CUTE_TEST_CHECK(server_start(server, "127.0.0.1:5000", &pk, &sk, NULL));
	CUTE_TEST_CHECK(client_connect(client, 501, "127.0.0.1:5000", &pk));

	// Assert connectivity.
	client_update(client, 0);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTING);
	server_update(server, 0);
	client_update(client, 0);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTED);

	// Now server stops responding.
	client_update(client, CUTE_KEEPALIVE_RATE);
	client_update(client, CUTE_KEEPALIVE_RATE);
	client_update(client, CUTE_KEEPALIVE_RATE);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTED);

	// Now client should disconnect.
	client_update(client, CUTE_KEEPALIVE_RATE);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_DISCONNECTED);

	client_disconnect(client);
	server_stop(server);

	client_destroy(client);
	server_destroy(server);

	return 0;
}

CUTE_TEST_CASE(client_connects_then_times_out, "Client should connect then dissappear. Server should detect and cull the connection.");
int client_connects_then_times_out()
{
	client_t* client = client_alloc(NULL);
	CUTE_TEST_CHECK_POINTER(client);
	server_t* server = server_alloc(NULL);
	CUTE_TEST_CHECK_POINTER(server);

	crypto_key_t pk, sk;
	CUTE_TEST_CHECK(crypto_generate_keypair(&pk, &sk));
	CUTE_TEST_CHECK(server_start(server, "127.0.0.1:5000", &pk, &sk, NULL));
	CUTE_TEST_CHECK(client_connect(client, 501, "127.0.0.1:5000", &pk));

	// Assert connectivity.
	client_update(client, 0);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTING);
	server_update(server, 0);
	client_update(client, 0);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_CONNECTED);

	// Now client stops responding. Server should start tracking long response times (no keepalives).
	server_update(server, CUTE_KEEPALIVE_RATE);
	server_update(server, CUTE_KEEPALIVE_RATE);
	server_update(server, CUTE_KEEPALIVE_RATE);

	// Server should cull time-out connection here.
	server_look_for_and_disconnected_timed_out_clients(server);

	// Client should report as disconnected.
	client_update(client, 0);
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_DISCONNECTED);

	client_disconnect(client);
	server_stop(server);

	client_destroy(client);
	server_destroy(server);

	return 0;
}

CUTE_TEST_CASE(test_max_clients_connection_denied, "Spawn many clients, to overflow the server. Should get connection denied response.");
int test_max_clients_connection_denied()
{
	// Startup the server.
	crypto_key_t pk, sk;
	CUTE_TEST_CHECK(crypto_generate_keypair(&pk, &sk));
	server_t* server = server_alloc(NULL);
	CUTE_TEST_CHECK_POINTER(server);
	CUTE_TEST_CHECK(server_start(server, "127.0.0.1:5000", &pk, &sk, NULL));

	// Spin up maximum capacity of clients.
	client_t** clients = (client_t**)CUTE_ALLOC(sizeof(void*) * CUTE_SERVER_MAX_CLIENTS + 1, NULL);
	for (int i = 0; i < CUTE_SERVER_MAX_CLIENTS; ++i)
	{
		clients[i] = client_alloc(NULL);
		CUTE_TEST_CHECK_POINTER(clients[i]);
		CUTE_TEST_CHECK(client_connect(clients[i], 4001 + i, "127.0.0.1:5000", &pk));
		client_update(clients[i], 0);
	}

	// Accept all client connections.
	server_update(server, 0);

	// Assert connectivity.
	for (int i = 0; i < CUTE_SERVER_MAX_CLIENTS; ++i)
	{
		client_update(clients[i], 0);
		CUTE_TEST_ASSERT(client_state_get(clients[i]) == CLIENT_STATE_CONNECTED);
	}

	// Try to connect one more client, but it should fail. Assert no connectivity.
	client_t* client = client_alloc(NULL);
	CUTE_TEST_CHECK(client_connect(client, 6000, "127.0.0.1:5000", &pk));
	client_update(client, 0); // Send connect packet.
	server_update(server, 0); // Respond with connection denied.
	client_update(client, 0); // Recieve connection denied.
	CUTE_TEST_ASSERT(client_state_get(client) == CLIENT_STATE_DISCONNECTED);

	client_disconnect(client);
	client_destroy(client);

	for (int i = 0; i < CUTE_SERVER_MAX_CLIENTS; ++i)
	{
		client_disconnect(clients[i]);
		client_destroy(clients[i]);
	}

	server_update(server, 0);
	server_stop(server);
	server_destroy(server);

	CUTE_FREE(clients, NULL);

	return 0;
}

CUTE_TEST_CASE(server_disconnect_a_client, "Server forcefully disconnects a client.");
int server_disconnect_a_client()
{
	crypto_key_t pk, sk;
	CUTE_TEST_CHECK(crypto_generate_keypair(&pk, &sk));
	server_t* server = server_alloc(NULL);
	CUTE_TEST_CHECK_POINTER(server);
	CUTE_TEST_CHECK(server_start(server, "127.0.0.1:5000", &pk, &sk, NULL));
	
	client_t* client0 = client_alloc(NULL);
	client_t* client1 = client_alloc(NULL);
	CUTE_TEST_CHECK(client_connect(client0, 6000, "127.0.0.1:5000", &pk));
	CUTE_TEST_CHECK(client_connect(client1, 6001, "127.0.0.1:5000", &pk));
	client_update(client0, 0);
	client_update(client1, 0);
	server_update(server, 0);
	client_update(client0, 0);
	client_update(client1, 0);
	CUTE_TEST_ASSERT(client_state_get(client0) == CLIENT_STATE_CONNECTED);
	CUTE_TEST_ASSERT(client_state_get(client1) == CLIENT_STATE_CONNECTED);

	server_event_t event;
	CUTE_TEST_CHECK(server_poll_event(server, &event));
	CUTE_TEST_ASSERT(event.type == SERVER_EVENT_TYPE_NEW_CONNECTION);
	cute::handle_t client_id = event.u.new_connection.client_id;

	server_disconnect_client(server, client_id, 1);
	client_update(client0, 0);
	CUTE_TEST_ASSERT(client_state_get(client0) == CLIENT_STATE_DISCONNECTED);
	CUTE_TEST_ASSERT(client_state_get(client1) == CLIENT_STATE_CONNECTED);

	client_disconnect(client0);
	client_destroy(client0);
	client_disconnect(client1);
	client_destroy(client1);
	server_stop(server);
	server_destroy(server);

	return 0;
}

CUTE_TEST_CASE(client_disconnects_itself_from_server, "Client disconnects itself from server, and server should recieve notification.");
int client_disconnects_itself_from_server()
{
	return 0;
}

// PRIORITIZED TODO:
// [ ] All packets should have unified read/write functions, implying the same headers and encryptors.
// [ ] Refactor connection request packet to use standardized packet form.
// [ ] Redundant disconnect packets (unreliable).
// [ ] Challenge response to prevent spoofing.
// [ ] Connect confirmation also needs client index + max client count.
// [ ] Optional "expect connect tokens" setting for the beginning of the handshake.

// Generalized packet format
	// HEADER
		// packet type (uint8_t)
		// * "CUTE 1.0" (9 bytes)
		// * game id (uint64_t)
		// sequence number (uin64_t or 24 bytes**)
	// PAYLOAD
		// -- begin encryption --
		// payload data
		// --  end encryption  --
// * Only present in connect token, otherwise is not actually in packet, but still the additional data.
// ** Typical packets use uint64_t, but connect tokens use full 24 bytes.

// Connect token format
	// HEADER
	// expire timestamp (uint64_t)
	// payload - secret_data
		// user id
		// session sequence offset
		// session key
		// user data (512 bytes)
		// the number of server addressess (uint16_t)
			// each server address
		// pad bytes

// The idea is connect token is obtained via REST API (somehow), and the client reads the token data.
// The connect_user_data is an entire encrypted packet, to be sent to the dedicated game server.
// The client reads the list of servers, and sends the connect_user_data to one server at a time.
// The game server can verify the sesion id is valid. The session id maps the player to a single
// playable session. This means it can be used to lookup any information from the backend about the player.

// TEST TODO
// [x] Keep alive packet
// [x] Client timeout
// [x] No server response on connect
// [x] Server timeout
// [x] Connection denied
// [x] Server forecfully disconnects client after connecting
// [ ] Client forecfully disconnects after connecting.
// [ ] Challenge response failure
// [ ] Connect token acceptance
// [ ] Connect token failure