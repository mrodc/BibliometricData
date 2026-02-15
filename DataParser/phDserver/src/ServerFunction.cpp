//
// Created by mrod on 6/20/25.
//

// #include "headers/PhdFunctions.h"
#include "ServerFunction.h"
#include "Simulator.h"


ServerFunction::ServerFunction()
{
    initCntryMap();

    if ( !mongoInUse) {
        mongoInUse = true;
        instance = new  mongocxx::instance();
    }
    mongoDBConnect("mongodb://localhost:27017/");

    months ={"","Jan","Feb", "Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

    dbIndexNames ={"ieeeKeyW","authorKW", "abstract",//"keywords",
                    "url","urlPDF", "authors", "publisher","published",
                    "referenceCnt", "citations","label"};
    dbNames.insert(make_pair("citations","Article Citation Count"));
    dbNames.insert(make_pair("doi","DOI"));
    dbNames.insert(make_pair("ieeeKeyW","IEEE Terms"));
    //dbNames.insert(make_pair("keywords","Abstract"));
    dbNames.insert(make_pair("label","Document Title"));
    dbNames.insert(make_pair("url","DOI"));
    dbNames.insert(make_pair("urlPDF","PDF Link"));
    dbNames.insert(make_pair("authors","Authors"));
    dbNames.insert(make_pair("authorKW","Author Keywords"));
    dbNames.insert(make_pair("affiliation","Author Affiliations"));
    dbNames.insert(make_pair("publisher","Publisher"));
    dbNames.insert(make_pair("referenceCnt","Reference Count"));
    dbNames.insert(make_pair("abstract","Abstract"));
    dbNames.insert(make_pair("keywords","label,abstract"));
    dbNames.insert(make_pair("published","Date Added To Xplore"));

    pFileFunction.insert({"ieee", &ServerFunction::readIeeeFiles});
    pFileFunction.insert({"dimensions", &ServerFunction::readDimensionsFiles});


    usrFunction.insert({"_contact", &ServerFunction::contact});
    usrFunction.insert({"integratorFunction", &ServerFunction::integratorFunction});
    usrFunction.insert({"differentiatorFunction", &ServerFunction::differentiatorFunction});
    usrFunction.insert({"productFunction", &ServerFunction::productFunction});
    usrFunction.insert({"bouncingFunction", &ServerFunction::bouncingFunction});
    usrFunction.insert({"climateFunction", &ServerFunction::climateFunction});
    usrFunction.insert({"demoGraphFunction", &ServerFunction::demoGraphFunction});
    usrFunction.insert({"projectFunction", &ServerFunction::projectFunction});


    usrFunction.insert({"readDataFolder", &ServerFunction::readDataFolder});
    usrFunction.insert({"_cancelApiQuery", &ServerFunction::cancelApiQuery});
    usrFunction.insert({"generalQuery", &ServerFunction::generalQuery});
    usrFunction.insert({"_crossRefApi", &ServerFunction::crossRefApi});
    usrFunction.insert({"_getCrossReference", &ServerFunction::getCrossReference});
    usrFunction.insert({"_getAllAuthors", &ServerFunction::getAllAuthors});
    usrFunction.insert({"_getAllReferences", &ServerFunction::getAllReferences});
    usrFunction.insert({"_findAllKeywords", &ServerFunction::findAllKeywords});
    usrFunction.insert({"_getKeyWordDoi", &ServerFunction::getKeyWordDoi});
    usrFunction.insert({"_getAffiliation", &ServerFunction::getAffiliation});
    usrFunction.insert({"_getPublisherData", &ServerFunction::getPublisherData});
    usrFunction.insert({"_getTypeData", &ServerFunction::getTypeData});
    usrFunction.insert({"_fetchAllData", &ServerFunction::fetchAllData});
    usrFunction.insert({"_summaryDataParser", &ServerFunction::summaryDataParser});
    usrFunction.insert({"_keywordDataParser", &ServerFunction::keywordDataParser});
    usrFunction.insert({"_reportAllKeyWords", &ServerFunction::reportAllKeyWords});


}

string ServerFunction::reportAllKeyWords(json value) {

    string pip = value.dump();

    auto bson = bsoncxx::from_json(pip);
    string range = bson["collection"].get_string().value.data();

    string sourceCollection = bson["Data"]["aggregation"]["sourceColl"].get_string().value.data();

    string dbase = bson["dbase"].get_string().value.data();
    auto pipe = bson["Data"]["aggregation"]["pipe"].get_array().value;

    mongocxx::pipeline p{};
    p.append_stages(pipe);

    auto dB = client->database(dbase);
    auto collectionM = dB.collection(sourceCollection);

    map<string, int> allYearsData = {};
    json yearlyData {};

    auto cursorM = collectionM.aggregate(p, mongocxx::options::aggregate{});
    for(auto &&item: cursorM) {
        auto year = item["data"]["year"].get_int32().value;
        auto objData = item["data"]["data"].get_document().view();
        map<string, int> yearData = {};
        auto kss  = mongoKeys(objData);
        for (string isO: kss) {
            auto cnt = objData[isO].get_int32().value;
            allYearsData[isO] += cnt;
            yearData[isO] += cnt;

        }

        auto sorted = dataSort( 0, yearData );
        json jsData;
        jsData["year"] = year;
        jsData["data"] = sorted;

        yearlyData.push_back(jsData);

        //std::cout << yearlyData[year].dump(4) << std::endl;

    }

    json result = dataSort( 100, allYearsData );



    std::cout << yearlyData.dump(4) << std::endl;


    json summary;
    summary["status"] = "stop";
    summary["from"] = "reportAllKeyWords() function";
    summary["function"] = range;
    summary["yearlyData"] = yearlyData;
    summary["dataType"] = "keywordsSummary";
    summary["allData"] = result;

    string done = cancelApiQuery(summary);


    return done;
}


string ServerFunction::keywordDataParser(json value) {
    string pip = value.dump();

    auto bson = bsoncxx::from_json(pip);

    string pipeName = bson["collection"].get_string().value.data();

    //auto agg = value["Data"]["aggregation"];

    string dbase = bson["dbase"].get_string().value.data();

    string sourceCollection = bson["Data"]["aggregation"]["sourceColl"].get_string().value.data();
    string targetCollection = bson["Data"]["aggregation"]["targetColl"].get_string().value.data();

    auto pipe = bson["Data"]["aggregation"][pipeName].get_array().value;

    mongocxx::pipeline p{};
    p.append_stages(pipe);

    auto dB = client->database(dbase);
    auto collectionM = dB.collection(sourceCollection);

    map<int, map<string,int>> yearlyMap;
    json jData;
    jData["name"] =  pipeName;
    jData["date"] = getDateTime();
    jData["data"] = vector<json>{};

    auto cursorM = collectionM.aggregate(p, mongocxx::options::aggregate{});
    for(auto &&item: cursorM) {

        int year = item["_id"].get_int32();

        json tempData;
        tempData["year"] = year;
        auto aData = item[pipeName].get_array().value;//.data();
        auto fnd = yearlyMap.find(year);
        if ( fnd == yearlyMap.end() ) {

            yearlyMap.insert(make_pair(year, map<string, int>()));
        }
        map<string, int> dataM = {};
        for(bsoncxx::array::element& element : aData) {
            auto b_str =  element.get_string();
            string st = b_str.value.data();
            trim_if( st,boost::is_any_of("- ."));
            if ( st.empty())
                continue;
            ++dataM[st];
        }
        tempData["data"] = dataM;
        jData["data"].push_back(tempData);
    }
    string jDataStr = jData.dump();
    auto bsonData = bsoncxx::from_json(jDataStr);

    auto targetC = dB.collection(targetCollection);
    targetC.insert_one(std::move(bsonData));
    json summary;
    summary["status"] = "stop";
    summary["from"] = "keywordDataParser() function";
    summary["data"] = pipeName;
    string done = cancelApiQuery(summary);


    return done;
};


string ServerFunction::summaryDataParser(json value) {
    string pip = value.dump();

    auto bson = bsoncxx::from_json(pip);

    string pipeName = bson["collection"].get_string().value.data();

    //auto agg = value["Data"]["aggregation"];

    string dbase = bson["dbase"].get_string().value.data();

    string sourceCollection = bson["Data"]["aggregation"]["sourceColl"].get_string().value.data();
    string targetCollection = bson["Data"]["aggregation"]["targetColl"].get_string().value.data();

    auto pipe = bson["Data"]["aggregation"][pipeName].get_array().value;

    mongocxx::pipeline p{};
    p.append_stages(pipe);

    auto dB = client->database(dbase);
    auto collectionM = dB.collection(sourceCollection);

    map<int, map<string,int>> yearlyMap;
    json jData;
    jData["name"] =  pipeName;
    jData["date"] = getDateTime();
    jData["data"] = vector<json>{};

    auto cursorM = collectionM.aggregate(p, mongocxx::options::aggregate{});
    for(auto &&item: cursorM) {

        int year = item["_id"].get_int32();

        json tempData;
        tempData["year"] = year;
        auto aData = item[pipeName].get_array().value;//.data();
        auto fnd = yearlyMap.find(year);
        if ( fnd == yearlyMap.end() ) {

            yearlyMap.insert(make_pair(year, map<string, int>()));
        }
        map<string, int> dataM = {};
        for(bsoncxx::array::element& element : aData) {
            auto b_str =  element.get_string();
            string st = b_str.value.data();
            trim_if( st,boost::is_any_of("- ."));
            if ( st.empty())
                continue;
            ++dataM[st];
        }
        tempData["data"] = dataM;
        jData["data"].push_back(tempData);
    }
    string jDataStr = jData.dump();
    auto bsonData = bsoncxx::from_json(jDataStr);

    auto targetC = dB.collection(targetCollection);
    targetC.insert_one(std::move(bsonData));
    json summary;
    summary["status"] = "stop";
    summary["from"] = "summaryDataParser() function";
    summary["data"] = pipeName;
    string done = cancelApiQuery(summary);


    return done;
};


string ServerFunction::fetchAllData(json value) {
    string valueStr = value.dump(4);
    auto agg = value["Data"]["aggregation"];

    return "";
};

string ServerFunction::getPublisherData(json value) {
    string valueStr = value.dump(4);
    cout << valueStr << endl;
    return "";
};
string ServerFunction::getTypeData(json value) {

    string valueStr = value.dump(4);
    cout << valueStr << endl;
    return "";
};

string ServerFunction::getAffiliation(json value){

    std::vector<std::string> sourceVector = {
         "affiliationCitation-dimensionsData"
        ,
        "affiliationCitation-ieeeData"

    };
    string dBase = value["dbase"];
    string targetColl = value["collection"];

    std::vector<std::string> doiDone;
    json keyWords = {};
    int records = 0;
    vector<string> pattern  = getLookWords("countries");
    for(string sName: sourceVector){
        auto dimStr = getAggregationDoc(sName);
        affiliationDataSource(dimStr,pattern, sName,targetColl, &doiDone, &records);
    }
    json summary;
    summary["status"] = "stop";
    summary["from"] = "getAffiliation() function";
    string done = cancelApiQuery(summary);

    return done;
}
void ServerFunction::affiliationDataSource(string dimStr,vector<string> cntryPattern, string sourceId, string targetColl, vector<string> *doiDone,int *records){
    auto jsonMatch = bsoncxx::from_json(dimStr);

    auto mDb = jsonMatch["database"].get_string().value.data();
    string collection = jsonMatch["collection"].get_string().value.data();

    auto matchDB = client->database(mDb);
    auto collectionM = matchDB.collection(collection);

    auto dateT = getDateTime();

    cout <<endl <<  dateT << " -- " <<sourceId << endl;


    //int records = 0;
    map<string, int> matchW;
    string pipeTemp = dimStr;

    vector<string> universities = {"University", "Institute","Polytechnic",
        "School of Engineering", "Universidad", "Univ.", "CINVESTAV",
        "Université", "Politecnico", "College", "NICT", "MIT",
        "Universitas", "School", "Siemens AG", "Universität", "Robert Bosch",
        "State Grid","Complex Systems", "Universiti"};
    auto bsonPipe = bsoncxx::from_json(pipeTemp);
    auto pipe = bsonPipe["pipeline"].get_array().value;
    mongocxx::pipeline pM{};
    pM.append_stages(pipe);

    string country, university, author, url, title;
    // string university;
    auto collectionData = matchDB.collection(targetColl);
    auto collectionCross = matchDB.collection("crossrefData");

    auto cursorM = collectionM.aggregate(pM, mongocxx::options::aggregate{});
    for(auto &&item: cursorM){
        string doi = item["doi"].get_string().value.data();
        to_lower(doi);
        auto doiFnd = find(doiDone->begin(), doiDone->end(), doi);
        if( doiFnd != doiDone->end()){
            continue;
        }
        bool oK = false;
        containsElement(mDb, "crossrefData","doi", doi, &oK);
        if(!oK)
            continue;

        auto crsObjView = collectionCross.find_one({make_document(kvp("doi", doi))});
        auto crsView = crsObjView->view();
        if (crsView.find("authors") == crsView.end()) {
            string crsVstr = bsoncxx::to_json( crsView);
            continue;
        }
        int year =  item["year"].get_int32().value;
        int cites = 0;


        doiDone->push_back(doi);
        (*records)++;
        cout<< *records<<endl;
        //auto crsObject = collectionCross.find_one({make_document(kvp("doi", doi))});

        //auto crsView = crsObject->view();

        string publisher = crsView["publisher"].get_string().value.data();
        string typePublic = crsView["type"].get_string().value.data();
        stripUnicode(typePublic) ;
        stripUnicode(publisher) ;
        if( collection.compare("ieeeData") == 0) {
            try{
                if ( doi.compare("10.1109/t2fuzz.2011.5949552") == 0)
                    trim(doi);
            //cout <<"DOI_0: " << doi << " -- Records: " << *records <<endl;
            url = item["url"].get_string().value.data();
            auto affiliation = item["affiliation"].get_document().view();
            auto afKeys = mongoKeys(affiliation);
            vector<string> refAts ;

            //auto cls = crsObject->view();
            auto autFind = crsView.find("authors");

            //cout <<"DOI_1: " << doi << " -- Records: " << *records <<endl;
            if( autFind != crsView.end()){
              auto crsAuthor = autFind->get_array().value;
                    for(auto &&atu:crsAuthor) {
                        auto aKs = mongoKeys(atu.get_document().view());
                        refAts.push_back( aKs[0]);

                    }
                }

            int autIndex = 0;
            for(auto skey: afKeys){

                vector<string> asp;
                boost::split(asp, skey,boost::is_any_of(" "),boost::token_compress_on);
                asp = removeVectorEmpties(asp);
                std::reverse(asp.begin(), asp.end());
                author = join(asp,", ") ;
                if( !refAts.empty() && autIndex < refAts.size()){
                    author = matchAuthors(author, refAts[autIndex]);
                }
                //author = skey;
                string temp = affiliation[skey].get_string().value.data();

                university= split_first_match(temp, universities);
                country = checkCntrName(temp, cntryPattern);
                stripUnicode(author) ;


                stripUnicode(university) ;
                stripUnicode(country) ;

                autIndex++;
                cites = item["citations"].get_int32().value;
                title = item["title"].get_string().value.data();
                url = item["url"].get_string().value.data();
                stripUnicode(title) ;
                stripUnicode(url) ;

                json jData;
                jData["collection"] = collection;
                jData["doi"] = doi;
                jData["author"] = author;
                jData["country"] = country;
                jData["university"] = university;
                jData["citations"] = cites;
                jData["title"] = title;
                jData["url"] = url;
                jData["year"] = year;
                jData["type"] = typePublic;
                jData["publisher"] = publisher;
                string jDataStr = jData.dump();
                auto bsonData = bsoncxx::from_json(jDataStr);

                collectionData.insert_one(std::move(bsonData));

            }
            continue;
        }
            catch (std::exception const &ex)
            {
                string st = "Can't init settings. "; //+  ex.what();
                cout << st << endl;
            }
            catch(...){
                string tct = "test";
                tct += "Trouble";

            }
        }
        try {
            if( collection.compare("dimensionsData") == 0){
                url = item["url"].get_string().value.data();
                string doiF = item["doi"].get_string().value.data();
                string affiliation = item["affiliation"].get_string().value.data();
                string affilUns = item["affils"].get_string().value.data();

                auto rawA = splitParenthesis(affiliation);
                auto onlyAffs = splitParenthesis(affilUns);
                for (int i=0; i< onlyAffs.size(); i++){

                    string affs = onlyAffs[i];
                    //string atr;
                    string nUv = getParenthesis(affs, &author);

                    university= split_first_match(nUv, universities);
                    string cntryStr = nUv + " " + rawA[i];
                    country = checkCntrName(cntryStr, cntryPattern);

                    if ( country.compare("UnKnown") == 0) {
                        country = "-";
                        trim(country);
                    }

                    if ( university.empty())
                        university = "-";
                    int cites = item["citations"].get_int32().value;

                    cites = item["citations"].get_int32().value;
                    title = item["title"].get_string().value.data();
                    url = item["url"].get_string().value.data();
                    stripUnicode(title) ;
                    stripUnicode(url) ;
                    stripUnicode(author) ;
                    stripUnicode(university) ;
                    stripUnicode(country) ;


                    json jData;
                    jData["collection"] = collection;
                    jData["name"] = "affiliation";
                    jData["author"] = author;
                    jData["country"] = country;
                    jData["university"] = university;
                    jData["doi"] = doi;
                    jData["citations"] = cites;
                   jData["year"] = year;
                    jData["type"] = typePublic;
                    jData["publisher"] = publisher;

                    string jDataStr = jData.dump();
                    auto bsonData = bsoncxx::from_json(jDataStr);

                    collectionData.insert_one(std::move(bsonData));

            }
        }
        }
        catch (std::exception const &ex)
        {
            string st = "Can't init settings. "; //+  ex.what();
            cout << st << endl;
        }
        catch(...){
            string tct = "test";
            tct += "Trouble";

        }
    }
}

string ServerFunction::getKeyWordDoi(json value){

    std::vector<std::string> sourceVector = {
        "matchUniqueSubstring-dimensionsData",
        "matchDuplicatesSubstring-dimensionsData",
        "groupUniqueIEEE-ieeeData",
        "groupDuplicatesIEEE-ieeeData"
        // "uniqueScopus-scopusData",
        // "duplicatesScopus-scopusData"

    };

    string vaueStr = value.dump();

    string dBase = value["dbase"];
    string targetColl = value["collection"];

    std::vector<std::string> doiDone;
    json keyWords = {};
    std::vector<bsoncxx::document::value> documents;
    int records = 0;
    string pattern ;
    vector<string> keyW = getLookWords();

    pattern = join(keyW,"|");
    for(string sName: sourceVector){
        auto dimStr = getAggregationDoc(sName);
        reviewAllDataSourceWithDoi(dimStr,pattern, sName, &documents, &doiDone, &records);
    }
    auto matchDB = client->database(dBase);


    auto collectionKeys = matchDB.collection(targetColl);

    //auto result = collectionKeys.insert_one(std::move(bson));
    try {
        auto insert_many_result = collectionKeys.insert_many(documents);
        // Check if the operation was acknowledged and print inserted count
        if (insert_many_result) {
            std::cout << "Inserted " << insert_many_result->inserted_count() << " documents successfully." << std::endl;
        }
    }
    catch (const mongocxx::operation_exception& e) {
        std::cerr << "Insertion failed: " << e.what() << std::endl;
    }

    json summary;
    summary["status"] = "stop";
    summary["from"] = "findAllKeywords() function";
    string done = cancelApiQuery(summary);

    return done;

}


string ServerFunction::findAllKeywords(json value){

    std::vector<std::string> sourceVector = {
        "matchUniqueSubstring-dimensionsData",
        "matchDuplicatesSubstring-dimensionsData",
        "groupUniqueIEEE-ieeeData",
        "groupDuplicatesIEEE-ieeeData",
        // "uniqueScopus-scopusData",
        // "duplicatesScopus-scopusData"

    };

    string vaueStr = value.dump();

    string dBase = value["dbase"];
    string targetColl = value["collection"];

    std::vector<std::string> doiDone;
    json keyWords = {};
    int records = 0;
    string pattern ;
    vector<string> keyW = getLookWords();

    pattern = join(keyW,"|");
    for(string sName: sourceVector){
        auto dimStr = getAggregationDoc(sName);
        keyWords = reviewAllDataSource(dimStr,pattern, sName, keyWords, &doiDone, &records);
    }
    auto matchDB = client->database(dBase);
    json finalData;
    finalData["name"] = "All Sources";

    finalData["keywords"] = keyWords;
    finalData["records_Count"] = records;
    finalData["date"] = getDateTime();

    string dataStr = finalData.dump();

    auto bson = bsoncxx::from_json(dataStr);
    auto collectionKeys = matchDB.collection(targetColl);

    auto result = collectionKeys.insert_one(std::move(bson));

    json summary;
    summary["status"] = "stop";
    summary["from"] = "findAllKeywords() function";
    string done = cancelApiQuery(summary);

    return done;

}

string ServerFunction::getAllReferences(json value)
{
    string    allRefs =
        R"({
    'item': 'reference',
    'pipeline':
[
    {
        '$match': {
            'authors': {
                '$exists': True
            },
            'references-count': {
                '$exists': True
            }
        }
    }, {
        '$project': {
            '_id': 0,
            'reference': 1,
            'title': 1,
            'doi': 1,
            'count': {
                '$subtract': [
                    '$references-count', '$ref_no_doi'
                ]
            }
        }
    }, {
        '$sort': {
            'count': -1
        }
    }
]
}
)";
    allRefs = filterQuery(allRefs);

    auto data = bsoncxx::from_json(allRefs);

    string db = value["Data"]["db"].get<string>();
    string collSource = value["Data"]["collection"].get<string>();
    string collTarget = value["Data"]["target"].get<string>();

    auto database = client->database(db);
    auto collection = database.collection(collSource);

    auto pipe = data["pipeline"].get_array().value;
    mongocxx::pipeline p{};
    p.append_stages(pipe);


    int setSize = 0;
    map<string, int> wordsType;
    map<string, int> classType;
    map<string, map<string, int>> groupType;

    auto cursor = collection.aggregate(p, mongocxx::options::aggregate{});
    for(auto &&doc: cursor){
        auto dcKeys = mongoKeys(doc);
        auto items = doc["reference"].get_document().view();
        auto title = doc["title"].get_string().value.data();
        auto rfCount = doc["count"].get_int32();
        dcKeys = mongoKeys(items);
        int sss = dcKeys.size();
        // auto itms = items->get_array().value;
        string doiKey = doc["doi"].get_string().value.data();
        vector<string> strs;
        for (auto itm: items) {
            auto itmView = itm.get_document().view();
            auto ks = mongoKeys(itmView);
            string doi = itmView["DOI"].get_string().value.data();
            if( doi.compare("NoDoi") ==  0)
                continue;
            strs.push_back(doi);

        }
        bsoncxx::builder::stream::document tempDoc{};

        string allrefs = join(strs,",");
        tempDoc << "doi" << doiKey << "title" << title;
        tempDoc << "references" << allrefs << "refCount" << rfCount;

        bsoncxx::document::value docT = tempDoc << bsoncxx::builder::stream::finalize;
        insertMongo(db, collTarget, bsoncxx::to_json(docT.view()));

    }
    json summary;
    summary["status"] = "stop";
    summary["from"] = "getAlReferences() function";
    string done = cancelApiQuery(summary);

    return done;

}


