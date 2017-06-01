#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
using namespace std;

struct Block {
	static int block_ctr;
	int block_address;
	Block() {
		this->block_address = this->block_ctr++;
	}
} ;
int Block::block_ctr = 0;


struct Node {
	static int node_ctr;
	int node_id;
	Node() {
		this->node_id = this->node_ctr++;
	}

	// a map from block_address to prob_owner
	map<int, Node*> blockAddress_probOwner_map;

	// a map from block_address to nodes who have the copy-set
	// blockAddress_copySet_map[ba] contains value only when this node is the true owner of ba
	map<int, vector<Node*> > blockAddress_copySet_map;

	// a map from block_adress to physical block data
	//  local_blockAddress_content_map[ba] contains value only when this node is the true owner of ba
	map<int, Block> local_blockAddress_content_map;

	// This function will be called when this node want to access a block
	// Parameter:
	//		block_address: (int) the block address the node wants to access to
	//      action: (int) 0 for read and 1 for write
	void access_block(int block_address, int action) {
		cout << "---------BEGIN----------" << endl;

		cout << "Node[" << this->node_id << "] starts to request the block [" << block_address << "] now." << endl;
		map<int, vector<Node*> >::iterator it;
		it = this->blockAddress_copySet_map.find(block_address);

		if (it != this->blockAddress_copySet_map.end()) {
			cout << "Node[" << this->node_id << "] is the turn owner of the block [" << block_address << "] now." << endl;
		}
		// when a fault occurs...
		else {
			cout << "Node[" << this->node_id << "] does not have the block [" << block_address << "] now." << endl;

			// look up the local map to find the prob owner
			Node* prob_owner = this->blockAddress_probOwner_map[block_address];
			cout << "According to node [" << this->node_id << "] the prob owner is node[" << prob_owner->node_id << "].  Start requesting..." << endl;

			// and send the request to it
			prob_owner->request_block(this, block_address, action);

			// after the request done, this node should now have the block corresponding the block_address
		}

		cout << "---------END----------" << endl;
	}

	// This function will be called when this node locally doesnot have the block physcially
	// Parameter:
	//		block_address: (int) the block address the node wants to access to
	void request_block(Node* requester, int block_address, int action) {

		cout << "Node[" << this->node_id << "] received a request from node["<< requester->node_id <<"] for block [" << block_address << "]" << endl;
	
		map<int, vector<Node*> >::iterator it;
		it = this->blockAddress_copySet_map.find(block_address);

		// if this node physcially has the block
		// that is this node is the true owner
		if (it != this->blockAddress_copySet_map.end()) {

			cout << "Node[" << this->node_id << "] indeed is the true owner the block [" << block_address << "]." << endl;

			// determine which action wants to be taken
			if (action == 0) {
				// if requester wants to read...
				// call the handler which handle the read action
				this->handle_read(requester, block_address);
			}
			else {
				// if requester wants to write...
				// call the handler which handle the write action
				this->handle_write(requester, block_address);

				// now this node should be the true owner of the block corresponding to the block_address
			}
		}
		else {
			cout << "Node[" << this->node_id << "] does not have the block [" << block_address << "]." << endl;
			
			// otherwise look up the local map
			Node* prob_owner = this->blockAddress_probOwner_map[block_address];

			cout << "According to node [" << this->node_id << "] the prob owner is node[" << prob_owner->node_id << "].  Start requesting..." << endl;

			// then set the block_address prob_owner as the requester
			this->blockAddress_probOwner_map[block_address] = requester;

			// and forward the request
			prob_owner->request_block(requester, block_address, action);
		}
	}

	// This function will be called when another node want to read the block which is owned by this node.
	// Parameter:
	//		requester: the requester of the request
	//		block_address: the block address the node wants to access to
	void handle_read(Node* requester, int block_address) {

		cout << "Node[" << this->node_id << "] is handling the reading request to block [" << block_address << "] from node[" << requester->node_id << "]" << endl;
		// add requester in the copy-set of the corresponding map value 
		this->blockAddress_copySet_map[block_address].push_back(requester);

		// set the copy block value to the requester
		requester->local_blockAddress_content_map[block_address] = this->local_blockAddress_content_map[block_address];
		requester->blockAddress_probOwner_map[block_address] = this;
	}

