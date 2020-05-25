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

void preprocessor(string fileInName, string fileOutName);
void training();
void classifier();
void cleanLine(vector<char> & line);

struct vocabdata
{
    string vocab;

    float grev;
    float brev;

    float pbrev;     //probability of good review
    float pgrev;      //probability of bad review
};
float accuracyofclassifier = 0;

//global for training data
vector<vocabdata> vdata;
//turn on when inputing trainning data
int vdatatrain = 0;

int main()
{

    // preprocessor("trainingSet.txt", "preprocessed_train.txt");
    // preprocessor("testSet.txt", "preprocessed_test.txt");
    vdatatrain = 1;
    preprocessor("exampleIn.txt", "exampleOut.txt");
    training();
    classifier();

    //cout << vdata[7].vocab << " " << vdata[7].pgrev << " " << vdata[7].pbrev << endl;

    return 0;
}



void preprocessor(string fileInName, string fileOutName)
{
    ifstream fileIn;
    ofstream fileOut;
    string curLine;

    //I don't know why the words here aren't colored
    vector<string> vocab; //so this holds all the words and data
    int* curPrepro = NULL;
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
                
                //makes vector for bayes computation
                if(vdatatrain == 1){
                    int it = -1;
                    for(int z = 0; z < vdata.size(); z++){
                        if(vdata[z].vocab == temp2){
                            it = z;
                            break;
                        }
                    }

                    if(it != -1)
                    {
                        if(curRatingStr == "1")
                            vdata[it].grev += 1.0;
                        else
                            vdata[it].brev += 1.0;
                    }
                    else{
                        vdata.push_back(vocabdata());
                        int curvec = vdata.size()-1;
                        vdata[curvec].vocab = temp2;
                        //add number of good/bad
                        if(curRatingStr == "1"){
                            vdata[curvec].grev = 1.0;
                            vdata[curvec].brev = 0.0;
                        }
                        else{
                            vdata[curvec].grev = 0.0;
                            vdata[curvec].brev = 1.0;
                        }
                    }
                }
                
                


                //initilize in vocab
                auto kiter = vocab.begin();
                while(kiter != vocab.end() && temp2.compare(*kiter) > 0)
                {
                    kiter++;
                }
                if(kiter == vocab.end())
                {
                    vocab.push_back(temp2);
                }
                else if(temp2.compare(*kiter) != 0)
                {   
                    vocab.emplace(kiter, temp2);          
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

        /*for (int x = 0; x<vdata.size(); ++x)
            cout << vdata[x].vocab << " " <<  vdata[x].brev << " " <<  vdata[x].grev << endl;//*/
        /*for (int x = 0; x<vocab.size(); ++x)
            cout << vocab[x] << endl;
        cout << "actual vocab size: " << vocab.size() << "\tnew vector size: " << vdata.size() << endl;//*/
    }

    //at this point, vocab should contain every word and allLines should contain every line
    //print the vocab into the output files
    auto hiter = vocab.begin();
    fileOut << *hiter;
    hiter++;
    while(hiter != vocab.end()) 
    {
        fileOut << ',' << *hiter;
        hiter++;
    }
    fileOut << ",classLabel" << endl;

    //actually make the preprocessed stuff
    curPrepro = new int[vocab.size() + 1];
    for(unsigned int i = 0; i < vocab.size() + 1; i++)
        curPrepro[i] = 0;
    //for every word in allLines
    for(auto iter = allLinesSplit.begin(); iter != allLinesSplit.end(); iter++)
    {
        for(auto kiter = iter->begin(); kiter != iter->end() - 1; kiter++)
        {
            //find a match in vocab
            for(auto jiter = vocab.begin(); jiter != vocab.end(); jiter++)//I could make this loop quite a bit faster, but I don't care
            {
                if(kiter->compare(*jiter) == 0)
                    curPrepro[jiter - vocab.begin()] = 1;
            }
        }

        //at this point, we've gone through every word in a line save the last thing (the rating)

        if(*(iter->end() - 1) == "0")
            curPrepro[vocab.size()] = 0;//get the rating and stick it onto curPrepro
        else if(*(iter->end() - 1) == "1")
            curPrepro[vocab.size()] = 1;
        else
            cout << "iter->end() - 1 = " << *(iter->end() - 1) << endl;

        //stick it into the output file
        fileOut << curPrepro[0];
        for(unsigned int i = 1; i < vocab.size() + 1; i++)
            fileOut << ',' << curPrepro[i];
        fileOut << endl;

        //reset curPrepro
        for(unsigned int i = 0; i < vocab.size(); i++)
            curPrepro[i] = 0;
    }
}

void training()
{   
    //insert an extra vocab on to hold data for parent review percents
    vector<vocabdata>::iterator it;
    it = vdata.begin();
    it = vdata.insert(it,0,vocabdata());

    vdata[0].vocab = " ";

    //to find the percent that a review is good or bad
    float numberg = 0.0;
    float numberb = 0.0;
    float tnumber = 0.0;

    //finds percentages for how often a word results in a good or bad review
    for(int i = 0; i < vdata.size(); i++){
        tnumber += vdata[i].grev + vdata[i].brev;
        numberg += vdata[i].grev;
        numberb += vdata[i].brev;

        vdata[i].pgrev = (vdata[i].grev) / (vdata[i].grev + vdata[i].brev);
        vdata[i].pbrev = (vdata[i].brev) / (vdata[i].grev + vdata[i].brev);
    }
    
    //calculate review percents
    vdata[0].pgrev = (numberg) / (tnumber);
    vdata[0].pbrev = (numberb) / (tnumber);

    cout << "Training Done" << endl;
}

//send it curPrero to find which words are in sentences
void classifier(vector)
{
    vector<int> spredics;
    int numcorrect;

    for(int i = 0; i < vpresent.size(); i++){
        float curgpredict = 0.0;
        float curbpredict = 0.0;
        //minus 2 because of the extra vocab word added
        for(int j = 0; j<vdata.size()-2; j++){
            if(vpresent[i] == 1)
            {
                //gets vocab word a present location and checks probailities and multiplies them on
                curgpredict = curgpredict * vdata[j].pgrev;
                curbpredict = curbpredict * vdata[j].pbrev;
            }
            i++
        }
        curgpredict = curgpredict * vdata[0].pgrev;
        curbpredict = curbpredict * vdata[0].pbrev;

        if(curgpredict > curbpredict)
            spredics.insert(spredics.end(),spredics.begin(),curgpredict);
        else
            spredics.insert(spredics.end(),spredics.begin(),curbpredict);

        if(vpresent[i] == spredics[spredics.size()-1])
            numcorrect += 1.0;
    }

    //accuracy of classifier
    accuracyofclassifier = numcorrect / ((float)(spredics.size()));
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