string ServerFunction::getAllAuthors(json value)
{
    map<string, map<string, string>> allAuthors;
    string crsRefData = R"({

    'collection': 'crossrefData',
    'pipeline':
[
    {
        '$match': {
            'authors': {
                '$exists': True
            }
        }
    }, {
        '$addFields': {
            'aSize': {
                '$size': '$authors'
            }
        }
    }, {
        '$project': {
            '_id': 0,
            'authors': 1,
            'doi': 1,
            'aSize': 1
        }
    }
]
})";
    crsRefData = filterQuery(crsRefData);

    auto data = bsoncxx::from_json(crsRefData);


    string db = value["Data"]["db"].get<string>();
    string collSource = value["Data"]["collection"].get<string>();
    string collTarget = value["Data"]["target"].get<string>();

    auto pipe = data["pipeline"].get_array().value;
    mongocxx::pipeline p{};
    p.append_stages(pipe);


    int setSize = 0;
    map<string, int> wordsType;
    map<string, int> classType;
    map<string, map<string, int>> groupType;

    auto database = client->database(db);
    auto collection = database.collection(collSource);

    auto cursor = collection.aggregate(p, mongocxx::options::aggregate{});

    for(auto &&doc: cursor){
        auto array = doc["authors"].get_array().value;
        for(auto item: array){
            auto items = item.get_document().view();
            auto ks = mongoKeys(items);

            // auto items = doc[item].get_document().view();
            // auto ks = mongoKeys(items);
            auto name = ks[0];

            // boost::trim_if(name, boost::is_any_of("-, "));
            // if ( name.empty())
            //     trim(name);
            string level = items[name].get_string().value.data();
            string doi = doc["doi"].get_string().value.data();

            if( allAuthors.find(name) == allAuthors.end()){
                allAuthors.insert(make_pair(name, map<string, string>()));
            }
            if(allAuthors[name].find(level) == allAuthors[name].end()){
                allAuthors[name].insert(make_pair(level,doi));
            }
            else
                allAuthors[name][level] += ", " + doi;
        }
    }

    for (auto& aItem: allAuthors) {
        auto key = aItem.first;
        if( starts_with(key,"Popoola"))
            trim(key);
        // boost::replace_all(key,",","");
        bsoncxx::builder::stream::document tempDoc{};
        tempDoc << "author" << key;
        for (auto& inLevel: aItem.second) {
            string inKey = inLevel.first;
            string inVal = inLevel.second;
            vector<string> result;
            boost::split(result, inVal, boost::is_any_of(","),boost::token_compress_on);
            int cnt = result.size();
            tempDoc << inKey << inVal;
            tempDoc << inKey+"Cnt" << cnt;

        }
        bsoncxx::document::value docT = tempDoc << bsoncxx::builder::stream::finalize;
        insertMongo(db, collTarget, bsoncxx::to_json(docT.view()));
    }

    json summary;
    summary["status"] = "stop";
    summary["from"] = "getAllAuthors() function";
    string done = cancelApiQuery(summary);

    return done;
}


