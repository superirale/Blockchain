
#include <iostream>
#include <cstdlib>
#include <string>
#include <cassert>
#include <ctime>
#include "Transaction.h"
#include "Base_Donnee.h"
#include "BlockChain.h"
#include <boost/asio.hpp>

#include "Serveur.h"
using std::cout; using std::endl;


// ^(?([^\r\n])\s)*[^\s+?/]+[^\n]*$


std::string random_string(size_t length)
{
	auto randchar = []() -> char
	{
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}

void updateTransactionList(Base_Donnee& base_de_donnee, BlockChain& blockchain, const Block& block)
{
	using Cuple = std::tuple<int, string, Block>;

	Cuple leading = blockchain.get_LeadingBlock();
	Cuple newbloc = Cuple(block.get_Header().get_NumeroBloc(), block.get_BlockHash(), block);
	do
	{

		base_de_donnee.update(std::get<2>(leading), Base_Donnee::NOT_VALIDATED);
		base_de_donnee.update(std::get<2>(newbloc), Base_Donnee::VALIDATED);

		leading = blockchain.get_PreviousBlock(leading);
		newbloc = blockchain.get_PreviousBlock(newbloc);
	} while (leading != newbloc);
}

int8_t receiveBlock(Base_Donnee& base_de_donnee,const Block& block,BlockChain& blockchain)
{
	// Check for the validity of the block, not the transaction inside !
	if (blockchain.size() == 0)
	{
		blockchain.push_back(block);
		return true;
	}
	if (!block.isValid())
		return false;
	// TODO or perhaps no, I consider as false, all the block that contains at least one transaction I can't find in my data base,
	// perhaps it is not the best option..., but as the block is build in 10 minutes, I consider normal that the peer should have "heard"
	// about this transaction before the block is received

	try {
		for (auto tr : block.get_Transactions_List())
		{
			if (base_de_donnee.get_status(tr) == Base_Donnee::NOT_FOUND)
			{
				std::cout << "This block has transaction unknown" << endl;
				return 0;
			}
		}
		if (block.get_Header().get_NumeroBloc() > std::get<0>(blockchain.get_LeadingBlock()))
		{
			// If I was mining, just stop it !
			std::cout << "This Block updates the BlockChain" << endl;
			blockchain.push_back(block);

			// + update the database
			updateTransactionList(base_de_donnee,blockchain,block);
		}
		else
		{
			blockchain.push_back(block);
		}
	}
	catch (std::exception e)
	{
		return -1;
	};
	return 1;
}




void test_integration_BlockCHain()
{
	// Creation of IDs
	Identite id("Franc", "Jerome");
	Identite id1("CAVA", "ETtoi");
	Identite id2("Hola", "QueTal");
	Identite id3("MuyBien", "YTu");
	Identite id4("Ou", "COmmenet");
	Identite id5("Reviser", "L'espagnol");
	vector<Identite>  ids = { id,id1,id2,id3,id4,id5 };

	// Creation of transaction then random transaction
	Transaction t(id, "facebook.com", "meilleur site internet");
	Transaction t1(id1, "google.com", "la vie priv�e on y tient");
	Transaction t2(id2, "microsoft.com", "ils ont un bon editeur de texte");
	Transaction t22(id2, "microsoft.fr", "ils sont cools en plus");
	Transaction t3(id3, "apple.com", "f**k le FBI");
	Transaction t4(id4, "codingame", "simply de best");
	Transaction t6(id, "chill.com", "ca existe?");
	Transaction t7(id1, "spion.com", "on s'amuse");
	Transaction t8(id2, "amazon.com", "riche comme cresus");
	Transaction t82(id2, "insa.fr", "hum hum hum");
	Transaction t9(id3, "holahola.com", "hablo espagnol");
	Transaction t10(id4, "darty.com", "LOL");

	vector<Transaction> all_Transaction;
	vector<Transaction> transactions1{ t1,t,t2,t22,t3,t4 };
	vector<Transaction> transactions2{ t6,t8,t82,t7,t9,t10 };
	all_Transaction.insert(all_Transaction.end(), transactions1.begin(), transactions1.end());
	all_Transaction.insert(all_Transaction.end(), transactions2.begin(), transactions2.end());

	for (int i(0); i < 2000; i++)
	{
		string nom_de_domaine = random_string(15);
		string information = random_string(35);
		all_Transaction.emplace_back(ids.at(rand() % ids.size()), nom_de_domaine, information);
	}

	// Insertion in the database
	Base_Donnee base_de_donnee;
	for (const auto &trs : all_Transaction)
	{
		if (base_de_donnee.push_back(trs) == true)
		{
			std::cout << "1";
		}
		else
			std::cout << "0";
	}
	cout << endl;

	// Get element in the database
	// string random = SHA25::sha256("HelloYou");
	// std::cout << "Element not in the database "  << (base_de_donnee.get(random).first == 4 ? "4" : "Error" ) << std::endl;
	// std::cout << "Hash of an element in the database " << 
	// 	base_de_donnee.get(all_Transaction.at(5).getHashTransaction()).second.getHashTransaction() << std::endl;
	// std::cout << all_Transaction.at(5).getHashTransaction();

	Block genesis(0);
	std::cout << "Genesis :" << genesis.get_BlockHash() << endl << genesis.get_PreviousBlockHash() << endl;
	std::shared_ptr<Block> ptr0 = std::make_shared<Block>(genesis);

	Block block1(ptr0, transactions1);
	std::shared_ptr<Block> ptr1 = std::make_shared<Block>(block1);
	Block block2(ptr1, transactions2);
	std::vector<Block> all_blocks;

	cout << "---------------------------- Test on the BlockChain ----------------------------" << endl;
	BlockChain blockchain;


	if (receiveBlock(base_de_donnee, genesis, blockchain) == 1)
	{
		std::cout << "Added " << genesis.get_Header().get_NumeroBloc() << endl;
		all_blocks.push_back(genesis);
	}

	// std::cout << blockchain.push_back(block1) << std::endl;
	// cout << (blockchain.find(t1) == true ? "la transaction t1 existe" : "t1 n'existe pas") << endl;
	// 
	// cout << (blockchain.find(t6) == true ? "la transaction t6 existe" : "t6 n'existe pas") << endl;
	// std::cout << blockchain.push_back(block2) << std::endl;
	// 
	// cout << (blockchain.find(t6) == true ? "la transaction t6 existe" : "t6 n'existe pas") << endl;

	// Block block3(5);
	// block3.set_Hash_Merkle_Root(SHA25::sha256("Hello world"));
	// 
	// 
	// 
	// cout << blockchain.push_back(block3) << endl;
	cout << "---------------------------- Test on the blocs ----------------------------" << endl;
	// cout << "Solving the problem" << endl;
	std::pair<unsigned long long, unsigned long long> nonce = block1.solveProofofWork();
	// 
	// cout << "Hash found : " << SHA25::sha256(string(block1.get_Header().get_HashMerkleRoot() + std::to_string(nonce.first) + std::to_string(nonce.second))) << endl;
	// cout << "The two nonce are" << nonce.first << " " << nonce.second << endl;
	// cout << endl;
	if (receiveBlock(base_de_donnee, block1, blockchain) == 1)
	{
		std::cout << "Added " << block1.get_Header().get_NumeroBloc() << endl;
		all_blocks.push_back(block1);
	}
	block2.solveProofofWork();
	if (receiveBlock(base_de_donnee, block2, blockchain) == 1)
	{
		std::cout << "Added " << block2.get_Header().get_NumeroBloc() << endl;
		all_blocks.push_back(block2);
	}
	std::shared_ptr<Block> ptrX = std::make_shared<Block>(block2);


	Block blocx(ptrX, transactions2);

	for (int i(0); i < 200; i++)
	{
		std::vector<Transaction> transactionX;

		int random = rand();
		// Trying to add block that should be delete afterwards
		if (random & 1)
		{

			for (int k(0); k < (rand() % 9); k++) {
				transactionX.clear();
				for (int j(0); j < 6; j++)
				{
					transactionX.push_back(all_Transaction.at(rand() % all_Transaction.size()));
				}
				std::shared_ptr<Block> ptrX1 = std::make_shared<Block>(all_blocks.at(rand() % all_blocks.size()));
				blocx = Block(ptrX1, transactionX);
				blocx.solveProofofWork();
				if (receiveBlock(base_de_donnee,blocx,blockchain) == 1)
				{
					std::cout << "Added " << blocx.get_Header().get_NumeroBloc() << endl;
					all_blocks.push_back(blocx);
				}
			}
		}

		for (int j(0); j < 6; j++)
		{
			transactionX.push_back(all_Transaction.at(rand() % all_Transaction.size()));
		}

		blocx = Block(ptrX, transactionX);
		blocx.solveProofofWork();
		ptrX = std::make_shared<Block>(blocx);
		if (receiveBlock(base_de_donnee, blocx, blockchain) == 1)
		{
			std::cout << "Added " << blocx.get_Header().get_NumeroBloc() << endl;
			all_blocks.push_back(blocx);
		}
	}

	blockchain.print();

	cout << endl << endl;
	blockchain.clear();

	cout << endl << endl;

	base_de_donnee.print();
}

int main()
{
	std::freopen("test.out", "w", stdout);
	srand(time(NULL));

	test_integration_BlockCHain();
	system("pause");
	return 0;
}




/*
/*
boost::asio::io_service io_service;
tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 7171);


std::shared_ptr<Peer> p = std::make_shared<Peer>(io_service, endpoint);
std::shared_ptr<Client> client = Client::create(io_service, endpoint, p);
p->addClient(client);
p->connexion();


//Serveur  t(io_service, endpoint);

io_service.run();
*/
