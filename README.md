# TinySearchEngine 

## BACKGROUND  

### Specs 

-Developed on MacOS Catalina. 
Note: an experimental Valgrind (doesn't come with Mac and group has to constantly race to update with each OS version) was used for the indexer and querier sections to clean up memory 
-Gumbo 3rd-party HTML5 Parser was used. To install, please go to https://github.com/google/gumbo-parser
-BeautifulSoup4 for Python text extractin. To install, "pip install beautifulsoup4" 

### Motivation  
Information on the project was sourced from here: https://www.cs.dartmouth.edu/~campbell/cs50/assignments.html. The course is taught by Professor Andrew T. Campbell. There is no affiliation between me and Dartmouth and all information was publicly available. Motivation behind this project was practicing developing in C and googling around for questions related to C kept coming to this link so I decided to try out the class's TinySearchEngine project (Labs 3-6). Because this was not done in a formal setting, I took some liberties in certain aspects of the assignments. 
1. In the Indexer I utilized Python for extracting text instead of using C. This was mostly a preference because I was already familiar with BeautifulSoup and to my knowledge C doesn't have as dedicated a parser as Python. 
2. The HTML-parser for the crawler the class provided is broken (assignments were posted in 2011) so I used a third-party package to get all the <href> tags for the crawler to then search.
3. I decided to change the SEED (starting url for crawler to begin) to JHU instead of Dartmouth 

### Search Engine 

The project revolves around developing the core parts of a search engine: Crawler, Indexer, and Querier. These are also the main directories within TinySearchEngine. Much of the background behind the TSE was sourced from the 2001 Stanford paper, "Searching the Web" (https://www.cs.dartmouth.edu/~campbell/cs50/searchingtheweb.pdf) 

**Crawler** -
a program that goes through the Web via links embedded in each web page. These web page's URLs are stored along with their depth level. The crawler starts its search with a SEED URL, downloads the associated page and begins extracting/moving along. Because of the vastness of the Web, there is no way to extract all the links. At some point, we have to stop. This is reflected in the program by artificially setting a depth limit of 3 meaning if I go from www.jhu.edu to FIRST\_URL and I go back to download FIRST\_URL, I'm at depth
1\. Going from the downloaded page at FIRST\_URL to SECOND\_URL will be depth 2 and so on. 
    -Note: Most of the documentation in these crawler files are already written by the Professor.

**Indexer** -
once we collected all our URLS that our crawler extracted and we also extract all the relevant text from these HTML pages that our crawler also saved for us. we now need to create an inverted index which consists of inverted lists of words that detail the locations (in this case, the text files) where the word appears. This data structure is then saved as a .dat file. As a functional test, we then recreate our index again from the .dat file, create another .dat file from this recreated index and compare it with the original. 

**Querier** - 
We now have a .dat file detailing our word locations. We can now make a rudimentary query engine for the user. The ranking algorithm is simple; it ranks by the # of times the word appears. (E.g. the  "without" appears in docId 49 4 times and docId 51 9 times. The querier will order the documents as 51 then 49). There are two cases the querier handles: AND and OR. The AND case is the search that we are most familiar with when we look up word conjunctions. When we look up "computer science" we expect text files that talk about computer science and not about any other science like "biology". This is an example of the AND case which can also be demonstrated as "computer AND science". The OR case is where we're interested in all articles that have either "computer" or "science" in the text files. To get the OR case, write your query with an "or" (e.g. "computer or science"). 

## USAGE 

The files are already created with jhu.edu as the seed. To run the querier, please go to "querier/bin" and run "./querier final\_index.dat". To build from scratch, run "build\_run\_.sh". 

## Shortcomings 

1. The crawler is not perfect. The amount of URLS that were searched was far lower than I anticipated as even a depth limit of 3 didn't really matter. I'm guessing that the Gumbo parser couldn't find all the href links. Even trying it on wikipedia.org doesn't generate a vast amount of URLs. As such, the crawler only generates a fraction of the anticipated amount of files. This will be a downstream impact to the indexer and querier as not enough information is generated. 