string ServerFunction::crossRefApi(json value){

    string data = value["Data"].get<string>();
    string dbase = value["dbase"].get<string>();
    string collection = value["collection"].get<string>();
    parseDOIFields(dbase, collection, data);
    string res = getCrossReference(value);
    return res;
}

string ServerFunction::generalQuery(json value){

    doiQueue.clear();
    json j = value["Data"];

    string s;
    json data;
    json info;

    string db = j["db"].get<string>();
    string coll = j["coll"].get<string>();
    actualRegisters = requiredRegisters = j["items"].get<int>();
    int page = j["page"].get<int>();

    expectedRegisters =  j["maxrec"].get<int>();
    bool foundOK = true;
    auto vect = lookForDois(db, coll, 0 , requiredRegisters,  page , &foundOK, &actualRegisters);

    if( !foundOK){
        doiQueue.clear();
        json summary;
        summary["status"] = "stop";

        cancelApiQuery(summary);
        return "";
    }
    foundRegisters = vect.size();
    doiQueue.push(vect);
    startPage = page ;//+ requiredRegisters;
    auto res = getCrossReference(value);

    return res;
}

string ServerFunction::cancelApiQuery(json value){
    json vJson;
    vJson["data"] = value;
    vJson["action"] = "doisDone";

    string d = vJson.dump();
    //jsonReply.push(d);
    return d;
}

string ServerFunction::processAction(json jsonDoc)
{
    string status = "Error message";
    string jsonStr = jsonDoc.dump();
    bool ok = false;
    auto action = jsonDoc["Action"].get<std::string>();
    auto fnc = getFunction(action, &ok);

    try
    {
        if (fnc != 0)
        {
            status = (this->*fnc)(std::move(jsonDoc));
        }
    }
    catch (const json::exception& e){
        const string es = e.what();
        status = "Error. " + es;

    }
    catch (std::exception const &ex)
    {
        const string es = ex.what();
        status = "Can't init settings. " +  es;
        cout << status << endl;
    }
    catch (...)
    {
        status += "Error. Critical  Error";

        cout << status << endl;
    }

    return status;
}

ServerFunction::srvrFunction ServerFunction::getFunction(string name, bool *ok)
{
    string nme = "none";
    ServerFunction::srvrFunction ptr = nullptr;

    *ok = false;
    auto p = usrFunction.find(name);
    if (p != usrFunction.end())
    {
        ptr = p->second;
        *ok = true;
    }
    return ptr;
}

ServerFunction::parseFileFunction ServerFunction::getParseFunction(std::string name, bool *ok) {
    string nme = "none";
    ServerFunction::parseFileFunction ptr = nullptr;

    *ok = false;
    auto p = pFileFunction.find(name);
    if (p != pFileFunction.end())
    {
        ptr = p->second;
        *ok = true;
    }
    return ptr;
}

void ServerFunction::notifyMessage(string name, json message)
{
    json msg;
    message["Date"] = "getdate";
    msg["action"] = name;
    msg["data"] = message;


    string reply = msg.dump();

}



string ServerFunction::contact(json jsonV)
{
    json m ;
    m["Status"]= "OK -- Connected.";
    m["From"] = jsonV["Data"].get<string>();

    notifyMessage("notifyMsg", m);

    return "OK";
}

string ServerFunction::rxMessage( string msg) {
    string endStr = "<|/|>";

    size_t pos = msg.find(endStr);

    if (pos != std::string::npos) {
        // If the substring is found, erase it
        msg.erase(pos, endStr.length());
    }
    json document = json::parse(msg);

    msg = "";
   string result =  processAction(std::move(document));

    return result;

}

std::string ServerFunction::integratorFunction( json doc) {
    std::string result;
     doc["newC"] = "NewValue";
    result = doc.dump();

    return result;
}

std::string ServerFunction::differentiatorFunction(json doc) {

    std::string result = doc.dump(4);

    return result;
}

std::string ServerFunction::productFunction(json doc) {

    std::string result = doc.dump(4);

    return result;
}

std::string ServerFunction::bouncingFunction(json doc) {

    Bouncing bounce(doc);

    Simulator* simulatorPtr;
    simulatorPtr = &bounce;
    simulatorPtr->doTheMath();
    auto jsonD = simulatorPtr->dataToJson();
    doc["Data"] = jsonD;
    std::string result = doc.dump();
    return result;
}

std::string ServerFunction::climateFunction(json doc) {

    std::string result = doc.dump(4);

    return result;
}

std::string ServerFunction::demoGraphFunction(json doc) {

    DemoClass demo(doc);

    Simulator* simulatorPtr;
    simulatorPtr = &demo;
    simulatorPtr->doTheMath();
    auto jsonD = simulatorPtr->dataToJson();
    doc["Data"] = jsonD;
    std::string result = doc.dump();
    return result;

}

std::string ServerFunction::projectFunction(json doc) {

    // std::string result = doc.dump(4);
    std::string result;
    doc["newC"] = "NewValue";
    result = doc.dump();

    return result;

}

string ServerFunction::readDataFolder(json doc)
{
    string status = "Error message";

    auto data = doc["Data"];//.get<string>();
    auto db = data["db"].get<string>();
    string pth = data["collection"].get<string>();
    vector<string> pathV;
    boost::split(pathV, pth,boost::is_any_of("_"), boost::token_compress_on);
    string collection = pathV[0] + "Data";
    bool ok = false;
    try {
        auto fnc = getParseFunction(pathV[0], &ok);
        if (fnc != 0)
        {
            status = (this->*fnc)(std::move(pth));
        }
    }
    catch (const json::exception& e){
        const string es = e.what();
        status = "Error. " + es;

    }
    catch (std::exception const &ex)
    {
        status = "Can't init settings. "; //+  ex.what();
        cout << status << endl;
    }
    catch (...)
    {
        status += "Error. Critical  Error";

        cout << status << endl;
    }
    return status;
}

