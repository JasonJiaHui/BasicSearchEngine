#include <iostream>
#include <fstream>
#include <istream>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "stmr.h"
#include "tool.h"
/*
This is the third assignment of comp9319, construct a basic search engine within memory and time limit.
In general, I use skiplist to store the inverted index and use last postition to retrival

	1. Index idea, 
		First I need three index file, filename.idx, posting.idx, words.idx
		filename.idx:
				a.txt
				b.txt
				c.txt
				...
		posting.idx:
				@0#3@1#2@3#2$-1$... ==>> @docId#count, "$-1$" denote there is no last position
				@4#3@5#2@6#2$0$...  ==>	 @4#3@5#2@6#2$0$, "$0$" denote the last position is from 0.

			example:
		index:	0 |term:apple| 16  |term:book|  32 |term:apple|       
				@0#3@1#2@3#2$-1$@0#2@1#2@2#2$-1$@5#3@6#2@7#2$0$..... 

		words.idx:
				apple 32 (means the last position of apple is at position 32)    
				book 100
				banana 123

	2. Search idea

		we first load words.idx, and find the position of search term, let's say "apple"(not stem), from read words.idx
		we can find that the last postion of postlist of "apple" is at index = 32 of posting.idx file, then we can use fseek
		to jump to this position 32, and then traverse the postlist unitil meet "$last postioin$", and then we jump to the last postion and traverse the postlist like before until the last position = -1.

		In general, I use the some ideas from skip list, that is every time I only record the last postion. I can be very efficiency.
*/



using namespace std;

string stopWords[460];

map<string, int> tempMap; 
map<string, int>::iterator tempIter; 

map<string, string> words;
map<string, string>::iterator wordIter;

map<string, int> positions;
map<string, int>::iterator posIter;

string fileName[2000];
int fileNum = 0;


// traverse all the filename in the specified directory
int trave_dir(const char* path)
{
	DIR *d;
	struct dirent *file;

	if(!(d = opendir(path))){
		cout << "error..." << endl;
		return -1;
	}
	while((file = readdir(d)) != NULL){
		if(strncmp(file->d_name, ".", 1) == 0)
			continue;

		fileName[fileNum++] = string(file->d_name);
	}
	closedir(d);
	// heapsort the filename
	heapSort(fileName, fileNum);

	return 0;
}

// Initialise stopwords from stopwords file
void initStopwords()
{
	int index = 0;
	string line = "";
	ifstream in("stopwords.txt");

	while(getline(in, line)){
		stopWords[index++] = line;
	}

	in.close();
}

// filter the stopwords by using binary search
bool filter(string str)
{
	int left = 0;
	int right = 459;
	int middle;
	while(left <= right){
		middle = left + ((right - left) >> 1);
		if(stopWords[middle].compare(str) > 0){
			right = middle - 1;
		}else if(stopWords[middle].compare(str) < 0){
			left = middle + 1;
		}else{
			return true;
		}
	}
	return false;
}

// stem the word by using external library stmr.c
string stem_process(string &orig_str)
{
	char* word = new char[orig_str.length() + 1];
	strcpy(word, orig_str.c_str());

	int end = stem(word, 0, strlen(word) - 1);
	word[end + 1] = 0;

	string res(word);
	delete []word;
	return res;
}

// backward search the postlist based on the last position
string backwardSearch(FILE* fp, int position)
{
	char ch;
	string postList = "";
	string prevPosition = "";

	if(position == -1){
		return "";
	}

	fseek(fp, position, SEEK_SET);
	while((ch = fgetc(fp)) != '$'){
		postList += ch;
	}

	while((ch = fgetc(fp)) != '$'){
		prevPosition += ch;
	}
	return backwardSearch(fp, stoi(prevPosition)) + postList;
}

// search the postlist based on the given position
string search(string dirPath, int position)
{
	FILE* fp;
	char ch;
	string res = "";
	string whole_idx_path = dirPath + "/posting.idx";

	fp = fopen(whole_idx_path.c_str(), "r");
	res = backwardSearch(fp, position);

	fclose(fp);

	return res;
}

