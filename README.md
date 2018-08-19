# BasicSearchEngine

This is the final project of comp9319[Web Data Compression and Search].

In order to meet the limitation of memory and time, I'd like to share my idea first.

Requirement:
      Memory limit: 32M[contain heap]
      Time limit: 5s
      SrcFile size: 200M+
      #SrcFile: 2000+
      Support multi-keywords search(max 5)
      Support PageRank
      
This is the third assignment of comp9319, construct a basic search engine within memory and time limit.
In general, I use skiplist to store the inverted index and use last postition to retrival

======The basic structure of the search engine:
		
	== a3search.cpp
	== makefile
	== stmr.c (external lib)
	== stmr.h (external lib)
	== tool.cpp (some sort funtion, explain below)
	== tool.h
	== words.tar (include stopwords.txt)
	== raw_data_legal_xml.zip (raw data of xml for index)
	== raw_data_man.zip (raw data of man doc in linux for index)
	== external_sort (a demo of external merge sort)
	== README.txt
  
======Main idea of search engine:

	1. First I need three index file, filename.idx, posting.idx, words.idx
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

	3. Core design:

		======Construct index:

				I use two map to record postlist info and position info.

				map<string, string> words 	==>> {apple: @0#3@1#2@3#2@4#4@5#5...., book: @1#2@3#3....}, record postlist info
				map<string, int> positions 	==>> {apple: 10, book:20} 									record position info

				Note that, in order to meed the memory limit, I write the words map to posting.idx every read 15M source file, and 
				then clear words map waiting for the next process. And position map is always maintain in the memory and write position
				informatin into words.idx before the program end.

				I have tested that, the position map would occupy 17M memroy compared with limit 32M, so I think it's reasonable to maintain position map in memory.

		======Search :

				1. Stem and initialise the query terms
				2. Load words.idx to position map, load filename.idx to string[].
				3. Find the postion of each search term based on position map, and then
					use this position to retrive the postlist by using fseek.
				4. Rank and output the result.

	4. Core function:

		1.==>>>string search(string dirPath, int position);  < search function >
					mainly use to retrive postlist info based on position

				like: search("path_posting.idx", "100")
						return return: @0#3@1#2@3#2@4#3@5#2@6#2@10#12.....@100#100.

		2.==>>>void pageRank(string postLists[], int length); < pageRank function>
					mainly use to get intersect of all the postlist and compute avg rank

				postingList = {@115#2@124#1@127#1@138#1@187#1@155#1@157#10@159#7@160#7@176#1@121#21@21#21",
								"@14#4@17#5@89#11@92#1@111#11@112#1@113#1@115#2@124#1@127#1@187#1",
								"@127#2@160#7@176#1@186#1@187#1@197#4@200#1@203#1@205#2@124#2@212#2@253#2"}

				like: pageRank(postList, 3).

						return 124#1.333333    Note that format is <fileNameId#AvgRank>
							   127#1.333333
							   187#1.000000

======Explain other component:

	== a3search.cpp	
			search engine core code

	== makefile
			decompress words.tar
			and compile stmr and tool

	== stmr.c (external lib)
	== stmr.h (external lib)
			external stemming libray

	== tool.cpp (some sort funtion, explain below)
	== tool.h
			tow main sort function(merge sort stable):
			1. Filename merge sort
			2. Postlist merget sort

	== words.tar (include stopwords.txt)
			stopwords.txt

	== README.txt
			outline the whole project