void ServerFunction::mongoDBConnect(string stringConnection) {
    mongoUri = new mongocxx::uri(stringConnection);
    client = new mongocxx::client(*mongoUri);

}
string ServerFunction::getHomeDir()
{
    return getpwuid(getuid())->pw_dir;
}
string ServerFunction::readIeeeFiles(string folderName){
    csvColIndex.clear();

    vector<string> pathV;
    boost::split(pathV, folderName,boost::is_any_of("_"), boost::token_compress_on);
    string collection = pathV[0] + "Data";

    string userDir = getHomeDir();

    fs::path dir(userDir);
    fs::path localPath (folderName);

    fs::path path = dir / localPath;
    struct stat sb;

    bool startFlag = true;

    bool ok = false;
    bool okIeee = false;
    vector<string> header;
    int processes = 0;
    int files = 0;

    for (const auto& entry : fs::directory_iterator(path)) {
        fs::path outfilename = entry.path();
        string outfilename_str = outfilename.string();

        if (stat(outfilename_str.c_str(), &sb) != 0 || (sb.st_mode & S_IFDIR))
            continue;  //this is a directory

        string searchName = outfilename.filename();
        string extstr = outfilename.extension();

        replace_all(searchName,extstr,"");

        std::stringstream strStream;
        std::ifstream ifs(outfilename_str);

        strStream << ifs.rdbuf();
        string rdFile = strStream.str();

        vector<string> data;
        boost::split(data, rdFile,boost::is_any_of("\n"), boost::token_compress_on);

        if( startFlag){
            startFlag = false;

            //header  = split(data[0],",");
            header = splitIeee(data[0]);


            int index = 0;
            for (auto s: header) {
                csvColIndex.insert(make_pair(s,index));
                index++;
            }

        }
        data.erase(data.begin());

        int hdrSize = header.size();
        processes = 0;

        for (auto item1:data) {
            string item = item1;
            if ( item.size() <= 0 )
                continue;
            auto tempData = splitIeee(item);
            if( hdrSize != tempData.size())
                hdrSize += 0;
            int doiPos = csvColIndex["DOI"];

            string doi = tempData[doiPos];
            string doiLower = doi;
            to_lower(doiLower);
            if( doi.empty() )
                continue;

            ok = false;

            processes++;

            bsoncxx::builder::stream::document ieeeDoc{};

            cout <<"DOI:: " << doi << endl;

            ieeeDoc << "searchTerms" << searchName;
            ieeeDoc << "doi" << doi;

            vector<string> allKW;

            for (string strIdx: dbIndexNames) {
                string dtaIdx = dbNames[strIdx] ;
                int column = csvColIndex[dtaIdx];
                string cellValue = tempData[column];

                try{
                    if( strIdx.compare("published") == 0){

                        if( cellValue.empty()){
                            cellValue = tempData[23];
                            if( cellValue.empty())
                                cellValue = "01 Jan 1970";
                        }
                        auto ieeeDate = parseIeeeDate(cellValue);
                        ieeeDoc << "published" << ieeeDate ;
                        continue;

                    }

                    if( strIdx.compare("url") == 0){

                        string url = "https://doi.org/" +doi;
                        ieeeDoc << strIdx << url ;
                        continue;

                    }
                    if( strIdx.compare("urlPDF") == 0){

                        ieeeDoc << strIdx << cellValue ;
                        continue;

                    }
                    if( strIdx.compare("abstract") == 0){

                        ieeeDoc << strIdx << cellValue ;
                        continue;

                    }

                    if( strIdx.compare("label") == 0 || strIdx.compare("publisher") == 0){

                        ieeeDoc << strIdx << cellValue;
                        continue;

                    }

                    if( strIdx.compare("citations") == 0 || strIdx.compare("referenceCnt") == 0){

                        int citCnt = -1;
                        try{
                            citCnt = stoi(cellValue);
                        }
                        catch(...){
                            citCnt = 0;
                        }
                        if( strIdx.compare("citations") == 0 ){
                            bsoncxx::builder::stream::document affilDoc{};
                            affilDoc << "inPapers" << citCnt;
                            bsoncxx::document::value affilF = affilDoc << bsoncxx::builder::stream::finalize;
                            ieeeDoc << strIdx << affilF;
                        }
                        else
                            ieeeDoc << strIdx << citCnt;

                        continue;

                    }
                    if( strIdx.compare("ieeeKeyW") == 0 || strIdx.compare("authorKW") == 0){
                        if (cellValue.empty())
                            continue;
                        auto kWrds = split(cellValue,";");
                        sort(kWrds.begin(), kWrds.end());
                        auto array_builder = ieeeDoc << strIdx << bsoncxx::builder::stream::open_array;
                        vector<string> tmpNewWords;
                        for (auto &k : kWrds) {
                            array_builder << k;

                            string nwrd = cleanWords(k);
                            if( !nwrd.empty()){
                                tmpNewWords.push_back(nwrd);
                            }
                        }
                        array_builder << bsoncxx::builder::stream::close_array;

                        allKW.insert(allKW.end(), kWrds.begin(), kWrds.end());

                        queryKeyWordsMongoDB("phdDB", "keywordsData", tmpNewWords);

                        continue;
                    }

                    // if( strIdx.compare("keywords") == 0){
                    //     auto ksw = split(dtaIdx,",");
                    //     string ksStr = "";
                    //     for (string sk:ksw) {
                    //         dtaIdx = dbNames[sk] ;
                    //         column = csvColIndex[dtaIdx];
                    //         string skStr = tempData[column];
                    //         ksStr += skStr;
                    //
                    //     }
                    //     map<string, int> matchWords;
                    //     auto searchKwords = queryKeyWordsMongoDB("scopusDB", "keywordsData", allKW);
                    //
                    //     matches(ksStr, searchKwords, &matchWords);
                    //     bsoncxx::builder::stream::document keysDoc{};
                    //
                    //     foreach (auto &ssk, matchWords) {
                    //         string k = ssk.first;
                    //         int kv = ssk.second;
                    //         keysDoc << k << kv;
                    //
                    //     }
                    //     bsoncxx::document::value finalKws = keysDoc << bsoncxx::builder::stream::finalize;
                    //     ieeeDoc << strIdx << finalKws;
                    //
                    //     continue;
                    // }
                    //
                    if( strIdx.compare("authors") == 0){
                        if (cellValue.empty())
                            continue;
                        auto auts = split(cellValue,";");
                        dtaIdx = dbNames["affiliation"] ;
                        column = csvColIndex[dtaIdx];
                        cellValue = tempData[column];

                        auto affils =  split(cellValue,";");
                        int affilIndex = 0;
                        bsoncxx::builder::stream::document authDoc{};

                        for (string aStr: auts) {
                            string autN = aStr;
                            string affN = affils[affilIndex];
                            authDoc << autN << affN;
                            affilIndex++;
                        }
                        bsoncxx::document::value finalAut = authDoc << bsoncxx::builder::stream::finalize;
                        ieeeDoc << strIdx << finalAut;
                        continue;
                    }
                }
                catch(...){
                    string tct = "test";
                    tct += "Trouble";
                }
            }
            bsoncxx::document::value fIeeeDoc = ieeeDoc << bsoncxx::builder::stream::finalize;
            auto dStr = bsoncxx::to_json(fIeeeDoc);
            cout << "DOI before: " << doi << endl;
            insertMongo("phdDB",collection, dStr);
            cout << "DOI after: " << doi << endl;
            if ( doi.compare("10.1109/GrC.2007.81") == 0)
                boost::trim(doi);


        }
        cout<< endl << "done -- Records: " << processes << "  "
             <<   getDateTime() << "  " << outfilename_str <<  endl;
    }

    //cout << endl << "Files: " <<files << "   Total: " << processes<<endl;
    processes+=0;

    return "OK. " + folderName + " ->> Done.";
}


vector<string> ServerFunction::splitIeee(string item){
    vector<string> tempData;

    replace_all(item,"\"\"","");
    string pattern = R""("([^"]+)")"";
    regex re(pattern);
    smatch m;
    boost::regex exp( pattern,boost::regex::icase ) ;
    boost::match_results<std::string::const_iterator> what;
    string::const_iterator start = item.begin() ;


    //map<string, int> matchWords;
    while ( boost::regex_search(start, item.cend(), what, exp) )
    {
        string f = what[0];
        string temp = f;

        replace_all(temp,",","-*-");
        replace_all(temp,"\"","");

        replace_all(item,f,temp);

        start = item.begin() ;

    }
    boost::split(tempData,item,boost::is_any_of(","),boost::token_compress_off);

    return tempData;
}
bsoncxx::types::b_date ServerFunction::parseIeeeDate(string strdate){
    bsoncxx::types::b_date date = bsoncxx::types::b_date{std::chrono::milliseconds{0}};
    auto mnt = split(strdate);
    auto fnd = find(months.begin(), months.end(), mnt[1]);
    if( fnd != months.end()){
        int pos = std::distance(months.begin(), fnd);
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') <<  pos;
        int day = stoi(mnt[0]);
        std::stringstream ssd;
        ssd << std::setw(2) << std::setfill('0') <<  day;
        string newDate = mnt[2] + "-" + ss.str() + "-" + ssd.str();
        date = strToDate(newDate);
    }

    return date;
}
bsoncxx::types::b_date ServerFunction::parseDimensionsDate(string dateStr){
    std::regex date_MMDDYYYY("^(0?[1-9]|1[0-2])\\/(0?[1-9]|[12][0-9]|3[01])\\/((19|20)\\d{2})$");
    std::regex date_YYYYMM("^\\d{4}-(0[1-9]|1[0-2])$");
    std::regex unsigned_YYYY("^\\d{4}$");

    vector<string> words;
    cout << "Original: " << dateStr  << endl;
    if (std::regex_match(dateStr, date_MMDDYYYY)) {
        boost::split(words, dateStr, boost::is_any_of("-/"));
        dateStr = words[2] + "-" + words[0] + "-" + words[1];

    } else {
        if (std::regex_match(dateStr, date_YYYYMM)) {
            vector<string> words;
            boost::split(words, dateStr, boost::is_any_of("-/"));
            dateStr = words[0] + "-" + words[1]+ "-" + "01" ;

        } else {
            if (std::regex_match(dateStr, unsigned_YYYY)) {
                dateStr =  dateStr + "-06-01";

            }
        }
    }


    auto newDate = strToDate(dateStr);
    return newDate;
}
bool ServerFunction::isStrNumber(string strNumber){
    try{
        size_t pos;

        std::stod(strNumber, &pos);

        return pos == strNumber.size();
    }
    catch (const std::invalid_argument& e) {
        // Not a valid number format
        return false;
    } catch (const std::out_of_range& e) {
        // Number is too large or too small for the target type
        return false;
    }
    return false;
}

