#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <string>

using namespace std;

void find_first(vector<pair<string, string>> gram, map<string, set<string>>& firsts, set<string>& non_terms, string non_term);

void find_follow(vector<pair<string, string>> gram, map<string, set<string>>& follows, map<string, set<string>> firsts, set<string>& non_terms, string non_term);

int main(int argc, char const* argv[])
{
	// Parsing the grammar file
	fstream grammar_file;
	grammar_file.open("./grammar.txt", ios::in);
	if (grammar_file.fail()) {
		cout << "Error in opening grammar file\n";
		return 2;
	}

	cout << "Grammar parsed from grammar file: \n";
	vector< pair<string, string> > gram;
	int count = 0;
	while (!grammar_file.eof()) {
		string buffer;
		getline(grammar_file, buffer);
		string delimiter = "->";
		string lhs = buffer.substr(0, buffer.find(delimiter));
		buffer.erase(0, lhs.length() + delimiter.length());
		pair<string, string> prod(lhs, buffer);
		gram.push_back(prod);
		cout << count++ << ".  " << gram.back().first << " -> " << gram.back().second << "\n";
	}
	cout << "\n";
	grammar_file.close();

	// Gather all non terminals
	set<string> non_terms;
	for (auto i = gram.begin(); i != gram.end(); ++i) {
		non_terms.insert(i->first);
	}
	cout << "The non terminals in the grammar are: ";
	for (auto i = non_terms.begin(); i != non_terms.end(); ++i) {
		cout << *i << " ";
	}
	cout << "\n";
	// Gather all terminals
	set<string> terms;
	std::string token;
	for (auto i = gram.begin(); i != gram.end(); ++i) {
		string temp = i->second;
		size_t pos = 0;
		while (pos != string::npos) {
			pos = temp.find(" ");
			token = temp.substr(0, pos);
			temp.erase(0, pos);
			if (pos != string::npos)
			{
				temp.erase(0, 1);
			}
			if (non_terms.find(token) == non_terms.end()) {
				terms.insert(token);
			}
		}
	}
	cout << "The terminals in the grammar are: ";
	for (auto i = terms.begin(); i != terms.end(); ++i) {
		cout << *i << " ";
	}
	cout << "\n\n";


	// Start symbol is first non terminal production in grammar
	string start_sym = gram.begin()->first;
	map< string, set<string> > firsts;
	for (auto non_term = non_terms.begin(); non_term != non_terms.end(); ++non_term) {
		if (firsts[*non_term].empty()) {
			find_first(gram, firsts, non_terms, *non_term);
		}
	}
	cout << "\nFirsts list: \n";
	for (auto it = firsts.begin(); it != firsts.end(); ++it) {
		cout << it->first << " : ";
		for (auto firsts_it = it->second.begin(); firsts_it != it->second.end(); ++firsts_it) {
			cout << *firsts_it << " ";
		}
		cout << "\n";
	}
	cout << "\n";

	map< string, set<string> > follows;
	// Find follow of start variable first
	follows[start_sym].insert("$");
	find_follow(gram, follows, firsts, non_terms, start_sym);
	// Find follows for rest of variables
	for (auto it = non_terms.begin(); it != non_terms.end(); ++it) {
		if (follows[*it].empty()) {
			find_follow(gram, follows, firsts, non_terms, *it);
		}
	}

	cout << "Follows list: \n";
	for (auto it = follows.begin(); it != follows.end(); ++it) {
		cout << it->first << " : ";
		for (auto follows_it = it->second.begin(); follows_it != it->second.end(); ++follows_it) {
			cout << *follows_it << " ";
		}
		cout << "\n";
	}
	cout << "\n";

	int** parse_table = new int* [non_terms.size()];
	for (int i = 0; i < non_terms.size(); i++) {
		parse_table[i] = new int[terms.size()];
		for (int j = 0; j < terms.size(); j++) {
			parse_table[i][j] = -1;
		}
	}
	for (auto prod = gram.begin(); prod != gram.end(); ++prod) {
		string rhs = prod->second;
		size_t pos = 0;
		token = "";
		set<string> next_list;
		bool finished = false;
		while (pos != string::npos) {
			pos = rhs.find(" ");
			token = rhs.substr(0, pos);
			rhs.erase(0, pos);
			if (pos != string::npos)
				rhs.erase(0, 1);
			if (non_terms.find(token) == non_terms.end()) {
				if (token != "e") {
					next_list.insert(token);
					finished = true;
					break;
				}
				continue;
			}

			set<string> firsts_copy(firsts[token].begin(), firsts[token].end());
			if (firsts_copy.find("e") == firsts_copy.end()) {
				next_list.insert(firsts_copy.begin(), firsts_copy.end());
				finished = true;
				break;
			}
			firsts_copy.erase("e");
			next_list.insert(firsts_copy.begin(), firsts_copy.end());
		}
		// If the whole rhs can be skipped through epsilon or reaching the end
		// Add follow to next list
		if (!finished) {
			next_list.insert(follows[prod->first].begin(), follows[prod->first].end());
		}


		for (auto ch = next_list.begin(); ch != next_list.end(); ++ch) {
			int row = distance(non_terms.begin(), non_terms.find(prod->first));
			int col = distance(terms.begin(), terms.find(*ch));
			int prod_num = distance(gram.begin(), prod);
			if (parse_table[row][col] != -1) {
				cout << "Collision at [" << row << "][" << col << "] for production " << prod_num << "\n";
				continue;
			}
			parse_table[row][col] = prod_num;
		}

	}
	// Print parse table
	cout << "Parsing Table: \n";
	cout << "   ";
	for (auto i = terms.begin(); i != terms.end(); ++i) {
		cout << *i << " ";
	}
	cout << "\n";
	for (auto row = non_terms.begin(); row != non_terms.end(); ++row) {
		cout << *row << "  ";
		for (int col = 0; col < terms.size(); ++col) {
			int row_num = distance(non_terms.begin(), row);
			if (parse_table[row_num][col] == -1) {
				cout << "- ";
				continue;
			}
			cout << parse_table[row_num][col] << " ";
		}
		cout << "\n";
	}
	cout << "\n";

	fstream string_file;
	string_file.open("./string.txt", ios::in);
	if (string_file.fail()) {
		cout << "Error in opening string file\n";
		return 2;
	}

	cout << "String from file: \n";
	char ch = ' ';
	string buffer = "";
	stack<string> st;
	vector<string> tokens;
	token = "";
	st.push("$");
	st.push("S");
	while (!string_file.eof()) {
		string buffer;
		getline(string_file, buffer);
		for (int i = 0; i < buffer.length(); i++) {
			ch = buffer[i];
			if (ch == ' ' || ch == '\t') {
				if (!token.empty()) {
					tokens.push_back(token);
					cout << token << " ";
					token = "";
				}
			}
			else if (ch == ';' || ch == '{' || ch == '}' || ch == '(' || ch == ')') {
				if (!token.empty()) {
					tokens.push_back(token);
					cout << token << " ";
				}
				token = ch;
				tokens.push_back(token);
				cout << token << " ";
				token = "";
			}
			else {
				token += ch;
			}
		}
	}
	cout << "\n";
	string_file.close();
	size_t pos = 0;
	tokens.push_back("$");

	// cout<<"Processing input string\n";
	bool accepted = true;
	vector<string> test;
	while (!st.empty()) {
		// If stack top same as input string char remove it

		if (tokens[0] == st.top()) {
			st.pop();
			tokens.erase(tokens.begin());
		}
		else if (non_terms.find(st.top()) == non_terms.end()) {
			cout << "Unmatched terminal found " << st.top() << "\n";
			accepted = false;
			break;
		}
		else {
			string stack_top = st.top();
			int row = distance(non_terms.begin(), non_terms.find(stack_top));
			int col = distance(terms.begin(), terms.find(tokens[0]));
			int prod_num = parse_table[row][col];

			if (prod_num == -1) {
				cout << "No production found in parse table\n";
				accepted = false;
				break;
			}

			st.pop();
			string rhs = gram[prod_num].second;
			if (rhs[0] == 'e') {
				continue;
			}
			pos = 0;
			token = "";
			while (pos != string::npos)
			{
				pos = rhs.find(" ");
				token = rhs.substr(0, pos);
				test.push_back(token);
				rhs.erase(0, pos);
				if (pos != string::npos)
					rhs.erase(0, 1);
			}
			while (!test.empty()) {
				st.push(test.back());
				test.pop_back();
			}
		}
	}

	if (accepted) {
		cout << "Input string is accepted\n";
	}
	else {
		cout << "Input string is rejected\n";
	}

	for (int i = 0; i < non_terms.size(); i++) {
		delete parse_table[i];
	}
	delete[] parse_table;
	return 0;
}

