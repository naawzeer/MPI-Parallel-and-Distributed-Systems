#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_IO_WIRES 256 // Amount of input/output wires
#define NUM_STAGES 8
#define MAX_MESSAGE_COUNT 1024

typedef enum SWITCH_POSITION {
	not_set,
	straight,
	crossover
}Switch_Position;

typedef struct SWITCHING_NETWORK_STAGE
{
	// The permutation of the network as its determined by the gluing function:
	int permutation[NUM_IO_WIRES];
	// The switches of the stage represented by their current state:	
	Switch_Position switches[NUM_IO_WIRES / 2];   
	// The switches should be connected to the wires after 
	// being permuted by the permutation of the stage
}Switching_Network_Stage;

typedef struct SWITCHING_NETWORK
{
	Switching_Network_Stage stages[NUM_STAGES];
	bool route_is_blocked[NUM_IO_WIRES];
	// route_is_blocked[i] should be set if the route for the [i]th 
	// input is blocked after routing
}Switching_Network;


// Structure for a message to be transmitted across the switching network
typedef struct MESSAGE
{
	// The input wire, where the message come from:
	int input_wire_id;
	// The output wire, to which the message has to be transmitted across the switching network:
	int output_wire_id;
	// Some data to be transmitted, which is not relevant for this exercise:	
	double data;            
}Message;

typedef struct MESSAGE_BUFFER
{
	int messages_available[NUM_IO_WIRES];
	Message messages[NUM_IO_WIRES][MAX_MESSAGE_COUNT];
}Message_Buffer;


int invert_least_significant_bit(int value)
{
	return value ^ 1;
}

bool least_significant_bit_is_set(int value)
{
	return value & 1;
}

// Determines whether the bit_idth bit of the value argument is set
// starting from the least significant bit
bool bit_at_position_is_set(int value, int bit_position)
{
	return (value >> bit_position) & 1;
}

int invert_bit_at_position(int value, int position)
{
	return value ^ (1 << position);
}

int set_lowest_bits(int amount)
{
	return (1 << (amount)) - 1;
}


// Performs a circular right shift on the value argument, assuming that only a certain amount
// of bits are used in this value starting from the least significant bit
int circular_right_shift(int value, int lowest_bits_used)
{
	int shifted_right = (value >> 1);
	unsigned int mask = set_lowest_bits(lowest_bits_used);
	int shifted_left  = (value << (lowest_bits_used - 1)) & mask;
	int circular_shift = (shifted_left) | (shifted_right);
	return circular_shift;
}

// Performs a circular right shift on the value argument on the lowest amount
// of bits in the value starting from the least significant bit and preserving all
// other bit values
int circular_right_shift_preserving_highest_bits(int value, int amount_of_lower_bits_to_shift)
{
	int mask_bits_to_shift = set_lowest_bits(amount_of_lower_bits_to_shift);
	int lower_bits_to_shift = value & mask_bits_to_shift;
	int upper_bits_to_keep = value & ~mask_bits_to_shift;
	
	int lower_bits_shifted = circular_right_shift(lower_bits_to_shift, amount_of_lower_bits_to_shift);

	int parts_put_together = upper_bits_to_keep | lower_bits_shifted;

	return parts_put_together;
}


// Allocates an empty message buffer
void message_buffer_create(Message_Buffer** message_buffer)
{
	*message_buffer = (Message_Buffer*)calloc(1, sizeof(Message_Buffer));
}

// A helper function checking if the message buffer is empty for all wires
bool message_buffer_is_empty(Message_Buffer* message_buffer)
{
	for (int wire_id = 0; wire_id < NUM_IO_WIRES; wire_id++)
		if (message_buffer->messages_available[wire_id] != 0)
			return false;
	return true;
}

// Fills a message buffer with random values so that it can be used as an input 
// message buffer
void message_buffer_initialize_as_input(Message_Buffer* message_buffer)
{
	int used_message_count = MAX_MESSAGE_COUNT / 2;
	for (int w_id = 0; w_id < NUM_IO_WIRES; w_id++)
	{
		message_buffer->messages_available[w_id] = used_message_count;
		for (int m_id = 0; m_id < used_message_count; m_id++)
		{
			message_buffer->messages[w_id][m_id].data = rand();
			message_buffer->messages[w_id][m_id].input_wire_id = w_id;
			message_buffer->messages[w_id][m_id].output_wire_id = rand() % NUM_IO_WIRES;
		}
	}
}