vector<string> ServerFunction::split(const string &input, string set ) {
    vector<string> result;

    boost::split(result, input, boost::is_any_of(set),boost::token_compress_on);

    return result;
}
bsoncxx::types::b_date ServerFunction::strToDate(string strDate)
{

    int tzd;
    DateTime dt;

    try{
        DateTimeParser::parse(DateTimeFormat::SORTABLE_FORMAT, strDate, dt, tzd);
    }
    catch(Poco::Exception &ex) {
        DateTimeParser::parse(DateTimeFormat::RFC1123_FORMAT, strDate, dt, tzd);
    }

    Timestamp ts = dt.timestamp();
    auto milli = ts.epochMicroseconds()/1000;
    bsoncxx::types::b_date dtw = bsoncxx::types::b_date{std::chrono::milliseconds{milli}};

    return dtw;
}
string ServerFunction::cleanWords(string newWord)
{
    newWord = Poco::toLower(newWord);
    bool startDigit =  isdigit(newWord[0]);
    if( startDigit){
        auto fnd = newWord.find(" ");
        if( fnd == string::npos)
            return "";
        newWord = newWord.substr(fnd + 1);
    }
    vector<string> publisherKeys = {"$", "µ", "Π","α","“","ℓ","("};
    string pattern = join(publisherKeys,"|");

    auto isthere = pattern.find(newWord[0]);

    if(isthere != string::npos )
        return "";
    isthere = newWord.find("(");

    if( isthere != string::npos)
        newWord = newWord.substr(0,isthere -1);

    trim(newWord);
    return newWord;
}

bsoncxx::v_noabi::oid  ServerFunction::insertMongo(string db, string coll, string data)
{
    int oK = 0;
    // try{

    auto bson = bsoncxx::from_json(data);
    auto database = client->database(db);
    auto collection = database[coll];

    //auto tpe = (*bson)
    auto result = collection.insert_one(std::move(bson));//insert_one(make_document(kvp("data", bson)));
    auto _id = result.value().inserted_id().get_oid().value;

    return _id;

}

string ServerFunction::getDateTime(){
    LocalDateTime now;
    auto formattedTime = DateTimeFormatter::format(now, DateTimeFormat::SORTABLE_FORMAT);
    return formattedTime;
}

vector<string> ServerFunction::queryKeyWordsMongoDB(string db, string coll, vector<string> tempKwords){

    vector<string> keyW;

    vector<string> newKwords;
    for(auto &str: tempKwords){
        string lw = str;
        to_lower(lw);
        newKwords.push_back(lw);
    }


    auto database = client->database(db);
    auto collection = database[coll];


    sort(newKwords.begin(), newKwords.end());
    newKwords.erase(unique(newKwords.begin(), newKwords.end()), newKwords.end());

    auto result =  collection.find_one({}) ;
    if( result ){
        auto id = result->view()["_id"].get_oid().value;
        auto ar = result->view()["keywords"].get_array().value;

        for (auto& e : ar) {
            string strValue = e.get_string().value.data();
            //to_lower(strValue);
            keyW.push_back(strValue);
            //sort(keyW.begin(), keyW.end());

        }
        if( newKwords.empty())
            return keyW;
        if( keyW != newKwords){
            vector<string> tempV;
            copy(begin(keyW), end(keyW), back_inserter(tempV));
            copy(begin(newKwords), end(newKwords), back_inserter(keyW));
            sort(keyW.begin(), keyW.end());
            keyW.erase(unique(keyW.begin(), keyW.end()), keyW.end());

            if( tempV != keyW){

                bsoncxx::builder::stream::document data_builder{};
                //data_builder << "_id" << 5;
                auto array_builder = data_builder << "keywords" << bsoncxx::builder::stream::open_array;
                for (auto &k : keyW) {

                    array_builder << k;
                }
                array_builder << bsoncxx::builder::stream::close_array;

                bsoncxx::document::value doc = data_builder << bsoncxx::builder::stream::finalize;

                auto update_one_result =
                    collection.update_one(make_document(kvp("_id", id)),
                                          make_document(kvp("$set", doc)));//make_document(kvp("foo", values)))));

            }
        }

    }
    else{
        json vect;
        vect["keywords"] = newKwords;
        insertMongo(db,coll,vect.dump());

    }

    return keyW;

}

string ServerFunction::readDimensionsFiles(string folderName) {
    csvColIndex.clear();

    vector<string> pathV;
    boost::split(pathV, folderName,boost::is_any_of("_"), boost::token_compress_on);
    string collection = pathV[0] + "Data";

    string userDir = getHomeDir();

    fs::path dir(userDir);
    fs::path localPath (folderName);

    fs::path path = dir / localPath;
    struct stat sb;

    bool startFlag = true;
    bool ok = false;
    bool okIeee = false;
    vector<string> header;
    int processes = 0;
    int files = 0;
    for (const auto& entry : fs::directory_iterator(path)) {
        fs::path outfilename = entry.path();
        string outfilename_str = outfilename.string();
        std::string capture;
        string extstr = outfilename.extension();
        if( extstr.compare(".zip") == 0 ||  extstr.compare(".xlsx") == 0)
            continue;
        if (stat(outfilename_str.c_str(), &sb) != 0 || (sb.st_mode & S_IFDIR))
            continue;  //this is a directory
        string searchName ;//= outfilename.filename();
        std::stringstream strStream;
        std::ifstream ifs(outfilename_str);
        strStream << ifs.rdbuf();
        string rdFile = strStream.str();
        auto filterN = parseCSVquoteNewLine(rdFile);
        vector<string> data = split(filterN,"\n");
        searchName = data[0];

        // auto tV = split(data[0],"Note:");
        // searchName = split(tV[0],"Criteria:")[1];
        // replace_all(searchName,"in full data.", "");
        // replace_all(searchName,"\"", "");
        // replace_all(searchName,"(", "");
        // replace_all(searchName,")", "");
        // boost::trim_if(searchName, boost::is_any_of("'\" "));
        data.erase(data.begin());
        if( startFlag){
            startFlag = false;
            header = splitIeee(data[0]);
            int index = 0;
            for (string s : header) {
                csvColIndex.insert(make_pair(s,index));
                index++;
            }

        }

        data.erase(data.begin());
        int hdrSize = header.size();
        processes = 0;
        for (string item1: data) {
            string item = item1;
            if ( item.empty())
                continue;
            auto tempData = splitIeee(item);
            if( hdrSize != tempData.size())
                hdrSize += 0;
            int doiPos = csvColIndex["DOI"];
            string doi = tempData[doiPos];
            string doiLower = doi;
            to_lower(doiLower);
            if( doi.empty() )
                continue;
            ok = false;
            processes++;
            bsoncxx::builder::stream::document dimenDoc{};
            dimenDoc << "searchTerms" << searchName;
            dimenDoc << "doi" << doi;

            bool registerOK = true;
            for (const auto& pair : csvColIndex) {
                std::string key = pair.first;
                if( key.compare("DOI") == 0)
                    continue;
                auto index = pair.second;
                string cellValue = tempData[index];
                if( key.compare("Publication date") == 0){
                    if( cellValue.empty()){
                        cout<< "DOI: " << doi << " -- " << key << ": " << cellValue << endl;
                        int tInd = csvColIndex["PubYear"];
                        cellValue = tempData[tInd];
                        if( cellValue.empty()){
                            registerOK = false;
                            continue;
                        }
                    }
                    auto dimDate = strToDate(cellValue);
                    dimenDoc << "published" << dimDate ;
                    continue;
                }

                if(key.find("Publication date") == 0 || key.find("PubYear") == 0)
                    continue;
                dimenDoc << key << cellValue ;
            }
            //if( registerOK){
            vector<string> allKW;
            bsoncxx::document::value fdomDoc = dimenDoc << bsoncxx::builder::stream::finalize;
            auto dStr = bsoncxx::to_json(fdomDoc);
            //insertMongo("phdDB",collection, dStr);
            if ( doi.compare("10.1007/s10207-017-0362-4") == 0)
                boost::trim(doi);

        }
        cout<< endl << "done -- Records: " << processes << "  "
             <<   getDateTime() << "  " << outfilename_str <<  endl;


    }
    return "OK. " + folderName + " ->> Done.";
}

std::string ServerFunction::parseCSVquoteNewLine(string text ){
    int txtSize = 0;
    std::string search_str = "\"";
    size_t start_index = 0;
    size_t first_Quote = text.find(search_str, start_index);
    while(first_Quote != std::string::npos) {
        start_index =  first_Quote + 1;
        size_t second_Quote = text.find(search_str, start_index);
        if(second_Quote != std::string::npos){
            int length = second_Quote - start_index ;
            std::string substring = text.substr(start_index, length);
            size_t newLine = substring.find("\n");
            if(newLine != std::string::npos){
                std::string repStr = substring;
                replace_all(repStr,"\n\r","<br>");
                replace_all(repStr,"\n","<br>");
                int strSize = repStr.size() +1;
                text.erase(start_index, length);
                text.insert(start_index,repStr);
                start_index += strSize;
            }
            else
                start_index += length+1;
        }
        first_Quote = text.find(search_str, start_index);
    }
    return text;
}



std::string ServerFunction::getAggregation(string dbase, string collName, string name) {
    auto database = client->database(dbase);
    auto collection = database.collection(collName);

    bsoncxx::builder::stream::document filter{};
    filter << "name" << name;
    bsoncxx::document::value docT = filter << bsoncxx::builder::stream::finalize;
    auto result = collection.find_one({docT});

    string pStr = bsoncxx::to_json(result->view());

    return pStr;
}
std::vector<std::string> ServerFunction::lookForDois(
    string dbase, string collName,int aggIndex , int items, int page, bool *foundOK ,int  *registers)
{
    vector<string> allDois;

    auto database = client->database(dbase);
    auto collection = database.collection(collName);

    string pip = getAggregation("scopusDB", "aggregationData", "checkCrossRef" );//aVec[aggIndex]; //aggregation index = 0
    //string pipStr = pip;
    replace_all(pip, "-1523", std::to_string(page));
    replace_all(pip, "-1524", std::to_string(items));
    pip = filterQuery(pip);
    auto bson = bsoncxx::from_json(pip);

    auto pipe = bson["pipeline"].get_array().value;

    mongocxx::pipeline p{};
    p.append_stages(pipe);



    *foundOK = false;
    bool oK = true;
    auto cursor = collection.aggregate(p, mongocxx::options::aggregate{});
    int count = 0;
    for (auto&& doc : cursor) {
        count++;
        *foundOK = true;
        std::string doiStr = doc["doi"].get_string().value.data();
        to_lower(doiStr);
        auto res = containsElement(dbase, "crossrefData","doi", doiStr, &oK);
        if(oK)
            continue;
        allDois.push_back(doiStr);
    }

    *registers = count;

    return allDois;

}



