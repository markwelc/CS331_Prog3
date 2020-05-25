#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

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
        grev = 0;
        brev = 0;
        pgrev = 0;
        pbrev = 0;
    }
};

float accuracyofclassifier = 0; //stores data for accuracy of the actual data set


int** preprocessor(string fileInName, string fileOutName, vector<vocabdata> & vocab, int & arrsize);
void training(vector<vocabdata> & vocab);
void classifier(vector<vocabdata> & vocab, int** mapmatrix, int arrsiz);
void cleanLine(vector<char> & line);

int main()
{
    vector<vocabdata> vocab;
    vector<vocabdata> dummy; //dummy vector for sending to ones that dont matter(not for training)
    dummy.clear(); //to clear dummy vector

    // preprocessor("trainingSet.txt", "preprocessed_train.txt");
    // preprocessor("testSet.txt", "preprocessed_test.txt");

    int arrsize = 0;
    //int** mapmatrix = preprocessor("exampleIn.txt", "exampleOut.txt",vocab,arrsize);
    
    preprocessor("trainingSet.txt", "preprocessed_train.txt",vocab,arrsize);
    training(vocab);

    arrsize = 0;
    int** mapmatrix = preprocessor("testSet.txt", "preprocessed_test.txt",vocab,arrsize);

    classifier(vocab,mapmatrix,arrsize);

    //cout << vdata[7].vocab << " " << vdata[7].pgrev << " " << vdata[7].pbrev << endl;

    return 0;
}



int** preprocessor(string fileInName, string fileOutName, vector<vocabdata> & vocab, int & arrsize)
{
    int numlines = 0; //keepting track for curPrepro

    ifstream fileIn;
    ofstream fileOut;
    string curLine;

    //I don't know why the words here aren't colored
    //vector<vocabdata> vocab; //so this holds all the words and data
    int** curPrepro = NULL;
    vector<vector<string>> allLinesSplit;
    //char curRating = '\0'; //holds the rating of the current line
    string curRatingStr = "";

    //this might be opening them prematurely, I don't know. It shouldn't matter though
    fileIn.open(fileInName);
    fileOut.open(fileOutName);

    char *start = NULL, *end = NULL;

    //for every line in the input file
    while(getline(fileIn, curLine))
    {
        vector<char> curLineV(curLine.begin(), curLine.end()); //turn it into a vector

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
                //auto newitr = curLineV.end();
                //end = &(*newitr);
                string temp2(start, end);

                //incrementing if already in vocab
                int it = -1;
                for(int z = 0; z < vocab.size(); z++)//so this just looks for the first match
                {
                    if(vocab[z].word == temp2){
                        it = z;
                        break;
                    }
                }

                if(it != -1) //if there was a match, increment the num of times it was in a good/bad review
                {
                    if(curRatingStr == "1")
                        vocab[it].grev += 1.0;
                    else
                        vocab[it].brev += 1.0;
                }

                //initilize in vocab
                auto kiter = vocab.begin();
                while(kiter != vocab.end() && temp2.compare(kiter->word) > 0)
                {
                    kiter++;
                }
                if(kiter == vocab.end())
                {
                    vocabdata* temp = new vocabdata();//it wasn't a pointer...
                    
                    //cout << "temp->word = " << temp->word << endl;
                    //temp->word = temp2;

                    vocab.push_back(*temp);
                    (vocab.end() - 1)->word = temp2;
                    if(curRatingStr == "1")//this is the only review with this word
                    {    
                        (vocab.end() - 1)->grev = 1.0;
                        (vocab.end() - 1)->brev = 0.0;
                    } 
                    else
                    {
                        (vocab.end() - 1)->grev = 0.0;
                        (vocab.end() - 1)->brev = 1.0;
                    }
                    //cout << " test1 " << vocab[0].word << endl;
                }
                else if(temp2.compare(kiter->word) != 0)
                {   
                    //we've found where the word should go
                    vocabdata* VocabDataEl = new vocabdata();
                    VocabDataEl->word = temp2;
                                            //apperantly you cant set after using the kiter->data, doesnt work all the time?
                    if(curRatingStr == "1") //set its rating. This is the only review with this word
                    {
                        VocabDataEl->grev = 1.0;
                        VocabDataEl->brev = 0.0; 
                    }
                    else
                    {
                        VocabDataEl->grev = 0.0; 
                        VocabDataEl->brev = 1.0; 
                    }

                    vocab.emplace(kiter, *VocabDataEl); //put an empty thing there
                    
                    //cout << "kiter->word = " <<  temp2 << endl;
                    //kiter->word = temp2; //set its word
                    
                }

                curLineSplit.push_back(temp2);
                start = NULL;
                end = NULL;
            }
        }

        curLineSplit.push_back(curRatingStr);//now, although every other number is gone, the rating is still there
        curRatingStr = "";
        allLinesSplit.push_back(curLineSplit);

        curLineV.clear();

        numlines ++; //keeping track of number of lines

        //this stuff just printing out for debugging?
        /*for (int x = 0; x<vocab.size(); ++x)
            cout << vocab[x].word << " " <<  vocab[x].brev << " " <<  vocab[x].grev << endl;//*/
        /*for (int x = 0; x<vocab.size(); ++x)
            cout << vocab[x] << endl;//*/
        //cout << "actual vocab size: " << vocab.size() << "\tnew vector size: " << vdata.size() << endl;//*/
    }

    //this stuff just printing out for debugging?
    /*for (int x = 0; x<vocab.size(); ++x)
        cout << vocab[x].word << " " <<  vocab[x].brev << " " <<  vocab[x].grev << endl;//*/

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
    int zx = 0; //counter for line we on
    for(auto iter = allLinesSplit.begin(); iter != allLinesSplit.end(); iter++)
    {
        if(zx > numlines - 1)
            break; //safety in case something gets messed up
        
        for(auto kiter = iter->begin(); kiter != iter->end() - 1; kiter++)
        {
            //find a match in vocab
            for(auto jiter = vocab.begin(); jiter != vocab.end(); jiter++)//I could make this loop quite a bit faster, but I don't care
            {
                if(kiter->compare(jiter->word) == 0)
                    curPrepro[zx][jiter - vocab.begin()] = 1;
            }
        }

        //at this point, we've gone through every word in a line save the last thing (the rating)

        if(*(iter->end() - 1) == "0")
            curPrepro[zx][vocab.size()] = 0;//get the rating and stick it onto curPrepro
        else if(*(iter->end() - 1) == "1")
            curPrepro[zx][vocab.size()] = 1;
        else
            cout << "iter->end() - 1 = " << *(iter->end() - 1) << endl;

        //stick it into the output file
        fileOut << curPrepro[zx][0];
        for(unsigned int i = 1; i < vocab.size() + 1; i++)
            fileOut << ',' << curPrepro[zx][i];
        fileOut << endl;

        //reset curPrepro
        //for(unsigned int i = 0; i < vocab.size(); i++)
        //    curPrepro[i] = 0;
        zx ++;
    }

    cout << "Preproccesor Complete" << endl;

    arrsize = numlines;
    return curPrepro;
}