// Creates a butterfly network
void switching_network_create_butterfly(Switching_Network** network)
{
	(*network) = (Switching_Network*)malloc(sizeof(Switching_Network));
	// No permutation is performed before the first stage
	// Note that this does not hold true for the omega network
	for (int wire_id = 0; wire_id < NUM_IO_WIRES; wire_id++)
		(*network)->stages[0].permutation[wire_id] = wire_id;

	// Iterating over the stages and the switches to set the wires according
	// to the gluing function of the butterfly network
	for (int stage_id = 1; stage_id < NUM_STAGES; stage_id++)
		for (int switch_id = 0; switch_id < NUM_IO_WIRES / 2; switch_id++)
		{
			// Computing the id of the switch for the cross edge in the next stages
			int cross_switch_id_new = invert_bit_at_position(switch_id, NUM_STAGES - stage_id - 1);

			// Handling that the upper input/output wire of a switch does not move down 
			// and the lower input/output wire of a switch does not move up
			if (cross_switch_id_new > switch_id)
			{
				// Straight edge:
				(*network)->stages[stage_id].permutation[2 * switch_id] = 2 * switch_id;
				// Cross edge:
				(*network)->stages[stage_id].permutation[2 * switch_id + 1] = 2 * cross_switch_id_new;
			}
			else
			{
				// Cross edge:				
				(*network)->stages[stage_id].permutation[2 * switch_id] = 2 * cross_switch_id_new + 1;
				// Straight edge:			
				(*network)->stages[stage_id].permutation[2 * switch_id + 1] = 2 * switch_id + 1;
			}
		}
}

// Creates a baseline network
void switching_network_create_baseline(Switching_Network** network)
{
	(*network) = (Switching_Network*)malloc(sizeof(Switching_Network));


	for (int wire_id = 0; wire_id < NUM_IO_WIRES; wire_id++)
		(*network)->stages[0].permutation[wire_id] = wire_id;

	// Iterating over the stages and the switches to set the wires according
	// to the gluing function of the baseline network
	for (int stage_id = 1; stage_id < NUM_STAGES; stage_id++)
		for (int wire_id = 0; wire_id < NUM_IO_WIRES; wire_id++)
		{
			int stage_bit_offset = NUM_STAGES - stage_id + 1;

			int new_wire_id = circular_right_shift_preserving_highest_bits(wire_id, stage_bit_offset);

			(*network)->stages[stage_id].permutation[wire_id] = new_wire_id;
		}
}

// Removes for each input wire the backmost message from the buffer containing the input messages, 
// transmits the message across the network according to the switch positions and appends it to
// the message buffer of the output wire
void switching_network_transmit_next_messages(Switching_Network* network,
	Message_Buffer* messages_in,
	Message_Buffer* messages_out)
{
	// Iterates across the input wires to transmit the messages
	for (int input_wire_id = 0; input_wire_id < NUM_IO_WIRES; input_wire_id++)
	{
		// Route is blocked so message cannot be transmitted or no messages available
		if (network->route_is_blocked[input_wire_id] || 
		    messages_in->messages_available[input_wire_id] == 0)
			continue;

		// Removing the backmost message of the input wire from the input message buffer
		Message m = messages_in->messages[input_wire_id][messages_in->messages_available[input_wire_id] - 1];
		messages_in->messages_available[input_wire_id]--;

		// Transmitting the message by tracing the message across stages consisting of wires and switches
		int wire_id_cur = input_wire_id;
		for (int stage_id = 0; stage_id < NUM_STAGES; stage_id++)
		{
			// Applying the permuation of the gluing function to the current wire
			wire_id_cur = network->stages[stage_id].permutation[wire_id_cur];

			// Applying the switch to the current wire
			int switch_id_cur = wire_id_cur / 2;
			Switch_Position switch_position_cur = network->stages[stage_id].switches[switch_id_cur];

			// If switch is in straight position id of the wire does not change
			if (switch_position_cur == straight)
				wire_id_cur = wire_id_cur;

			// If switch is in crossover position the last bit of the id of the wire has to be inverted
			else if (switch_position_cur == crossover)
				wire_id_cur = invert_least_significant_bit(wire_id_cur);

			// If a message arrives at a switch, which has not yet been set, but the route 
			//  blocked for the input wire has not been set, an error has obviously occured
			else
			{
				printf("Error, switch not set i: %i, cur: %i, o: %i\n", input_wire_id, wire_id_cur, m.output_wire_id);
				return;
			}
		}

		// Checking if the message has been transmitted to the wrong output
		int output_wire_id = wire_id_cur;
		if (output_wire_id != m.output_wire_id)
		{
			printf("Error, message arrived at wrong output: %i, %i, %i\n",
				input_wire_id, output_wire_id, m.output_wire_id);
			return;
		}

		// Appending the transmitted message to the output message buffer of the output wire
		messages_out->messages[output_wire_id][messages_out->messages_available[output_wire_id]] = m;
		messages_out->messages_available[output_wire_id]++;
	}
}