string ServerFunction::filterQuery(string pip){

    boost::replace_all(pip, "'", "\"");
    replace_all(pip, "\t", "");
    replace_all(pip, "\n", "");
    //replace_all(pip, " ", "");
    replace_all(pip, "True", "true");
    replace_all(pip, "False", "false");
    replace_all(pip, "None", "null");

    trim(pip);

    return pip;
}

string ServerFunction::getCrossReference(json value)
{

    if( !doiQueue.empty()){
        string doiUrl = doiQueue.pop();
        json vJson;
        vJson["data"] = doiUrl;
        vJson["action"] = "getCrossReference";
        auto d = vJson.dump();

        string z = "Hello";

        auto size = d.size();


        return d;//"Getting: " + doiUrl;
        //utility.parseDOIFields(doiUrl,&tmpId, searchKeyWords);
    }

    int count = startPage + actualRegisters;

    string status = (count < expectedRegisters) ? "ongoing" : "stop";
    if(actualRegisters< requiredRegisters )
        status = "stop";
    json summary;
    summary["status"] = status;
    summary["found"] = foundRegisters;
    summary["actual"] = count ;//> 0 ? (count + 1) : 0 ;
    summary["expected"] = expectedRegisters;
    summary["date"] = getDateTime();


    json vJson;
    vJson["data"] = summary;
    vJson["action"] = "doisDone";

    string d = vJson.dump();
    //jsonReply.push(d);


    return d;//"getCrossReference() done...";

}

bsoncxx::document::view ServerFunction::containsElement(string db, string coll, string id, string value, bool *oK)
{
    bsoncxx::v_noabi::oid _id ;//= bsoncxx::v_noabi::oid::ObjectId(32);

    bsoncxx::builder::stream::document tempDoc{};

    tempDoc << id << value;
    bsoncxx::document::value docT = tempDoc << bsoncxx::builder::stream::finalize;
    *oK = false;
    auto database = client->database(db);
    auto collection = database[coll];

    bsoncxx::document::view vl ;
    if( auto result =  collection.find_one({docT})){
        vl = result->view();
        *oK = true;
        _id = result->view()["_id"].get_oid().value;
    }

    return vl;

}

bool ServerFunction::parseDOIFields(string dbase, string coll, string urlDoiE)
{
    bool oK = false;

    try{

        // json data = queryURL(urlDoiE);
        json data = json::parse(urlDoiE);
        if( data.contains("Error"))
            return false;
        bsoncxx::builder::basic::document docN = bsoncxx::builder::basic::document{};

        string text = data.dump();

        json message = data["message"];

        json author = message["author"];

        string doiId = "";
        if( message.contains("DOI"))
            doiId = message["DOI"].get<string>();

        containsElement(dbase, "crossrefData","doi", doiId, &oK);
        if( oK )
            return  oK;
        string published = "";
        if( message.contains("published")){
            auto pub = message["published"]["date-parts"];
            //auto tp = pub.is_array();
            auto dateArray = pub[0];

            int d = distance(dateArray.begin(), dateArray.end());
            if(d < 3){
                if( message.contains("created")){
                    if(message["created"].contains("date-time")){
                        auto datetime = message["created"]["date-time"].get<string>();
                        published = split(datetime,"T")[0];
                    }
                    else{
                        pub = message["created"]["date-parts"];
                        dateArray = pub[0];
                    }
                }
                else
                    doiId+="";
                //pub = message["created"]["date-parts"];
                //dateArray = pub[0];
            }
            if( published.empty()){
                vector<string> date;
                for(auto& pp:dateArray){
                    if( pp.is_string())
                        date.push_back(pp.get<string>());
                    else{
                        if( pp.is_number()){
                            int pt = pp;
                            std::stringstream outStr;
                            outStr << std::setw(2) << std::setfill('0') << pt;
                            std::string s = outStr.str();
                            date.push_back(s);
                        }
                    }
                }
                if( date.size() < 3)
                    date.push_back("01");
                published = boost::algorithm::join(date, "-");
            }
            docN.append(kvp("published", published));
        }

        if( published.empty() && message.contains("created")){

            if(message["created"].contains("date-time")){
                auto datetime = message["created"]["date-time"].get<string>();
                published = split(datetime,"T")[0];
            }

            docN.append(kvp("published", published));
        }

        string docType = "";
        if( message.contains("type")){
            auto dctype = message["type"].get<string>();
            docN.append(kvp("type", dctype));

        }
        string docTitle = "";
        if( message.contains("title")){
            auto dctitle = message["title"][0].get<string>();
            docN.append(kvp("title", dctitle));

        }
        string docAbstract = "";
        if( message.contains("abstract")){
            auto dcAbs = message["abstract"].get<string>();
            docN.append(kvp("abstract", dcAbs));

        }

        auto autArray = bsoncxx::builder::basic::array{};

        vector<json> authr;

        for( auto &aut : author){
            map<string, string> sM;
            string name = "";
            string seq = "";
            if( aut.contains("family"))
                name = aut["family"].get<string>();
            if( aut.contains("given")){
                string tmG =  aut["given"].get<string>();
                name += ", " + tmG;
            }
            if( aut.contains("sequence") )
                seq = aut["sequence"].get<string>();

            if( name.empty())
                continue;
            //sM.insert(make_pair(name, seq));
            if( !name.empty() ){
                auto tdc = make_document(kvp(name,seq));
                autArray.append(tdc);
            }

        }

        string publisher = "";
        if (message.contains("publisher")){
            publisher = message["publisher"].get<string>();
        }

        if( message.contains(("reference"))){
            auto refs = message["reference"];
            int noDoi = 0;
            for(auto &&e :refs){
                auto keys = Keys(e);
                if( !e.contains("DOI")){
                    e["DOI"] = "NoDoi";
                    noDoi++;
                }
            }

            string tmp = refs.dump();
            docN.append(kvp("reference", bsoncxx::from_json(tmp).view()));
            docN.append(kvp("ref_no_doi", noDoi));
            docN.append(kvp("references-count", message["references-count"].get<int>()));

        }
        else
            doiId+="";

        if( !autArray.view().empty())
            docN.append(kvp("authors", autArray));


        if( !publisher.empty())
            docN.append(kvp("publisher", publisher));

        docN.append(kvp("doi", doiId));
        docN.append(kvp("collection", coll));

        auto dStr = bsoncxx::to_json(docN);
        insertMongo(dbase,"crossrefData", dStr);
        oK = true;
        // if( result > 0 ){
        //     cout << '\n' << dStr << '\n';
        //     result ++;
        // }
    }
    catch (const json::exception& e)
    {
        string ex = e.what();
        cout << ex << '\n';
    }
    catch(Poco::Exception &ex) {
        string lcl =  ex.displayText();
        auto s = lcl.size();
    }
    catch(...){
        string tct = "test";
        tct += "Trouble";

    }
    return oK;
}

vector<string> ServerFunction::Keys(json obj)
{
    vector<string> vector;

    for(auto &item : obj.items()){
        auto key = item.key();
        vector.push_back(key);
    }

    return vector;
}


vector<string> ServerFunction::mongoKeys(bsoncxx::v_noabi::document::view view){
    std::vector<std::string> doc_keys;
    std::transform(begin(view), end(view), std::back_inserter(doc_keys), [](bsoncxx::v_noabi::document::element ele) {
        // note that key() returns a string_view
        return ele.key().data();
    });
    return doc_keys;
}

std::string ServerFunction::getAggregationDoc(string objName)
{
    std::vector<std::string> source;
    boost::split(source,objName,boost::is_any_of("-"),boost::token_compress_off);

    auto database = client->database("scopusDB");
    auto collection = database.collection("aggregationData");

    bsoncxx::builder::basic::document filter_builder;
    filter_builder.append(bsoncxx::builder::basic::kvp("name", source[0]));
    filter_builder.append(bsoncxx::builder::basic::kvp("collection", source[1]));
    auto query_filter = filter_builder.view();

    auto result  = collection.find_one(query_filter);

    if( !result.has_value()){
        return "Error. Found Nothing!!!";
    }
    auto data = result->view();

    string json = bsoncxx::to_json(data);

    return json;

}


void ServerFunction::reviewAllDataSourceWithDoi(string dimStr,string pattern, string sourceId, std::vector<bsoncxx::document::value> *dataObject, vector<string> *doiDone, int *records){

    auto jsonMatch = bsoncxx::from_json(dimStr);

    auto mDb = jsonMatch["database"].get_string().value.data();
    string collection = jsonMatch["collection"].get_string().value.data();

    auto matchDB = client->database(mDb);
    auto collectionM = matchDB.collection(collection);

    auto dateT = getDateTime();

    cout <<endl <<  dateT << " -- " <<sourceId << endl;


    //map<string, int> matchW;

    string pipeTemp = dimStr;

    auto bsonPipe = bsoncxx::from_json(pipeTemp);
    auto pipe = bsonPipe["pipeline"].get_array().value;
    mongocxx::pipeline pM{};
    pM.append_stages(pipe);

    string country, university, author;
    // string university;

    auto cursorM = collectionM.aggregate(pM, mongocxx::options::aggregate{});
    for(auto &&item: cursorM){


        string searchStr = item["search"].get_string().value.data();
        auto doi = item["_id"].get_string().value.data();

        auto doiFnd = find(doiDone->begin(), doiDone->end(), doi);
        if( doiFnd != doiDone->end()){
            continue;
        }
        doiDone->push_back(doi);
        auto year = item["year"].get_int32().value;

        auto matchW = matchKeywords(searchStr, pattern);
        if( matchW.empty())
            continue;

        for (auto kwsM: matchW) {
            auto keyW = kwsM.first;
            auto cntW = kwsM.second;
            string kName = keyW;
            string doiK = doi;
            bsoncxx::builder::stream::document tempDoc{};
            dataObject->push_back(tempDoc << "count" << cntW << "year" << year << "collection" << collection
                        << "doi" << doiK << "name" << kName << bsoncxx::builder::stream::finalize);
        }


        (*records)++;

    }
    //return dataObject;


}

