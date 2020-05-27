#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <bits/stdc++.h>
#include <math.h>

using namespace std;

struct vocabdata
{
    string word;

    float grev;
    float brev;

    float pgrev;     //probability of good review
    float pbrev;      //probability of bad review

    vocabdata()
    {
        word = "default***"; //lets hope we never see this again
        grev = 0.0;
        brev = 0.0;

        pgrev = 0.0;
        pbrev = 0.0;
    }
};

 //stores data for accuracy of the actual data set


int** preprocessor(string fileInName, string fileOutName, vector<vocabdata> & vocab, int & arrsize, bool vocabMutable);
void training(vector<vocabdata> & vocab, int** mapmatrix, int arrsize);
float classifier(vector<vocabdata> & vocab, int** mapmatrix, int arrsiz);
void cleanLine(vector<char> & line);
int readFile(string fileInName, vector<vocabdata> & vocab, bool vocabMutable, vector<vector<string>> & allLinesSplit);
// returns the number of lines read
// if vocabMutable is true, it will add every word in the file to vocab
// after running, allLinesSplit will be a 2D vector of strings where each row is a full review, and each cell is a word in that review
    // the last cell in each row shows whether the review is positive or negative
void addToVocab(string newWord, string rating, vector<vocabdata> & vocab, vector<vocabdata>::iterator position);

int main()
{
    vector<vocabdata> vocab;
    bool vocabMutable = true;
    //vector<vocabdata> dummy; //dummy vector for sending to ones that dont matter(not for training)
    
    string resultdata;  //string to hold result data
    float resultfloat;
    
    //dummy.clear(); //to clear dummy vector

    int arrsize = 0;//will hold the number of reviews
    
    //training data
    int** mapmatrixtrain = preprocessor("trainingSet.txt", "preprocessed_train.txt",vocab,arrsize, vocabMutable);
    vocabMutable = false;
    training(vocab,mapmatrixtrain,arrsize);

    cout << "\nTesting on Training Data" << endl; 

    resultdata += "Training Data set Accuracy:\t";
    resultfloat = classifier(vocab,mapmatrixtrain,arrsize);
    resultdata += to_string(resultfloat);
    resultdata += "\n\n";


    //test data
    arrsize = 0;
    int** mapmatrix = preprocessor("testSet.txt", "preprocessed_test.txt",vocab,arrsize, vocabMutable);

    cout << "\nTesting on Test Data" << endl; 

    resultdata += "Testing Data set Accuracy:\t";
    resultfloat = classifier(vocab,mapmatrix,arrsize);
    resultdata += to_string(resultfloat);

    ofstream resultfileOut;
    resultfileOut.open("result.txt");
    resultfileOut << resultdata << endl;
    return 0;
}




