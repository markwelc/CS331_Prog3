#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

using namespace std;

void preprocessor(string fileInName, string fileOutName);
void classifier();
void cleanLine(vector<char> & line);

int main()
{

    preprocessor("trainingSet.txt", "preprocessed_train.txt");
    //preprocessor("testSet.txt", "preprocessed_test.txt");

    classifier();

    return 0;
}



void preprocessor(string fileInName, string fileOutName)
{
    ifstream fileIn;
    ofstream fileOut;
    string curLine;

    //I don't know why the words here aren't colored
    vector<string> vocab; //so this holds all the words
    int* curPrepro = NULL;
    // vector<string> allLines;
    vector<vector<string>> allLinesSplit;

    //this might be opening them prematurely, I don't know. It shouldn't matter though
    fileIn.open(fileInName);
    fileOut.open(fileOutName);

    char *start = NULL, *end = NULL;

    //for every line in the input file
    while(getline(fileIn, curLine))
    {
        vector<char> curLineV(curLine.begin(), curLine.end()); //turn it into a vector
        cleanLine(curLineV);//clean it up

        //put the current line in the corresponding all lines vector
        // string temp(curLineV.begin(), curLineV.end());
        // if(!temp.empty())
        //     allLines.push_back(temp);

        //as we put words into vocab, we're also going to put them into allLinesSplit
        vector<string> curLineSplit;

        //for every word in curLineV, put that word into vocab if it is not already there
        for (auto iter = curLineV.begin(); iter != curLineV.end(); ++iter)
        {
            if(!start && isalnum(*iter))
                start = &(*iter);
            if(start && !isalnum(*iter))
                end = &(*iter);

            if(start && end)
            {
                string temp2(start, end);
                auto kiter = vocab.begin();
                while(kiter != vocab.end() && temp2.compare(*kiter) > 0)
                    kiter++;
                if(kiter == vocab.end())
                    vocab.push_back(temp2);
                else if(temp2.compare(*kiter) != 0)
                    vocab.emplace(kiter, temp2);

                curLineSplit.push_back(temp2);

                start = NULL;
                end = NULL;
            }
        }
        
        allLinesSplit.push_back(curLineSplit);

        curLineV.clear();
    }

    //at this point, vocab should contain every word and allLines should contain every line

    //print the vocab into the output files
    auto iter = vocab.begin();
    fileOut << *iter;
    while(iter != vocab.end()) 
    {
        fileOut << ',' << *iter;
        iter++;
    }
    fileOut << endl;

    //actually make the preprocessed stuff
    curPrepro = new int[vocab.size() + 1];
    for(unsigned int i = 0; i < vocab.size() + 1; i++)
        curPrepro[i] = 0;
    //for every word in allLines
    //cout << "I'd be very surprised if this didn't show up" << endl;
    for(auto iter = allLinesSplit.begin(); iter != allLinesSplit.end(); iter++)
    {
        //cout << "double boo" << endl;
        for(auto kiter = iter->begin(); kiter != iter->end(); kiter++)
        {
            //cout << "boo" << endl;
            //find a match in vocab
            for(auto jiter = vocab.begin(); jiter != vocab.end(); jiter++)
            {
                if(kiter->compare(*jiter) == 0)
                {
                    curPrepro[jiter - vocab.begin()] = 1;
                    //cout <<  *kiter << " == " << *jiter << endl;
                }
                // else
                // {
                //     cout << *kiter << " != " << *jiter << endl;
                // }
                
            }
        }
        //stick it into the output file
        fileOut << curPrepro[0];
        for(unsigned int i = 1; i < vocab.size() + 1; i++)
            fileOut << ',' << curPrepro[i];
        fileOut << endl;
    }
}



void classifier()
{
    //stuff
    return;
}



void cleanLine(vector<char> & line)
{
    auto iPtr = line.begin();
    int i = 0;
    while(iPtr != line.end())
    {
        line.at(i) = tolower(*iPtr);
        if(iswspace(*iPtr))
            line.at(i) = ' ';//change all whitespaces to spaces
        if(isalnum(*iPtr) || isspace(*iPtr))//if the character isn't an alphanumeric character or a whitspace
        {
            iPtr++;
            i++;
        }
        else
            line.erase(iPtr);
    }
}