// This function resets all the switches in the network and deletes the route is block flag
// for all wires, so that new routes can be computed
void switching_network_reset(Switching_Network* network)
{
	for (int stage_id = 0; stage_id < NUM_STAGES; stage_id++)
	{
		for (int switch_id = 0; switch_id < NUM_IO_WIRES / 2; switch_id++)
			network->stages[stage_id].switches[switch_id] = not_set;
	}
	for (int wire_id = 0; wire_id < NUM_IO_WIRES; wire_id++)
		network->route_is_blocked[wire_id] = false;
}



// Sets the switch positions of a single wire for its next message to route
void switching_network_route_baseline_or_butterfly_single_wire(Switching_Network* network,
	int input_wire_id,
	Message_Buffer* messages_in)
{
	// Next message to route
	int message_id = messages_in->messages_available[input_wire_id] - 1;
	// No message available
	if (message_id == -1)
		return;

	// Output wire for the message
	int output_wire_id = messages_in->messages[input_wire_id][message_id].output_wire_id;

	// Cache the switch positions in a temporary array
	// so that they can be applied later if the route is not blocked
	int switch_ids_on_route[NUM_STAGES];
	Switch_Position switch_position_required_on_route[NUM_STAGES];

	// Tracing the message through the stages
	int wire_id_cur = input_wire_id;
	for (int stage_id = 0; stage_id < NUM_STAGES; stage_id++)
	{
		// Applying the permutation of the current stage to the wire
		wire_id_cur = network->stages[stage_id].permutation[wire_id_cur];

		// Computing the id of the switch of the current stage, where the message passes through
		int switch_id_cur = wire_id_cur / 2;

		// Determining the whether the message has to be routed to bottom or top output of the switch
		// by checking whether the bit of the output address corresponding to the current stage is set
		bool route_bottom = bit_at_position_is_set(output_wire_id, NUM_STAGES - stage_id - 1);

		// Determining the switch position, depending on whether the messages arrives from
		// top or bottom input of the switch and has to be routed to top or bottom output of the 
		// switch
		Switch_Position position_required;

		// If message arrives from top (least sign. bit not set) and has to be routed to top
		if ((least_significant_bit_is_set(wire_id_cur) == false && !route_bottom) ||
			// or message arrives from bottom (least sign. bit set) and has to be routed to bottom
			(least_significant_bit_is_set(wire_id_cur) == true && route_bottom))
			// the switch has to be set to the straight position
			position_required = straight;

		// Else:
		// If message arrives from top and has to be routed to bottom
		// or message arrives from bottom and has to be routed to top
		// the switch has to be set to the crossover position
		else
			position_required = crossover;


		// Applying the switch to the wire
		// If switch is in straight position id of the wire does not change
		if (position_required == straight)
			wire_id_cur = wire_id_cur;

		// If switch is in crossover position the last bit of the id of the wire has to be inverted
		else if (position_required == crossover)
			wire_id_cur = invert_least_significant_bit(wire_id_cur);

		// Switch is either already in the right position because of a previous route
		// of another input wire or not yet set
		if (network->stages[stage_id].switches[switch_id_cur] == not_set ||
			network->stages[stage_id].switches[switch_id_cur] == position_required)
		{
			switch_ids_on_route[stage_id] = switch_id_cur;
			switch_position_required_on_route[stage_id] = position_required;
		}
		// Switch is already set in the wrong position because of a previous route 
		// of another input wire thus the route for the current input wire is blocked
		// In this case the route is block flag has to be set for the current input wire
		// and no switches have to be manipulated
		else
		{
			network->route_is_blocked[input_wire_id] = true;
			return;
		}
	}


	// Checking if the message has been transmitted to the wrong output
	if (output_wire_id != wire_id_cur)
	{
		printf("Error, message arrived at wrong output: %i, %i, %i\n",
			input_wire_id, output_wire_id, wire_id_cur);
		return;
	}


	// If the function arrives at this point a route has been found from the input
	// wire to the correct output wire, and all switch ids and positions have been cached
	// in the temporary arrays. Thus we have to insert both the switch ids and switch positions
	// into the actual network
	for (int stage_id = 0; stage_id < NUM_STAGES; stage_id++)
		network->stages[stage_id].switches[switch_ids_on_route[stage_id]] =
		switch_position_required_on_route[stage_id];
}

