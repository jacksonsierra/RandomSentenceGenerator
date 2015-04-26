/**
 * File: random-sentence-generator.cpp
 * -----------------------------------
 * Presents a short program capable of reading in
 * context-free grammar files and generating arbitrary
 * sentences from them.
 */

#include <iostream>     //For cout
#include <fstream>      //For filestream
#include <string>       //For getline
#include <cctype>       
#include "console.h"    //For console
#include "simpio.h"     //For getLine
#include "strlib.h"     //For toLowerCase, trim
#include "vector.h"     //For Vector
#include "hashmap.h"    //For HashMap
#include "filelib.h"    //For helper functions
#include "random.h"     //For randomInteger
using namespace std;

//Constants
static const string kGrammarsDirectory = "grammars/";
static const string kGrammarFileExtension = ".g";
static const string kNonterminalOpenBracket = "<";
static const string kNonterminalCloseBracket = ">";
static const string kStartNonterminal = "<start>";
static const int kNumberofRandomSentences = 3;

//Prototypes
static string getFileName();
static bool isValidGrammarFilename(string filename);
static string getNormalizedFilename(string filename);
static void readInFile(ifstream& file, HashMap<string, Vector<string> >& nonterminals, string filename);
static void setNonterminalExpansions(HashMap<string, Vector<string> >& nonterminals, const Vector<string> linesInFile);
static bool isDigitString(string str);
static void generateRandomSentences(const HashMap<string, Vector<string> >& nonterminals);
static string evaluateSentence(const HashMap<string, Vector<string> >& nonterminals, Vector<string> workingString);
static string evaluateNonterminal(const HashMap<string, Vector<string> >& nonterminals, string nonterminal);
static string cleanNonterminal(string word, string& addedPunctuation);

/**
 * Function: main
 * The main prompts the user for a file,
 * reads that file in if valid, and
 * then prints out three random settings based
 * on the file's grammar.  This continues until the user
 * opts out of printing any further settings.
 */
int main() {
    while (true) {
        ifstream file;
        HashMap<string, Vector<string> > nonterminals;
        string filename = getFileName();
        if (filename.empty()) break;
        readInFile(file, nonterminals, filename);
        generateRandomSentences(nonterminals);
    }
    cout << "Thanks for playing!" << endl;
    return 0;
}

/**
 * Function: getFileName
 * Function that prompts the user for a filename and
 * returns it if it is valid, as determined by helper
 * function isValidGrammarFilename, or is null,
 * the exit key for the program.  If invalid,
 * the user is reprompted for a valid file.
 */
static string getFileName() {
    while (true) {
        string filename = trim(getLine("Name of grammar file? [<return> to quit]: "));
        if (filename.empty() || isValidGrammarFilename(filename)) return filename;
        cout << "Failed to open the grammar file named \"" << filename << "\". Please try again...." << endl;
    }
}

/**
 * Function: isValidGrammarFilename
 * Predicate function that receives a filename as a string,
 * calls a helper function getNormalizedFilename to clean the user input,
 * and then returns whether the filestream was able to successfully
 * check the file's existence.
 * Parameters: filename inputted by user
 */
static bool isValidGrammarFilename(string filename) {
    string normalizedFileName = getNormalizedFilename(filename);
    ifstream infile(normalizedFileName.c_str());
    return !infile.fail();
}

/**
 * Function: getNormalizedFilename
 * This function appends the requisite information for the
 * main to determine if the file exists. The path is
 * specified by global constants, and the ultimate output
 * is returned as a string to the calling function.
 * Parameters: filename inputted by user
 */
static string getNormalizedFilename(string filename) {
    string normalizedFileName = kGrammarsDirectory + filename;
    if (!endsWith(normalizedFileName, kGrammarFileExtension))
        normalizedFileName += kGrammarFileExtension;
    return normalizedFileName;
}

/**
 * Function: readInFile
 * This function does most of the work to prepare the environment
 * to generate random sentences.  It first opens the file that has only
 * been referenced via filename to date in the program. It then
 * reads each line of the file into a Vector<string>. Finally,
 * it enlists a helper function to determine what are the non
 */
static void readInFile(ifstream& file, HashMap<string, Vector<string> >& nonterminals, string filename) {
    file.open(getNormalizedFilename(filename));
    Vector<string> linesInFile;
    readEntireFile(file, linesInFile);
    setNonterminalExpansions(nonterminals, linesInFile);
}

/**
 * Procedure: setNonterminalExpansions
 * Procedure function that populates a HashMap passed by reference with 
 * nonterminals as the keys, and the corresponding expansions, 
 * stored as Vector<string>, as the values. Concretely this is done
 * by looping through each line of the file, via a referenced
 * Vector<string> parameter also passed, and determining if the
 * line is a nonterminal declaration. Once determined, the consistent
 * format of the file allows one to loop through the explicit
 * expansions associated with that nonterminal and put them into a Vector<string>.
 * Parameters: nonterminal to expansions HashMap from the main, Vector<string>
 * containing the individual lines of the file.
 */
