#!/usr/bin/env python3

from bs4 import BeautifulSoup
import sys 
import subprocess 

'''
File: extract.py 
Input: url filename 
Output: a text file in "texts/" corresponding to the URL. The text will come from anything enclosed within <p> tags. 

NOTE: This is not a perfect method to extract text because some helpful text is not taken and in the case with running JHU as the seed url,
there can be some garbage text that is still captured (e.g. "You are looking at a modal window") 
'''

## pass in the filename argument and use bs4 to get text from p tags 
def extract(filename):
    tot = [] 
    try: 
        with open(filename, "r") as reader:
            tot = [line.rstrip('\n') for line in reader] 
            tot_str = " ".join(tot)
            soup = BeautifulSoup(tot_str, "html.parser")
            texts = soup.findAll("p")
            texts = [text.get_text(" ").strip().replace(",", "") for text in texts if text] 
            return " ".join(texts)
    except FileNotFoundError as f:
        raise SystemExit(f"{filename} is not found in target directory")


def write_to(extract:str, filename_arg):
    fn = f"text_{filename_arg}"
    with open(fn, "w") as writer:
        writer.write(extract) 
    subprocess.call(f"mv {fn} texts", shell=True)


## driver for reading in file argument to be parsed with extract(). 
## driver for writing 
def main():
    try:
        arg = sys.argv[1] 
    except IndexError:
        raise SystemExit("Need filename to read\n")
    str_ext = extract(arg) 
    write_to(str_ext, arg[5:]) 

if __name__ == "__main__": 
    main()

    