int** preprocessor(string fileInName, string fileOutName, vector<vocabdata> & vocab, int & arrsize, bool vocabMutable)
{
    int numlines = 0; //keepting track for curPrepro

    int** curPrepro = NULL;
    vector<vector<string>> allLinesSplit;

    ofstream fileOut;
    fileOut.open(fileOutName);

    numlines = readFile(fileInName, vocab, vocabMutable, allLinesSplit);

    //at this point, vocab should contain every word and allLines should contain every line
    //print the vocab into the output files
    auto hiter = vocab.begin();
    fileOut << hiter->word;
    hiter++;
    while(hiter != vocab.end()) 
    {
        fileOut << ',' << hiter->word;
        hiter++;
    }
    fileOut << ",classLabel" << endl;

    //actually make the preprocessed stuff
    curPrepro = new int*[numlines];
    for(int i = 0; i < numlines; i++)
        curPrepro[i] = new int[vocab.size() + 1];
    for(int j = 0; j < numlines; j++){
        for(unsigned int i = 0; i < vocab.size() + 1; i++)
            curPrepro[j][i] = 0;
    }
    //for every word in allLines
    int curReview = 0; //counter for line we're on
    for(auto iter = allLinesSplit.begin(); iter != allLinesSplit.end(); iter++)
    {
        if(curReview > numlines - 1)
            break; //safety in case something gets messed up
        
        for(auto kiter = iter->begin(); kiter != iter->end() - 1; kiter++)
        {
            //find a match in vocab
            for(auto jiter = vocab.begin(); jiter != vocab.end(); jiter++)//I could make this loop quite a bit faster, but I don't care
            {
                if(kiter->compare(jiter->word) == 0)
                    curPrepro[curReview][jiter - vocab.begin()] = 1;
            }
        }

        //at this point, we've gone through every word in a line save the last thing (the rating)

        if(*(iter->end() - 1) == "0")
            curPrepro[curReview][vocab.size()] = 0;//get the rating and stick it onto curPrepro
        else if(*(iter->end() - 1) == "1")
            curPrepro[curReview][vocab.size()] = 1;
        else
            cout << "iter->end() - 1 = " << *(iter->end() - 1) << endl;

        //stick it into the output file
        fileOut << curPrepro[curReview][0];
        for(unsigned int i = 1; i < vocab.size() + 1; i++)
            fileOut << ',' << curPrepro[curReview][i];
        fileOut << endl;

        curReview ++;
    }

    cout << "\nPreproccesor Complete\n" << endl;

    arrsize = 0;
    arrsize = numlines;
    return curPrepro;
}

void training(vector<vocabdata> & vocab, int** mapmatrix, int arrsize)
//finds P(review = good)
//for each word, finds P(word is in good review) and P(word is in bad review)
{   
    //insert an extra vocab on to hold data for parent review percents
    vocabdata* temp = new vocabdata();
                    
    vocab.push_back(*temp);
    (vocab.end() - 1)->word = "**";

    //to find the percent that a review is good or bad
    float numberg = 0.0;
    float numberb = 0.0;
    float tnumber = 0.0;//total number of times the word is used

    for(int i = 0; i < arrsize; i++){
        if(mapmatrix[i][vocab.size()-1] == 1)
            numberg += 1.0;
        else
            numberb += 1.0;

        tnumber++; //could do different way but yolo
    }

    //finds percentages for how often a word results in a good or bad review
    for(unsigned int i = 0; i < vocab.size(); i++)
    {
        vocab[i].pgrev = (float)((vocab[i].grev + 1) / (numberg + 2));//calculates the probability that the word appears in a good review
        vocab[i].pbrev = (float)((vocab[i].brev + 1) / (numberb + 2));//calculates the probability that the word appears in a bad review
    }
    
    //calculate probability that sentence is g or b 
    (vocab.end() - 1)->pgrev = (float)((numberg) / (tnumber));//this is the number of good sentences divided by number of sentences
    (vocab.end() - 1)->pbrev = (float)((numberb) / (tnumber));

    cout << "\nTraining Done" << endl;
}

//send it curPrero to find which words are in sentences
float classifier(vector<vocabdata> & vocab, int** mapmatrix, int arrsize)
{
    vector<float> spredics;//this contains the percent for if classifier thinks sentence is good or bad
    float accuracyofclassifier = 0;//contains the overall accuracy

    int numcorrect = 0;

    for(int i = 0; i < arrsize; i++){
        //starting values, equal the P(good) and P(bad)
        float curgpredict = log10((vocab.end() - 1)->pgrev);
        float curbpredict = log10((vocab.end() - 1)->pbrev);

        //minus 2 because of the extra vocab word added
        for(unsigned int j = 0; j<vocab.size()-2; j++){
            if(mapmatrix[i][j] == 1)
            {
                //gets vocab word a present location and checks probailities and multiplies them on
                //need to compute as log(calculation)
                curgpredict += log10(vocab[j].pgrev);//P(word = present | review = good)
                curbpredict += log10(vocab[j].pbrev);
            }
            else
            {
                curgpredict += log10(1 - vocab[j].pgrev);//P(word = absent | review = good)
                curbpredict += log10(1 - vocab[j].pbrev);
            }
        }

        //decides if sentence is good or bad off which percent is higher
        if(curgpredict > curbpredict)
            spredics.push_back(1);
        else
            spredics.push_back(0);

        if(mapmatrix[i][vocab.size()-1] == spredics[spredics.size()-1])
            numcorrect += 1;
    }

    cout << "\nClassifier is Done" << endl;

    //accuracy of classifier
    accuracyofclassifier = numcorrect / ((float)(spredics.size()));

    cout << "Classifier Accuracy: " << accuracyofclassifier << endl;

    return accuracyofclassifier;
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
        if(isalpha(*iPtr) || isspace(*iPtr))//if the character isn't an alphanumeric character or a whitspace
        {
            iPtr++;
            i++;
        }
        else
            line.erase(iPtr);
    }
}