static void setNonterminalExpansions(HashMap<string, Vector<string> >& nonterminals, const Vector<string> linesInFile) { 
    for(int fileLine = 0; fileLine < linesInFile.size(); fileLine++) {  //Loop through each line of the file via the Vector<string>
        if(!linesInFile[fileLine].empty() && linesInFile[fileLine].substr(0,1) == kNonterminalOpenBracket) {  //Check if the line contains a nonterminal
            int fileLineOffsetOne = fileLine + 1;
            if(fileLineOffsetOne <= linesInFile.size() - 1 && isDigitString(linesInFile[fileLineOffsetOne])) {  //Check if that same line is also succeeded by an integer
                Vector<string> expansions;
                int numExpansions = stringToInteger(linesInFile[fileLineOffsetOne]);
                for(int expansionIndex = 0; expansionIndex < numExpansions; expansionIndex++) {  //Loop through each of the expansions for the nonterminal
                    expansions.add(linesInFile[fileLine + 2 + expansionIndex]);  //Add each individual expansion to a Vector<string>, all of which start two lines after the nonterminal declaration
                }
                nonterminals[linesInFile[fileLine]] = expansions;  //Add the nonterminal and expansions as a key and value, respectively, to the HashMap
            }
        }
    }
}

/**
 * Function: isDigitString
 * Predicate function that is passed a string and
 * and determines if it represents a digit. Used to
 * convert the number of expansions decreeded in the file
 * for a nonterminal.
 * Parameter: string
 */
static bool isDigitString(string str) {
    if (str.length() == 0) return false;
    for (int i = 0; i < str.length(); i++) {
        if (!isdigit(str[i])) return false;
    }
    return true;
}

/**
 * Procedure: generateRandomSentences
 * Function that takes in the HashMap of nonterminals and expansions and 
 * looks to print random sentences based on a specified global constant.
 * The function uses a for loop to create each sentence, and primarily
 * leverages a helper function to evaluate the <start> nonterminal(s)
 * specified in the file, referred to here as the workingString.
 * Parameters: nonterminal to expansions HashMap from the main
 */
static void generateRandomSentences(const HashMap<string, Vector<string> >& nonterminals) {
    Vector<string> workingString = nonterminals[kStartNonterminal];
    for(int i = 1; i <= kNumberofRandomSentences; i++) {
        cout << "\n" << i << ".) " << evaluateSentence(nonterminals, workingString) << endl;
    }
    cout << endl;
}

/**
 * Function: evaluateSentence
 * This function ultimately returns a randomly generated sentence for output. 
 * It does so by looping through each expansion in the working string,
 * always the <start> nonterminal, and searching for the first nonterminal.
 * Based on the grammar rules in place, each nonterminal is expanded
 * to create a final sentence(s) of terminals. This same process is repeated
 * over and over until all the nonterminals are expanded. Concretely this function
 * continues to run as long as an expansion has a nonterminal, and then
 * reads in each component of the working string expansion to see if further
 * nonterminals exist.  All nonterminals are evaluated using a helper function,
 * evaluateNonterminal; if the component is a terminal, it is passed into the
 * existing working string, a Vector<string>. Once all nonterminals are removed,
 * one of the working strings is randomly returned for sentence generation as a string.
 * Parameters: nonterminal to expansions HashMap from the main as a constant reference,
 * and crucially a copy of the workingString Vector<string> so that it can be mainpulated
 * for each random case
 */
static string evaluateSentence(const HashMap<string, Vector<string> >& nonterminals, Vector<string> workingString) {
    for(int i = 0; i < workingString.size(); i++) {  //Loop through each of the <start> expansions
        while(workingString[i].find(kNonterminalOpenBracket) != string::npos) {  //If the current expansion has a nonterminal, continue evaluating
            string sentence;
            istringstream stream(workingString[i]);
            string word;
            while(stream >> word) {  //Get the individual components in the given expansion
                string addedPunctuation;
                if(word.substr(0,1) == kNonterminalOpenBracket) {  //If a nonterminal
                    word = evaluateNonterminal(nonterminals, cleanNonterminal(word, addedPunctuation));  //Expand the nonterminal, ensuring punctuation is removed for HashMap
                }
                sentence += (" " + word + addedPunctuation);  //Add evaluated terminal/nonterminal to the individual sentence
            }
            workingString[i] = sentence;  //Add the sentence to the working Vector<string>,
        }
    }
    return workingString[randomInteger(0, workingString.size() - 1)];  //Randomly return one of the evaluated sentences containined in the working string Vector<string>
}

/**
 * Function: evaluateNonterminal
 * Function that randomly chooses an expansion given
 * an associated nonterminal. The HashMap containing both keys
 * and values is passed by constant reference to facilitate
 * the expansion return.
 * Parameters: nonterminal to expansions HashMap from the main as a constant reference,
 * nonterminal being evaluated as a string
 */
static string evaluateNonterminal(const HashMap<string, Vector<string> >& nonterminals, string nonterminal) {
    Vector<string> expansions = nonterminals[nonterminal];
    string chosenExpansion = expansions[randomInteger(0, expansions.size() - 1)];
    return chosenExpansion;
}

/**
 * Function: cleanNonterminal
 * Simple function that rids a nonterminal of additional punctuation
 * that may be a part of the sentence, but will cause any
 * value retrieval from the corresponding HashMap to fail.
 */
static string cleanNonterminal(string word, string& addedPunctuation) {
    int posOpenBracket = word.find(kNonterminalOpenBracket);
    int posCloseBracket = word.find(kNonterminalCloseBracket);
    addedPunctuation = word.substr(posCloseBracket + 1);
    return word.substr(posOpenBracket, posCloseBracket + 1);
}