// write filename into filename.idx 
void writeFileIndex(string dirPath)
{
 	string filename_path = dirPath + "/filename.idx";
    ofstream filename_idx;
    filename_idx.open (filename_path, ofstream::out | ofstream::app);

   	int i = 0;
   	for(i = 0; i < fileNum; i++){
   		filename_idx << fileName[i] << endl;
   	}

   	filename_idx.close();
}

// load filename to filename[] from filename.idx
void readFileIndex(string dirPath)
{
	ifstream in(dirPath + "/filename.idx");

	int index = 0;
	string line = "";

	while(getline(in, line)){
		fileName[index++] = line;
	}

	in.close();
}

// load words to positions map from words.idx
void readWordIndex(string dirPath)
{
	ifstream in(dirPath + "/words.idx");

	string line = "";
	string key = "";
	int spaceIndex = 0;
	int value = 0;
	while(getline(in, line)){
			
		spaceIndex = line.find(' ');
		key = line.substr(0, spaceIndex);
		value = stoi(line.substr(spaceIndex + 1));
		positions[key] = value;

	}
	in.close();
}

// search the postlist when there is one query term.
void initPostlist(string* records, string res, int count_at)
{
	int i = 0;
	int pos = 0;

	while((pos = res.find('@')) != string::npos){
		if(pos != 0){
			records[i++] = res.substr(0, pos);
		}
		res.erase(0, pos + 1); 
	}
	records[i++] = res;
	postlistMergeSort(records, 0, count_at - 1);
}

// rank the result based on their average frequency
void pageRank(string postLists[], int length)
{
	// @14#4@17#5@89#11 ==>    string[] = {14#4, 17#3, 89#11...}
	// merge sort by value =>  string[] = {89#11, 14#4, 17#3...}
	int i = 0;
	int count_at = 0;
	int fileIndex;

	// when there is only one query terms
	if(length == 1){

		for(i = 0; i < postLists[0].length(); i++){
			if(postLists[0].at(i) == '@')
				count_at++;
		}

		string* records = new string[count_at];
		initPostlist(records, postLists[0], count_at);

		for(i = 0; i < count_at; i++){
			fileIndex = stoi(records[i].substr(0, records[i].find('#')));
			cout << fileName[fileIndex] <<  endl;
		}
		delete [] records;

	}else{
		// multi query term search, and intersect all their result
		// and rank the intersection
		int j, len, index = 0;
		int min_at = 3000;
		int min_index = 0;
		bool contain = true;
		string pattern = "";
		string temp = "";
		string min_string = "";
		//*********************

		for(i = 0; i < length; i++){
			len = 0;
			for(j = 0; j < postLists[i].length(); j++){
				if(postLists[i].at(j) == '@'){
					len++;
				}
			}
			if(min_at > len){
				min_at = len;
				min_index = i;
			}
		}

		string* records = new string[min_at];
		min_string = postLists[min_index];

		i = 0;
		double value = 0;
		while(i < min_string.length()){
			if(min_string.at(i) == '@'){
				pattern += '@';
				i++;
				while(i < min_string.length() && min_string.at(i) != '#'){
					pattern += min_string.at(i);
					i++;
				}
				pattern += '#';
				i++;
				contain = true;
				value = 0;
				for(j = 0; j < length; j++){
					if(j == min_index)
						continue;
					int pos = postLists[j].find(pattern);
					if(pos == string::npos){
						contain = false;
						break;
					}
					temp = "";
					pos += pattern.length();
					while(pos < postLists[j].length() && postLists[j].at(pos) != '@'){
						temp += postLists[j].at(pos);
						pos++;
					}
					value += stod(temp);
				}

				if(contain){
					temp = "";
					while(i < min_string.length() && min_string.at(i) != '@'){
						temp += min_string.at(i);
						i++;
					}
					value = (value + stod(temp)) / length;
					records[index++] = pattern.substr(1) + to_string(value);
				}
				pattern = "";
			}else{
				i++;
			}
		}

		postlistMergeSort(records, 0, index - 1);

		for(i = 0; i < index; i++){
	
			fileIndex = stoi(records[i].substr(0, records[i].find('#')));
			cout << fileName[fileIndex] <<  endl;
		}

		delete []records;

		// no intersection
		if(index == 0){
			cout << endl;
		}
	}

}