// This function finds a route for the input wires in the ascending order of their id
// Thus this function first finds a route for the [0]th wire,
// then for the [1]th wire, if this route is still not blocked, 
// then for the [2]th wire, if this route is still not blocked and so on.
// As a consequence, the lower the id of a wire the higher its priority, and the
// wires having a high id typically starve.
void switching_network_route_baseline_or_butterfly_static_priority(Switching_Network* network, 
																	Message_Buffer* messages_in)
{
	switching_network_reset(network);
	for (int wire_id = 0; wire_id < NUM_IO_WIRES; wire_id++)
		switching_network_route_baseline_or_butterfly_single_wire(network, wire_id, messages_in);
}

// This function finds a route for the input wires by prioritizing the input wires in a round robin
// manner across subsequent invocations of this routing function
void switching_network_route_baseline_or_butterfly_cyclic_priority(Switching_Network* butterfly_network,
	Message_Buffer* messages_in)
{
	static int invocation_id = 0;
	switching_network_reset(butterfly_network);
	for (int wire_id = 0; wire_id < NUM_IO_WIRES; wire_id++)
		switching_network_route_baseline_or_butterfly_single_wire(butterfly_network,
			(wire_id + invocation_id) % NUM_IO_WIRES, messages_in);
	invocation_id++;
}

//  This function creates an baseline network, an input message buffer, and routes and transfers 
//  all messages of this message buffer across the baseline network
void transfer_messages_baseline(bool use_fair_routing)
{
	Switching_Network* baseline;
	Message_Buffer* messages_in;
	message_buffer_create(&messages_in);
	message_buffer_initialize_as_input(messages_in);
	Message_Buffer* messages_out;
	message_buffer_create(&messages_out);

	switching_network_create_baseline(&baseline);

	int pass_id = 0;
	while (!message_buffer_is_empty(messages_in))
	{
		if (use_fair_routing)
			switching_network_route_baseline_or_butterfly_cyclic_priority(baseline, messages_in);
		else
			switching_network_route_baseline_or_butterfly_static_priority(baseline, messages_in);

		switching_network_transmit_next_messages(baseline, messages_in, messages_out);
		pass_id++;
	}

	if (use_fair_routing)
		printf("Passes required for baseline network with fair routing: %i\n", pass_id);
	else
		printf("Passes required for baseline network with unfair routing: %i\n", pass_id);

	free(baseline);
	free(messages_in);
	free(messages_out);
}

// This function creates a butterfly network, an input message buffer, filled with random messages, 
// and routes and transfers  all messages of this message buffer across the baseline network
void transfer_messages_butterfly(bool use_fair_routing)
{
	Switching_Network* butterfly;
	Message_Buffer* messages_in;
	message_buffer_create(&messages_in);
	message_buffer_initialize_as_input(messages_in);
	Message_Buffer* messages_out;
	message_buffer_create(&messages_out);

	switching_network_create_butterfly(&butterfly);

	int pass_id = 0;
	while (!message_buffer_is_empty(messages_in))
	{
		if (use_fair_routing)
			switching_network_route_baseline_or_butterfly_cyclic_priority(butterfly, messages_in);
		else
			switching_network_route_baseline_or_butterfly_static_priority(butterfly, messages_in);

		switching_network_transmit_next_messages(butterfly, messages_in, messages_out);
		pass_id++;
	}
	if (use_fair_routing)
		printf("Passes required for butterfly-network with fair routing: %i\n", pass_id);
	else
		printf("Passes required for butterfly-network with unfair routing: %i\n", pass_id);

	free(butterfly);
	free(messages_in);
	free(messages_out);
}

int main(int argc, char** args)
{
	transfer_messages_baseline(false);
	transfer_messages_baseline(true);
	transfer_messages_butterfly(false);
	transfer_messages_butterfly(true);

	return 0;
}