void find_first(vector<pair<string, string>> gram, map<string, set<string>>& firsts, set<string>& non_terms, string non_term) {

	//cout<<"\nFinding firsts of "<<non_term<<"\n";

	for (auto it = gram.begin(); it != gram.end(); ++it) {
		// Find productions of the non terminal
		if (it->first != non_term) {
			continue;
		}
		size_t pos = 0;
		std::string token;
		//cout<<"Processing production "<<it->first<<"->"<<it->second<<"\n";
		string rhs = it->second;
		// Loop till a non terminal or no epsilon variable found
		while (pos != string::npos) {
			pos = rhs.find(" ");
			token = rhs.substr(0, pos);
			rhs.erase(0, pos);
			if (pos != string::npos)
			{
				rhs.erase(0, 1);
			}
			// If first char in production a non term, add it to firsts list
			if (non_terms.find(token) == non_terms.end()) {
				firsts[non_term].insert(token);
				break;
			}
			else {
				// If char in prod is non terminal and whose firsts has no yet been found out
				// Find first for that non terminal
				if (firsts[token].empty()) {
					find_first(gram, firsts, non_terms, token);
				}
				// If variable doesn't have epsilon, stop loop
				if (firsts[token].find("e") == firsts[token].end()) {
					firsts[non_term].insert(firsts[token].begin(), firsts[token].end());
					break;
				}

				set<string> firsts_copy(firsts[token].begin(), firsts[token].end());

				// Remove epsilon from firsts if not the last variable
				if (pos != string::npos) {
					firsts_copy.erase("e");
				}

				// Append firsts of that variable
				firsts[non_term].insert(firsts_copy.begin(), firsts_copy.end());
			}
		}

	}
}