json ServerFunction::reviewAllDataSource(string dimStr,string pattern, string sourceId, json dataObject, vector<string> *doiDone, int *records){

    auto jsonMatch = bsoncxx::from_json(dimStr);

    auto mDb = jsonMatch["database"].get_string().value.data();
    string collection = jsonMatch["collection"].get_string().value.data();

    auto matchDB = client->database(mDb);
    auto collectionM = matchDB.collection(collection);

    auto dateT = getDateTime();

    cout <<endl <<  dateT << " -- " <<sourceId << endl;


    map<string, int> matchW;

    string pipeTemp = dimStr;

    auto bsonPipe = bsoncxx::from_json(pipeTemp);
    auto pipe = bsonPipe["pipeline"].get_array().value;
    mongocxx::pipeline pM{};
    pM.append_stages(pipe);

    string country, university, author;
    // string university;

    auto cursorM = collectionM.aggregate(pM, mongocxx::options::aggregate{});
    for(auto &&item: cursorM){


        string searchStr = item["search"].get_string().value.data();
        auto doi = item["_id"].get_string().value.data();

        auto doiFnd = find(doiDone->begin(), doiDone->end(), doi);
        if( doiFnd != doiDone->end()){
            continue;
        }
        doiDone->push_back(doi);
        auto year = item["year"].get_int32().value;
        // if(year != 2025)
        //     continue;
        auto matchW = matchKeywords(searchStr, pattern);
        auto size = matchW.size();
        if( matchW.empty())
            continue;
        json yr = year;
        //json pointer = getJsonElement(dataObject, "year", yr);
        int pos = getJsonElement(dataObject, "year", yr);

        if( pos >= 0){
            auto yrr = dataObject[pos]["year"].get<int>();
            auto dataPtr = dataObject[pos]["data"].get<std::map<std::string,int>>();


            for(auto &item: matchW){
                auto kyM = item.first;
                auto vlM = item.second;
                auto isT = dataPtr.find(kyM);
                if( isT != dataPtr.end()){
                    isT->second+=vlM;
                    continue;
                }
                dataPtr.insert(pair<string, int>(kyM, vlM));

            }
            dataObject[pos]["count"] = dataPtr.size();
            dataObject[pos]["data"] = dataPtr;
            //std::cout<<"NEXT YEAR: " << yrr << endl << std::setw(4) << dataObject << endl;
            auto sss = dataObject.size();
        }
        else{

            json preData;
            preData["year"] =  year;
            preData["data"] = matchW;
            preData["count"] = matchW.size();

            dataObject.push_back(preData);
            std::cout << "YEAR: " << year << endl;
            auto ss = dataObject.size();
        }
        (*records)++;

    }
    return dataObject;


}


map<string, int> ServerFunction::matchKeywords(string inputString, string pattern){
    string error = "Error. ";
    map<string, int> mapObj;
    try{
        // string pattern = "\\b(" + join(keyW,"|") + ")\\b";
        // if( mode.compare("wholeOnly") != 0 )
        //     pattern = join(keyW,"|");
        boost::regex exp( pattern,boost::regex::icase ) ;
        boost::match_results<std::string::const_iterator> what;
        string::const_iterator start = inputString.begin() ;
        //map<string, int> matchWords;
        while ( boost::regex_search(start, inputString.cend(), what, exp) )
        {
            string f = what[0];
            string found(Poco::toLower(f));
            if( found.compare("and") == 0){
                start = what[0].second;
                continue;
            }
            auto pos = mapObj.find(found);
            if( pos == mapObj.end())
                mapObj.insert(pair<string, int>(found, 1));
            else
                pos->second++;

            start = what[0].second ;
        }

    }
    catch(boost::exception const&  ex){
        error = "Error Exception";
    }
    catch(std::exception const&  ex)
    {
        error = "Can't init settings. %s", ex.what();
    }

    return mapObj;

}

int ServerFunction::getJsonElement(json obj, string key, json value)
{
    json found = NULL;

    auto type = obj.type();
    int index = -1;
    auto jObj = find_if(obj.begin(), obj.end(), [&](json j){


        auto kis = j.contains(key);

        if( kis){
            auto vl = j[key];
            if(vl != value)
                kis = false;
        }
        index++;
        return kis;//j.contains(key);

    });

    if( jObj == obj.end()){
        index = -1;

    }
    return index;
}

vector<string> ServerFunction::getLookWords(string what)
{
    std::vector<std::string> source;


    auto database = client->database("phdDB");
    auto collection = database.collection("stringKeyWords");

    // bsoncxx::builder::basic::document filter_builder;
    // filter_builder.append(bsoncxx::builder::basic::kvp("words", source[0]));
    // filter_builder.append(bsoncxx::builder::basic::kvp("collection", source[1]));
    // auto query_filter = filter_builder.view();

    auto result  = collection.find_one({});


    if( !result.has_value()){
        return {};
    }
    auto data = result->view();

    auto words = data[what].get_string().value.data();
    string json = bsoncxx::to_json(data);

    vector<string> keyWords;
    boost::split(keyWords, words, boost::is_any_of("\n\r\t"),boost::token_compress_on);
    for (std::string& s : keyWords) {
        if ( what.compare("words") == 0 )
            to_lower(s);
        boost::algorithm::trim(s);
    }
    // Remove empty strings
    keyWords.erase(std::remove_if(keyWords.begin(), keyWords.end(),
                                        [](const std::string& s) { return s.empty(); }),
                         keyWords.end());

    sort(keyWords.begin(), keyWords.end());
    keyWords.erase(unique(keyWords.begin(), keyWords.end()), keyWords.end());


    return keyWords;

}
vector<string> ServerFunction::removeVectorEmpties(vector<string> vect){
    vect.erase(std::remove_if(vect.begin(), vect.end(),
                              [](const std::string& s) { return s.empty(); }),
               vect.end());

    return vect;

}

string ServerFunction::matchAuthors(string match, string with){
    string result;
    vector<string> source;
    vector<string> target;

    boost::split(source, match,boost::is_any_of(",. "),boost::token_compress_on);
    source = removeVectorEmpties(source);

    boost::split(target, with,boost::is_any_of(",. "),boost::token_compress_on);
    target = removeVectorEmpties(target);

    int size = source.size();
    if( size != target.size())
        return match;

    for(auto index=0; index< size; index++){
        if( !starts_with(target[index], source[index]))
            return match;
    }
    return with;
}

string ServerFunction::vectorContains(vector<string> source, vector<string> query){
    string result = "NotFound";

    for(string qry: query){
        bool fnd = containsSubstringInVector(source, qry, &result);
        if (fnd )
            break;
    }

    return result;
}
bool ServerFunction::containsSubstringInVector(const std::vector<std::string>& vec, const std::string& substring, string *result) {

    auto it = std::find_if(vec.begin(), vec.end(), [&](const std::string& s) {
        return s.find(substring) != std::string::npos;
    });
    *result = it != vec.end() ? it->data(): "";
    return it != vec.end();
}

string ServerFunction::checkCntrName(string cntryStr, vector<string> cntrys){


    string cntry = split_first_match(cntryStr, cntrys, LAST_MATCH);


    if( starts_with(cntry,"United States")){
        cntry = "United States";
        return cntry;
    }

    auto fnd = countryName.find(cntry);
    cntry = fnd != countryName.end() ? fnd->second : cntry;

    auto isT = find(cntrys.begin(), cntrys.end(), cntry);
    if( isT == cntrys.end()){
        cntry = "UnKnown";
    }

    return cntry;
}

string ServerFunction::initCntryMap(){

    countryName.insert(pair<string, string>("Perú", "Perú"));
    countryName.insert(pair<string, string>("Peru", "Perú"));

    string cntry = "United States";


    countryName.insert(pair<string, string>("Meta Reality Labs", cntry));
    countryName.insert(pair<string, string>("Carnegie Mellon", cntry));

    countryName.insert(pair<string, string>(cntry, cntry));
    countryName.insert(pair<string, string>("USA", cntry));
    countryName.insert(pair<string, string>("U.S.A", cntry));
    countryName.insert(pair<string, string>("U.S.A.", cntry));
    countryName.insert(pair<string, string>("U.S.", cntry));
    countryName.insert(pair<string, string>("California", cntry));
    countryName.insert(pair<string, string>("Michigan", cntry));
    countryName.insert(pair<string, string>("Utah", cntry));
    countryName.insert(pair<string, string>("Irvine", cntry));
    countryName.insert(pair<string, string>("Maryland", cntry));
    countryName.insert(pair<string, string>("Brown", cntry));
    countryName.insert(pair<string, string>("Los Angeles", cntry));
    countryName.insert(pair<string, string>("Louisiana", cntry));



    cntry = "England";

    countryName.insert(pair<string, string>("Lancaster", cntry));
    countryName.insert(pair<string, string>("England", cntry));
    countryName.insert(pair<string, string>("London", cntry));
    countryName.insert(pair<string, string>("UK", cntry));
    countryName.insert(pair<string, string>("U.K", cntry));
    countryName.insert(pair<string, string>("U.K.", cntry));
    countryName.insert(pair<string, string>("United Kingdom", cntry));
    countryName.insert(pair<string, string>("Caceres", cntry));



    cntry = "México";
    countryName.insert(pair<string, string>("México", cntry));
    countryName.insert(pair<string, string>("Mexico", cntry));
    countryName.insert(pair<string, string>("Guadalajara", cntry));

    cntry = "China";
    countryName.insert(pair<string, string>("China", cntry));
    countryName.insert(pair<string, string>("Hong Kong", cntry));
    countryName.insert(pair<string, string>("Beijing", cntry));
    countryName.insert(pair<string, string>("china", cntry));
    countryName.insert(pair<string, string>("Sichuan", cntry));
    countryName.insert(pair<string, string>("Tsinghua", cntry));
    countryName.insert(pair<string, string>("Beihang", cntry));


    countryName.insert(pair<string, string>("Malaysia", "Malaysia"));
    countryName.insert(pair<string, string>("Malayisa", "Malaysia"));

    countryName.insert(pair<string, string>("Germany", "Germany"));
    countryName.insert(pair<string, string>("Berlin", "Germany"));
    countryName.insert(pair<string, string>("Darmstadt", "Germany"));
    countryName.insert(pair<string, string>("Deutschland", "Germany"));
    countryName.insert(pair<string, string>("Fraunhofer", "Germany"));
    countryName.insert(pair<string, string>("Darmstadt", "Germany"));





    countryName.insert(pair<string, string>("India", "India"));
   countryName.insert(pair<string, string>("india", "India"));
    countryName.insert(pair<string, string>("INDIA", "India"));
    countryName.insert(pair<string, string>("Pune", "India"));
    countryName.insert(pair<string, string>("Burdwan", "India"));
    countryName.insert(pair<string, string>("Amrita", "India"));
    countryName.insert(pair<string, string>("Vellore", "India"));





    countryName.insert(pair<string, string>("South Korea", "South Korea"));
    countryName.insert(pair<string, string>("Korea", "South Korea"));
    countryName.insert(pair<string, string>("Morroco", "nMorocco"));
    countryName.insert(pair<string, string>("Brasil", "Brazil"));
    countryName.insert(pair<string, string>("FRANCE", "France"));

    countryName.insert(pair<string, string>("Netherlands", "Netherlands"));
    countryName.insert(pair<string, string>("The Netherlands", "Netherlands"));
    countryName.insert(pair<string, string>("Torino", "Italy"));

    countryName.insert(pair<string, string>("Turkey", "Turkey"));
    countryName.insert(pair<string, string>("Kayseri", "Turkey"));


    countryName.insert(pair<string, string>("Russian Federation", "Russia"));

    countryName.insert(pair<string, string>("UAE", "United Arab Emirates"));

    countryName.insert(pair<string, string>("Algérie", "Algeria"));

    countryName.insert(pair<string, string>("Victoria", "Australia"));


    return cntry;
}