int readFile(string fileInName, vector<vocabdata> & vocab, bool vocabMutable, vector<vector<string>> & allLinesSplit)
{
    //these are for literally reading from the file
    ifstream fileIn;
    int numlines = 0;
    string curLine;

    //these are more temporary things
    string curRatingStr = "";
    char *start = NULL, *end = NULL;

    fileIn.open(fileInName);

    //for every line in the input file
    while(getline(fileIn, curLine))//get a review
    {
        vector<char> curLineV(curLine.begin(), curLine.end()); //turn it into a vector of characters

        //get the reviews rating
        for (auto iter = curLineV.end(); iter != curLineV.begin(); --iter)//this is an absurd solution, but it works
        {
            if(*iter == '1')
            {
                curRatingStr = "1";
                break;
            }
            else if(*iter == '0')
            {
                curRatingStr = "0";
                break;
            }
        }

        cleanLine(curLineV);//clean it up

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
                string curWordInReview(start, end);

                //look through vocab to see if/where the word is
                auto kiter = vocab.begin();
                while(kiter != vocab.end() && curWordInReview.compare(kiter->word) > 0)
                {
                    kiter++;
                }

                if(vocabMutable)//dont want to add words not in training set to vocab
                {
                    addToVocab(curWordInReview, curRatingStr, vocab, kiter);
                    curLineSplit.push_back(curWordInReview);
                }
                else if(curWordInReview.compare(kiter->word) == 0) //only adds word that are in vocab to the linesplit
                        curLineSplit.push_back(curWordInReview);
                
                start = NULL;
                end = NULL;
            }

        }

        curLineSplit.push_back(curRatingStr);//now, although every other number is gone, the rating is still there
        allLinesSplit.push_back(curLineSplit);

        curLineV.clear();
        curRatingStr = "";

        numlines ++; //keeping track of number of lines
    }

    return numlines;
}



void addToVocab(string newWord, string rating, vector<vocabdata> & vocab, vector<vocabdata>::iterator position)
{
    if(position == vocab.end())//new word
    {
        vocabdata* temp = new vocabdata();

        vocab.push_back(*temp);
        (vocab.end() - 1)->word = newWord;
        if(rating == "1")
        {    
            (vocab.end() - 1)->grev = 1.0;
            (vocab.end() - 1)->brev = 0.0;
        } 
        else
        {
            (vocab.end() - 1)->grev = 0.0;
            (vocab.end() - 1)->brev = 1.0;
        }
    }
    else if(newWord.compare(position->word) != 0)//new word
    {   
        //we've found where the word should go
        vocabdata* VocabDataEl = new vocabdata();
        VocabDataEl->word = newWord;
                                //apperantly you cant set after using the kiter->data, doesnt work all the time?
        if(rating == "1") //set its rating.
        {
            VocabDataEl->grev = 1.0;
            VocabDataEl->brev = 0.0; 
        }
        else
        {
            VocabDataEl->grev = 0.0; 
            VocabDataEl->brev = 1.0; 
        }

        vocab.emplace(position, *VocabDataEl); //put an empty thing there
    }
    else //existing word
    {
        if(rating == "1")
            position->grev += 1.0;
        else
            position->brev += 1.0;
    }
}