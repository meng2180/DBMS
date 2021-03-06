/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DBMS.cpp
 * Author: weizy
 * 
 * Created on November 1, 2018, 7:35 PM
 */


#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "head/DBMS.h"
#include "head/Global.h"
#include "head/Tuple.h"
#include "exception/head/FileNotFoundException.h"

using namespace std;

DBMS::DBMS(int memorySize) {
	int lru_size = memorySize/4;
	cout << "LruSize = " << lru_size << endl;
	lru = new LruCache<string, Block *>(lru_size);
	loadDatabases();
}


DBMS::~DBMS() {
	delete lru;

}
void DBMS::loadDatabases() {
	FILE * dbs;
	if ((dbs = fopen("data/databases.db", "r")) == NULL) {
		throw FileNotFoundException("FileNotFoundException: file data/databases.db not found");
	}
	char * p = (char*)malloc(MAX_DATABASE_NAME);
	while (fscanf(dbs, "%s", p) != EOF) {
		databases.push_back(p);
		p = (char*)malloc(MAX_DATABASE_NAME);
	}
	free(p);	//多申请了一个空间  删除以免内存泄露
}

void DBMS::initialDictionary(const char * dicName) {
	cout << "DBMS::inititalDictionary()" << endl;
	Dictionary::getDictionary()->setCurDatabaseName(dicName);
    FILE * dicFile;
    //data/school/school.desc
    string dicDescName(dicName);	//school
    dicDescName = "data/" + dicDescName + "/" + dicDescName + ".desc";
    cout << "dicDescName : " << dicDescName << endl;

    if ((dicFile = fopen(dicDescName.c_str(), "r")) == NULL) {
        throw FileNotFoundException("FileNotFoundException: can't not open " + dicDescName);
    }
    int totalRelationship;
    fscanf(dicFile, "%d", &totalRelationship);
    
    for (int i = 0; i < totalRelationship; i++) {
    	unsigned int totalBlock;
        int totalProperty;
        char * relName = (char*)malloc(Global::MAX_RELATION_FILE_NAME);
        char * relFileName = (char*)malloc(Global::MAX_RELATION_FILE_NAME);

        fscanf(dicFile, "%u", &totalBlock);
        fscanf(dicFile, "%d", &totalProperty);
        fscanf(dicFile, "%s", relName);
        fscanf(dicFile, "%s", relFileName);

        Relation * rel = new Relation(totalBlock, totalProperty, relName, relFileName);
        for (int j = 0; j < totalProperty; j++) {
        	char  attrName[MAX_ATTRIBUTE_NAME];
        	fscanf(dicFile, "%s", attrName);
        	rel->addAttribute(attrName);
        }
        for (int j = 0; j < totalProperty; j++) {
            char * type = (char*) malloc(Global::TYPE_LENGTH);
            int value;
            fscanf(dicFile, "%s", type);
            fscanf(dicFile, "%d", &value);
            int typeToInt;
            if (strcmp(type, "int")==0){
                typeToInt = Global::INTEGER;
            } else if (strcmp(type, "char") == 0){
                typeToInt = Global::CHAR;
            } else if (strcmp(type, "varchar") == 0){
                typeToInt = Global::VARCHAR;
            } else if (strcmp(type, "float") == 0){
                typeToInt = Global::FLOAT;
            } else if (strcmp(type, "double") == 0){
            	typeToInt = Global::DOUBLE;
            }
            
            rel->addType(typeToInt, value, j);
        }
        Dictionary::getDictionary()->addRelation(rel);
    }

    fclose(dicFile);
    
    printf("initial success!!!\n");
}
void DBMS::test() {
	FILE * testFile;
	if ((testFile = fopen("testFile.ts", "r")) == NULL) {
		throw FileNotFoundException("testFile.ts");
	}
	unsigned int totalBlock = Dictionary::getDictionary()->getRelation(0)->getTotalBlock();
	Block * block = new Block(totalBlock, Dictionary::getDictionary()->getRelation(0));
	string blockName(Dictionary::getDictionary()->getRelation(0)->getRelationName());
	string id = to_string(totalBlock);
	blockName.append(id);

	for (int i = 0; i < 500; i++) {
		char name1[20];
		char name2[20];
		int num;
		fscanf(testFile, "%s%s%d", name1, name2, &num);
		Tuple * tup = new Tuple(Dictionary::getDictionary()->getRelation(0));
		tup->addChar(name1, Dictionary::getDictionary()->getRelation(0)->getTypeValue(0));
		tup->addVarchar(name2, strlen(name2));
		tup->addInteger(num);
		tup->processData();
//		cout << "freespace : " << block->getFreespace() << endl;
		if (block->getFreespace() > 200) {
			block->addTuple(tup->getResult(), tup->getTupLength());
		} else {
			Block * b = lru->put(blockName, block);
			if (b) {
//				b->printBlock();
				delete b;
			}
			totalBlock += 1;
			Relation * r = Dictionary::getDictionary()->getRelation(0);
			r->setTotalBlock(totalBlock);
			totalBlock = Dictionary::getDictionary()->getRelation(0)->getTotalBlock();
			block = new Block(totalBlock, Dictionary::getDictionary()->getRelation(0));
			id = to_string(totalBlock);
			blockName = Dictionary::getDictionary()->getRelation(0)->getRelationName();
			blockName.append(id);

			block->addTuple(tup->getResult(), tup->getTupLength());
		}
		delete tup;
	}
	Block * b = lru->put(blockName, block);	//------------差点出错------------------最后一块也要放入lru中
	if (b) {
		delete b;
	}
	fclose(testFile);
	Dictionary::getDictionary()->writeBack();
	cout << "test() finish" << endl;

}
void DBMS::test2() {
	FILE * testFile;
	if ((testFile = fopen("testFile2.ts", "r")) == NULL) {
		throw FileNotFoundException("testFile2.ts");
	}
	unsigned int totalBlock = Dictionary::getDictionary()->getRelation(1)->getTotalBlock();
	Block * block = new Block(totalBlock, Dictionary::getDictionary()->getRelation(1), 4);
	string blockName(Dictionary::getDictionary()->getRelation(1)->getRelationName());
	string id = to_string(totalBlock);
	blockName.append(id);
	for (int i = 0; i < 500; i++) {
		char str1[20];
		char str2[20];
		fscanf(testFile, "%s%s", str1,str2);
		Tuple * tup = new Tuple(Dictionary::getDictionary()->getRelation(1));
		tup->addChar(str1, Dictionary::getDictionary()->getRelation(1)->getTypeValue(0));
		tup->addVarchar(str2, strlen(str2));
		tup->processData();

		if (block->getFreespace() > 200) {
			block->addTuple(tup->getResult(), tup->getTupLength());
		} else {
			Block * b = lru->put(blockName, block);
			if (b) {
//				b->printBlock();
				delete b;
			}
			totalBlock += 1;
			Dictionary::getDictionary()->getRelation(1)->setTotalBlock(totalBlock);
			block = new Block(totalBlock, Dictionary::getDictionary()->getRelation(1), 4);
			id = to_string(totalBlock);
			blockName = Dictionary::getDictionary()->getRelation(1)->getRelationName();
			blockName.append(id);
			block->addTuple(tup->getResult(), tup->getTupLength());
		}
		delete tup;
	}
	Block * b = lru->put(blockName, block);		//这一部很关键 不要忘了 不然会少一块没有被写回文件
	if (b) {
		delete b;
	}
	fclose(testFile);
	Dictionary::getDictionary()->writeBack();
	cout << "test2() finish" << endl;

}