// initialise search process
void initSearch (string index_folder_path, string params[], int len)
{
	int i, position = 0;
	string* records = new string[len];

	for(i = 0; i < len; i++){
		posIter = positions.find(params[i]);
		if(posIter != positions.end()){
			records[i] = search(index_folder_path, posIter->second);
		}else{
			cout << endl;
			return ;
		}
	}

	pageRank(records, len);

	delete []records;
}


int main(int argc, char const *argv[])
{
	DIR *d;
	FILE *fp;
	int ch = 0;
	int index = 0;
	bool firstFlag = false;
	char childPath[256];

	string token = "";
	string stemToken = "";

	// define threshold is 15M which means we update the postlist read every 15M of source file
	int num = 0;
	int threshold = 15 * 1024 * 1024;


    string index_folder_path(argv[2]);
	string whole_idx_path = "./" + index_folder_path;

    if(!(d = opendir(whole_idx_path.c_str()))){		
		mkdir(whole_idx_path.c_str() , 0777);
		firstFlag = true;
	}

	if(firstFlag){
		trave_dir(argv[1]);
		writeFileIndex(index_folder_path);

		// initialise index file: posting.idx, words.idx and filename.idx
	 	string posting_path = whole_idx_path + "/" + "posting.idx";
	    ofstream posting_idx;
	    posting_idx.open (posting_path, ofstream::out | ofstream::app);

	 	string words_path = whole_idx_path + "/" + "words.idx";
	    ofstream words_idx;
	    words_idx.open (words_path, ofstream::out | ofstream::app);

		initStopwords();

		// Parameters of skip list
		int skipIndex = 0;
		int skipCounter = 0;
		int skipLastPos = 0;
		string skipRecord = "";

		// traverse the filename which stored in the fileName[2000]
		for(index = 0; index < fileNum; index++){
			memset(childPath, 0, sizeof(childPath));
			sprintf(childPath, "%s/%s", argv[1], fileName[index].c_str());

			fp = fopen(childPath, "r");
		    while((ch = fgetc(fp)) != EOF){
		    	num++;

		    	if(isalpha(ch)){
					token += tolower(ch);
					continue;
		    	}

		    	if(token.length() > 2){
		    		if(!filter(token)){
						stemToken = stem_process(token);
						tempIter = tempMap.find(stemToken);
						if(tempIter == tempMap.end()){
							tempMap[stemToken] = 1;
						}else{
							tempMap[stemToken]++;
						}
		    		}
		    	}
		    	token = "";
			}

			for(tempIter = tempMap.begin(); tempIter != tempMap.end(); tempIter++){
				string key = tempIter->first;
				int value = tempIter->second;
				wordIter = words.find(key);

				if(wordIter == words.end()){
					words[key] = "@" + to_string(index) + "#" + to_string(value);
				}else{
					words[key] += "@" + to_string(index) + "#" + to_string(value);
				}
			}
			tempMap.clear();

			// if it reached the threshold, write the words map to postlist file
			// and clear word map waiting for the next 15M...
			// Note that I record the last postion of each term and append this position
			// at the end of postlist, detail information can be seen above...
			if(num >= threshold){
				for(wordIter = words.begin(); wordIter != words.end(); wordIter++){
					skipRecord = "";
					posIter = positions.find(wordIter->first);

					// update the last position of the previous postlist
					if(posIter == positions.end()){
						skipRecord = wordIter->second + "$" + to_string(-1) + "$";
						positions[wordIter->first] = skipLastPos;
						skipLastPos += skipRecord.length();
					}else{
						skipRecord = wordIter->second + "$" + to_string(posIter->second) + "$";
						positions[wordIter->first] = skipLastPos;
						skipLastPos += skipRecord.length();
					}
					posting_idx << skipRecord;

				}
				words.clear();
				num = 0;
			}
			fclose(fp);
		}
		// for the remaining terms of the word map
		// update word map to postlist index file
		for(wordIter = words.begin(); wordIter != words.end(); wordIter++){
			skipRecord = "";
			posIter = positions.find(wordIter->first);
			if(posIter == positions.end()){
				skipRecord = wordIter->second + "$" + to_string(-1) + "$";
				positions[wordIter->first] = skipLastPos;
				skipLastPos += skipRecord.length();
			}else{
				skipRecord = wordIter->second + "$" + to_string(posIter->second) + "$";
				positions[wordIter->first] = skipLastPos;
				skipLastPos += skipRecord.length();
			}
			posting_idx << skipRecord;
		}

		// Finally write the postition of postlist of each term into words index file.
		for(posIter = positions.begin(); posIter != positions.end(); posIter++){
			words_idx << posIter->first << " " << posIter->second << endl;
		} 

		// close postlist and words index file.
		posting_idx.close();
		words_idx.close();

		/////////////////////// Perform Normal Search Part Frist Time /////////////////////// 
		// step1: collect and initialise search term
		// step2: searching...
		string option(argv[3]);
		if(option != "-c"){

			int i;
			string stemToken = "";
			int paramNum = argc - 3;

			string* patterns = new string[paramNum];

			for(i = 0; i < paramNum; i++){
				string token(argv[i + 3]);
				transform(token.begin(), token.end(), token.begin(),::tolower);
				stemToken = stem_process(token);
				patterns[i] = stemToken;
			}

			initSearch(index_folder_path, patterns, paramNum);
			delete [] patterns;
		}else{
			/////////////////////// Perform Concept Search at first time /////////////////////// 
			// cout << "perform concept search at first time..." << endl;
			// cout << "argc: " << argc << endl;

			string cNum(argv[4]);

			int i;
			string stemToken = "";
			int paramNum = argc - 5;

			string* patterns = new string[paramNum];

			for(i = 0; i < paramNum; i++){
				string token(argv[i + 5]);
				transform(token.begin(), token.end(), token.begin(),::tolower);
				stemToken = stem_process(token);
				patterns[i] = stemToken;
			}

			initSearch(index_folder_path, patterns, paramNum);
			delete [] patterns;

		}
	}else{
		/////////////////////// Next Time Search /////////////////////// 
		// step1: load filename index;
		// step2: load words index;
		// step3: collect and initialise search term
		// step4: searching...

		string option(argv[3]);

		// step1 and step2: load filename and words
 		readWordIndex(index_folder_path);
 		readFileIndex(index_folder_path);

		if(option != "-c"){

	 		// step3: collect and initialise search term
			int i;
			string stemToken = "";
			int paramNum = argc - 3;
			string* patterns = new string[paramNum];

			for(i = 0; i < paramNum; i++){
				string token(argv[i + 3]);
				transform(token.begin(), token.end(), token.begin(),::tolower);
				stemToken = stem_process(token);
				patterns[i] = stemToken;
			}

			// step4: searching...
			initSearch(index_folder_path, patterns, paramNum);
			delete [] patterns;

		}else{
			/////////////////////// Perform Concept Search at next time /////////////////////// 
			// cout << "perform concept search at next time..." << endl;
			string cNum(argv[4]);

			int i;
			string stemToken = "";

			// cout << "argc: " << argc << endl;
			int paramNum = argc - 5;

			string* patterns = new string[paramNum];

			for(i = 0; i < paramNum; i++){
				string token(argv[i + 5]);
				transform(token.begin(), token.end(), token.begin(),::tolower);
				stemToken = stem_process(token);
				patterns[i] = stemToken;
			}

			initSearch(index_folder_path, patterns, paramNum);
			delete [] patterns;
		}
	}

	return 0;
}