void find_follow(vector<pair<string, string>> gram, map<string, set<string>>& follows, map<string, set<string>> firsts, set<string>& non_terms, string non_term) {

	// cout<<"Finding follow of "<<non_term<<"\n";

	for (auto it = gram.begin(); it != gram.end(); ++it) {

		// finished is true when finding follow from this production is complete
		bool finished = true;
		string rhs = it->second;
		string ch;
		size_t pos = 0;
		string token;

		while (pos != string::npos) {
			pos = rhs.find(" ");
			token = rhs.substr(0, pos);
			rhs.erase(0, pos);
			if (pos != string::npos) {
				rhs.erase(0, 1);
			}
			if (token == non_term) {
				finished = false;
				break;
			}
		}

		while (pos != string::npos && !finished) {
			pos = rhs.find(" ");
			token = rhs.substr(0, pos);
			rhs.erase(0, pos);
			if (pos != string::npos) {
				rhs.erase(0, 1);
			}
			// If non terminal, just append to follow
			if (non_terms.find(token) == non_terms.end()) {
				follows[non_term].insert(token);
				finished = true;
				break;
			}

			set<string> firsts_copy(firsts[token]);
			// If char's firsts doesnt have epsilon follow search is over 
			if (firsts_copy.find("e") == firsts_copy.end()) {
				follows[non_term].insert(firsts_copy.begin(), firsts_copy.end());
				finished = true;
				break;
			}
			// Else next char has to be checked after appending firsts to follow
			firsts_copy.erase("e");
			follows[non_term].insert(firsts_copy.begin(), firsts_copy.end());

		}


		// If end of production, follow same as follow of variable
		if (pos == string::npos && !finished) {
			// Find follow if it doesn't have
			if (follows[it->first].empty()) {
				find_follow(gram, follows, firsts, non_terms, it->first);
			}
			follows[non_term].insert(follows[it->first].begin(), follows[it->first].end());
		}

	}

}
