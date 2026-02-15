//
// Created by mrod on 6/20/25.
//

#ifndef PHDFUNCTIONS_H
#define PHDFUNCTIONS_H

#include "globalHeaders.h"
#include "PhdQueue.h"

class ServerFunction
{
public:
    ServerFunction();
    mongocxx::instance *instance;
    typedef std::string (ServerFunction::*srvrFunction)(json doc);
    typedef std::string (ServerFunction::*parseFileFunction)(string path);

    std::string processAction(json doc);

    ServerFunction::srvrFunction getFunction(std::string name, bool *ok);
    ServerFunction::parseFileFunction getParseFunction(std::string name, bool *ok);

    void notifyMessage(std::string name, json message);

    std::string contact(json doc);
    std::string rxMessage( string msg);


    //services functions
    std::string integratorFunction(json doc);
    std::string differentiatorFunction(json doc);
    std::string productFunction(json doc);
    std::string bouncingFunction(json doc);
    std::string climateFunction(json doc);
    std::string demoGraphFunction(json doc);
    std::string projectFunction(json doc) ;
    std::string readDataFolder(json doc);

    string generalQuery(json value);
    string cancelApiQuery(json value);
    string getCrossReference(json value);
    string crossRefApi(json value);
    string getAllAuthors(json value);
    string getAllReferences(json value);
    string findAllKeywords(json value);
    string getKeyWordDoi(json value);
    string getAffiliation(json value);
    string getPublisherData(json value);
    string getTypeData(json value);
    string fetchAllData(json value) ;
    string summaryDataParser(json value) ;
    string keywordDataParser(json value) ;

    string reportAllKeyWords(json value) ;

    std::string readIeeeFiles(string folderName);
    std::string readDimensionsFiles(string folderName);

    //generic functions
    void mongoDBConnect(string stringConnection);


    string getHomeDir();
    vector<string> splitIeee(string item);
    bsoncxx::types::b_date parseIeeeDate(string strdate);
    bsoncxx::types::b_date parseDimensionsDate(string dateStr);
    bool isStrNumber(string strNumber);
    vector<string> split(const string &input, string set= "\t \n\r");
    bsoncxx::types::b_date strToDate(string strDate);
    string cleanWords(string newWord);
    bsoncxx::v_noabi::oid  insertMongo(string db, string coll, string data);
    string getDateTime();
    vector<string> queryKeyWordsMongoDB(string db, string coll, vector<string> tempKwords);
    std::string parseCSVquoteNewLine(string text );

    std::string getAggregation(string db, string collection, string name);
    std::vector<std::string> lookForDois(string dbase, string collName,int aggIndex , int items, int page, bool *foundOK ,int  *registers);
    bsoncxx::document::view containsElement(string db, string coll, string id, string value, bool *oK);


    string filterQuery(string pip);
    bool parseDOIFields(string db, string coll, string urlDoiE);
    vector<string> Keys(json obj);
    vector<string> mongoKeys(bsoncxx::v_noabi::document::view view);

    std::string getAggregationDoc(string objName);
    map<string, int> matchKeywords(string inputString, string pattern);
    json reviewAllDataSource(string dimStr,string pattern, string sourceId, json dataObject, vector<string> *doiDone, int *records);
    int getJsonElement(json obj, string key, json value);
    vector<string> getLookWords(string what = "words");
    void reviewAllDataSourceWithDoi(string dimStr,string pattern, string sourceId, std::vector<bsoncxx::document::value>* dataObject, vector<string> *doiDone, int *records);
    void affiliationDataSource(string dimStr,vector<string> pattern, string sourceId, string targetColl, vector<string> *doiDone,int *records);
    vector<string> removeVectorEmpties(vector<string> vect);
    string matchAuthors(string match, string with);
    string vectorContains(vector<string> source, vector<string> query);
    bool containsSubstringInVector(const std::vector<std::string>& vec, const std::string& substring, string *result) ;
    string initCntryMap();
    string checkCntrName(string cntry, vector<string> cntryPattern);
    vector<string> split(string text, vector<string> delim);
    string regexEscape( string word);
    string split_first_match(string& input,  vector<string> delim, bool lastMatch = false) ;
    string toRegexPattern(vector<string> delim) ;

    vector<string> splitParenthesis(string input);
    string getParenthesis(string text, string* author) ;
    void stripUnicode(string & str);

    json dataSort( int threshold, map<string, int> );


    //vars
    map<string,int> csvColIndex;
    map<string, string> dbNames;
    map<string, string> countryName;
    mongocxx::client *client;

    vector<string> months;
    vector<string> dbIndexNames;

    string searchKeyWords;
    int startPage;
    int itemsPerPage;
    int actualRegisters;

    int requiredRegisters;
    int expectedRegisters;
    int foundRegisters;
    PhdQueue<string> doiQueue;
    PhdQueue<string> jsonReply;



private:
    map<std::string, srvrFunction> usrFunction;
    map<std::string, parseFileFunction> pFileFunction;

    mongocxx::uri *mongoUri{};
    //mongocxx::client *mongoClient{};

};



#endif //PHDFUNCTIONS_H