void training(vector<vocabdata> & vocab)
{   
    //insert an extra vocab on to hold data for parent review percents
    vocabdata* temp = new vocabdata();//it wasn't a pointer...
                    
    //cout << "temp->word = " << temp->word << endl;
    //temp->word = temp2;
                    
    vocab.push_back(*temp);
    (vocab.end() - 1)->word = " ";

    //vdata[0].vocab = " ";

    //to find the percent that a review is good or bad
    float numberg = 0.0;
    float numberb = 0.0;
    float tnumber = 0.0;

    //finds percentages for how often a word results in a good or bad review
    for(int i = 0; i < vocab.size(); i++){
        //Keeps track of good and bad words for P(good) and P(bad) calculation
        tnumber += vocab[i].grev + vocab[i].brev;
        numberg += vocab[i].grev;
        numberb += vocab[i].brev;

        //each word is independent, this finds the Probabilities?
        vocab[i].pgrev = (float)((vocab[i].grev) / (vocab[i].grev + vocab[i].brev));
        vocab[i].pbrev = (float)((vocab[i].brev) / (vocab[i].grev + vocab[i].brev));
        //cout << (vocab[i].grev + vocab[i].brev) << endl;
    }
    
    //calculate review percents
    (vocab.end() - 1)->pgrev = (float)((numberg) / (tnumber));
    (vocab.end() - 1)->pbrev = (float)((numberb) / (tnumber));

    //for testing
    //cout << "good prediction: " << (vocab.end() - 1)->pgrev << "\tbad prediction: " << (vocab.end() - 1)->pbrev << endl;
    cout << "\nTraining Done" << endl;
}

//send it curPrero to find which words are in sentences
void classifier(vector<vocabdata> & vocab, int** mapmatrix, int arrsize)
{
    //this contains the percent for if classifier thinks sentence is good or bad
    vector<float> spredics;

    int numcorrect = 0;

    for(int i = 0; i < arrsize; i++){
        //starting values, equal the P(good) and P(bad)
        float curgpredict = (vocab.end() - 1)->pgrev;
        float curbpredict = (vocab.end() - 1)->pbrev;

        //minus 2 because of the extra vocab word added
        for(int j = 0; j<vocab.size()-2; j++){
            if(mapmatrix[i][j] == 1)
            {
                //gets vocab word a present location and checks probailities and multiplies them on
                curgpredict = curgpredict * vocab[j].pgrev;
                curbpredict = curbpredict * vocab[j].pbrev;
            }
        }

        //multiplies on total prediction
        curgpredict = curgpredict * vocab[vocab.size()-1].pgrev;
        curbpredict = curbpredict * vocab[vocab.size()-1].pbrev;

        //testing
        //cout << "good prediction: " << curgpredict << "\tbad prediction: " << curbpredict << endl;

        //decides if sentence is good or bad off which percent is higher
        if(curgpredict > curbpredict)
            spredics.push_back(1);//curgpredict);
        else
            spredics.push_back(0);//curbpredict);//*/

        if(mapmatrix[i][vocab.size()-1] == spredics[spredics.size()-1])
            numcorrect += 1.0;
    }
    cout << "\nClassifier is Done" << endl;

    //accuracy of classifier
    accuracyofclassifier = numcorrect / ((float)(spredics.size()));

    cout << "Classifier Accuracy: " << accuracyofclassifier << endl;

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
        if(isalpha(*iPtr) || isspace(*iPtr))//if the character isn't an alphanumeric character or a whitspace
        {
            iPtr++;
            i++;
        }
        else
            line.erase(iPtr);
    }
}