	// This function will be called when another node want to write the block which is owned by this node.
	// Parameter:
	//		requester: the requester of the request
	//		block_address: the block address the node wants to access to
	void handle_write(Node* requester, int block_address) {
		
		cout << "Node[" << this->node_id << "] is handling the writing request to block [" << block_address << "] from node[" << requester->node_id << "]" << endl;
		map<int, vector<Node*> >::iterator it;
		it = this->blockAddress_copySet_map.find(block_address);

		// add this node in the copy-set of the corresponding map value 
		it->second.push_back(this);

		// set the requester's copy-set information as this node's
		requester->blockAddress_copySet_map[block_address] = it->second;

		// set the copy block value to the requester
		requester->local_blockAddress_content_map[block_address] = this->local_blockAddress_content_map[block_address];

		// erase the copy set locally
		// which means that from now on requester is the true owner
		this->blockAddress_copySet_map.erase(it);
		this->blockAddress_probOwner_map[block_address] = requester;
	}

	// This function will be called when this node want to be initialized.
	// I decide to set prob_owner of every block as the same node
	// with the condition that the caller of this function should ensure 
	// the entry-series of all nodes of the same block_address all generates a loop
	// For example, assume there's 5 nodes, then every blockAddress_probOwner_map of block_address 0 will be as follow:
	//		node[0].blockAddress_probOwner_map[0] = node[1]
	//		node[1].blockAddress_probOwner_map[0] = node[2]
	//		node[2].blockAddress_probOwner_map[0] = node[3]
	//		node[3].blockAddress_probOwner_map[0] = node[4]
	//		node[4].blockAddress_probOwner_map[0] = node[0]
	// The reason is that in circular way, a node will always eventually find the true owner 
	// because the request is able to be sent to every nodes
	// Parameter:
	//		block_size: the totall number of blocks 
	//					*** Here for simplicity we assume that block_size implies the block_address array is:
	//							[0, 1, 2, ... , block_size-1]
	//		next_node: prob_owner
	void initialize(int block_size, Node* next_node) {
		for (int i = 0; i < block_size; i++)
		{
			this->blockAddress_probOwner_map[i] = next_node;
		}
	}


	// This function will be called when initially the system assigns 
	// the physcially block content of the block address locally
	// Parameter:
	//		block_address: the address of the block_content
	//		block_content: the content of the block_address
	void insert_block(int block_address, Block block_content) {
		this->local_blockAddress_content_map[block_address] = block_content;
		this->blockAddress_copySet_map[block_address].push_back(this);
	}
	
	
} ;
// static counter
int Node::node_ctr = 0;

int main()
{
	int node_len = 3;
	int block_size = 6;

	Node* nodes = new Node[node_len];
	Block* blocks = new Block[block_size];

	random_shuffle(&blocks[0], &blocks[block_size]);

	cout << "initializing----" << endl;

	for (int i = 0; i < block_size; i++) {
		int node_index = i % node_len;
		int block_address = blocks[i].block_address;
		nodes[node_index].insert_block(block_address, blocks[i]);
		cout << "Node [" << node_index << "] has the block [" << block_address << "]" << endl;
	}

	for (int i = 0; i < node_len; i++)
	{
		int next_node_index = (i + 1) % node_len;
		nodes[i].initialize(block_size, &nodes[next_node_index]);
	}

// test section

	// nodes[0] want to read block[0]
	cout << "Demo: nodes[0] want to read block[0]" << endl;
	nodes[0].access_block(0, 0);

	// nodes[0] want to write block[0]
	cout << endl << "Demo: nodes[0] want to write block[0]" << endl;
	nodes[0].access_block(0, 1);

	// nodes[1] want to read block[0]
	cout << endl << "Demo: nodes[1] want to read block[0]" << endl;
	nodes[1].access_block(0, 0);

	// nodes[1] want to read block[0]
	cout << endl << "Demo: nodes[1] want to write block[0]" << endl;
	nodes[1].access_block(0, 1);

	// nodes[2] want to read block[0]
	cout << endl << "Demo: nodes[2] want to read block[0]" << endl;
	nodes[2].access_block(0, 0);

	// nodes[2] want to read block[0]
	cout << endl << "Demo: nodes[2] want to write block[0]" << endl;
	nodes[2].access_block(0, 1);
	
	return 0;
}