string ServerFunction::toRegexPattern(vector<string> delim) {
    vector<string> tempLim;

    for (string stL:delim) {
        tempLim.push_back(regexEscape(stL));
    }
    string lookFor = join(tempLim, "|");
    return lookFor;
}


vector<string> ServerFunction::split(string text, vector<string> delim) {

    vector<string> resV;
    string lookFor = toRegexPattern(delim);
    boost::regex pattern(lookFor);
    boost::smatch results;
    boost::regex separator(lookFor);
    boost::algorithm::split_regex(resV, text, separator);

    return resV;

}
string ServerFunction::regexEscape( string word){
    const boost::regex esc("[.^$|()\\[\\]{}*+?\\\\]");
    const std::string rep("\\\\&");
    return regex_replace(word, esc, rep,
                         boost::match_default | boost::format_sed);
}

// Function to split a string into two parts based on the first regex match
string ServerFunction::split_first_match(string& inputString,  vector<string> delimeters, bool lstMatch) {
    boost::smatch match_results;
    std::vector<std::string> parts;
    string pattStr = toRegexPattern(delimeters);
    //string pfixPatt = toRegexPattern(preFixPattern);


    if ( !lstMatch)
        pattStr = "((-\\*-)?[Ùłá-úa-zA-Z ]+)?(" + pattStr +")([Ùłá-úa-zA-Z ]+)?(-\\*-)?" ;
    //pattStr = "(-\\*-)?[a-zA-Z ]*(" + pattStr +")([a-zA-Z ]+)?(-\\*-)?" ;

    string firstMatch = "";
    try{
        // string pattern = "\\b(" + join(keyW,"|") + ")\\b";
        // if( mode.compare("wholeOnly") != 0 )
        //     pattern = join(keyW,"|");
        //boost::regex exp( pattStr,boost::regex::icase ) ;
        boost::regex exp( pattStr ) ;
        boost::match_results<std::string::const_iterator> what;
        string::const_iterator start = inputString.begin() ;
        //map<string, int> matchWords;
        while ( boost::regex_search(start, inputString.cend(), what, exp) )
        {
            auto wf = what[0].first;
            firstMatch = what[0];
            start = what[0].second;
            trim_if( firstMatch,boost::is_any_of("-* "));
            if (!lstMatch)
                break;
         }
        // else {
        //     string notFnd = "Not found";
        //     auto s = inputString.size();
        // }

    }
    catch(boost::exception const&  ex){
        cout<< "Error. Exception split_first_match" << endl;
    }
    catch(std::exception const&  ex)
    {
        cout <<   "Error. Finding words. Exception split_first_match. " <<   ex.what() << endl;
    }
    if ( firstMatch.empty()) {
        replace_all(inputString, "-*-",",");
        vector<string> vs;

        boost::split(vs, inputString, boost::is_any_of(";,"));
        firstMatch = vs[0];
        trim(firstMatch);
    }


    return firstMatch;

}

vector<string> ServerFunction::splitParenthesis(string text) {

    vector<string> results;

    //string text = "Liu-*- Chou‐Yuan (Department of Computer Science and Information Engineering-*- National United University-*- Miaoli-*- 360-*- Taiwan); Chang-*- Chin‐Chen (Department of Computer Science and Information Engineering-*- National United University-*- Miaoli-*- 360-*- Taiwan); Way-*- Der‐Lor (Department of NewMedia Art-*- Taipei National University of Arts-*- Taipei-*- 112-*- Taiwan); Tai-*- Wen‐Kai (Department of Computer Science and Information Engineering-*- National Taiwan University of Science and Technology-*- Taipei-*- 106-*- Taiwan)";

    //string text = "Demus-*- Christoph (Hochschule Mittweida-*- Technikumplatz 17-*- 09648-*- Mittweida-*- Deutschland; Fraunhofer-Institut für Sichere Informationstechnologie-*- Rheinstraße 75-*- 64295-*- Darmstadt-*- Deutschland); Schütz-*- Mina (Hochschule Darmstadt-*- Schöfferstraße 3-*- 64295-*- Darmstadt-*- Deutschland); Probol-*- Nadine (Hochschule Darmstadt-*- Schöfferstraße 3-*- 64295-*- Darmstadt-*- Deutschland); Pitz-*- Jonas (Hochschule Darmstadt-*- Schöfferstraße 3-*- 64295-*- Darmstadt-*- Deutschland); Siegel-*- Melanie (Hochschule Darmstadt-*- Schöfferstraße 3-*- 64295-*- Darmstadt-*- Deutschland); Labudde-*- Dirk (Hochschule Mittweida-*- Technikumplatz 17-*- 09648-*- Mittweida-*- Deutschland; Fraunhofer-Institut für Sichere Informationstechnologie-*- Rheinstraße 75-*- 64295-*- Darmstadt-*- Deutschland)";
    string vowels = "()";
    int OpenP = 0;
    int strBegin = 0;
    int strEnd = 0;
    int startSearch = 0;
    size_t index = text.find_first_of(vowels,startSearch);

    while (index != std::string::npos) {
        // std::cout << "First vowel found at index: " << index << std::endl;
        // std::cout << "Character at that index is: " << text[index] << std::endl;
        if ( text[index] == '('  ) {
            OpenP +=1;
            if ( OpenP > 1)
                OpenP+=0;
        }
        else {
            if ( text[index] == ')'  ) {
                OpenP -=1;
            }
        }
        if ( OpenP == 0) {
            strEnd = index+1;
            std::string sub = text.substr(strBegin, strEnd - strBegin);
            trim_if(sub, boost::is_any_of("; "));
            results.push_back(sub);
            //std::cout << sub << std::endl;

            strBegin = strEnd;
        }
        startSearch = index+1;
        index = text.find_first_of(vowels,startSearch);

    }

    return results;
}


string ServerFunction::getParenthesis(string text, string* author) {

    string found = "";
    //string text = "Liu-*- Chou‐Yuan (Department of Computer Science and Information Engineering-*- National United University-*- Miaoli-*- 360-*- Taiwan); Chang-*- Chin‐Chen (Department of Computer Science and Information Engineering-*- National United University-*- Miaoli-*- 360-*- Taiwan); Way-*- Der‐Lor (Department of NewMedia Art-*- Taipei National University of Arts-*- Taipei-*- 112-*- Taiwan); Tai-*- Wen‐Kai (Department of Computer Science and Information Engineering-*- National Taiwan University of Science and Technology-*- Taipei-*- 106-*- Taiwan)";

    //string text = "Demus-*- Christoph (Hochschule Mittweida-*- Technikumplatz 17-*- 09648-*- Mittweida-*- Deutschland; Fraunhofer-Institut für Sichere Informationstechnologie-*- Rheinstraße 75-*- 64295-*- Darmstadt-*- Deutschland); Schütz-*- Mina (Hochschule Darmstadt-*- Schöfferstraße 3-*- 64295-*- Darmstadt-*- Deutschland); Probol-*- Nadine (Hochschule Darmstadt-*- Schöfferstraße 3-*- 64295-*- Darmstadt-*- Deutschland); Pitz-*- Jonas (Hochschule Darmstadt-*- Schöfferstraße 3-*- 64295-*- Darmstadt-*- Deutschland); Siegel-*- Melanie (Hochschule Darmstadt-*- Schöfferstraße 3-*- 64295-*- Darmstadt-*- Deutschland); Labudde-*- Dirk (Hochschule Mittweida-*- Technikumplatz 17-*- 09648-*- Mittweida-*- Deutschland; Fraunhofer-Institut für Sichere Informationstechnologie-*- Rheinstraße 75-*- 64295-*- Darmstadt-*- Deutschland)";
    string vowels = "()";
    int OpenP = 0;
    int strBegin = 0;
    int strEnd = 0;
    int startSearch = 0;
    size_t index = text.find_first_of(vowels,startSearch);

    while (index != std::string::npos) {
        // std::cout << "First vowel found at index: " << index << std::endl;
        // std::cout << "Character at that index is: " << text[index] << std::endl;
        if ( text[index] == '('  ) {
            if ( OpenP == 0 ) {
                *author = text.substr(strBegin, index - strBegin);
                replace_all(*author,"-*-",",");
                trim(*author);
                strBegin = index + 1;
            }
            OpenP +=1;
            if ( OpenP > 1)
                OpenP+=0;
        }
        else {
            if ( text[index] == ')'  ) {
                OpenP -=1;
            }
        }
        if ( OpenP == 0) {
            strEnd = index;
            found = text.substr(strBegin, strEnd - strBegin);
            trim_if(found, boost::is_any_of("; "));
            break;
            //std::cout << sub << std::endl;

            strBegin = strEnd;
        }
        startSearch = index+1;
        index = text.find_first_of(vowels,startSearch);

    }

    return found;
}

void ServerFunction::stripUnicode(string & str)
{
    str.erase(remove_if(str.begin(),str.end(), [](char c){return !(c>=0 && c <128);}), str.end());
}

json ServerFunction::dataSort( int threshold, map<string, int> allData) {
    json j= {};
    int sorted = allData.size();

    for (auto &&item: allData) {
        string name = item.first;
        int count = item.second;
        json docT;
        docT["name"] = name;
        docT["value"] = count;
        j.push_back(docT);
    }
    // Sort the array based on the "id" key
    std::sort(j.begin(), j.end(), [](const json &a, const json &b) {
        return a["value"] > b["value"]; // Descending order
    });

    json result = {};

    // Use std::copy_if to copy elements greater than the threshold
    std::copy_if(j.begin(), j.end(),
                 std::back_inserter(result),
                 [threshold](json x) { return x["value"] >= threshold; });

    int items = result.size();

    json done;
    done["total"] = sorted;
    done["found"] = items;
    done["data"] = result;
    return done;

}
