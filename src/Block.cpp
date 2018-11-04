/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Block.cpp
 * Author: weizy
 * 
 * Created on November 1, 2018, 7:06 PM
 */

#include "head/Block.h"
#include <stdlib.h>
#include <vector>
#include <iostream>

using namespace std;

Block::Block(block_id blockId, int attr, int maxTupLength) {


    fileName = nullptr;

    this->blockId = blockId;
    blockSize = 4;
    tups = 0;							//初始化块中包含的元组数为0
    this->attr = attr;

    position_start start = sizeof(blockId) + sizeof(blockSize) + sizeof(freeSpace) + sizeof(tups);
    freeSpace = new Position(start, 1024*blockSize - start); 	//后面再计算具体的数据

    cout << "start position : " << start << endl;

    change = false;
    block = (char*)malloc(1024*4);		//默认大小 4K

    this->maxTupLength = maxTupLength;
    maxTups = (1024*blockSize - sizeof(blockId) - sizeof(blockSize)
    		- sizeof(Position) - sizeof(tups))/(sizeof(Position) + maxTupLength);//可容纳的最多的元组数

    initBlock();
}
Block::Block(block_id blockId, int attr, int maxTupLength, block_size blockSize){
	fileName = nullptr;
	this->blockId = blockId;
	this->blockSize = blockSize;
	tups = 0;
	this->attr = attr;

	position_start start = sizeof(blockId) + sizeof(blockSize) + sizeof(freeSpace) + sizeof(tups);
	freeSpace = new Position(start, 1024*blockSize - start);

	cout << "start position : " << start << endl;

	change = false;
	block = (char*)malloc(1024*blockSize);
	this->maxTupLength = maxTupLength;
	maxTups = (1024*blockSize - sizeof(blockId) - sizeof(blockSize)
    		- sizeof(Position) - sizeof(tups))/(sizeof(Position) + maxTupLength);

	initBlock();
}

Block::~Block() {
	free(block);
	while (!pos.empty()){
		Position * p = pos.back();
		delete p;
		pos.pop_back();
	}
	cout << "~Block()" << endl;

}

void Block::initBlock(){
	char * b = block;
	int index = 0;

	//blockId
	const char * bId = (char*)&blockId;
	for (unsigned int i = 0; i < sizeof(blockId); i++,bId++,index++){
		b[index] = *bId;
	}

	//blockSize
	const char * bs = (char*)&blockSize;
	for (unsigned int i = 0; i < sizeof(blockSize); i++,bs++,index++){
		b[index] = *bs;
	}

	//freeSpace
	position_start positionStart = freeSpace->getStart();
	const char * fs = (char*)&positionStart;
	for (unsigned int i = 0; i < sizeof(freeSpace->getStart()); i++,fs++,index++){
		b[index] = *fs;
	}
	offset_length offset = freeSpace->getLength();
	fs = (char*)&offset;
	for (unsigned int i = 0; i < sizeof(freeSpace->getLength()); i++,fs++,index++){
		b[index] = *fs;
	}

	//tups
	const char * tup = (char*)&tups;
	for (unsigned int i = 0; i < sizeof(tups); i++,tup++,index++){
		b[index] = *tup;
	}

	cout << "index = " << index <<endl;
}

int Block::getFreespace(){
	return freeSpace->getLength();
}
void Block::updateTups(int value){
	char * b = block;
	this->tups = value;
	const char * tup = (char*)&tups;
	int index = TUPLES_START;
	for (unsigned int i = 0; i < sizeof(tups); i++,tup++,index++){
		b[index] = *tup;
	}
}
void Block::updateFreeSpace(const position_start start, const offset_length length){
	this->freeSpace->setStart(start);
	this->freeSpace->setLength(length);
	char * b = block;

	int index = FREE_SPACE_START;
	char * fs = (char*)&start;
	for (unsigned int i = 0; i < sizeof(start); i++,fs++,index++){
		b[index] = *fs;
	}
	char * fl = (char*)&length;
	for(unsigned int i = 0; i < sizeof(length); i++,fl++,index++){
		b[index] = *fl;
	}
}
void Block::addPosition(Position * position){
	pos.push_back(position);
	char * b = block;
	int index = freeSpace->getStart();

	position_start positionStart = position->getStart();
	offset_length positionOffset = position->getLength();
	char * ps = (char*)&positionStart;
	for (unsigned int i = 0; i < sizeof(positionStart); i++){
		b[index] = *ps;
		ps++;
		index++;
	}
	char *pl = (char*)&positionOffset;
	for (unsigned int i = 0; i < sizeof(positionOffset); i++){
		b[index] = *pl;
		pl++;
		index++;
	}
}

void Block::addTuple(const char *p, int tupSize){
	Position * ps = new Position(freeSpace->getStart(), tupSize);

	int blockIndex = freeSpace->getStart() + freeSpace->getLength() - tupSize;
	char * b = block;
	for (int i = 0; i < tupSize; i++, blockIndex++){
		b[blockIndex] = p[i];
	}

	updateTups(tups+1);
	addPosition(ps);
	updateFreeSpace(freeSpace->getStart() + POSITION_SIZE, freeSpace->getLength() - POSITION_SIZE - tupSize);
}


void Block::printBlock(){

}





